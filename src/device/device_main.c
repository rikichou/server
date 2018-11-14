
#include "debug.h"

int newDeviceAdd(device_t *pDev)
{
	debug("device", "Now add device ... ");

	/* check if the device is exist */

	/* add device */

	debug("device", "Add device success");

	/* register timer handle to send command to device */
	
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
				
				newDeviceAdd(pDev);
                
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


