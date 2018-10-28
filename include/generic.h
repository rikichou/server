#ifndef __GENERIC_H__ 
#define __GENERIC_H__

//#include <entryd.h>

/* Global Data Struct */
struct global{

    /* if this true, stop looping functions */
    int exit;
    
    int signalPipe[2];

    int stdFd;

    #ifdef CONFIG_CONSOLE
    int startConsole;
    #endif  

    #ifdef CONFIG_EXT_PERIPHERAL
    int asyncFd;
    #endif 

    /* global function apply */
    int applyAll;
};

extern struct global g;



/* platform dependences */
#ifndef __platform 
#define __platform 
#endif 


#endif /* #ifndef __GENERIC_H__*/

