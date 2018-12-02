
/*
Copyright (c) 2014,  Shenzhen Hexicom Technologies Co., Ltd.
All rights reserved.

File         : ipcClient.c
Status       : Current
Description  : 

Author       : Liu Chuansen
Contact      : 179712066@qq.com

Revision     : 2014-04 ~ 2014-10
Description  : Primary released

## Please log your description here for your modication ##

Revision     : 
Modifier     : 
Description  : 

*/





#include "ipc.h"
#include "ipcClient.h"

#include <stringHelper.h>
#include <systemHelper.h>

unsigned long g_ipcIdentity = 0;

const char * ipcStatusString(int status)
{    
    return stringAt(IPC_STATUS_NAMES, status, 0);
}


static ipcPacket_t * ipcWaitResponse
    (
        int sock, 
        uint32_t identity, 
        uint32_t requestId, 
        int msTimeout
    )
{
    int ret;
    ipcHeader_t hdr;
    ipcPacket_t *pkt = NULL;
    ssize_t recvSize;
    ssize_t size = 0;
    
    struct timeval tv;
    fd_set rfds;
	
    requestId = (requestId & 0xFFFFFFFC) | IPC_RESPONSE;
	
    debug("ipc.client", "Identity:%08X, pid:%d, op:%d, wait for %d ms", identity, requestId >> 16, (requestId >> 8) & 0xff, msTimeout);                
    
    do
    {
		FD_ZERO(&rfds);
		FD_SET(sock, &rfds);
		
		tv.tv_sec = msTimeout / 1000;
		tv.tv_usec = (msTimeout % 1000) * 1000;

		ret = select(sock + 1, &rfds, NULL, NULL, &tv);

		/* do not break by a signal */
		if ((ret < 0) && (errno == EINTR))
		{
			ret = 1;
			continue;
		}
        
        if (ret > 0) 
        {
            if (FD_ISSET(sock, &rfds)) 
            {                                 
                recvSize = recv(sock, &hdr, sizeof(ipcHeader_t), MSG_PEEK);
			
				while((recvSize == -1) && (errno == EINTR))
				{
					recvSize = recv(sock, &hdr, sizeof(ipcHeader_t), MSG_PEEK);
				}

                if (recvSize <= 0)
                {   
                    debug("ipc.client", "recv(socket=%d) return %d", sock, recvSize);
                    return NULL;                    
                }
                else if (recvSize < sizeof(ipcHeader_t)) 
                {       
                    debug("ipc.client", "recv() size is too small(%d, recive size:%d, header size:%d)", sock, recvSize, sizeof(ipcHeader_t));                
                    recvSize = recv(sock, &hdr, sizeof(ipcHeader_t), MSG_TRUNC);                                    
                    continue;
                }

                recvSize = sizeof(hdr) + ntohl(hdr.dataOffset) + ntohl(hdr.dataSize);
				
                debug("ipc.client", "get response, size:%d syncId:%08X, identity:%08X, pid:%d, op:%d", recvSize, ntohl(hdr.syncId), ntohl(hdr.identity), ntohl(hdr.requsetId) >> 16, (ntohl(hdr.requsetId) >> 8) & 0xff);

                if ((ntohl(hdr.syncId) != IPC_SYNC_ID)
                    || (ntohl(hdr.identity) != identity)
                    || (ntohl(hdr.requsetId) != requestId)
                    || ((recvSize < 0) || (recvSize > IPC_MSG_MAX_SIZE))
                    )
                {
					recvSize = recv(sock, &hdr, sizeof(ipcHeader_t), MSG_TRUNC);
                    continue;
                }
				
                
                pkt = (ipcPacket_t *)malloc(recvSize);
                if (pkt == NULL) 
                {
                    debug("ipc.client", "malloc(%d)", recvSize);                                        
                    recvSize = recv(sock, &hdr, sizeof(ipcHeader_t), MSG_TRUNC);
					
					while((recvSize == -1) && (errno == EINTR))
					{
						recvSize = recv(sock, &hdr, sizeof(ipcHeader_t), MSG_TRUNC);
					}
					
                    return NULL;
                }

                size = 0;
                while (size < recvSize)
                {
                    int readSize = recv(sock, ((uint8_t *)pkt) + size, recvSize - size, 0);

					while((readSize == -1) && (errno == EINTR))
					{
						readSize = recv(sock, ((uint8_t *)pkt) + size, recvSize - size, 0);
					}
					
                    if (readSize <= 0)
                    {
                        debug("ipc.client", "line:%d, readSize=%d errno=%d(%m)", __LINE__, readSize, errno);    
                        break;
                    }
                    else
                    {
                        size += readSize;
                    
                        if (size >= recvSize)
                        {
                            debug("ipc.client", "line:%d, size=%d, recvSize=%d", __LINE__, size, recvSize);    
                            break;
                        }
                    }
                }
        
                if (size < recvSize) 
                {
                    debug("ipc.client", "recv() not return the expected size(get %d, expected %d)", size, recvSize);
                    free(pkt);
                    continue;
                }

                return pkt;
            }
        }
    }while(ret > 0);

    if (ret < 0)
    {
        debug("ipc.client", "select() failed, return %d, errno = %d", ret, errno);
    }
    
    return NULL;
}


int ipcConnect
    (
        const char *localClientPath, 
        const char *localServerPath, 
        int msTimeout
    )
{
    int sock, size;
    struct sockaddr_un client, server;
    struct timeval tv;
    
    if ((localClientPath == NULL) || (localServerPath == NULL))
    {
        return -1;
    }
    
    debug("ipc.client", "process [%s] is connecting to [%s] (timeout=%d)", localClientPath, localServerPath, msTimeout);
    
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    
    if(sock < 0) 
    {
        debug("ipc.client", "call socket() return : %s(%d)", strerror(errno), errno);
        return -1;
    }

    /* bind client address to this socket */
    memset(&client, 0, sizeof(client));
    client.sun_family = AF_UNIX;
    sprintf(client.sun_path, "%s", localClientPath);

    size = offsetof(struct sockaddr_un, sun_path) + strlen(client.sun_path);

    #ifdef IPC_ABSTRACT_NAMESPACE
    client.sun_path[0] = 0;
    #else    
    /* remove if exist */
    unlink(client.sun_path);
    #endif 
    
    if(bind(sock, (struct sockaddr *)&client, size) < 0) 
    {
        debug("ipc.client", "call bind() return: %s", strerror(errno));
        close(sock);
        return -1;
    }

    /* connect to server */
    memset(&server, 0, sizeof(server));
    server.sun_family = AF_UNIX;

    sprintf(server.sun_path, "%s", localServerPath);

    size = offsetof(struct sockaddr_un, sun_path) + strlen(server.sun_path);

    #ifdef IPC_ABSTRACT_NAMESPACE
    server.sun_path[0] = 0;
    #endif 
        
    if(connect(sock, (struct sockaddr *)&server, size) < 0) 
    {
        debug("ipc.client", "call connect() return: %s", strerror(errno));
        close(sock);
        return -1;
    }

    /* set socket timeout */

    if (msTimeout <= 0)
    {
        msTimeout = IPC_CONNECT_DEFAULT_TIMEOUT;
    }
    
    tv.tv_sec = msTimeout / 1000;
    tv.tv_usec = (msTimeout % 1000) * 1000;
    
    if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) 
    {
        debug("ipc.client", "call setsockopt: %s\n", strerror(errno));        
        close(sock);
        return -1;
    }

    return sock;
}

static uint32_t ipcHash(const char *str)
{
    uint32_t hash = 0;
    
    while (*str)
    {
        hash = 31 * hash + *str ++;
    }
    
    return hash;
}

void jsonConvertToDevieFormat(char *buff)
{
    int i;

    for (i = 0; i < strlen(buff); i ++)
    {
        if (buff[i] == '\"')
        {
            buff[i] = '\'';
        }
    }

}

/* TODO ... add wait response */
int ipcDeviceRequest(int sock, char *json)
{
    //jsonConvertToDevieFormat(json);

	uint32_t len = strlen(json);

	int ret = send(sock, json, len, 0);
	
	while((ret == -1) && (errno == EINTR))
	{
		ret = send(sock, json, len, 0);	
	}
	    
    if (ret != len)
    {
        debug("ipc", "send (socket=%d, size=%d) return %d", sock, len, ret);
    }
   
    return ret;	
}

/*
 send a message 
*/
ipcPacket_t *ipcRequest
    (
        int sock,       /* socket descriptor return by ipcConnect */
        uint32_t identity,
        const char *command, /* the registered string in server */
        int operation, /* */
        int parameterSize, 
        int parameterNum, 
        void *parameterData,
        int msTimeout
    )
{
    ssize_t ret;
    uint32_t requestId;
    uint32_t reqSize, commandLength, dataLength;
    ipcPacket_t *req, *ack;

    if ((command == NULL) || (command[0] == '\0'))
    {
        return NULL;
    }

    debug("ipc.client", "process(pid=%d) sends command [%s](data Size:%d * %d) on socket(%d)", 
        getpid(), command, parameterSize, parameterNum, sock);
    
    requestId = (getpid() << 16) | ((operation & 0xff) << 8) | IPC_REQUEST;
    
    dataLength = (parameterNum && parameterData) ? parameterSize * parameterNum : 0;
    commandLength = strlen(command) + 1;
    
    reqSize = sizeof(ipcHeader_t) + commandLength + dataLength;   

    if (reqSize > IPC_MSG_MAX_SIZE)
    {
        debug("ipc.client", "request message length(%d > %d) is too large", reqSize, IPC_MSG_MAX_SIZE);
        return NULL;
    }
    
    req = (ipcPacket_t *)malloc(reqSize);

    if (req == NULL)
    {
        debugf("malloc", "malloc(%d)", reqSize);
        return NULL;
    }
   
    req->header.syncId = htonl(IPC_SYNC_ID);
    req->header.requsetId = htonl(requestId);
    req->header.identity = htonl(identity);
    req->header.ackStatus = htonl(IPC_STATUS_OK);

    req->header.checkSum = 0;
    req->header.commandHash = htonl(ipcHash(command));
    req->header.commandOffset = 0;
    req->header.commandLength = htonl(commandLength);

    req->header.dataOffset = htonl(commandLength);
    req->header.dataSize = htonl(dataLength);
    req->header.itemNum = htonl(parameterNum);
    req->header.itemSize = htonl(parameterSize);
    
    strcpy((char*)&req->payload[ntohl(req->header.commandOffset)], command);

    if (dataLength && parameterData)
    {
        memcpy(&req->payload[ntohl(req->header.dataOffset)], parameterData, dataLength);
    }
    
    ret = send(sock, req, reqSize, 0);

    if (ret == -1)
    {
        debug("ipc.client", "send() return -1, the server may be crashed!", ret);
        return NULL;
    }
    
    if (ret != reqSize)
    {
        debug("ipc.client", "send() return %d (socket=%d, size=%d, request=%08X)", ret,  sock, reqSize, requestId);
    }

    free(req);

    if (msTimeout <= 0)
    {
        msTimeout = IPC_REQUEST_DEFAULT_TIMEOUT;
    }    

    ack = ipcWaitResponse(sock, identity, requestId, msTimeout);

        
    if (ack == NULL)
    {
        debug("ipc.client", "send timeout(socket=%d, size=%d, request=%08X)", sock, reqSize, requestId);
    }
    else 
    {
        if (ntohl(ack->header.ackStatus) != IPC_STATUS_OK)
        {
            debug("ipc.client", "receive error:%s(%d) (socket=%d, size=%d, request=%08X)", ipcStatusString(ntohl(ack->header.ackStatus)), ntohl(ack->header.ackStatus), sock, reqSize, requestId);
        }
    }
    
    return ack;
}

int ipcCheck(int sock)
{
    uint8_t check = 0x55;
	int ret;

	ret = send(sock, &check, sizeof(check), 0);

	while ((ret == -1) && (errno == EINTR))
	{
		ret = send(sock, &check, sizeof(check), 0);
	}
	
    if (ret > 0)
    {
        return 0;
    }
	
    return -1;
}


int ipcEntryc(int fd, const char *command, int argc, char **argv, int *returnValue)
{
    int ret, i;
    ipcPacket_t *pkt;
    int msgLen = 0;
    char msgBuffer[IPC_ENTRYC_MSG_MAX_SIZE];

    if (command == NULL)
    {
        return IPC_STATUS_ARGV;
    }
 
    if (argv)
    {
        for (i = 0; (i < argc) && (argv[i]); i ++)
        {
            if (msgLen < sizeof(msgBuffer)/sizeof(msgBuffer[0]) - 1)
            {
                msgLen += sprintf(msgBuffer + msgLen, "%s ", argv[i]);
            }
        }
    }

    msgBuffer[sizeof(msgBuffer) - 1] = '\0';

    pkt = ipcRequest
        (
            fd,
            g_ipcIdentity ++,
            command,
            IPC_OP_SET,
            msgLen ? msgLen + 1 : 0,
            msgLen ? 1 : 0, 
            msgBuffer,
            0 /* 0 - default timeout */
        );

    if (pkt == NULL)
    {
        return IPC_STATUS_FAIL;
    }

    ret = ipcStatus(pkt);
    
    if ((ret == IPC_STATUS_OK) && returnValue 
            && (ipcDataSize(pkt) == sizeof(*returnValue)))
    {
        memcpy(returnValue, ipcData(pkt, typeof(returnValue)), sizeof(*returnValue));
    }

    free(pkt); 
    
    return ret;
}


int ipcGetNum(int fd, const char *command, int op, int *num)
{
    int ret;
    ipcPacket_t *pkt;
    pkt = ipcRequest
        (
            fd,
            g_ipcIdentity ++,
            command,
            !op ? IPC_OP_NUM : op,
            0,
            0, 
            NULL,
            0 /* 0 - default timeout */
        );

    if (pkt == NULL)
    {
        return IPC_STATUS_FAIL; /* timeout or nomem */
    }

    ret = ipcStatus(pkt);

    if (ipcOk(pkt) && !ipcSizeCheck(pkt, sizeof(*num), 1))
    {
        ret = IPC_STATUS_ARGV;
    }
    
    if ((ret == IPC_STATUS_OK) && num)
    {
        memcpy(num, ipcData(pkt, typeof(num)), sizeof(*num));
    }

    free(pkt);    

    return ret;
}


