
#include <string.h>

#include "debug.h"
#include "device_main.c"

/* library */
#include "libipc/ipcType.h"
#include "libcore/timer.h"

static device_info_t *device_head = NULL;

device_t *device_find(unsigned char *sn, device_info_t *head)
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

device_info_t *device_add(device_info_t **phead, device_t *pDev)
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
		return TIMER_REMOVE;
	}

	
}

int newDeviceAdd(int fd, device_t *pDev)
{
	debug("device", "Now add device ... \n");

	/* check if the device is exist */
	if (device_find(pDev->sn, device_head))
	{
		debug("device", "Device already exists!\n");
		return 0;
	}
	
	/* add device */
	device_info_t *pdev_info = NULL;
	if ((pdev_info=device_add(&device_head, pDev)) != NULL)
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
            
        case IPC_OP_SET:
            {
                gmrpConfig_t *pConfig;
                
                pConfig = ipcData(request, typeof(pConfig));
                
                gmrpConfigSet(pConfig);
                
                ipcAck(IPC_STATUS_OK);
            }
            break;
		/* gmrp port config */
        case IPC_OP_GET1:		
            {
                int num = 0;
                portMask_t *ports;
                gmrpPortConfig_t *pConfigs = NULL;
                
                ports = ipcData(request, typeof(ports));
                num = portMaskNum(ports);

                if (num <= 0)
                {
                    ipcAck(IPC_STATUS_ARGV);
                    return ;
                }
                pConfigs = (gmrpPortConfig_t *)malloc(sizeof(*pConfigs) * num);
                
                if (pConfigs == NULL)
                {
                    debugf("malloc", "malloc(%d)", sizeof(*pConfigs) * num);
                    ipcAck(IPC_STATUS_NOMEM);
                    return; 
                }

                /* get configs */
                int p = 0, count = 0;
                for (p = 0; ((p = portMaskGetNext(ports, p)) >= 0) && (count < num); p ++)
                {
					if (p == PORT_CPU)
					{
						continue;
					}
				
                    ret = gmrpPortConfigGet(p, &pConfigs[count]);

                    if (ret == 0)
                    {
                        ipcAck(IPC_STATUS_OP_FAILED);
                        free(pConfigs);
                        return;
                    }

                    count ++;
                }

                ipcResponse(fd, request, IPC_STATUS_OK, sizeof(*pConfigs), count, pConfigs);
                free(pConfigs);
            }            
            break;
            
        case IPC_OP_SET1:
            {   
                int num = 0;
                gmrpPortConfig_t *pConfigs = NULL;

                num = ipcItemNum(request);
                pConfigs = ipcData(request, typeof(pConfigs));

                if (!ipcSizeCheck(request, sizeof(*pConfigs), num) || num <= 0)
                {
                    ipcAck(IPC_STATUS_ARGV);
                    return;
                }

                /* config set */
                ret = gmrpPortConfigSet(pConfigs, num);

				if (ret <= 0)
				{
					if (ret == -1)
					{
						ipcAck(IPC_STATUS_GMRP_NOT_ENABLED);				
					}
					else
					{
						ipcAck(IPC_STATUS_ARGV);		
					}
					return ;
				}

                ipcAck(IPC_STATUS_OK);
            }
            break;
		case IPC_OP_GET2:		//get dynamic multicast entrys
			break;

		case IPC_OP_NUM:		//get dynamic multicast entry num
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


