
#include "device_main.h"
#include "libipc/ipcClient.h"
#include "lib/debug.h"
#include "lib/stringHelper.h"
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

#define NDB_MAX_CH_DATA	50

int NDB_data_parse(char *string, char *msg_buff, int size)
{
	int num, i, len=0;
	char *var[NDB_MAX_CH_DATA];

	num = stringSplit(string, ';', var, sizeof(var)/sizeof(var[0]));

	for (i = 0; i < num; i ++)
	{
		int d1,d2,d3,d4,d5,d6;
		char tmp[128];
		
		sscanf(var[i], "%d,%d,%d,%2d%2d%2d%s", &d1,&d2,&d3,&d4,&d5,&d6,tmp);
		printf("%d %d %d %d %d %d %s\n", d1,d2,d3,d4,d5,d6,tmp);
		
		if (d6 == 4)
		{
			int q;
			float data;
			sscanf(tmp, "%08x", &q);
			data = *((float *)(&q));
			len += snprintf(msg_buff+len, size-len, "\"%d\",\"%d\",\"%d\",\"%d\",\"%f\"\r\n", d1,d2,d3,d4,data);
		}
		else if (d6 == 8)
		{
			long q;
			double data;
			
			sscanf(tmp, "%16lx", &q);
			data = *((double *)(&q));
			
			len += snprintf(msg_buff+len, size-len, "\"%d\",\"%d\",\"%d\",\"%d\",\"%lf\"\r\n", d1,d2,d3,d4,data);
		}
	}

	return num;
}


int ipc_NDB_data_update(int fd, char *string, unsigned int sn[DEVICE_SN_LEN])
{
	float temp = 11.23;
	int device_num;
	char msg_buff[128];

	/*.how to get temperature by rules */
	NDB_data_parse(string, msg_buff, sizeof(msg_buff)/sizeof(msg_buff[0]));

	int ret = ipcDeviceRequest(fd, msg_buff);

	return 0;
}



