
/*
Copyright (c) 2014,  Shenzhen Hexicom Technologies Co., Ltd.
All rights reserved.

File         : ipc.h
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




#ifndef __IPC_H__
#define __IPC_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>


#define IPC_SYNC_ID 0xA5A55A5A 


#define IPC_CONNECT_DEFAULT_TIMEOUT   1000  /* ms */
#define IPC_REQUEST_DEFAULT_TIMEOUT   5000  /* ms */

#define IPC_MSG_MAX_SIZE   (64 * 1024)
#define IPC_MSG_WARNING_SIZE    (10 * 1024)

#define IPC_LISTEN_QUEUE_LEN  64  /* server listening queue num */

#define IPC_SERVER_PORT 8888

//inline uint32_t ipcHash(const char *str);

#endif /* __IPC_H__ */


