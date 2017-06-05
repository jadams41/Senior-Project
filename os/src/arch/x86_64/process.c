#include <stdint-gcc.h>
#include "idt.h"
#include "process.h"
#include "memoryManager.h"

extern uint64_t saved_rsp;
extern uint64_t saved_rbp;
extern void perform_syscall(int);

PROC_context *curProc = 0, *nextProc = 0, rootProc;
static int numProcesses = 0;

PROC_context *procListHead = 0;

void PROC_run(){
    perform_syscall(SYS_START);
}

PROC_context *PROC_create_kthread(kproc_t entry_point, void *arg){
    //allocate a new context struct
    PROC_context *newProcCtx = (PROC_context*)kmalloc(sizeof(PROC_context));
    newProcCtx->pid = ++numProcesses;

    //allocate and prepare a new stack
    void *new_stack = MMU_alloc_user_pages(0x100000 / 4096);
    uint64_t *stack_accessor = (uint64_t*)new_stack + 0x100000/sizeof(uint64_t*) - 1;
    uint16_t *selector_accessor;

    newProcCtx->rbp = (uint64_t)stack_accessor;
    newProcCtx->stack_bottom = (uint64_t)new_stack;

    //fill in the iret info
    //2 byte segment selector
    selector_accessor = (uint16_t*)stack_accessor;
    *selector_accessor = 0x0;
    stack_accessor -= 1;

    //return rsp
    *stack_accessor = (uint64_t)(stack_accessor - 40 - 16); //NOTE: if shit breaks, make sure this is correct
    stack_accessor -= 1;

    //return rflags
    *stack_accessor = 0x0; //leaves interrupts disabled, NOTE: if shit breaks look here
    stack_accessor -= 1;

    //return cs should be 0x8
    selector_accessor = (uint16_t*)stack_accessor;
    *selector_accessor = 0x8;
    stack_accessor -= 1;

    //return rip
    *stack_accessor = (uint64_t)entry_point;
    stack_accessor -= 1;

    //put the register values (all initially 0 onto the stack)
    int i;
    for(i = 0; i < 16; i++){
        *stack_accessor = 0;
        if(i == 0){
            *stack_accessor = (uint64_t)arg; //rbp
        }
        stack_accessor -= 1;
    }
    newProcCtx->rsp = ((uint64_t)stack_accessor) + 8;

    //put at the beginning of the list
    newProcCtx->next = procListHead;
    procListHead = newProcCtx;

    return newProcCtx;
}

void PROC_reschedule(){
    // if(curProc){
    //     if(!nextProc){
    //         nextProc = curProc;
    //     }
    //     else {
    //         while(nextProc->next){
    //             nextProc->next = curProc;
    //         }
    //     }
    // }
    // if(!nextProc){
    //     nextProc = &rootProc;
    //     //return to the master context
    // }
    // curProc = nextProc;
    // nextProc = nextProc->next;
    if(!curProc || !curProc->next){
        if(!procListHead){
            nextProc = &rootProc;
        }
        else {
            nextProc = procListHead;
        }
    }
    else {
        nextProc = curProc->next;
    }
}

void yield(){
    perform_syscall(SYS_YIELD);
}

void kexit(){
    asm volatile ("INT $129");
}