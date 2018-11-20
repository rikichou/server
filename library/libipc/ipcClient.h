
/*
Copyright (c) 2014,  Shenzhen Hexicom Technologies Co., Ltd.
All rights reserved.

File         : ipcClient.h
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




#ifndef __IPCCLIENT_H__
#define __IPCCLIENT_H__

#include "ipcType.h"

#define IPC_ENTRYC_MSG_MAX_SIZE 2048

int ipcConnect
    (
        const char *localClientPath, 
        const char *localServerPath, 
        int msTimeout
    );

int ipcDeviceRequest(int sock, char *json);

ipcPacket_t *ipcRequest
    (
        int sock,       /* socket descriptor return by ipcConnect */
        uint32_t identity,
        const char *command, /* the registered string in server */
        int operation, /* */
        int parameterSize, 
        int parameterNum, 
        void *parameterData,
        int msTimeout
    );

int ipcCheck(int sock);

int ipcEntryc(int fd, const char *command, int argc, char **argv, int *returnValue);
int ipcGetNum(int fd, const char *command, int op, int *num);

const char * ipcStatusString(int status);

extern unsigned long g_ipcIdentity;

#endif /* __IPCCLIENT_H__ */


