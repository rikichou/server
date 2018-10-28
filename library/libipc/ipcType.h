
/*
Copyright (c) 2014,  Shenzhen Hexicom Technologies Co., Ltd.
All rights reserved.

File         : ipcType.h
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




#ifndef __IPCTYPE_H__
#define __IPCTYPE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <debug.h>

#include <arpa/inet.h> // ntohl etc.


//#define IPC_ABSTRACT_NAMESPACE 

typedef struct
{
    uint32_t syncId;
    uint32_t requsetId; /* | pid | op | ack | */
    uint32_t identity; /* a random value in request, response send it back */
    uint32_t ackStatus;

    uint32_t checkSum;  /* not used now */
    uint32_t commandHash;  /* include end with 0 */
    uint32_t commandOffset;    
    uint32_t commandLength; /* include ending char '\0'*/
    
    uint32_t dataOffset;
    uint32_t dataSize;    
    uint32_t itemSize;
    uint32_t itemNum;
}ipcHeader_t;

typedef struct
{
    ipcHeader_t header;
    uint8_t payload[0];
}ipcPacket_t;

typedef enum
{
    IPC_OP_NONE = 0,
    IPC_OP_GET = 1,
    IPC_OP_SET,
    IPC_OP_NEW,
    IPC_OP_DEL,
    IPC_OP_NUM,
    /* some api needs more than one */    
    IPC_OP_GET1,
    IPC_OP_GET2,
    IPC_OP_GET3,
    IPC_OP_GET4,
    IPC_OP_GET5,
    
    IPC_OP_SET1,
    IPC_OP_SET2,
    IPC_OP_SET3,
    IPC_OP_SET4,
    IPC_OP_SET5,

    IPC_OP_NEW1,
    IPC_OP_NEW2,
    IPC_OP_NEW3,
    IPC_OP_NEW4,
    IPC_OP_NEW5,
    
    IPC_OP_DEL1,
    IPC_OP_DEL2,
    IPC_OP_DEL3,
    IPC_OP_DEL4,
    IPC_OP_DEL5,

    IPC_OP_NUM1,
    IPC_OP_NUM2,
    IPC_OP_NUM3,
    IPC_OP_NUM4,
    IPC_OP_NUM5,

    IPC_OP_USR1,
    IPC_OP_USR2,
    IPC_OP_USR3,
    IPC_OP_USR4,
    IPC_OP_USR5,
    IPC_OP_USR6,
    IPC_OP_USR7,
    IPC_OP_USR8,
    IPC_OP_USR9,
    IPC_OP_USR10,
    
    IPC_OP_END
}ipcOp_t;

typedef enum 
{
    IPC_STATUS_OK = 0,
    IPC_STATUS_FAIL,
    IPC_STATUS_ARGV,  // invalid arguments
    IPC_STATUS_BUSY,
    IPC_STATUS_NOMEM, // 
    IPC_STATUS_UNDEF, // command not found
    IPC_STATUS_INVALID, // invalid request     
    IPC_STATUS_NOOBJ, // 
    IPC_STATUS_ERRACK,
    IPC_STATUS_FULL,
    IPC_STATUS_OP_FAILED, /* operation failed */
    IPC_STATUS_FUNC_DISABLED, /* this function is disabled */
    IPC_STATUS_NOT_SUPPORTED, /* not supported */
    IPC_STATUS_END
}ipcStatus_t;

#define IPC_STATUS_NAMES "" \
    "IPC_STATUS_OK\0" \
    "IPC_STATUS_FAIL\0" \
    "IPC_STATUS_ARGV\0" \
    "IPC_STATUS_BUSY\0" \
    "IPC_STATUS_NOMEM\0" \
    "IPC_STATUS_UNDEF\0" \
    "IPC_STATUS_INVALID\0" \
    "IPC_STATUS_NOOBJ\0" \
    "IPC_STATUS_ERRACK\0" \
    "IPC_STATUS_FULL\0" \
    "IPC_STATUS_OP_FAILED\0" \
    "IPC_STATUS_FUNC_DISABLED\0" \
    "IPC_STATUS_NOT_SUPPORTED\0" \
    "\0"

typedef enum
{
    IPC_REQUEST = 0,
    IPC_RESPONSE     
}ipcPacketType_t;


#define ipcData(pkt, type) (type)(&(pkt)->payload[ntohl((pkt)->header.dataOffset)])
#define __ipcSizeCheck(pkt, size, num)  \
    ((ntohl((pkt)->header.itemNum) == (num)) \
        && (ntohl((pkt)->header.itemSize) == (size)) \
        && (ntohl((pkt)->header.dataSize) == (size) * (num)))
#ifdef LIB_DEBUG
#define ipcSizeCheck(pkt, size, num) ((!__ipcSizeCheck(pkt, size, num)) \
   ? (debugf("ipc", "ipcSizeCheck() failed, expected:%d * %d, got:%d * %d(%d)", size, num, ntohl((pkt)->header.itemSize), ntohl((pkt)->header.itemNum), ntohl((pkt)->header.dataSize)), __ipcSizeCheck(pkt, size, num)) \
   : __ipcSizeCheck(pkt, size, num))
#else
#define ipcSizeCheck(pkt, size, num) ((!__ipcSizeCheck(pkt, size, num)) \
   ? (printf("ipcSizeCheck() failed, expected:%d * %d, got:%d * %d(%d)", size, num, ntohl((pkt)->header.itemSize), ntohl((pkt)->header.itemNum), ntohl((pkt)->header.dataSize)), __ipcSizeCheck(pkt, size, num)) \
   : __ipcSizeCheck(pkt, size, num))
#endif

#define ipcStatus(pkt) (ntohl((pkt)->header.ackStatus))
#define ipcItemNum(pkt) (ntohl((pkt)->header.itemNum))
#define ipcItemSize(pkt) (ntohl((pkt)->header.itemSize))
#define ipcDataSize(pkt) (ntohl((pkt)->header.dataSize))

#define ipcOk(pkt) (ipcStatus(pkt) == IPC_STATUS_OK)

#endif /* __IPCTYPE_H__ */


