
#include "device_main.h"
#include "libipc/ipcClient.h"
#include "lib/debug.h"

int ipc_device_main_set(device_info_t *info)
{
	/* prepare data */
    char msgbuffer[256];
    int channel = 0;
    int slave_addr = 0;
    int function_code = 0;
    int reg_addr = 0;
    int reg_num = 0;
    int value_len = 0;
    int value = 0;

	/* generate cmd data */
    int len = sprintf(msgbuffer, sizeof(msgbuffer), "{'SN':%02x%02x%02x%02x%02x%02x,'DESSET':1,%d,%d,%d,%d,%d,%d,%d}",
		info->sn[0], info->sn[1], info->sn[2], info->sn[3], info->sn[4], info->sn[5],
		channel, slave_addr, function_code, reg_addr, reg_num, value_len, value);

	/* send packet */
	int ret;
	ipcPacket_t *pkt;
	pkt = ipcRequest
		  (
			  info->fd,
			  g_ipcIdentity ++,
			  "device",
			  IPC_OP_SET,
			  len,
			  1, 
			  info,
			  0 /* 0 - default timeout */
		  );
	
	if (pkt == NULL)
	{
		return IPC_STATUS_FAIL; /* timeout or nomem */
	}
	
	ret = ipcStatus(pkt);
	
	free(pkt);	  
	
	return ret;  
}


