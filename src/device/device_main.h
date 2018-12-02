#ifndef __DEVICE_MAIN_H__
#define __DEVICE_MAIN_H__

#define DEVICE_SN_LEN	8

typedef struct
{
	char sn[DEVICE_SN_LEN*2+1];
}device_t;

typedef struct device_info
{
	int fd;
	char sn[DEVICE_SN_LEN*2+1];
	struct device_info *next;
}device_info_t;

void devicePreInit();

#endif
