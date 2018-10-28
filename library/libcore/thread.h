/*
 * thread header
 * 
 * File: thread.h
 * Date: 2018-9-8 17:32:33
 * 
 */


#ifndef __THREAD_H__
#define __THREAD_H__


typedef void * threadHandle_t;

threadHandle_t threadAddListeningFile(const char *name, int fd, void *data, void (*function)(void *));
void threadRemoveListeningFile(threadHandle_t handle);
threadHandle_t threadAddPollingFunction(const char *name, void *data, void (*function)(void *));
void threadRemovePollingFunction(threadHandle_t handle);
void threadSchedule(int slot);
void threadFreeAll(void);
void threadExit(void);


#endif /* __THREAD_H__ */

