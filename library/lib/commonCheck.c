
/*
Copyright (c) 2014,  Shenzhen Hexicom Technologies Co., Ltd.
All rights reserved.

File         : commonCheck.c
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



/*
 * commonCheck.c
 * 
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "commonCheck.h"
#include "debug.h"

/* valid mask return 1 - 32 */
int netMaskToBits(struct in_addr *nm)
{
    int i, bits = 0;
    int phase1 = 1;
    unsigned long addr;

    if (nm == NULL)
    {
        return 0;
    }
    
    addr = ntohl(nm->s_addr);

    for (i = 0; i < 32; i ++)
    {
        if (phase1 && !(addr & (1 << (31 - i))))
        {
            if (i == 0)
            { // first bit can not be 0
                return 0;
            }                
            bits = i;
            phase1 = 0;
        }
        else if (!phase1 && (addr & (1 << (31 - i))))
        {
            return 0;
        }    
    }
    
    if (phase1)
    {
        bits = 32;
    }

    return bits;
}

int validHostIp(struct in_addr *ip)
{
    int fa;
    fa = ip ? (ntohl(ip->s_addr) >> 24) & 0xff : 0;

    if ((fa > 223) || (fa == 0) || (fa == 127))
    {
        return 0;
    }

    return 1;
}

int validNetMask(struct in_addr *nm)
{
    return netMaskToBits(nm) ? 1 : 0;
}

int validHostIpAndMask(struct in_addr *ip, struct in_addr *nm)
{
    if (!validHostIp(ip))
    {
        check("ip", "Invalid Host IP Address(%s)", inet_ntoa(*ip));
        return 0;
    }

    if (!validNetMask(nm))
    {
        check("ip", "Invalid Subnet Mask(%s)", inet_ntoa(*nm));    
        return 0;
    }

    /* Host IP should not be Net Address or broadcast address */
    if ((ip->s_addr & nm->s_addr) == ip->s_addr)
    {
        check("ip", "IP Address(%s) is a network address", inet_ntoa(*ip));        
        return 0;
    }

    if ((ip->s_addr | ~nm->s_addr) == ip->s_addr)
    {
        check("ip", "IP Address(%s) is a broadcast address", inet_ntoa(*ip));        
        return 0;
    }
    /*
    if ((ntohl(ip->s_addr) & ntohl(nm->s_addr)) == ntohl(ip->s_addr))
    {
        return 0;
    }

    if ((ntohl(ip->s_addr) | (~ntohl(nm->s_addr))) == ntohl(ip->s_addr))
    {
        return 0;
    }
    */

    return 1;
}


int validHostIpGroup(struct in_addr *ip, struct in_addr *nm, struct in_addr *gw)
{
    if (!validHostIpAndMask(ip, nm))
    {
        return 0;
    }

    if (!validHostIpAndMask(gw, nm))
    {
        return 0;
    }

    if ((ip->s_addr & nm->s_addr) != (gw->s_addr & nm->s_addr))
    {
        check("ip", "IP Address and Gateway Address are not in same network");            
        return 0;
    }  

    return 1;
}


int validMulticastIpAddr(struct in_addr *ip)
{
    uint32_t addr = ntohl(ip->s_addr);    
    return ((addr <= 0xEFFFFFFF) && (addr > 0xE0000000)) ? 1 : 0;
}
/* ://_.% 0-9 A-Z a-z & =   */
int validUrl(const char *url)
{
    int i;
    
    if ((url == NULL) || (url[0] == '\0'))
    {
        check("url", "empty url");    
        return 0;
    }

    for (i = 0; url[i] != '\0'; i ++)
    {
        if (!(
            ((url[i] >= '0') && (url[i] <= '9'))
            || ((url[i] >= 'Z') && (url[i] <= 'Z'))
            || ((url[i] >= 'a') && (url[i] <= 'z'))
            || (url[i] == '_')            
            || (url[i] == ':')
            || (url[i] == '/')
            || (url[i] == '.')            
            || (url[i] == '%')
            || (url[i] == '&')
            || (url[i] == '+')
            || (url[i] == '?')
            || (url[i] == '=')            
        ))
        {
            check("url", "invalid char(%02X) in url", url[i]);
            return 0;
        }   
    }

    return 1;    

}

