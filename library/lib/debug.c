
/*
Copyright (c) 2014,  Shenzhen Hexicom Technologies Co., Ltd.
All rights reserved.

File         : debug.c
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



#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>

#include "debug.h"


struct debugId
{
    char *id;
    struct debugId * next;
};

static struct debugId *s_debugIdHead = NULL;
static int s_debugAll = 0;

static uint64_t debugUpTime(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)(ts.tv_sec * 1000 + ts.tv_nsec/(1000*1000));
}

/*
    return 
    -1 - don't have any valid string
*/
int debugStringIndexOf(const char *str, char tok, const char **next)
{
    const char *p;
    if ((str == NULL) || (*str == '\0'))
    {
        return -1;
    }
    
    p = str;
    while(*p && (*p != tok))
    {
        p ++;
    }

    if (next)
    {
        *next = *p ? p + 1 : p;
    }
    
    return p - str;
}

int debugTest(const char *id)
{
    struct debugId *p;
    /* empty means accept all */
    if (s_debugAll)
    {
        return 1;
    }

    p = s_debugIdHead;
    while(p)
    {
        /* it may a little slow, but only for debug purpose */
        if (!strcmp(id, p->id))
        {
            return 1;
        }
        p = p->next;
    }
    return 0;    
}

void debugPrintAll(int enable)
{
    s_debugAll = enable;
}

void debugInjectId(const char *id)
{
    struct debugId *p;
    if (id == NULL) return;
    
    p = (struct debugId *)malloc(sizeof(struct debugId));
    if (p)
    {
        p->id = strdup(id);
        p->next = s_debugIdHead;
        s_debugIdHead = p;
    }
}

void debugFreeId(const char *id)
{
    struct debugId *p = s_debugIdHead, *prev = NULL;

    while(p)
    {
        if (p->id && !strcmp(p->id, id))
        {
            struct debugId *next;
            next = p->next;
            
            if (prev == NULL)
            {
                s_debugIdHead = p->next;
            }
            else 
            {
                prev->next = p->next;
            }
            
            free(p->id);
            free(p);
            
            p = next;
        }
        else 
        {
            p = p->next;
        }
    }
}


void debugFreeAllIds(void)
{
    struct debugId *p = s_debugIdHead, *q;

    while(p)
    {
        if (p->id)
        {
            free(p->id);
        }
        q = p;
        p = p->next;
        free(q);
    }
    s_debugIdHead = NULL;
}

void debugDumpIds(void)
{
    struct debugId *p = s_debugIdHead;

    while(p)
    {
        if (p->id)
        {
            printf("%s ", p->id);
        }        
        p = p->next;
    }
}

/* string likes wan,lan,fork */
void debugInjectIdString(const char *idString)
{
    int ret; 
    const char *p = NULL, *next;
    
    p = idString;
    
    while((ret = debugStringIndexOf(p, ',', &next)) != -1)
    {
        if (ret > 0)
        {
            char *n = strndup(p, ret);
            if (n)
            {
                debugInjectId(n);    
                free(n);            
            }
        }
        p = next;
    }
}


void debugTestAndLoadDynamicIds(void)
{
    char path[128];
	struct stat fileStat;
    static time_t lastModifiedTime = 0;
    FILE *fp;
    char line[1024];
    
    sprintf(path, "/tmp/debug.%d", getpid());

    /* get file stat */
	if (stat(path, &fileStat) < 0)
    {
        /* file remove or not exist */
		return;
	}

    /* file exist, check if changed */
    
    if (lastModifiedTime != fileStat.st_mtime)
    {   
        lastModifiedTime = fileStat.st_mtime;

        fp = fopen(path, "r");

        if (fp == NULL)
        {
            /* not exist */
            return;
        }

        /* free all */
        debugFreeAllIds();

        while(!feof(fp)) 
        {
            line[0] = '\0';
            if (fgets(line, sizeof(line), fp))                
            {
                /* trim right */
                size_t len = strlen(line);
                while(len && isspace(line[len - 1]))
                {
                    --len;
                }
                line[len] = '\0';
                
                sysLog(LOG_INFO, "Load Debug Ids:%s", line);
                debugInjectIdString(line);
            } 
        }

        fclose(fp);
    }
}

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void writeDebugToFile(const char *debug_file_path, char *buffer, int len)
{
	/* save to file */
	int fd = open(debug_file_path, O_RDWR|O_CREAT|O_APPEND, 0777);

	if (fd < 0)
	{
		debug("device", "Failed to open debug file (%d)%s", fd, debug_file_path);
		return;
	}

	int ret = write(fd, buffer, len);

	if (ret < 0)
	{
		close(fd);
		debug("device", "Failed to write message to debug file (%d)%s", ret, debug_file_path);
		return;
	}
	
	close(fd);

}

int localDebug(int force, const char *id, const char *format, ...)
{
	va_list args;
    uint64_t upTimeMs;
    char buffer[DEBUG_BUFFER_SIZE];
    int len = 0, ret;
    int overFlow = 0;

	memset(buffer, 0, sizeof(buffer));
	
    if (force || debugTest(id))
    {
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
        
        upTimeMs = debugUpTime();
        fprintf(stderr, "[%08d.%03d](%s): ", (int)(upTimeMs / 1000), (int)(upTimeMs % 1000), id);

        if (overFlow)
        {
            len = fprintf(stderr, "<overflow,expected=%d>%s%s", overFlow, buffer, "\n");            
        }
        else 
        {
            len = fprintf(stderr, "%s%s", buffer, "\n");    

			// just for debug
			writeDebugToFile("log/debug.file", buffer, len);
        }
    }
    
    return len;
}

/* syslog */

#if 0
int g_logLevel = LOG_DEBUG;

static const char *s_logLevelName[] = {
    "EMERG",	/* system is unusable */
    "ALERT",	/* action must be taken immediately */
    "CRIT",		/* critical conditions */
    "ERR",		/* error conditions */
    "WARNING",	/* warning conditions */
    "NOTICE",	/* normal but significant condition */
    "INFO",		/* informational */
    "DEBUG"	/* debug-level messages */
};


void setLogLevel(int level)
{
    if ((level >= 0) && (level < sizeof(s_logLevelName)/sizeof(s_logLevelName[0])))
    {
        g_logLevel = level;
    }
}


void localLog(int level, const char *format, ...)
{
	va_list args;
	int len = 0;
	char buffer[SYSLOG_BUFFER_SIZE]; 

	len = sprintf(buffer, "[%s]: ", s_logLevelName[level]);
	va_start (args, format);
	len = vsnprintf (buffer + len, sizeof(buffer) - len - 1, format, args);
 	va_end (args);
    
	fprintf(stdout, "%s\n", buffer);
}
#endif 

void openLog(const char *name)
{
    openlog(name, LOG_PID | LOG_NDELAY, 0);        
}

void closeLog(void)
{
    closelog();
}

