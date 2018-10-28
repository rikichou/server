
/*
Copyright (c) 2014,  Shenzhen Hexicom Technologies Co., Ltd.
All rights reserved.

File         : systemHelper.h
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





#ifndef __SYSTEMHELPER_H__
#define __SYSTEMHELPER_H__

#include <stdint.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <signal.h>

#include "etherAddr.h"

#ifndef SYS_RESOLV_PATH
#define SYS_RESOLV_PATH "/etc/resolv.conf"
#endif 

#ifndef DNSMASQ_RESOLV_PATH
#define DNSMASQ_RESOLV_PATH "/tmp/resolv.dnsmasq.conf"
#endif 

/* process */
int sendSignal(int pid, int sig);
int tryKill(int pid);
int getPidFromFile(const char *pidFile);


/* file */
#define TMP_FILE_FORMAT  "/tmp/%d_%08X.tmp"

int fileExist(const char *file);
uint32_t fileSize(const char *path);
int fileLineNum(const char *path);

uint32_t fileRead(const char *path, uint32_t offset, uint8_t *buffer, uint32_t size);
uint32_t fileWrite(const char *path, const uint8_t *buffer, uint32_t size);


ssize_t safeWrite(int fd, const void *buf, size_t count);
ssize_t fullWrite(int fd, const void *buf, size_t len);
int writePidFile(const char *path);    

int fileSort(const char *path);

int logCopy(const char *inFile, const char *outFile, int reverse);

void getTmpFilePath(char *path, int size);


/* interface utiltity */
int setNetifAddress(const char *ifname, etherAddr_t *hwaddr);
int getNetifHwAddr(const char *ifname, etherAddr_t *hwaddr);
int getNetifIpAddr(const char *ifname, struct in_addr *ipaddr);
int getNetifNetMask(const char *ifname, struct in_addr *mask);
int getNetifIndex(const char *ifname, int *index);

int callSystem(const char *format, ...);

int parseResolv(const char *path, struct in_addr *dns, int num);
int getOsDns(struct in_addr *dns, int num);

#ifndef RTF_UP
/* Keep this in sync with /usr/src/linux/include/linux/route.h */
#define RTF_UP          0x0001	/* route usable                 */
#define RTF_GATEWAY     0x0002	/* destination is a gateway     */
#define RTF_HOST        0x0004	/* host entry (net otherwise)   */
#define RTF_REINSTATE   0x0008	/* reinstate route after tmout  */
#define RTF_DYNAMIC     0x0010	/* created dyn. (by redirect)   */
#define RTF_MODIFIED    0x0020	/* modified dyn. (by redirect)  */
#define RTF_MTU         0x0040	/* specific MTU for this route  */
#ifndef RTF_MSS
#define RTF_MSS         RTF_MTU	/* Compatibility :-(            */
#endif
#define RTF_WINDOW      0x0080	/* per route window clamping    */
#define RTF_IRTT        0x0100	/* Initial round trip time      */
#define RTF_REJECT      0x0200	/* Reject route                 */
#endif


struct route
{
    char device[64];
    struct in_addr destination;
    struct in_addr gateway;
    struct in_addr netmask;
    uint32_t mtu;
    uint32_t window;
    uint32_t flags;
    uint32_t ref;
    uint32_t use;
    uint32_t metric;
    uint32_t irtt;
};


int getOsRouteNum(void);

int getOsRoute
    (
        struct route *route, 
        int num, 
        int defaultOnly,    /* if true, match default entries */
        const char *device, /* if not empty, match the device name */
        int flagMask        /* if > 0, match the set flags */
    );


enum
{
    ROUTE_ADD = 0,
    ROUTE_DELETE,    
};

int setOsRoute
    (
        int op, 
        struct in_addr *dest, 
        struct in_addr *netmask, 
        struct in_addr *gateway, 
        int metric
    );

int flushOsRoute(void);

int osRouteFind(struct route *route, const char *device, struct in_addr *dest, struct in_addr *netmask, struct in_addr *gateway, int metric);

int getOsDefaultGateway(struct in_addr *gateway);


typedef struct
{
    struct in_addr ipAddr;
    etherAddr_t hwAddr;
    int hwType;
    unsigned long flags;
    char device[64];
}arpItem_t;

int getOsArpNum(void);

int getOsArp
    (
        arpItem_t *arp,
        int num,
        const char *device,
        int flagMask
    );

int getOsVersion(char *version, int size);
int getOsMemInfo(uint32_t *total, uint32_t *unused);


typedef struct 
{
    /* Linux 2.4.x has only first four */
    unsigned long long usr, nic, sys, idle;
    unsigned long long iowait, irq, softirq, steal;
    unsigned long long total;
    unsigned long long busy;
}jiffyCounts_t;

int getOsCpuJiffy(int cpu, jiffyCounts_t *jiffy);


int getOsIfList(char (*ifName)[32], int num);

int getProcessPidGroup(const char *name, int pidGroup[], int pidNum);
int getProcessPid(const char *name);


void hexDump (const void * memory, int offset, int extent);

/* uptime */
/* in ms */
uint64_t upTime(void);
/* in second */
uint32_t upSecond(void);

uint32_t snmpUpTime(void);

time_t timeAt(uint32_t upSec);

const char *timeToString(time_t time, char *buffer, int size);
const char *upSecondToString(uint32_t seconds, char *buffer, int size);

/* a > b */
#define upTimeAfter(a, b) ((int64_t)(b) - (int64_t)(a) < 0)
/* a > b */
#define upSecondAfter(a, b) ((int32_t)(b) - (int32_t)(a) < 0)


/* byte order */
#define __swap16(s) ((((s) & 0xff) << 8) | (((s) >> 8) & 0xff))
#define __swap32(l) (((l) >> 24) | \
  (((l) & 0x00ff0000) >> 8)  | \
  (((l) & 0x0000ff00) << 8)  | \
  ((l) << 24))

uint64_t __swap64(uint64_t llong);

#if __BYTE_ORDER==__LITTLE_ENDIAN
    /* little endian */
	#define ntohl64(llong) __swap64(llong)
	#define htonl64(llong) __swap64(llong)

	#define letohl64(llong) (llong)
	#define letohl(l)  (l)
	#define letohs(s)  (s)

#ifndef htole64
	#define htole64(llong) (llong)
#endif 

	#define htolel(l) (l)
	#define htoles(s) (s)
    
#else
    /* big endian */
	#define ntohl64(llong) (llong)
	#define htonl64(llong) (llong)

	#define letohl64(llong) __swap64(llong)
	#define letohl(l)  __swap32(l)
	#define letohs(s)  __swap16(s)

#ifndef htole64
	#define htole64(llong) __swap64(llong)
#endif 

	#define htolel(l) __swap32(l)
	#define htoles(s) __swap16(s)
    
#endif /*  */

#ifndef __packed 
#define __packed __attribute__((__packed__))
#endif 

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif /* */

#ifndef inRange
#define inRange(v, min, max)  (((v) >= (min)) && ((v) <= (max)))
#endif 

#endif /* __SYSTEMHELPER_H__ */
