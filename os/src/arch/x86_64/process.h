#ifndef PROCESS
#define PROCESS
#include <stdint-gcc.h>

#define USER_MEM_START 0X
#define STACK_SIZE 0x100000

struct PROC_context {
    uint64_t rsp;
    uint64_t rbp;
    uint64_t stack_bottom;
    int pid;
    struct PROC_context *next;
};
typedef struct PROC_context PROC_context;
/**
  * runs until all processes have completed
  * main driver function
  */
void PROC_run();

typedef void (*kproc_t)(void*);
/**
  * adds a new thread to the multitasking system
  * allocates a new stack in the virtual memory_info
  * initializes thread's context so that entry_point gets called when run the first time
  *** this context will include:
  ***** IP needs to point to beginning of entry_point function
  ***** put arg in the first argument register
  ***** put all stack info on the stack
  */
PROC_context *PROC_create_kthread(kproc_t entry_point, void *arg);

void PROC_reschedule(void);

// static inline void yield(void)
// {
//     //   asm volatile ( "INT $123" );
// }
void yield(void);
void kexit(void);

#endif
