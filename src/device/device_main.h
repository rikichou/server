#ifndef __DEVICE_MAIN_H__
#define __DEVICE_MAIN_H__

#define DEVICE_SN_LEN	8

typedef struct
{
	unsigned int sn[DEVICE_SN_LEN];
}device_t;

typedef struct device_info
{
	int fd;
	unsigned int sn[DEVICE_SN_LEN];
	struct device_info *next;
}device_info_t;

void devicePreInit();

#endif
