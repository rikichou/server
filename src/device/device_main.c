
#include <string.h>

#include "debug.h"
#include "device_main.c"

/* library */
#include "libipc/ipcType.h"
#include "libcore/timer.h"

static device_info_t *device_head = NULL;

device_t *device_find_by_sn(unsigned char *sn, device_info_t *head)
{
	device_info_t *p = head;

	while (p)
	{
		if (!memcmp(sn, p->sn, DEVICE_SN_LEN))
		{
			return p;
		}
		
		p = p->next;
	}

	return NULL;
}

device_t *device_find_by_fd(int fd, device_info_t *head)
{
	device_info_t *p = head;

	while (p)
	{
		if (fd == p->sn)
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
		return -1;
	}

	/* clean malloc memory */
	memset(p, 0, sizeof(*p));

	/* fill data */
	p->fd = fd;
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

	return 0;
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

int newDeviceAdd(int fd, device_t *pDev)
{
	debug("device", "Now add device ... \n");

	/* check if the device is exist */
	if (device_find_by_sn(pDev->sn, device_head))
	{
		debug("device", "Device already exists!\n");
		return 0;
	}
	
	/* add device */
	device_info_t *pdev_info = NULL;
	if ((pdev_info=device_add(&device_head, pDev, fd)) != NULL)
	{
		debug("device", "Failed to add device");
		return -1;
	}
	else
	{
		debug("device", "Add device success\n");
	}

	/* JUST FOR DEBUG : register timer handle to send command to device */
	timerAdd("device_cmd_set_timer", 0, 1000, debug_cmd_send, NULL, pdev_info);
	
	return 0;
}

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int debug_message_save(unsigned char *sn, char *msg, int size)
{
	char buffer[256];
	struct tm  *tp; 
	time_t t = time(NULL);	
	tp = localtime(&t);

	int len = sprintf(buffer, "%d/%d/%d %d:%d:%d %s\n", tp->tm_year+1900, tp->tm_mon+1, tp->tm_mday, tp->tm_hour,tp->tm_min,tp->tm_sec, msg);

	/* file name */
	char file_name[64];
	sprintf(file_name, "%02x%02x%02x%02x%02x%02x", sn[0], sn[1], sn[2], sn[3], sn[4], sn[5]);

	/* save to file */
	in fd = open(file_name, O_WRONLY|O_CRAET);

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

void debug_info_deal(int fd, char *msgbuffer, int size)
{
	/* get device info */
	device_info_t *device = device_find_by_fd(fd, device_head);

	if (!device)
	{
		debug("device", "Failed to find device by fd : %d", fd);
		return ;
	}

	/* save message buffer */
	debug_message_save(device->sn, msgbuffer, size);
}

static void ipcDeviceConfigEntry(int fd, int op, ipcPacket_t *request)
{
    int ret;
    
    switch (op)
    {
    	/* device register request */
        case IPC_OP_NEW:
            {
                device_t *pDev;
                
                pDev = ipcData(request, typeof(device_t));
				
				newDeviceAdd(fd, pDev);
                
                ipcAck(IPC_STATUS_OK);
            }
            break;
        /* device upload data to server */
        case IPC_OP_UPLOAD_DATA:
            {
				/* get data size */
				int size = ipcDataSize(request);
				
                /* get data info */
				char *msgbuffer;
                msgbuffer = ipcData(request, typeof(msgbuffer));
                
                /* deal data */
				debug_info_deal(fd, msgbuffer, size);
				
                ipcAck(IPC_STATUS_OK);
            }
            break;
        default:
            ipcAck(IPC_STATUS_ARGV);
            break;
    }
}


void devicePreInit()
{
    ipcHandleAdd("device", ipcDeviceConfigEntry);
}


