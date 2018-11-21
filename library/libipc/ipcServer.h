
/*
Copyright (c) 2014,  Shenzhen Hexicom Technologies Co., Ltd.
All rights reserved.

File         : ipcServer.h
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




#ifndef __IPCSERVER_H__
#define __IPCSERVER_H__

#include "ipcType.h"


int ipcSocketInit(void);
int ipcAccept(int sock);

int ipcResponse
    (
        int sock, 
        ipcPacket_t *request, 
        uint32_t status, 
        int parameterSize, 
        int parameterNum,         
        void *parameterData
    );

#define ipcSimpleAck(fd, req, status) ipcResponse(fd, req, status, 0, 0, NULL)


typedef void (*ipcHandle_t)(int fd, int op, void *request);

struct ipcHandleList
{
    const char *command;
    ipcHandle_t function;
    int countInHead;    
    struct ipcHandleList *next;
};

struct ipcHandleList *ipcHandleAdd(const char *command, ipcHandle_t function);

ipcHandle_t ipcHandleGet(const char *command);

void ipcHandleRemoveAll(void);

void ipcReceive(void *data);

void ipcInit(void);

void ipcStart(const char * name);
void ipcExit(const char * name);

int ipcClientsProcessId(int *pids, int size);

int ipcDeviceSimpleResponse(int sock, int status);

/*
    ipc quick macros 
*/
#define ipcAck(status) ipcSimpleAck(fd, request, status)
#define ipcDeviceAck(status) ipcDeviceSimpleResponse(fd, status)


#endif /* __IPCSERVER_H__ */


