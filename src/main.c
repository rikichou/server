/*
	Author      : zhourui
	Contact     : 657708642@qq.com
	Description : dazhou 
*/

#include <stdio.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

/* open */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>

/* usr */
#include <main.h>
#include "device_main.h"

#undef SYSLOG_TO_DEBUG 
#define SYSLOG_TO_DEBUG  1

/* global variable */
struct global g;

static void signalReceive(void *data);

static void baseInit(void)
{
	/* signal */
	threadAddListeningFile("sys.signal", g.signalPipe[0], NULL, signalReceive);
	
    /* register timer */
    threadAddPollingFunction("sys.timer", NULL, timerPolling);
    
    /* start public modules */
    //ipcStart("server");

	/* start device server */
	ipcDeviceStart("device_server");
}

static void globalDeinit(void)
{
    if (g.stdFd >= 0)
    {
        close(g.stdFd);
    }   

    debugFreeAllIds();
}

static void baseDeinit(void)
{
    ipcExit("server");

    /* free all timers */
    timerFreeAll();
    
    /* free listening handles and polling handlers */
    threadFreeAll();

	globalDeinit();

    sysLog(LOG_INFO, "Entryd exit!");  

    closeLog();
}

static void signalReceive(void *data)
{
    int sig;
    if(read(g.signalPipe[0], &sig, sizeof(sig)) >= 0) 
    {
        debug("signal", "Catch signal: %d", sig); 

    	switch(sig)
        {
            case SIGTERM:
            case SIGKILL:
            case SIGSEGV:
            case SIGSTOP:
            case SIGABRT:
            case SIGQUIT:
            case SIGINT:                
                entryLog("signal", LOG_INFO, "Catch STOP signal(%d)", sig);
                threadExit();
    		    break;
    	}        
    }
}


static void signalHandle(int sig)
{
    int ret;
    ret = send(g.signalPipe[1], &sig, sizeof(sig), MSG_DONTWAIT);
    //assert(ret >= 0, "Could not send signal: %d", sig);
}

static void signalInit(void)
{
    /* Init signal handle */
    socketpair(AF_UNIX, SOCK_STREAM, 0, g.signalPipe);

    signal(SIGUSR1, signalHandle);
    signal(SIGUSR2, signalHandle);
    signal(SIGTERM, signalHandle); // TERM(kill)
    //signal(SIGKILL, signalHandle); // KILL(kill -9)
    signal(SIGSEGV, signalHandle);
    signal(SIGSTOP, signalHandle);
    signal(SIGABRT, signalHandle);
    signal(SIGQUIT, signalHandle);
    signal(SIGINT, signalHandle); // INT
    //signal(SIGCHLD, signalHandle);
}

static void globalInit(int argc, char **argv)
{   
    static const char *s_usage = 
        "Usage: entryd [options]\n"
        "   option:\n"
        "       -r FILE                 Redirect standard output to specific File or Device\n"
        "       -d DBGID[,DBGID]...     Enable debug entries with attached ids\n"
        "       -D                      Enable all debug entries\n" 
        "       -h                      Print this help\n"
        ;
    int ch;
    const char *redirectPath = NULL;
    const char *debugId = "";
    int debugAll = 0;

    while((ch = getopt(argc, argv, "r:cM:H:sd:Dvh")) != -1)
    {
        switch(ch){
            case 'r':
                redirectPath = optarg;
                break;
            case 'd':
                debugId = optarg;
                break;
            case 'D':
                debugAll = 1;
                break;            
            case 'h':
                printf("%s", s_usage);
                exit(0);
                break;
             default:
                printf("Unknown Option:%c\n", ch);
                break;
        }
    }

    memset(&g, 0, sizeof(g));

    debugPrintAll(debugAll);
    debugInjectIdString(debugAll ? "" : debugId);

    g.stdFd = -1;

    if (redirectPath)
    {
        struct termios opt;    
        g.stdFd = open(redirectPath, O_NONBLOCK | O_RDWR);

        if (g.stdFd >= 0)
        {
            //dup2(g.stdFd, 0);
            dup2(g.stdFd, 1);
            dup2(g.stdFd, 2);    
        
            tcgetattr(g.stdFd, &opt); 
            //opt.c_iflag |= ICRNL;
            opt.c_oflag |= OPOST | ONLCR;
                    
            tcsetattr(g.stdFd, TCSANOW, &opt);
        }
    }

	signalInit();  

    /* enable apply function */
    g.applyAll = 1;
}

static void firstInit(int argc, char **argv)
{
    openLog("entryd");

    /* get global optoins */
    globalInit(argc, argv);

    /* init ipc before any module */
    ipcInit();
}


static void startAll(void)
{
	/* start user model */

	
	
	return ;
}

static void stopAll(void)
{
	/* stop user model */

	return ;
}

static void preInit(void)
{
	/* device module pre init */
	devicePreInit();
}

int main(int argc, char **argv)
{
	firstInit(argc, argv);

	preInit();

	/* 1. base system init */
	baseInit();

	/* 2. start all function module */
    startAll();

	/* 3. start tread */
    threadSchedule(100);

	/* 4. stop all function module */
    stopAll();

	/* 5. stop base system */
    baseDeinit();

	return 0;
}

