
#include "device_main.h"
#include "libipc/ipcClient.h"
#include "lib/debug.h"
#include "libcjson/cJSON.h"

int ipc_device_main_set(device_info_t *info)
{
	/* prepare json data */
	char buff[256];
	int channel = 0, slave_addr = 0, function_code = 0, reg_addr = 0, reg_num = 0, value_len = 0, value = 0;

	cJSON *root =cJSON_CreateObject();

	if (!root)
	{
		debug("ipc", "Failed to get root object!");
		return -1;
	}
	
	cJSON_AddNumberToObject(root, "packetType", IPC_REQUEST);
	cJSON_AddNumberToObject(root, "operation", IPC_OP_SET);
	cJSON_AddStringToObject(root, "sn", info->sn);

	int len = sprintf(buff, "1,%d,%d,%d,%d,%d,%d,%d", channel, slave_addr, function_code, reg_addr, reg_num, value_len, value);
	cJSON_AddStringToObject(root, "desset", buff);
	
	/* send json */
	char *jsonString = cJSON_Print(root);
	
	int ret = ipcDeviceRequest(info->fd, jsonString);

	if (jsonString)
	{
		free(jsonString);
	}

	cJSON_Delete(root);

	return 0;
}


