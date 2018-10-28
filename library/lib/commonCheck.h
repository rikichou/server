
/*
Copyright (c) 2014,  Shenzhen Hexicom Technologies Co., Ltd.
All rights reserved.

File         : commonCheck.h
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




#ifndef __COMMONCHECK_H__
#define __COMMONCHECK_H__

#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
 
#define zeroIp(ip) ((ip)->s_addr == 0)

#define netMaskFromBits(_nm, _bits) do \
    { \
        (_nm)->s_addr = htonl(~((1 << (32 - _bits)) - 1)); \
    }while(0)

int netMaskToBits(struct in_addr *nm);
int validHostIp(struct in_addr *ip);
int validNetMask(struct in_addr *nm);
int validNetwork(struct in_addr *ip);
int validHostIpAndMask(struct in_addr *ip, struct in_addr *nm);
int validHostIpGroup(struct in_addr *ip, struct in_addr *nm, struct in_addr *gw);
int validMulticastIpAddr(struct in_addr *ip);
int validUrl(const char *url);

#endif /* __COMMONCHECK_H__ */

