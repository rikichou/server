
/*
Copyright (c) 2014,  Shenzhen Hexicom Technologies Co., Ltd.
All rights reserved.

File         : configFile.h
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





#ifndef __CONFIGFILE_H__
#define __CONFIGFILE_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>


#define CONFIGFILE_HASH_ARRAY_SIZE  129
#define CONFIGFILE_LINE_MAX_SIZE  2048

/* only limit for configItemPrint() */
#define CONFIGFILE_PRINT_BUFFER_SIZE 1024


struct configItem
{
    char *name;
    char *value;
    struct configItem *next;
};

typedef struct configItem * configHead_t[CONFIGFILE_HASH_ARRAY_SIZE];


configHead_t *configFileParse(const char *path);
configHead_t *configHeadNew(void);
void configHeadFree(configHead_t head);

void configItemSet(configHead_t head, const char *name, const char *value);
int configItemPrint(configHead_t head, const char *name, const char *format, ...);

void configItemSetQuote(configHead_t head, const char *name, const char *value);
int configItemPrintQuote(configHead_t head, const char *name, const char *format, ...);

void configItemUnset(configHead_t head, const char *name);

const char *configItemGet(configHead_t head, const char *name);

int configFileOutput(const char *path, configHead_t head, int export);
int configFileWrite(const char *path, configHead_t head);


int configFileSet(const char *path, const char *name, const char *value);
int configFileUnset(const char *path, const char *name);


#endif /* __CONFIGFILE_H__ */



