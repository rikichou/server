
/*
 * thread function
 * File: thread.c
 * Date: 2018/9/8/17:32:01
 * 
 */
#include "thread.h"

#include <debug.h>

enum
{
    THREAD_ACTIVE = 0,
    THREAD_SUSPEND = 1,
    THREAD_REMOVE = 2,
};


struct thread
{
    const char *name;
    int fd;
    int status;
    void *data;
    void (*function)(void *);
    struct thread *next;
};
 
 
static struct thread *s_threadListeningHead = NULL;
static struct thread *s_threadPollingHead = NULL;
static int s_threadExit = 0;

threadHandle_t threadAddListeningFile(const char *name, int fd, void *data, void (*function)(void *))
{
    struct thread *p;
    
    p = (struct thread *)malloc(sizeof(struct thread));
    if (p == NULL)
    {
        debugf("malloc", "malloc(%d)", sizeof(struct thread));
        return NULL;
    }

    p->name = name;
    p->fd = fd;
    p->data = data;
    p->status = THREAD_ACTIVE;
    p->function = function;

    p->next = s_threadListeningHead;
    s_threadListeningHead = p;
    
    debug("thread", "Listening thread(%s,fd=%d) registered", p->name, p->fd);

    return p;
}

void threadFreeListeningFile(struct thread *thread)
{
    struct thread *p, *prev = NULL;
    p = s_threadListeningHead;

    while(p)
    {
        if (p == thread)
        {
            if (prev)
            {
                prev->next = p->next;
            }
            else
            {
                s_threadListeningHead = p->next;
            }

            free(p);

            debug("thread", "Remove Listening thread(%s,fd=%d)", p->name, p->fd); 

            return ;
        }
        else
        {
            prev = p;
            p = p->next;
        }
    }
}

void threadRemoveListeningFile(threadHandle_t handle)
{
    struct thread *p, *prev = NULL;
    p = s_threadListeningHead;
    while(p)
    {        
        if (p == handle)
        {
            p->status = THREAD_REMOVE;
            return;
        }
        else 
        {
            prev = p;
            p = p->next;
        }
    }
}


threadHandle_t threadAddPollingFunction(const char *name, void *data, void (*function)(void *))
{
    struct thread *p;
    
    p = (struct thread *)malloc(sizeof(struct thread));
    if (p == NULL)
    {
        debugf("malloc", "malloc(%d)", sizeof(struct thread));
        return NULL;
    }

    p->name = name;
    p->fd = 0;
    p->data = data;
    p->status = THREAD_ACTIVE;
    p->function = function;

    p->next = s_threadPollingHead;
    s_threadPollingHead = p;
    
    debug("thread", "Polling thread(%s) registered", p->name);

    return p;
}

void threadFreePollingFunction(struct thread *thread)
{
    struct thread *p, *prev = NULL;
    p = s_threadPollingHead;

    while(p)
    {
        if (p == thread)
        {
            if (prev)
            {
                prev->next = p->next;
            }
            else
            {
                s_threadPollingHead = p->next;
            }

            free(p);

            debug("thread", "Remove Polling thread(%s,fd=%d)", p->name, p->fd); 

            return ;
        }
        else
        {
            prev = p;
            p = p->next;
        }
    }
}


void threadRemovePollingFunction(threadHandle_t handle)
{
    struct thread *p, *prev = NULL;
    p = s_threadPollingHead;
    while(p)
    {        
        if (p == handle)
        {
            p->status = THREAD_REMOVE; 
            return ;
        }
        else 
        {
            prev = p;
            p = p->next;
        }
    }
}

void threadSchedule(int slot)
{
    struct thread *thread, *next;
    struct timeval tv;
    fd_set fds;
    int maxfd, ret;
         
    while(!s_threadExit)
    {
        tv.tv_sec = slot / 1000;
        tv.tv_usec = (slot % 1000) * 1000;

        maxfd = -1;
         
        FD_ZERO(&fds);
 
        thread = s_threadListeningHead;
        while(thread)
        {
            if ((thread->fd >= 0) && (thread->status == THREAD_ACTIVE))
            {
                FD_SET(thread->fd, &fds);
                if (thread->fd > maxfd)
                {
                    maxfd = thread->fd;
                }
            }
            thread = thread->next;
        }

        ret = select(maxfd + 1, &fds, NULL, NULL, &tv);
        if (ret > 0) 
        {         
            thread = s_threadListeningHead;
            while(thread)
            {
                next = thread->next;

                if (thread->status == THREAD_REMOVE)
                {
                    threadFreeListeningFile(thread);
                    
                    thread = next;                       

                    continue;
                }
							
                if (FD_ISSET(thread->fd, &fds) && thread->function && (thread->status == THREAD_ACTIVE))
                {
                    thread->function(thread->data);
                }
                
				thread = next;
            }
        }

        thread = s_threadPollingHead;
        while(thread)
        {
            next = thread->next;

            if (thread->status == THREAD_REMOVE)
            {
                threadFreePollingFunction(thread);
                
                thread = next;                       

                continue;
            }            
            
            if (thread->function && (thread->status == THREAD_ACTIVE))
            {
                thread->function(thread->data);
            }
            thread = next;
        }


		
        debug("thread", "goes next(s_threadExit=%d)", s_threadExit);
    }  
}

void threadFreeAll(void)
{
    struct thread *p, *next = NULL;

    p = s_threadPollingHead;
    while(p)
    {        
        next = p->next;
        debug("thread", "Remove Polling thread(%s)", p->name);
        free(p); 
        p = next;    
    }    
    s_threadPollingHead = NULL;

    p = s_threadListeningHead;
    while(p)
    {        
        next = p->next;
        debug("thread", "Remove Listening thread(%s,fd=%d)", p->name, p->fd);
        free(p); 
        p = next;    
    }    
    s_threadListeningHead = NULL;

}

void threadExit(void)
{
    s_threadExit = 1;
}

