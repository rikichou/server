
/*
Copyright (c) 2014,  Shenzhen Hexicom Technologies Co., Ltd.
All rights reserved.

File         : systemHelper.c
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




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <time.h>

#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <fcntl.h>

#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <inttypes.h>

#include "debug.h"

#include "systemHelper.h"


#ifndef STANDALONE

#include "stringHelper.h"

#else
static char *trim(char *s)
{
	size_t len = strlen(s);
	size_t lws;

	/* trim trailing whitespace */
	while (len && isspace(s[len-1]))
    {
        --len;
    }   
	

	/* trim leading whitespace */
	if (len)
    {
		lws = strspn(s, " \n\r\t\v");
		if (lws) 
        {
			len -= lws;
			memmove(s, s + lws, len);
		}
	}
    
	s[len] = '\0';

    return s;
}


static char *trimRight(char *s)
{
	size_t len = strlen(s);

	/* trim trailing whitespace */
	while (len && isspace(s[len-1]))
    {
		--len;
    }   
    
	s[len] = '\0';
    
    return s;
}
#endif 


#ifndef STANDALONE 

int sendSignal(int pid, int sig)
{
    if (pid > 0)
    {
        return kill(pid, sig);
    }
    return 0;
}

int tryKill(int pid)
{
    return sendSignal(pid, SIGTERM);    
}

int getPidFromFile(const char *pidFile)
{
    FILE *fd;
    int pid = 0;
    char buffer[128];
    
    fd = fopen(pidFile, "r");
    if (fd != NULL)
    {
        if(fgets(buffer, sizeof(buffer) - 1, fd))
        {
            buffer[sizeof(buffer) - 1 ]= '\0';                        
            pid = strtoul(buffer, NULL, 0);
        }
        fclose(fd);
    }

    return pid;    
}


int fileExist(const char *file)
{
	FILE *fp ;
	if (file && file[0])
    {
		fp = fopen(file, "r");
		if (fp)
        {
			fclose(fp);
			return 1;
		}		
	}
	return 0;
}


uint32_t fileSize(const char *path)
{
	struct stat buff;
    
	if (stat(path, &buff) < 0)
    {
		return 0;
	}
    
	return buff.st_size;
}

int fileLineNum(const char *path)
{
    int count;
    
    FILE *fp = fopen(path, "r");

    if (fp == NULL)
    {
        return 0;
    }

    count = 0;
    
    while(!feof(fp)) 
    {
        if (fscanf(fp, "%*[^\n]\n") < 0) 
        {
            fclose(fp);        
            return 0;   
        }

        count ++;
    }
    
    fclose(fp);

    return count;                   
}


uint32_t fileRead(const char *path, uint32_t offset, uint8_t *buffer, uint32_t size)
{
	FILE* fp;
	size_t length, blockSize, readSize, leftSize;
	
	fp = fopen(path, "r");
	if (fp == NULL)
    {
		return 0;
	}

    if (fseek(fp, offset, SEEK_SET) < 0)
    {
        return 0;
    }    

    blockSize = 1024 * 4;

    leftSize = size;    
    while (leftSize > 0)
    {
        readSize = leftSize > blockSize ? blockSize : leftSize;        
        length = fread(&buffer[size - leftSize], sizeof(uint8_t), readSize, fp);

        leftSize -= length;
        
        if (length < readSize)
        {            
            break;
        }        
    }

	fclose(fp);	
	
	return size - leftSize;
}


uint32_t fileWrite(const char *path, const uint8_t *buffer, uint32_t size)
{    
    FILE * fp;
    size_t locWrite, numLeft, numWrite = 0;
    
    if ((fp = fopen(path, "w+b")) == NULL) 
    {
        return 0;
    } 

    locWrite = 0;
    numLeft = size;
    while (numLeft > 0) 
    {
        numWrite = fwrite(&buffer[locWrite], sizeof(*buffer), numLeft, fp);
        if (numWrite < numLeft) 
        {
            break;
        }
        locWrite += numWrite;
        numLeft -= numWrite;
    }

    fflush(fp);
    fclose(fp);

    return locWrite;
}

ssize_t safeWrite(int fd, const void *buf, size_t count)
{
	ssize_t n;

	do {
		n = write(fd, buf, count);
	} while (n < 0 && errno == EINTR);

	return n;
}

ssize_t fullWrite(int fd, const void *buf, size_t len)
{
	ssize_t cc;
	ssize_t total;

	total = 0;

	while (len) 
    {
		cc = safeWrite(fd, buf, len);

		if (cc < 0) 
        {
			if (total) 
            {
				/* we already wrote some! */
				/* user can do another write to know the error code */
				return total;
			}
			return cc;	/* write() returns -1 on failure. */
		}

		total += cc;
		buf = ((const char *)buf) + cc;
		len -= cc;
	}

	return total;
}


int writePidFile(const char *path)
{
	int fd, ret;
	char buf[sizeof(int)*3 + 2];
	struct stat sb;

	if (!path)
		return 0;
	/* we will overwrite stale pidfile */
	fd = open(path, O_WRONLY|O_CREAT|O_TRUNC, 0666);
    
	if (fd < 0)
		return -1;

	/* path can be "/dev/null"! Test for such cases */
	ret = (fstat(fd, &sb) == 0) && S_ISREG(sb.st_mode);

	if (ret) 
    {
		/* few bytes larger, but doesn't use stdio */
        sprintf(buf, "%d\n", getpid());
		fullWrite(fd, buf, strlen(buf));
	}
    
	close(fd);

    return ret;
}


#endif /* not STANDALONE */


static int compareString(const void *a, const void *b)
{
    char *x = *((char **)a);
    char *y = *((char **)b);

    return strcmp(x, y);
}


/* this is not standard function */
static char *_strrstr(const char *haystack, const char *needle)
{
    const char *p;
    int stackLen = strlen(haystack);
    int needleLen = strlen(needle);

    if (!needleLen || (needleLen > stackLen))
    {
        return NULL;
    }

    for (p = haystack + (stackLen - needleLen); p >= haystack; p --)
    {
        if ((*p == *needle) && !memcmp(p, needle, needleLen))
        {
            return (char *)p;
        }
    }
    return NULL;
}

int fileSort(const char *path)
{
    FILE *fp;
    char buffer[4094];
    int line_count = 0, l, ret = 0;

    struct line_list
    {
        char *string;
        struct line_list *next;
    };

    struct line_list *head = NULL, *line;
    char **plines = NULL;

    fp = fopen(path, "r+");
    if (fp == NULL)
    {
        return ret;
    }

    while(!feof(fp))
    {       
        buffer[0] = 0;
        if (fgets(buffer, sizeof(buffer) - 1, fp))
        {
            buffer[sizeof(buffer) - 1] = '\0';
            line = (struct line_list *) malloc(sizeof(struct line_list));
            if (line == NULL)
            {
                goto err_exit;
            }
            trimRight(buffer);
            line->string = strdup(buffer);
            line->next = head;
            head = line;
            line_count ++;
        }
    }

    fclose(fp);
    fp = NULL;

    if (line_count && head)
    {
        plines = (char **)malloc(sizeof(char *) * line_count);
        if (plines == NULL)
        {
            goto err_exit;
        }
        l = 0;
        line = head;
        while(line && l < line_count)
        {
            plines[l] = line->string;
            line = line->next;
            l ++;
        }
        line_count = l;
       
        qsort(plines, line_count, sizeof(char *), compareString);

        fp = fopen(path, "w");
        if (fp == NULL)
        {
            goto err_exit;
        }

        for (l = 0; l < line_count; l ++)
        {
            fprintf(fp, "%s\r\n", plines[l]);
        }

       fclose(fp);
	   ret = 1;
    }

err_exit:
    if (plines)
    {
        free(plines);
    }

    line = head;
    while(line)
    {
        if (line->string) 
        {
            free(line->string);
            line->string = NULL;
        }
        head = line->next;
        free(line);
        line = head;
    }

    return ret;
}


#ifndef STANDALONE 

/* Log Functions */
static int logWrite(FILE *out, char *logLine)
{
    fprintf(out, "%s", logLine);
    
    return 1;
}

#if 0
static int logWrite(FILE *out, char *logLine)
{
    static const char *logLevelString[] = 
    {
        "Emergency",    /* system is unusable */
        "Alert",        /* action must be taken immediately */
        "Critical",     /* critical conditions */
        "Error",        /* error conditions */
        "Warning",      /* warning conditions */
        "Notice",       /* normal but significant condition */
        "Informational",/* informational */
        "Debug"         /* debug-level messages */
    };

    static const char *logLevelBrief[] = 
    {
        "[EMERG]",        /* system is unusable */
        "[ALERT]",        /* action must be taken immediately */
        "[CRIT]",         /* critical conditions */
        "[ERROR]",        /* error conditions */
        "[WARNING]",      /* warning conditions */
        "[NOTICE]",       /* normal but significant condition */
        "[INFO]",         /* informational */
        "[DEBUG]",        /* debug-level messages */
        "[UNKNOWN]"       /* unknown */
    };

    int i;
    int level = sizeof(logLevelBrief)/sizeof(logLevelBrief[0]) - 1;

    const char *time = logLine;
    const char *message;
    const char *process;
    int pid = 0;
    char *p, *s;
    
    p = strchr(logLine, '[');

    if (p == NULL)
    {
        return 0;
    }
    *p ++ = '\0'; /* set end of time */

    s = p;
    
    p = strchr(p, ']'); 
    if (p == NULL)
    {
        return 0;
    }
    *p ++ = '\0';
	
	//printf("<level:%s>\n", s);

    /* get level */
    for (i = 0; i < sizeof(logLevelString)/sizeof(logLevelString[0]); i ++) 
    {
        if (!strcmp(s, logLevelString[i]))
        {
            level = i;
            break;
        }
    }

    /* get process name and pid */
    s = p;

    /* get message */ 
    p = strchr(p, ':'); 
    if (p == NULL)
    {
        return 0;
    }
    *p ++ = '\0';

    message = p;

    /* origin process and pid is pointed with s */
    /* trim the space */
    while (isspace(*s))
    {
        s ++;
    }

    process = s;
	//printf("<process:%s>\n", process);

	/* get the end of process */
	p = strchr(s, '[');
	if (p)
	{
		/* p ==> [1005] */
		/* p ==> [root-1005] */
		*p ++ = '\0';
	}
    
    /* if process name is too long , cut it */ 
	if (strlen(s) > 6)
	{
		s[5] = '~';
		s[6] = '\0';
	}
	
	/* get pid */
	if (p)
	{
		if ((s = strchr(p, '-')) != NULL)
		{
			pid = strtoul((s + 1), NULL, 10);
		}
		else 
		{
			//printf("<-pid:%s>\n", p);
			pid = strtoul(p, NULL, 10);
		}
	}
	
    //fprintf(out, "%s[%-7s] [%05d-%-6s]:%s", time, logLevelBrief[level], pid, process, message);
    
    fprintf(out, "%s%-9s:(%s)%s", time, logLevelBrief[level], process, message);
    
    return 1;
}
#endif

static int logDup(FILE *in, FILE *out)
{
    int count = 0;
    char buffer[4096];
    
    while(!feof(in))
    {
        if (fgets(buffer, sizeof(buffer), in))
        {
            count += logWrite(out, buffer);
        }
    }
    
    return count;
}

static int logDupReverse(FILE *in, FILE *out)
{
    int count = 0;
    char buffer[4096];

    if (fgets(buffer, sizeof(buffer), in))
    {
		count = logDupReverse(in, out);
        
        count += logWrite(out, buffer);
    }
    
    return count;
}


int logCopy(const char *inFile, const char *outFile, int reverse)
{
    FILE *fpIn, *fpOut;
    int count;

    fpIn = fopen(inFile, "r");
    if (fpIn == NULL)
    {
        return -1;
    }

    fpOut = fopen(outFile, "w+");
    if (fpOut == NULL)
    {
        return -1;
    }

    if (reverse)
    {
        count = logDupReverse(fpIn, fpOut);
    }
    else 
    {
        count = logDup(fpIn, fpOut);
    }    

    fclose(fpIn);
    fclose(fpOut);

    return count;
}


void getTmpFilePath(char *path, int size)
{
    snprintf(path, size, TMP_FILE_FORMAT, getpid(), (uint32_t)time(NULL));
    path[size - 1] = '\0';
}

/* interface utiltity */

int setNetifAddress(const char *ifname, etherAddr_t *hwaddr)
{
	int sock, ret;
	struct ifreq ifr;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
    {
        return -1;
    }

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name) - 1);
	memcpy(ifr.ifr_hwaddr.sa_data, hwaddr, sizeof(etherAddr_t));
    
	ret = ioctl(sock, SIOCSIFHWADDR, &ifr);

	close(sock);
    
	return (ret < 0) ? -1 : 0;
}



int getNetifHwAddr(const char *ifname, etherAddr_t *hwaddr)
{
	int sock, ret;
	struct ifreq ifr;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0) 
    {   
        return -1;
    }

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name) - 1);
    
	ret = ioctl(sock, SIOCGIFHWADDR, &ifr);

	if(!ret && hwaddr)
    {
        memcpy(hwaddr, ifr.ifr_hwaddr.sa_data, sizeof(etherAddr_t));
    }

	close(sock);
    
	return (ret < 0) ? -1 : 0;
}


int getNetifIpAddr(const char *ifname, struct in_addr *ipaddr)
{
	int sock, ret;
	struct ifreq ifr;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
    {
        return -1;
    }

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name) - 1);

    ret = ioctl(sock, SIOCGIFADDR, &ifr);

	if(!ret && ipaddr)
    {
        memcpy(ipaddr, &((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr, sizeof(*ipaddr));
    }

	close(sock);
    
	return (ret < 0) ? -1 : 0;
}

int getNetifNetMask(const char *ifname, struct in_addr *mask)
{
	int sock, ret;
	struct ifreq ifr;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
    {
        return -1;
    }

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name) - 1);

    ret = ioctl(sock, SIOCGIFNETMASK, &ifr);

	if(!ret && mask)
    {
        memcpy(mask, &((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr, sizeof(*mask));
    }

	close(sock);
    
	return (ret < 0) ? -1 : 0;
}


int getNetifIndex(const char *ifname, int *index)
{
	int sock, ret;
	struct ifreq ifr;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
    {
        return -1;
    }

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name) - 1);

    ret = ioctl(sock, SIOCGIFINDEX, &ifr);

	if(!ret && index)
    {
        *index = ifr.ifr_ifindex;
    }

	close(sock);
    
	return (ret < 0) ? -1 : 0;
}

int parseResolv(const char *path, struct in_addr *dns, int num)
{
    FILE* fp;
    char buffer[128];
    const char *prefix = "nameserver";
    int len, count;
    
    fp = fopen(path, "r");
    if (fp == NULL)
    {
        return -1;
    }        
    
    len = strlen(prefix);
    count = 0;
    
    while(!feof(fp) && dns && (count < num)) 
    {
        buffer[0] = 0;
        if (fgets(buffer, sizeof(buffer) - 1, fp))
        {
            buffer[sizeof(buffer) - 1 ]= '\0';         
            if (strstr(buffer, prefix) != NULL)
            {
                if (inet_aton(&buffer[len + 1], &dns[count]))
                {
                    count ++;
                }
            }
        } 
    }
    
    fclose(fp);

    return count;
}

int getOsDns(struct in_addr *dns, int num)
{
    return parseResolv(SYS_RESOLV_PATH, dns, num);
}


int getOsRouteNum(void)
{
    int count;

    count = fileLineNum("/proc/net/route");

    if (count > 0)
    {
        return (count - 1); /* remove the first line */
    }

    return 0;
}


int getOsRoute
    (
        struct route *route, 
        int num, 
        int defaultOnly,    /* if true, match default entries */
        const char *device, /* if not empty, match the device name */
        int flagMask        /* if > 0, match the set flags */
    )
{
    int ret = 0;
    int count = 0;

    char devname[64];
    unsigned long dest, gateway, mask;
    int flags, ref, use, metric, mtu, window, irtt;
    
    FILE *fp = fopen("/proc/net/route", "r");

    if (fp == NULL)
    {
        return -1;
    }

    if (fscanf(fp, "%*[^\n]\n") < 0) 
    { /* Skip the first line. */
        goto ERROR;		   /* Empty or missing line, or read error. */
    }    

    while(count < num)
    {
        ret = fscanf(fp, "%63s%lx%lx%X%d%d%d%lx%d%d%d\n",
                devname, &dest, &gateway, &flags, &ref, &use, &metric, &mask,
                &mtu, &window, &irtt);

        if (ret != 11)
        {
            if ((ret < 0) && feof(fp))
            {
                break;
            }
            goto ERROR;
        }
        /* always return the up entries */
        if (!(flags & RTF_UP))
        {
            continue;
        }
        
        if (defaultOnly && (dest != 0))
        {
            continue;
        }

        if (device && strcmp(device, devname))
        {
            continue;
        }

        if (flagMask && ((flagMask & flags) != flagMask))
        {
            continue;
        }
       
        strcpy(route[count].device, devname);
        route[count].destination.s_addr = dest;
        route[count].gateway.s_addr = gateway;
        route[count].netmask.s_addr = mask;
        route[count].flags = flags;
        route[count].ref = ref;
        route[count].use = use;
        route[count].metric = metric;
        route[count].mtu = mtu;
        route[count].window = window;
        route[count].irtt = irtt;

        count ++;
    }

    fclose(fp);

    return count;
    
ERROR:

    fclose(fp);
    
    return -1;                    
}

int setOsRoute(int op, struct in_addr *dest, struct in_addr *netmask, struct in_addr *gateway, int metric)
{
    int ret;
    char cmdBuffer[512];
    int isDefault, isHost;

    if ((op == ROUTE_DELETE) || (op == ROUTE_ADD))
    {
        char destAddr[64];
        char nmAddr[64];
        char gwAddr[64];

        char metricSet[32];
    
        isDefault = !(dest->s_addr) ? 1 : 0;
        isHost = (isDefault || (netmask->s_addr == 0xFFFFFFFFUL)) ? 1 : 0;

        safeStrncpy(destAddr, inet_ntoa(*dest), sizeof(destAddr));
        safeStrncpy(nmAddr, inet_ntoa(*netmask), sizeof(nmAddr));
        safeStrncpy(gwAddr, inet_ntoa(*gateway), sizeof(gwAddr));

        if ((op == ROUTE_ADD) && metric)
        {
            sprintf(metricSet, "metric %d", metric);
        }
        else 
        {
            metricSet[0] = '\0';
        }

        if (isHost)
        {
            /* 
            route (add|del)  (default|-host ADDR) gw GW
            */
            sprintf(cmdBuffer, "route %s %s %s gw %s %s", 
                (op == ROUTE_ADD) ? "add" : "del",
                isDefault ? "" : "-host",
                isDefault ? "default" : destAddr,
                gwAddr,
                metricSet
                );
        }
        else 
        {
            /* 
            route (add|del) -net ADDR netmask NM gw GW
            */
            sprintf(cmdBuffer, "route %s -net %s netmask %s gw %s %s", 
                (op == ROUTE_ADD) ? "add" : "del",
                destAddr,
                nmAddr,
                gwAddr,
                metricSet
                );
        }
    }
    else 
    {
        return -1;
    }

    ret = system(cmdBuffer);

    debug("route", "%s", cmdBuffer);
    debug("system", "call system(%s) return %d", cmdBuffer, ret);

    return ret;    
}

int flushOsRoute(void)
{
    struct route *route = NULL;    
    int i, num, ret;
    int count = 0;

    num = getOsRouteNum();

    if (num > 0)
    {
        route = (struct route *)malloc(sizeof(*route) * num);

        if (route == NULL)
        {
            return 0;
        }

        ret = getOsRoute(route, num, 0, NULL, 0);

        if (ret <= 0)
        {
            return ret;
        }
        
        for (i = 0; i < ret; i ++)
        {
            /* remove items which gw > 0 */
            if (route[i].gateway.s_addr)
            {
                setOsRoute(ROUTE_DELETE, &route[i].destination, &route[i].netmask, &route[i].gateway, 0);

                count ++;
            }
        }

		free_s(route);
    }    

    return count;
}

int osRouteFind(struct route *route, const char *device, struct in_addr *dest, struct in_addr *netmask, struct in_addr *gateway, int metric)
{
    int i;
    struct route *routeAll = NULL;
    int osRouteNum = getOsRouteNum();
    int findNum = -1;

    if (osRouteNum > 0)
    {
       routeAll = (struct route *)malloc(sizeof(*routeAll) * osRouteNum);

       if (NULL == routeAll)
       {
            debug("route", "in function %s() malloc %d failed", __FUNCTION__, sizeof(*routeAll) * osRouteNum);
            return 0;
       }

        findNum = getOsRoute(routeAll, osRouteNum, 0, device, 0); 

        for (i = 0; i < findNum; i ++)
        {
            if (((routeAll[i].destination.s_addr & routeAll[i].netmask.s_addr) == (dest->s_addr & netmask->s_addr))
                && (routeAll[i].gateway.s_addr == gateway->s_addr)
                && (routeAll[i].metric == metric))
            {
                if (route)
                {
                    memcpy(route, &routeAll[i], sizeof(struct route));  
                }
                 
                char buf[128];
                int len = 0;
                memset(buf, 0, sizeof(buf));

                len = sprintf(buf, "dest:%s ", inet_ntoa(*dest));
                sprintf(buf + len, "netmask:%s ", inet_ntoa(*netmask));
                
                debug("route", "find route %s gateway:%s ok", buf, inet_ntoa(*gateway));

                if (routeAll)
                {
                    free(routeAll);  
                }
                         
                return 1;
            }
        }
    }

    if (routeAll)
    {
        free(routeAll);  
    }

    return 0; 
}


int getOsDefaultGateway(struct in_addr *gateway)
{
    struct route route;

    if (gateway == NULL)
    {
        return 0;
    }

    if (getOsRoute(&route, 1, 1, NULL, 0) <= 0)
    {
        return -1;
    }

    /* in ppp connection, default gateway is 0 */
    if (route.gateway.s_addr == 0)
    {
        struct route ppp;
        if (getOsRoute(&ppp, 1, 0, route.device, RTF_HOST) <= 0)
        {
            /* not found, use the zero gateway */
            gateway->s_addr = route.gateway.s_addr;
        }
        else 
        {
            gateway->s_addr = ppp.destination.s_addr;
        }
    }
    else
    {
        gateway->s_addr = route.gateway.s_addr;
    }
    
    return 1;
}


int getOsArpNum(void)
{
    int count;

    count = fileLineNum("/proc/net/arp");

    if (count > 0)
    {
        return (count - 1); /* remove the first line */
    }

    return 0;
}

int getOsArp
    (
        arpItem_t *arp,
        int num,
        const char *device,
        int flagMask
    )
{
    int ret = 0;
    int count = 0;

    char dev[64];
    char ip[64];
    char hw[64];
    char mask[64];
    int type, flags;
    
    
    FILE *fp = fopen("/proc/net/arp", "r");

    if (fp == NULL)
    {
        return -1;
    }

    if (fscanf(fp, "%*[^\n]\n") < 0) 
    { /* Skip the first line. */
        goto ERROR;		   /* Empty or missing line, or read error. */
    }    

    while(count < num)
    {
        mask[0] = '-'; mask[1] = '\0';
        dev[0] = '-'; dev[1] = '\0';
        
        ret = fscanf(fp, "%s 0x%x 0x%x %s %s %s\n",
                ip, &type, &flags, hw, mask, dev);

        if (ret < 4)
        {
            if ((ret < 0) && feof(fp))
            {
                break;
            }
            goto ERROR;
        }
        
        if (device && strcmp(device, dev))
        {
            continue;
        }

        if (flagMask && ((flagMask & flags) != flagMask))
        {
            continue;
        }

        /* modify by zhourui:ignor the item which flags is zero */
        if (flags == 0)
        {
            continue;
        }

        inet_aton(ip, &arp[count].ipAddr);
        stringToEtherAddr(hw, &arp[count].hwAddr, 0);        
        
        arp[count].hwType = type;        
        arp[count].flags = flags;        
        
        strncpy(arp[count].device, dev, sizeof(arp[count].device));
        arp[count].device[sizeof(arp[count].device) - 1] = '\0';

        count ++;
    }

    fclose(fp);

    return count;
    
ERROR:

    fclose(fp);
    
    return -1;                    
}


int getOsVersion(char *version, int size)
{
    FILE *fd;
    char buf[64];
    int len = 0;
        
    fd = fopen("/proc/sys/kernel/ostype", "r");

    if ((fd != NULL) && fgets(buf, sizeof(buf) - 1, fd))
    {    
        buf[sizeof(buf) - 1 ]= '\0'; 
        trim(buf);
        if (len < size)
        {
            len += snprintf(version + len, size - len, "%s", buf);
        }        
    }

    if (fd) 
    {
        fclose(fd);
    }

    fd = fopen("/proc/sys/kernel/osrelease", "r");
    if ((fd != NULL) && fgets(buf, sizeof(buf) - 1, fd))
    {
        buf[sizeof(buf) - 1 ]= '\0'; 
        trim(buf);
        if (len < size)
        {
            len += snprintf(version + len, size - len, "%s", buf);
        }        
    }
    if (fd)
    {
        fclose(fd);
    }


    fd = fopen("/proc/sys/kernel/version", "r");
    if ((fd != NULL) && fgets(buf, sizeof(buf) - 1, fd))
    {
        buf[sizeof(buf) - 1 ]= '\0'; 
        trim(buf);
        if (len < size)
        {
            len += snprintf(version + len, size - len, "%s", buf);
        }
    }    
    if (fd) 
    {
        fclose(fd);
    }
    
    return len;
}


int getOsMemInfo(uint32_t *total, uint32_t *unused)
{
    FILE *fp;
    char val[256], *p;
    
    fp = fopen("/proc/meminfo", "r");
    if (fp == NULL)
    {
        return 0;
    }

    if (fgets(val, sizeof(val) - 1, fp) && total)
    {
        val[sizeof(val) - 1] = '\0';
        p = val;
        while(*p)
        {
            if (isdigit(*p))
            {
                *total = strtoul(p, NULL, 10);
                break;
            }
            p ++;
        }                
    }

    if (fgets(val, sizeof(val) - 1, fp))
    {
        val[sizeof(val) - 1] = '\0';    
        p = val;
        while(*p)
        {
            if (isdigit(*p))
            {
                *unused = strtoul(p, NULL, 0);
                break;
            }
            p ++;
        }    
    }
    
    fclose(fp);    

    return 1;
}


/*
/proc/stat

cpu  77 0 15347 822470 4 49 46 0

*/


static int readCpuJiffy(FILE *fp, jiffyCounts_t *jiffy)
{
#if 1
	static const char fmt[] = "cpu %llu %llu %llu %llu %llu %llu %llu %llu";
#else
	static const char fmt[] = "cp%*s %llu %llu %llu %llu %llu %llu %llu %llu";
#endif
	int ret;
    char line[1024];

    if (!fgets(line, sizeof(line), fp) || line[0] != 'c')
    {
        return 0;
    }

	ret = sscanf(line, fmt,
			&jiffy->usr, &jiffy->nic, &jiffy->sys, &jiffy->idle,
			&jiffy->iowait, &jiffy->irq, &jiffy->softirq,
			&jiffy->steal);
    
	if (ret >= 4) 
    {
		jiffy->total = jiffy->usr + jiffy->nic + jiffy->sys + jiffy->idle
			+ jiffy->iowait + jiffy->irq + jiffy->softirq + jiffy->steal;
		/* procps 2.x does not count iowait as busy time */
		jiffy->busy = jiffy->total - jiffy->idle - jiffy->iowait;
	}

	return ret;
}


/* return 10 * percentage */
int getOsCpuJiffy(int cpu, jiffyCounts_t *jiffy)
{
    FILE *fp;
    int ret;
         
    fp = fopen("/proc/stat", "r");
    if (fp == NULL)
    {
        return 0;
    }

    ret = readCpuJiffy(fp, jiffy);

    fclose(fp);

    return ret ? 1 : 0;
}


int getOsIfList(char (*ifName)[32], int num)
{
    FILE *fp;
    char val[256], *p;
    int count = 0, size = 0;
    char name[10];

    if(NULL == ifName)
    {
        return 0;
    }
    
    fp = fopen("/proc/net/dev", "r");
    if (fp == NULL)
    {
        return 0;
    }

    while (fgets(val, sizeof(val) - 1, fp))
    {
        if(count >= num)
        {
            break;
        }
        
        val[sizeof(val) - 1] = '\0';
        if((p = strchr(val, ':')))
        {            
            size = (void *)p - (void *)val;
            memcpy(name, val, size);
            name[size] = '\0';
            
            p = name;
            size = 0;
            while(*p == ' ' && *p != '\0')
            {
                p++;
                size ++;
            }
            
            memmove(name, name + size, sizeof(name)- size);

            memcpy(ifName + count, name, sizeof(name));
            
            count ++;
        }
    }
    
	return count;
}


int callSystem(const char *format, ...)
{
	va_list args;
    char buffer[1024];
    int len = 0, ret;
    int overFlow = 0;

    va_start (args, format);
    ret = vsnprintf (buffer, sizeof(buffer), format, args);
    va_end (args);
    
    /* overflow */
    if (ret >= sizeof(buffer))
    {
        overFlow = ret;
        len += sizeof(buffer) - 1; /* remove the end terminal */
    }
    else if (ret > 0)
    {
        len += ret;
    }  

    if (overFlow)
    {
        len = fprintf(stderr, "%s:<overflow,expected=%d>%s%s", __FUNCTION__, overFlow, buffer, "\n");            
    }

    ret = system(buffer);
    
    debug("system", "call system(%s) return %d", buffer, ret);

    return ret;  
}    

#endif /* not STANDALONE  */
/*
 X86:
 PID TTY      TIME CMD

 busybox:
 PID Uid VmSize Stat Command

*/
int getProcessPidGroup(const char *name, int pidGroup[], int pidNum)
{
    char buffer[512];
    FILE *fp; 
    int cmdOffset;
    int fullMatch;
    int nameLen;
    int count;
    int ret;
    char *p, *pCommand;
    const char *tryCmd1 = "ps -A 2> /dev/null";
    

    cmdOffset = 0;
    count = 0;    
    nameLen = strlen(name);
    
    if (strchr(name, '/'))
    {
        /* if name has '/', do not remove the folder name */
        fullMatch = 1;
    }
    else
    {
        fullMatch = 0;
    }

    ret = system(tryCmd1);
    
    fp = popen((ret = 0) ? "ps -A" : "ps", "r");

    if (fp == NULL)
    {
        printf("popen failed:%s(%d)\n", strerror(errno), errno);
        return -1;
    }

    while(!feof(fp) && (count < pidNum)) 
    {
        buffer[0] = 0;
        if (fgets(buffer, sizeof(buffer) - 1, fp))
        {
            buffer[sizeof(buffer) - 1] = '\0';
            
            trimRight(buffer);
            /* try to get the command offset */
            if (!cmdOffset)
            {
                p = _strrstr(buffer, "init");
                if (p != NULL)
                {
                    cmdOffset = p - buffer;
                }
            }
            else if (strlen(buffer) > cmdOffset)
            {
                pCommand = &buffer[cmdOffset];
            
                /* remove the string after the first space */
                p = strchr(pCommand, ' ');
                if (p)
                {
                    *p = '\0';
                }

                p = pCommand;
                
                if (!fullMatch)
                {
                    /* point to the name */
                    p = strrchr(pCommand, '/');
                    if (p == NULL)
                    {
                        /* not found, point to the first byte */
                        p = pCommand;
                    }
                    else 
                    {
                        p += 1;
                    }
                }
                /* compare it */
                if (!strncmp(p, name, nameLen))
                {
                    /* the first is the pid */
                    pidGroup[count] = strtoul(buffer, NULL, 10);
                    count ++;
                }
            }
        } 
    }    

    pclose(fp);
    
    return count;
}

int getProcessPid(const char *name)
{
    int pid;
    int ret;
    
    ret = getProcessPidGroup(name, &pid, 1);

    return (ret > 0) ? pid : ret; 
}

#ifndef STANDALONE 

void hexDump (const void * memory, int offset, int extent)
{
	unsigned char* origin = (unsigned char *)(memory);
	unsigned field = sizeof (extent) + sizeof (extent);
	unsigned block = 0x10;
	unsigned lower = block * (offset / block);
	unsigned upper = block + lower;
	unsigned index = 0;
	char buffer [80];
	char * output;

#define DIGITS_HEX 		"0123456789ABCDEF"

	fprintf(stderr, "\r\n");
	while (lower < extent)
	{
		output = buffer + field;
		for (index = lower; output-- > buffer; index >>= 4)
		{
			*output = DIGITS_HEX [index & 0x0F];
		}
		output = buffer + field;
		*output++ = ' ';
		for (index = lower; index < upper; index++)
		{
			if (index < offset)
			{
				*output++ = ' ';
				*output++ = ' ';
			}
			else if (index < extent)
			{
				*output++ = DIGITS_HEX [(origin [index] >> 4) & 0x0F];
				*output++ = DIGITS_HEX [(origin [index] >> 0) & 0x0F];
			}
			else
			{
				*output++ = ' ';
				*output++ = ' ';
			}
			*output++ = ' ';
		}
		for (index = lower; index < upper; index++)
		{
			if (index < offset)
			{
				*output++ = ' ';
			}
			else if (index < extent)
			{
				unsigned c = origin [index];
				*output++ = isprint (c)? c: '.';
			}
			else
			{
				*output++ = ' ';
			}
		}
		*output++ = '\n';
		*output++ = '\0';
		fprintf(stderr, "%s", buffer);
		lower += block;
		upper += block;
	}

#if 1
	output = buffer;
	*output++ = '\n';
	*output++ = '\0';
	fprintf(stderr, "%s\n", buffer);
#endif

	return;
}

/* byte order */
uint64_t __swap64(uint64_t llong)
{
	uint32_t low = (uint32_t) (llong & 0x00000000FFFFFFFFLL);
	uint32_t high = (uint32_t) ((llong & 0xFFFFFFFF00000000LL) >> 32);
	low = __swap32(low);
	high = __swap32(high);
	llong = high + (((uint64_t)low) << 32);
	return llong;
}

#endif /* not STANDALONE  */

/* uptime */
/* in ms */
uint64_t upTime(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    
    return ts.tv_sec * 1000 + ts.tv_nsec/(1000*1000);
}

uint32_t upSecond(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);    
    return ts.tv_sec;
}


uint32_t snmpUpTime(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);     
    return ts.tv_sec * 100UL + ts.tv_nsec / (1000 * 1000 * 10);    
}


time_t timeAt(uint32_t upSec)
{
    uint32_t now = time(NULL);
    uint32_t tick = upSecond();
    uint32_t diff;
        
    /* almost not happen ? */
    if (tick < upSec)
    {
        diff = (0xFFFFFFFFULL - upSec) + upSec;
    }
    else 
    {
        diff = tick - upSec;
    }

    return (now > diff) ? now - diff : 0;
}

const char *timeToString(time_t time, char *buffer, int size)
{
    struct tm tm;

    gmtime_r(&time, &tm); 
    
    if (buffer)
    {
        snprintf(buffer, size, "%04u.%02u.%02u-%02u:%02u:%02u", 
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec);
    }

    return buffer;
}


const char *upSecondToString(uint32_t seconds, char *buffer, int size)
{
    int day, hour, min, sec;
    
    day = seconds / (60 * 60 * 24);
    seconds = seconds % (60 * 60 * 24);
    hour = seconds / (60 * 60);
    seconds = seconds % (60 * 60);
    min = seconds / 60;
    sec = seconds % 60;
    
    if (buffer)
    {
        if (day > 0)
        {
            snprintf(buffer, size, "%d %s %d %s %d %s %d %s", day, (day > 1) ? "Days" : "Day", 
                hour, (hour > 1) ? "Hours" : "Hour",
                min, (min > 1) ? "Mins" : "Min",
                sec, (sec > 1) ? "Secs" : "Sec");
        }
        else if (hour > 0)
        {
            snprintf(buffer, size, "%d %s %d %s %d %s", hour, (hour > 1) ? "Hours" : "Hour",
                min, (min > 1) ? "Mins" : "Min",
                sec, (sec > 1) ? "Secs" : "Sec");
        }
        else if (min > 0)
        {
            snprintf(buffer, size, "%d %s %d %s", min, (min > 1) ? "Mins" : "Min",
                sec, (sec > 1) ? "Secs" : "Sec");            
        }
        else 
        {
            snprintf(buffer, size, "%d %s", sec, (sec > 1) ? "Secs" : "Sec");            

        }
    }

    return buffer;
}


#ifdef STANDALONE 

#define TOOLS_NAME "utils"

/*
util :
Usage :
    pidOf PROCESS -- print process's pids
    fileSort FILE -- sort file lines in order
    upTime        -- print system Tick in millisecond
        
*/
const static char * s_usage = 
    "Built in :\n"
    "    pidof PROCESS -- print process's pids\n"
    "    filesort FILE -- sort file lines in order\n"
    "    uptime        -- print system Tick in millisecond\n"   
    ;

int main(int argc, char *argv[])
{
    const char *appName = NULL;

    if (argc > 0)
    {
        appName = strrchr(argv[0], '/');
        if (appName == NULL)
        {
            appName = argv[0];
        }
        else 
        {
            appName += 1;
        }

        argc --;
        argv ++;

        if (!strcmp(appName, TOOLS_NAME))
        {
            appName = NULL;
            
            /* get app Name again */
            if (argc > 0)
            {
                appName = strrchr(argv[0], '/');
                if (appName == NULL)
                {
                    appName = argv[0];
                }            
                else
                {
                    appName += 1;
                }    
                argc --;
                argv ++;                
            }
        }        
    }

    if (appName)
    {
        if (!strcmp(appName, "pidof"))
        {
            if (argc < 1)
            {
                printf("Uasge:\n  pidof PROCESS\n");
                return 1;
            }
            else 
            {
                int i;
                int pid[128]; 
                int count = getProcessPidGroup(argv[0], pid, sizeof(pid)/sizeof(pid[0]));
                for (i = 0; i < count; i ++)
                {
                    printf("%d%s", pid[i], (i == count - 1) ? "" : " ");
                }
                return 0;
            }
        }
        else if (!strcmp(appName, "uptime"))
        {
            if ((argc > 0) && !strcmp(argv[0], "-s"))
            {
                printf("%u", upSecond());
            }
            else 
            {
                printf("%llu", upTime());
            }
            
            return 0;
        }
        else if (!strcmp(appName, "filesort"))
        {
            if (argc < 1)
            {
                printf("Uasge:\n  filesort FILE\n");
                return 1;
            }
            else
            {
                if (fileSort(argv[0]))
                {
                    printf("File '%s' has been successfully sorted\n", argv[0]);
                    return 0;
                }
                else 
                {
                    printf("***Sorting failed\n");        
                    return 1;
                }
            }
        }  
        else 
        {
            printf("***Unknown Applet Name:%s\n", appName);        
            return 1;
        }
    }
    else 
    {
        printf("%s", s_usage);
    }

    return 0;
}

#endif /* STANDALONE */

