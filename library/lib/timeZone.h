
/*
Copyright (c) 2014,  Shenzhen Hexicom Technologies Co., Ltd.
All rights reserved.

File         : timeZone.h
Status       : Current
Description  : 

Author       : Liu Chuansen
Contact      : 179712066@qq.com

Revision     : 2014-04 ~ 2014-10
Description  : Primary released

## Please log your description here for your modication ##

Revision     : 
Modifier     : 
Description  : 

*/


#ifndef __TIMEZONE_H__ 
#define __TIMEZONE_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <sys/time.h>

int timeZoneInfoListGet(char *data, int size);
int timeZoneInfoListSize(void);

int timeZoneNum(void);

int32_t timeZoneOffsetSeconds(int zoneId);

const char *timeZoneInfoGet(int zoneId, char *buffer, int length);

int validTimeZoneId(int zoneId);

#endif /* #ifndef __TIMEZONE_H__*/

