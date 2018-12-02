#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <netinet/in.h>

#include "ipc.h"
#include "ipcServer.h"
#include "libcjson/cJSON.h"

#include <systemHelper.h>

int ipcSocketInit(void)
{
	int sockfd, addrlen, confd, len, i;
	struct sockaddr_in serveraddr;

	/* 1. socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if(sockfd < 0) 
	{		 
		debug("ipc", "call socket() return : %s(%d)", strerror(errno), errno);
		return -1;
	}

	/* 2. binding server addr */
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(IPC_SERVER_PORT);
	
	bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
 
	/* 3. listen */
	if(listen(sockfd, IPC_LISTEN_QUEUE_LEN) < 0) 
	{
		debug("ipc", "call listen() return: %s", strerror(errno));
		return -1;
	}

	return sockfd;
}

int ipcAccept(int sock)
{
    int fd;
    struct sockaddr_in addr;
    socklen_t size;   

    memset(&addr, 0, sizeof(addr));

    size = sizeof(addr);
    fd = accept(sock, (struct sockaddr *)&addr, &size);
    if(fd < 0) 
    {
        debug("ipc", "call accept() return: %s", strerror(errno));
        return -1;
    }

    return fd;
}

int ipcDeviceResponse(int sock, char *json)
{
	uint32_t ackSize = strlen(json);

	int ret = send(sock, json, ackSize, 0);
	
	while((ret == -1) && (errno == EINTR))
	{
		ret = send(sock, json, ackSize, 0);	
	}
	    
    if (ret != ackSize)
    {
        debug("ipc", "send (socket=%d, size=%d) return %d", sock, ackSize, ret);
    }
   
    return ret;	
}

int ipcDeviceSimpleResponse(int sock, int status)
{
	/* get response json code */
	cJSON *root =cJSON_CreateObject();

	if (!root)
	{
		debug("ipc", "Failed to get root object!");
		return -1;
	}
	
	cJSON_AddNumberToObject(root, "packetType", IPC_RESPONSE);
	cJSON_AddNumberToObject(root, "ack", status);

	/* send json */
	char *jsonString = cJSON_Print(root);
	
	int ret = ipcDeviceResponse(sock, jsonString);

	if (jsonString)
	{
		free(jsonString);
	}

	cJSON_Delete(root);

	return ret;
}

int ipcResponse
    (
        int sock, 
        ipcPacket_t *request, 
        uint32_t status, 
        int parameterSize, 
        int parameterNum,         
        void *parameterData
    )
{
    long ret;
    uint32_t ackSize, dataLength;
    ipcPacket_t *ack;
        
    if (request == NULL)
    {
        return 0;
    }

    debug("ipc", "response to client (pid=%d, command=%s) (data Size:%d * %d) on socket(%d)", 
        ntohl(request->header.requsetId) >> 16, &request->payload[ntohl(request->header.commandOffset)], parameterSize, parameterNum, sock);

    dataLength = (parameterNum && parameterData) ? parameterSize * parameterNum : 0;   
    ackSize = sizeof(ipcHeader_t) + ntohl(request->header.commandLength) + dataLength;  


    if (ackSize > IPC_MSG_WARNING_SIZE)
    {       
        warning("ipc", "big size warning: pid=%d, command=%s, socket=%d", ntohl(request->header.requsetId) >> 16,
            &request->payload[ntohl(request->header.commandOffset)], sock);

        warning("ipc", "data size(%d = %d * %d)", parameterSize * parameterNum, parameterSize, parameterNum);
        
    }


    if (ackSize > IPC_MSG_MAX_SIZE)
    {
        debug("ipc", "response messag length(%d > %d) is too large", ackSize, IPC_MSG_MAX_SIZE);
        return 0;
    }
    
    ack = (ipcPacket_t *)malloc(ackSize);

    if (ack == NULL)
    {
        debug("ipc", "malloc(%d)", ackSize);
        return 0;
    }

    memcpy(ack, request, sizeof(ipcHeader_t) + ntohl(request->header.commandLength));
   
    ack->header.requsetId = htonl((ntohl(ack->header.requsetId) & 0xFFFFFFFC) | IPC_RESPONSE);
    ack->header.ackStatus = htonl(status);

    ack->header.checkSum = 0;

    ack->header.dataOffset = htonl(ack->header.commandLength);
    ack->header.dataSize = htonl(dataLength);
    ack->header.itemNum = htonl(parameterNum);
    ack->header.itemSize = htonl(parameterSize);
       
    if (dataLength && parameterData)
    {
        memcpy(&ack->payload[ntohl(ack->header.dataOffset)], parameterData, dataLength);
    }

	ret = send(sock, ack, ackSize, 0);
	
	while((ret == -1) && (errno == EINTR))
	{
		ret = send(sock, ack, ackSize, 0);	
	}
	    
    if (ret != ackSize)
    {
        debug("ipc", "send (socket=%d, size=%d, request=%08X) return %d", sock, ackSize, ntohl(ack->header.requsetId), ret);
    }

    free(ack);
   
    return ret;
}

#include <thread.h>

/* variables */
struct ipcHandleList *s_ipcHandleHead[257] = { NULL };
static int s_ipcSocket = -1;
static int s_ipcDeviceSocket = -1;



struct ipcConnection
{
    int     fd;
    char    *name;
    void    *handle;
    struct ipcConnection *next;
};

typedef struct ipcConnection ipcConnection_t;

static ipcConnection_t *s_ipcConnectionHead = NULL;

static uint32_t ipcHash(const char *str)
{
    uint32_t hash = 0;
    
    while (*str)
    {
        hash = 31 * hash + *str ++;
    }
    
    return hash;
}

struct ipcHandleList *ipcHandleAdd(const char *command, ipcHandle_t function)
{
    uint32_t i;
    struct ipcHandleList *handle;
    
    handle = (struct ipcHandleList *)malloc(sizeof(*handle));
    
    if (handle == NULL)
    {
        debugf("malloc", "malloc(%d)", sizeof(*handle));
        return NULL;
    }
    
    memset(handle, 0, sizeof(*handle));

    handle->command = command;
    handle->function = function;

    i = ipcHash(handle->command) % (sizeof(s_ipcHandleHead)/sizeof(s_ipcHandleHead[0]));

    handle->next = s_ipcHandleHead[i];
    handle->countInHead = s_ipcHandleHead[i] ? (s_ipcHandleHead[i]->countInHead + 1) : 1;    
    s_ipcHandleHead[i] = handle;

    debug("ipc", "Register Handle [%-3d(%-3d)], command [%s]", i, handle->countInHead, handle->command);    
    
    return handle;
}


void ipcHandleRemoveAll(void)
{
    struct ipcHandleList *handle, *next;
    int i;

    for (i = 0; i < (sizeof(s_ipcHandleHead)/sizeof(s_ipcHandleHead[0])); i ++)
    {
        handle = s_ipcHandleHead[i];
        while(handle)
        {
            debug("ipc", "Remove Handle [%s]", handle->command);            
            next = handle->next;            
            free(handle);
            handle = next;
        }
        s_ipcHandleHead[i] = NULL;
    }    
}


ipcHandle_t ipcHandleGet(const char *command)
{
    uint32_t i;
    struct ipcHandleList *handle;
    
    i = ipcHash(command) % (sizeof(s_ipcHandleHead)/sizeof(s_ipcHandleHead[0]));

    handle = s_ipcHandleHead[i];

    while(handle)
    {
        if (!strcmp(handle->command, command))
        {
            debug("ipc", "Found command(%s) handle at head[%d], which has %d items", command, i, s_ipcHandleHead[i]->countInHead);
            return handle->function;
        }
        handle = handle->next;
    }
    
    return NULL;
}


void ipcConnectionClean(int all)
{
    ipcConnection_t *p, *next, *prev = NULL;

    p = s_ipcConnectionHead;

    while(p)
    {
        next = p->next;
        if (all || (p->fd < 0))
        {
            if (!all && (p->fd < 0))
            {
                debug("ipc", "client(%s) may be terminated, remove it", p->name);
            }

            if (p->fd >= 0)
            {
                close(p->fd);
				debug("ipc", "close client %d\n", p->fd);
            }
            threadRemoveListeningFile(p->handle);            
            free(p->name);
            free(p);
            
            if (prev == NULL)
            {
                s_ipcConnectionHead = next;
            }
            else 
            {
                prev->next = next;
            }
        }
        else 
        {
            prev = p;
        }
        p = next;
    }

}

int ipcClientsProcessId(int *pids, int size)
{
    int count = 0;
    ipcConnection_t *p;

    p = s_ipcConnectionHead;

    while(p)
    {
        if (p->name)
        {
            const char *pc = strrchr(p->name, '.');

            if (pc && (count < size))
            {
                pids[count] = strtoul(pc + 1, NULL, 10);

                if (pids[count] > 0)
                {
                    count ++;
                }
            }
        }
        p = p->next;
    }

    return count;
}

#define IPC_MAX_CYCLE_TIME	10
static void ipcPacketProcess(void *data)
{
    ipcConnection_t *ipc = data;
    ipcHeader_t hdr;   
    ipcPacket_t *pkt = NULL;
    ssize_t recvSize;
    ipcHandle_t function;

    if (ipc == NULL)
    {
        return ;
    }

    debug("ipc", "receive from %s on socket(%d)", ipc->name, ipc->fd);    

	int max_cycle_time = IPC_MAX_CYCLE_TIME;
	recvSize = recv(ipc->fd, &hdr, sizeof(ipcHeader_t), MSG_PEEK);
	while((recvSize == -1) && (errno == EINTR) && (max_cycle_time>0))
	{
		recvSize = recv(ipc->fd, &hdr, sizeof(ipcHeader_t), MSG_PEEK);	
		max_cycle_time --;
	}

    if(recvSize <= 0) 
    {
        debug("ipc", "recv(socket=%d) return %d, the client seems crash", ipc->fd, recvSize);
        close(ipc->fd);
        ipc->fd = -1;
        /* clean the unused connections */
        ipcConnectionClean(0);
        return;
    }
    else if (recvSize < sizeof(hdr))
    {
        debug("ipc", "recv() size is too small(%d, recive size:%d, header size:%d)", ipc->fd, recvSize, sizeof(ipcHeader_t));                   
        recvSize = recv(ipc->fd, &hdr, recvSize, MSG_TRUNC);
        return ;
    }

    debug("ipc", "get request, size:%d syncId:%08X, identity:%08X, pid:%d, op:%d", recvSize, ntohl(hdr.syncId), ntohl(hdr.identity), ntohl(hdr.requsetId) >> 16, (ntohl(hdr.requsetId) >> 8) & 0xff);

    recvSize = sizeof(hdr) + ntohl(hdr.dataOffset) + ntohl(hdr.dataSize);
   
    /* check if it is correct one */
    if ((ntohl(hdr.syncId) != IPC_SYNC_ID)
        || ((ntohl(hdr.requsetId) & 0x03) != IPC_REQUEST)
        || ((recvSize < 0) || (recvSize > IPC_MSG_MAX_SIZE))
        )
    {
        debug("ipc", "invalid request");

		debug("ipc", "");

        /* remove the command */
        hdr.commandLength = htonl(0);
        
        ipcResponse(ipc->fd, (ipcPacket_t *)&hdr, IPC_STATUS_INVALID, 0, 0, NULL);        

        //0TODO: clear the recv buffer
        max_cycle_time = IPC_MAX_CYCLE_TIME;
        recvSize = recv(ipc->fd, &hdr, sizeof(ipcHeader_t), MSG_TRUNC);
		while((recvSize == -1) && (errno == EINTR) && (max_cycle_time>0))
		{
			debug("ipc", "while recycle!!");
			recvSize = recv(ipc->fd, &hdr, sizeof(ipcHeader_t), MSG_TRUNC);	
			max_cycle_time --;
		}
		
        return ;
    }    

    pkt = (ipcPacket_t *)malloc(recvSize);
    if (pkt == NULL) 
    {
        debugf("malloc", "malloc(%d)", recvSize); 

        /* remove the command */
        hdr.commandLength = htonl(0);
        
        ipcResponse(ipc->fd, (ipcPacket_t *)&hdr, IPC_STATUS_NOMEM, 0, 0, NULL);

        //0TODO: clear the recv buffer        
        recvSize = recv(ipc->fd, &hdr, sizeof(ipcHeader_t), MSG_TRUNC);
		max_cycle_time = IPC_MAX_CYCLE_TIME;
		while((recvSize == -1) && (errno == EINTR) && (max_cycle_time>0))
		{
			recvSize = recv(ipc->fd, &hdr, sizeof(ipcHeader_t), MSG_TRUNC);
			max_cycle_time --;
		}
		
        return;
    }
    
    if (recv(ipc->fd, pkt, recvSize, 0) != recvSize) 
    {
        debug("ipc", "recv() not return the expected size(%d)", recvSize);
        free(pkt);
        return;
    }    
    
    function = ipcHandleGet((const char *)&pkt->payload[ntohl(pkt->header.commandOffset)]);

    if (function)
    {
        function(ipc->fd, (ntohl(pkt->header.requsetId) >> 8) & 0xff, pkt);
        debug("ipc", "function command(%s) executed", (const char *)&pkt->payload[ntohl(pkt->header.commandOffset)]);
    }
    else 
    {
        debug("ipc", "Unknown command(%s), please check!", (const char *)&pkt->payload[ntohl(pkt->header.commandOffset)]);    
        /* remove the command */
        hdr.commandLength = htonl(0);
        
        ipcResponse(ipc->fd, (ipcPacket_t *)&hdr, IPC_STATUS_UNDEF, 0, 0, NULL);        
    }

    free(pkt);
}


void ipcReceive(void *data)
{
    int fd;
    char name[128];
    ipcConnection_t *client = NULL;    
    
    fd = ipcAccept(s_ipcSocket);

    if (fd < 0)
    {
        debug("ipc", "ipcAccept(%d) failed", s_ipcSocket);
        return ;
    }
    
    client = (ipcConnection_t *)malloc(sizeof(ipcConnection_t));
    if (client == NULL)
    {
        debugf("malloc", "malloc(%d) failed", sizeof(ipcConnection_t));
        close(fd);
        return ;
    }
    
    debug("ipc", "ipcAccept(%d) return name:%s", s_ipcSocket, name);
    
    client->fd = fd;
    client->name = strdup(name);
    client->handle = threadAddListeningFile(client->name, client->fd,  client, ipcPacketProcess);
    client->next= s_ipcConnectionHead;
    s_ipcConnectionHead = client;      
}


void ipcInit(void)
{
    s_ipcSocket = -1;
    s_ipcConnectionHead = NULL;
    memset(s_ipcHandleHead, 0, sizeof(s_ipcHandleHead));
}


void ipcStart(const char * name)
{
    s_ipcSocket = ipcSocketInit();
    assert(s_ipcSocket >= 0, "Failed to init IPC socket on %s", name); 
    debug("ipc", "Open IPC socket(%d) on %s", s_ipcSocket, name);

    threadAddListeningFile("sys.ipc", s_ipcSocket, NULL, ipcReceive);
}


void ipcExit(const char * name)
{
    ipcHandleRemoveAll();
    ipcConnectionClean(1);    
    close(s_ipcSocket);
	close(s_ipcDeviceSocket);
	debug("ipc", "IPC Exit!\n");
    unlink(name);
}

// ****************************************************  ipc for device

int ipcDeviceSocketInit(void)
{
	int sockfd, addrlen, confd, len, i;
	struct sockaddr_in serveraddr;

	/* 1. socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if(sockfd < 0) 
	{		 
		debug("ipc", "call socket() return : %s(%d)", strerror(errno), errno);
		return -1;
	}

	/* 1. set socket reuse */
	int val = 1;
	int ret = setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(void *)&val,sizeof(int));
	if(ret == -1)
	{
		debug("ipc", "socket setfd failed!!\n");
		exit(1);
	}


	/* 2. binding server addr */
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(IPC_SERVER_DEVICE_PORT);
	
	if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
	{
		debug("ipc", "socket binding failed!!\n");
	}
 
	/* 3. listen */
	if(listen(sockfd, IPC_LISTEN_QUEUE_LEN) < 0) 
	{
		debug("ipc", "call listen() return: %s", strerror(errno));
		return -1;
	}

	return sockfd;
}

static void jsonConvertToStandardFormat(char buff[IPC_MSG_MAX_SIZE])
{
	int i;

	for (i = 0; i < IPC_MSG_MAX_SIZE; i ++)
	{
		if (buff[i] == '\'')
		{
			buff[i] = '\"';
		}
	}
}

static void ipcDevicePacketProcess(void *data)
{
	/* receive data */
	int ret;
	char buff[IPC_MSG_MAX_SIZE];
	ipcConnection_t *ipc = data;
	int max_cycle_time = IPC_MAX_CYCLE_TIME;

    if (ipc == NULL)
    {
    	debug("ipc", "data is NULL!");
        return ;
    }

	memset(buff, 0, sizeof(buff));
	
	ssize_t recvSize = recv(ipc->fd, buff, sizeof(buff), MSG_PEEK);
	while((recvSize == -1) && (errno == EINTR) && (max_cycle_time>0))
	{
		debug("ipc", "IPC cycle!!\n");
		recvSize = recv(ipc->fd, buff, sizeof(buff), MSG_PEEK);	
		max_cycle_time --;
	}

    if(recvSize <= 0)
    {
        debug("ipc", "recv(socket=%d) return %d, the client seems crash", ipc->fd, recvSize);
        close(ipc->fd);
        ipc->fd = -1;
        /* clean the unused connections */
        ipcConnectionClean(0);
        return;
    }
	
    if ((ret=recv(ipc->fd, buff, sizeof(buff), 0)) < 0) 
    {
        debug("ipc", "recv() return (%d)", ret);
        return;
    } 	

	/*
		json convert
	*/
	//jsonConvertToStandardFormat(buff);

	/*
		process packet by json
	*/
    cJSON * root = NULL;
    cJSON * item = NULL;

	root = cJSON_Parse(buff);

	if (!root)
	{
		debug("ipc", "Failed to parse JSON : %s", buff);
		return;
	}

	debug("ipc", "%s", cJSON_Print(root));

	/*
		check packet type
	*/
	item = cJSON_GetObjectItem(root, "packetType");
		
	if (!item)
	{
		cJSON_Delete(root);
		debug("ipc", "Failed to get packetType from packet!");
		return ;
	}

	if (item->valueint != IPC_REQUEST)
	{
		cJSON_Delete(root);
		debug("ipc", "Received RESPONSE type packet!");
		return ;
	}

	/*
		get command, and find recall
	*/
	ipcHandle_t function;
	
	item = cJSON_GetObjectItem(root, "command");
	
	if (!item)
	{
		cJSON_Delete(root);
		debug("ipc", "Failed to get cmd from packet!");
		return ;
	}

    function = ipcHandleGet(item->valuestring);

    if (function)
    {
    	item = cJSON_GetObjectItem(root, "operation");

		if (!item)
		{
			cJSON_Delete(root);
			debug("ipc", "Failed to get operation from packet!");
			return ;
		}
		
        function(ipc->fd, item->valueint, (void *)root);
    }
    else 
    {
        debug("ipc", "Unknown command(%s), please check!", (const char *)item->valuestring);
        
        //ipcResponse(ipc->fd, (ipcPacket_t *)&hdr, IPC_STATUS_UNDEF, 0, 0, NULL);        
    }

	cJSON_Delete(root);
}


void ipcDeviceReceive(void *data)
{
    int fd;
    char name[128];
    ipcConnection_t *client = NULL;    
    
    fd = ipcAccept(s_ipcDeviceSocket);

    if (fd < 0)
    {
        debug("ipc", "ipcAccept(%d) failed", s_ipcDeviceSocket);
        return ;
    }
    
    client = (ipcConnection_t *)malloc(sizeof(ipcConnection_t));
    if (client == NULL)
    {
        debugf("malloc", "malloc(%d) failed", sizeof(ipcConnection_t));
        close(fd);
        return ;
    }
    
    debug("ipc", "ipcAccept(%d) return name:%s", s_ipcDeviceSocket, name);
    
    client->fd = fd;
    client->name = strdup(name);       
    client->handle = threadAddListeningFile(client->name, client->fd,  client, ipcDevicePacketProcess);
    client->next= s_ipcConnectionHead;
    s_ipcConnectionHead = client;      
}

void ipcDeviceStart(const char * name)
{
	s_ipcDeviceSocket = ipcDeviceSocketInit();
	assert(s_ipcDeviceSocket >= 0, "Failed to init IPC socket on %s", name); 
	debug("ipc", "Open IPC socket(%d) on %s", s_ipcDeviceSocket, name);

	threadAddListeningFile("sys.ipc", s_ipcDeviceSocket, NULL, ipcDeviceReceive);
}

