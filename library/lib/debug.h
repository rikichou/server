
/*
Copyright (c) 2014,  Shenzhen Hexicom Technologies Co., Ltd.
All rights reserved.

File         : debug.h
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




#ifndef __DEBUG_H__ 
#define __DEBUG_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

/* size define */

#define DEBUG_BUFFER_SIZE   4096
#define SYSLOG_BUFFER_SIZE  4096

#define FORCE_CHECK_DEBUG   0 
#define ENTRYLOG_TO_DEBUG   1
#define FORCE_WARNING_DEBUG 0

/* debug */
/* parsing the debug Ids */
int debugStringIndexOf(const char *str, char tok, const char **next);

int debugTest(const char *id);
void debugPrintAll(int enable);
void debugInjectId(const char *id);
void debugFreeId(const char *id);
void debugFreeAllIds(void);
void debugDumpIds(void);
void debugInjectIdString(const char *idString);
void debugTestAndLoadDynamicIds(void);

int localDebug(int force, const char *id, const char *format, ...);

#define _debug(id, format, ...) \
    localDebug(0, id, "[%s,%s,%d], "format, __FILE__, __FUNCTION__,__LINE__, ##__VA_ARGS__)

#define _check(id, format, ...) \
    localDebug(FORCE_CHECK_DEBUG, id, format, ##__VA_ARGS__)

/* with line info */
#define _debugf(id, format, ...) \
    localDebug(0, id, format " @[%s,%s,%d]", ##__VA_ARGS__, __FILE__, __FUNCTION__, __LINE__ )

#define _warning(id, format, ...) \
    localDebug(FORCE_WARNING_DEBUG, id, format " @[%s,%s,%d]", ##__VA_ARGS__, __FILE__, __FUNCTION__, __LINE__ )





/* assert */    
#define _assert(ok, fmt, ...) do \
    {  \
        if (!(ok)){ \
            fprintf(stderr, "***ASSERT***[%s,%s,%d]: " fmt, __FILE__, __FUNCTION__, __LINE__ , ##__VA_ARGS__); \
            fprintf(stderr, "%s", "\n"); \
            exit(1); \
        } \
    } while(0)


/* debug */
#undef debug        
#undef debugf        
#undef check        
#undef assert
        
// #ifdef LIB_DEBUG
#if 1
#define debug(id, format, ...)      _debug(id, format, ##__VA_ARGS__)
#define debugf(id, format, ...)     _debugf(id, format, ##__VA_ARGS__)
#define check(id, format, ...)      _check(id, format, ##__VA_ARGS__)
#define assert(ok, fmt, ...)        _assert(ok, fmt, ##__VA_ARGS__) 
#define warning(id, format, ...)    _warning(id, format, ##__VA_ARGS__)
        
#else
        
#define debug(id, format, ...)
#define debugf(id, format, ...)
#define check(id, format, ...)
#define assert(ok, fmt, ...) 
#define warning(id, format, ...)
        
#endif 

/* syslog */        

#include <syslog.h>

void openLog(const char *name);
void closeLog(void);

#define sysLog(level, format, ...) syslog(level, format, ##__VA_ARGS__)

#define SYSLOG_TO_DEBUG  0
#define __entryLog(id, level, format, ...) \
 do  \
 { \
    int _var =  SYSLOG_TO_DEBUG; \
    if (_var) \
    { \
        debug(id, format, ##__VA_ARGS__); \
    } \
    else \
    { \
        syslog(level, format, ##__VA_ARGS__); \
    } \
 }while(0) 
#if ENTRYLOG_TO_DEBUG
#define entryLog(_id, _level, _format, _args...) __entryLog(_id, _level, _format, ##_args)
#else
#define entryLog(_id, _level, _format, _args...)  sysLog(_level, _format, ##_args)
#endif 
#if 0
void setLogLevel(int level);
void localLog(int level, const char *format, ...);

extern int g_logLevel;

#define sysLog(level, format, ...) \
do { \
    if (level <= g_logLevel) \
    { \
        localLog(level, format, ##__VA_ARGS__); \
        syslog(level, format, ##__VA_ARGS__); \
    } \
}while(0)

#endif 

/* this is only for source insight show reference */    
#ifdef do_not_define_this_macro
#define LOG_EMERG 0
#define LOG_ALERT 1
#define LOG_CRIT  2
#define LOG_ERR   3
#define LOG_WARNING  4
#define LOG_NOTICE   5
#define LOG_INFO     6
#define LOG_DEBUG    7
#endif 


#endif /* #ifndef __DEBUG_H__*/

