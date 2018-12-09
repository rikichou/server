
#ifndef __REQ_DEVICE_MAIN_H__
#define __REQ_DEVICE_MAIN_H__

#include "device_main.h"
#include "libcjson/cJSON.h"

int ipc_device_main_set(device_info_t *info);
int ipc_NDB_main_set(cJSON *root, device_info_t *info);
int ipc_NDB_data_update(int fd, int subdevice, const char *string, const char *sn);

#endif
