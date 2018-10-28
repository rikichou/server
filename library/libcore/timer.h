
/*
Copyright (c) 2014,  Shenzhen Hexicom Technologies Co., Ltd.
All rights reserved.

File         : timer.h
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
 * timer header
 *
 * Copyright (C)  All rights reserved.
 * 
 * File: timer.h
 * Date: 2014-2-10
 * 
 */


#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdint.h>
#include <time.h>
#include <sys/time.h>

#define	TIMER_MAX_NUM	64

#define	TIMER_CONTINUE	0
#define	TIMER_REMOVE	1

#define REASON_DEFAULT 0
#define REASON_FIRST  10

typedef void *timerHandle_t;
typedef int (*timerFunction_t)(void *data, int reason);
typedef void (*timerOnExit_t)(void *data);

/*
    return values:
        NULL - failed to add a timer
        other values - the handle of the timer
*/


timerHandle_t timerAdd(
    const char *name, /* Timer name */
    unsigned long period_ms, /* common period in millisecond */
    unsigned long delay_ms, /* if set to 0, the timer function will call at once, other wait delay ms to call the first time */
    timerFunction_t function, 
    timerOnExit_t on_exit, 
    void *data
    );

#define delayDo(_s, _f, _d) timerAdd("delayDo", (_s) * 1000, (_s) * 1000, _f, NULL, _d)

#define delayDo2(_period, _delay, _fun, _data) timerAdd("delayDo", (_period) * 1000, (_delay) * 1000, _fun, NULL, _data)
/*
    return value:
    1 - the timer is running
    0 - the timer is not in queue.
*/
int timerTest(timerHandle_t handle);
void timerRemove(timerHandle_t handle);
void timerReactivate(timerHandle_t handle, int msec, int reason);
int timerUpdatePeriod(timerHandle_t handle, unsigned long ms);
int timerSchedule(void);
void timerPolling(void *data);
void timerFreeAll(void);



#endif /* __TIMER_H__ */

