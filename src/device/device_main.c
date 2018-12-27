
#include <string.h>
#include <unistd.h>

#include "debug.h"
#include "device_main.h"
#include "req_device_main.h"

/* library */
#include "libipc/ipcType.h"
#include "libipc/ipcServer.h"
#include "libcore/timer.h"
#include "libcjson/cJSON.h"

static device_info_t *device_head = NULL;

char buff[512];

const char *device_sn_to_string(unsigned int sn[DEVICE_SN_LEN])
{
	snprintf(buff, sizeof(buff), "%02x%02x%02x%02x%02x%02x%02x%02x", sn[0],sn[1],sn[2],sn[3],sn[4],sn[5],sn[6],sn[7]);

	return buff;
}

void device_sn_parse(const char *sn_string, unsigned int sn[DEVICE_SN_LEN])
{
	sscanf(sn_string, "%02x%02x%02x%02x%02x%02x%02x%02x", &sn[0],&sn[1],&sn[2],&sn[3],&sn[4],&sn[5],&sn[6],&sn[7]);

	debug("device", "%02x%02x%02x%02x%02x%02x%02x%02x", sn[0],sn[1],sn[2],sn[3],sn[4],sn[5],sn[6],sn[7]);
}

device_info_t *device_find_by_sn(unsigned int sn[DEVICE_SN_LEN], device_info_t *head)
{
	device_info_t *p = head;

	while (p)
	{
		if (!memcmp(sn, p->sn, sizeof(p->sn)))
		{
			return p;
		}
		
		p = p->next;
	}

	return NULL;
}

device_info_t *device_find_by_fd(int fd, device_info_t *head)
{
	device_info_t *p = head;

	while (p)
	{
		if (fd == p->fd)
		{
			return p;
		}
		
		p = p->next;
	}

	return NULL;
}


device_info_t *device_add(device_info_t **phead, device_t *pDev, int fd)
{
	device_info_t *p = (device_info_t *)malloc(sizeof(device_info_t));

	if (!p)
	{
		debug("device", "Failed to add device, Malloc(%d) Failed\n", sizeof(device_info_t));
		return NULL;
	}

	/* clean malloc memory */
	memset(p, 0, sizeof(*p));

	/* fill data */
	p->fd = fd;
	p->next = NULL;
	memcpy(p->sn, pDev->sn, sizeof(p->sn));
	

	/* insert to list */
	if (*phead == NULL)
	{
		*phead = p;
	}
	else
	{
		p->next = *phead;
		*phead = p;
	}

	return p;
}

int debug_cmd_send(void *data, int reason)
{
	device_info_t *info = (device_info_t *)data;

	if (!info)
	{
		debug("device", "Device info is NULL!!");
		return TIMER_REMOVE;
	}

	int ret = ipc_device_main_set(info);

	if (ret != IPC_STATUS_OK)
	{
		debug("device", "Failed to send debug cmd to device!");
	}
	
	return TIMER_REMOVE;
}

void device_info_updata(device_info_t *dev_info, device_t *dev, int fd)
{
	dev_info->fd = fd;
	debug("device", "update device fd %d", fd);
}

int deviceRemove(int *fd)
{
	int sock = *fd;
	device_info_t *p = device_head;
	device_info_t *pre = p;

	while (p)
	{
		if (sock == p->fd)
		{
			if (p == device_head)
			{
				device_head = NULL;
			}
			else
			{
				pre->next = p->next;
			}

			debug("device", "Remove device %s", device_sn_to_string(p->sn));
			free(p);

			return 0;
		}
		
		p = p->next;
	}

	debug("device", "(device remove)not found device fd %d", sock);
	return 0;
}

int newDeviceAdd(int fd, device_t *pDev)
{
	device_info_t *pdev_info = NULL;

	debug("device", "Now add device ... \n");

	/* check if the device is exist */
	if ((pdev_info=device_find_by_sn(pDev->sn, device_head)) != NULL)
	{
		device_info_updata(pdev_info, pDev, fd);
		debug("device", "Device already exists!\n");
		/* JUST FOR DEBUG : register timer handle to send command to device */
		timerAdd("device_cmd_set_timer", 0, 1000, debug_cmd_send, NULL, pdev_info);		
		return 0;
	}
	
	/* add device */
	if ((pdev_info=device_add(&device_head, pDev, fd)) == NULL)
	{
		debug("device", "Failed to add device %s", device_sn_to_string(pDev->sn));
		return -1;
	}
	else
	{
		debug("device", "Add device (%s  fd:%d) success", device_sn_to_string(pDev->sn), fd);
	}

	/* JUST FOR DEBUG : register timer handle to send command to device */
	timerAdd("device_cmd_set_timer", 0, 1000, debug_cmd_send, NULL, pdev_info);
	
	return 0;
}

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int debug_message_save(unsigned int sn[DEVICE_SN_LEN], char *data)
{
	char buffer[256];
	struct tm  *tp; 
	time_t t = time(NULL);	
	tp = localtime(&t);

	int len = sprintf(buffer, "%d/%d/%d %d:%d:%d data:(%s)\n", tp->tm_year+1900, tp->tm_mon+1, tp->tm_mday, tp->tm_hour,tp->tm_min,tp->tm_sec, data);

	/* file name */
	char file_name[64];
	sprintf(file_name, "log/%s.log", device_sn_to_string(sn));

	/* save to file */
	int fd = open(file_name, O_RDWR|O_CREAT|O_APPEND, 0777);

	if (fd < 0)
	{
		debug("device", "Failed to open log file (%d)%s", fd, file_name);
		return -1;
	}

	int ret = write(fd, buffer, len);

	if (ret < 0)
	{
		close(fd);
		debug("device", "Failed to write log to log file (%d)%s", ret, file_name);
		return -1;
	}
	
	close(fd);
	return 0;
}

int debug_info_deal(int fd, cJSON *root)
{
	device_t device;
	
	int NDB_fd = ipcNDBSocketGet();

	if (NDB_fd == -1)
	{
		debug("device", "have any windows client connect to server??");
		return -1;
	}

	/* get sn code */
	cJSON *item = cJSON_GetObjectItem(root, "sn");
	
	if (!item)
	{
		debug("device", "No sn code!");
		return -1;
	}
	
	device_sn_parse(item->valuestring, device.sn);

	/* check if the device is exist */
	if (device_find_by_sn(device.sn, device_head) == NULL)
	{
		debug("device", "Device not exists!\n");
		return -2;
	}

	/* get temperature */
	item = cJSON_GetObjectItem(root, "data");

	if (!item)
	{
		debug("device", "No temperature!");
		return -3;
	}


	/* save message buffer */
	ipc_NDB_data_update(NDB_fd, item->valuestring, device.sn);

	return 0;
}

static void ipcDeviceConfigEntry(int fd, int op, void *data)
{
    int ackStatus;
	cJSON * root = (cJSON *)data;
    
    switch (op)
    {
    	/* device register request */
        case IPC_OP_NEW:
            {
            	device_t device;
				
            	/* get sn code */
				cJSON *item = cJSON_GetObjectItem(root, "sn");

				if (!item)
				{
					debug("device", "No sn code!");
					return ;
				}

				device_sn_parse(item->valuestring, device.sn);

				if (newDeviceAdd(fd, &device) < 0)
				{
					ipcDeviceAck(IPC_STATUS_FAIL);
				}
				else
				{
					ipcDeviceAck(IPC_STATUS_OK);
				}
				
				
            }
            break;
        /* device upload data to server */
        case IPC_OP_UPLOAD_DATA:
            {				                
                /* deal data */
				if (debug_info_deal(fd, root) < 0)
				{
					ipcDeviceAck(IPC_STATUS_FAIL);
				}
				else
				{
					ipcDeviceAck(IPC_STATUS_OK);
				}
            }
            break;
        default:
            ipcDeviceAck(IPC_STATUS_ARGV);
            break;
    }
}

static void ipcNDBConfigEntry(int fd, int op, void *data)
{
    int ackStatus;
	cJSON * root = (cJSON *)data;
    
    switch (op)
    {
    	/* device register request */
        case IPC_OP_SET:
            {
            	device_info_t *pdev;
            	device_t device;
				
            	/* get sn code */
				cJSON *item = cJSON_GetObjectItem(root, "sn");

				if (!item)
				{
					debug("device", "No sn code!");
					return ;
				}

				device_sn_parse(item->valuestring, device.sn);

				if ((pdev=device_find_by_sn(device.sn, device_head)) == NULL)
				{
					debug("device", "Device %s didn`t registered!\n", device.sn);
					return ;
				}

				if (ipc_NDB_main_set(root, pdev) < 0)
				{
					debug("device", "failed to send message to device %s by socket %d\n", device_sn_to_string(pdev->sn), pdev->fd);
				}
				else
				{
					debug("device", "send message to device %s(socket %d) success", device_sn_to_string(pdev->sn), pdev->fd);
				}
				
            }
            break;
        default:
            ipcDeviceAck(IPC_STATUS_ARGV);
            break;
    }
}


void devicePreInit()
{
    ipcHandleAdd("device", ipcDeviceConfigEntry);
    ipcHandleAdd("NDB_cmd_set", ipcNDBConfigEntry);
}


