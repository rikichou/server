
#include "device_main.h"
#include "libipc/ipcClient.h"
#include "lib/debug.h"
#include "libcjson/cJSON.h"

const char *device_sn_to_string(unsigned int sn[DEVICE_SN_LEN]);

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
	cJSON_AddStringToObject(root, "sn", device_sn_to_string(info->sn));

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

int ipc_NDB_main_set(cJSON *root, device_info_t *info)
{
	/*.message prepare */
	char *jsonString = cJSON_Print(root);
	
	int ret = ipcDeviceRequest(info->fd, jsonString);

	if (jsonString)
	{
		free(jsonString);
	}

	return 0;
}

int ipc_NDB_data_update(int fd, int subdevice, const char *string, unsigned int sn[DEVICE_SN_LEN])
{
	float temp = 11.23;
	int device_num;
	char msg_buff[128];

	/*.how to get temperature by rules */

	memset(msg_buff, 0, sizeof(msg_buff));

	snprintf(msg_buff, sizeof(msg_buff), "%s,%d,%s\n", device_sn_to_string(sn), subdevice, string);

	int ret = ipcDeviceRequest(fd, msg_buff);

	return 0;
}



