#include <stdint-gcc.h>
#include "drivers/cmos/cmos.h"
#include "drivers/interrupts/idt.h"
#include "drivers/memory/memoryManager.h"
#include "process.h"
#include "utils/printk.h"

extern uint64_t saved_rsp;
extern uint64_t saved_rbp;
extern void perform_syscall(int);

PROC_context *curProc = 0, *nextProc = 0, rootProc;
int numProcesses = 0;

PROC_context *procListHead = 0;

/* idea taken from https://samwho.dev/blog/2013/06/01/context-switching-on-x86/ */
void PROC_pushcli(){
	asm("CLI");
	if(curProc){
		curProc->cli_count += 1;
	}
}

void PROC_popcli(){
	if(curProc){
		if(curProc->cli_count <= 0){
			printk_err("attempted to pop cli, but there wasn't a cli pushed\n");
			asm("CLI");
			asm("HLT");
		}
		if(--(curProc->cli_count) == 0){
			asm("STI");
		}
	}
}

void PROC_run(){
	perform_syscall(SYS_START);
}

PROC_context *PROC_create_kthread(kproc_t entry_point, void *arg){
	//allocate a new context struct
	PROC_context *newProcCtx = (PROC_context*)kmalloc(sizeof(PROC_context));
	newProcCtx->pid = ++numProcesses;
	newProcCtx->cli_count = 0;

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

	newProcCtx->next = 0;
	//put at the end of the list
	if(!procListHead){
		procListHead = newProcCtx;
	}
	else {
		PROC_context *walker = procListHead;
		while(1){
			if(walker->next == 0){
				walker->next = newProcCtx;
				break;
			}
			walker = walker->next;
		}
	}

	return newProcCtx;
}

void PROC_reschedule(){
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

/********** BEGIN PROCESS MANAGEMENT/BLOCKING FUNCTIONS **********/

static void addCtxToPQ(ProcessQueue *pq, PROC_context *ctx){
	ctx->next = (PROC_context*)0;

	if(!pq->head){
		pq->head = ctx;
	}
	else {
		PROC_context *walker = pq->head;
		while(walker->next) walker = walker->next;
		walker->next = ctx;
	}
}

void PROC_block_on(ProcessQueue *pq, int enable_ints){
	if(!pq){
		printk_err("tried to block current process, but the process queue has not been initialized\n");
		return;
	}

	int mypid = curProc->pid;

	PROC_context *listWalker = procListHead, *lastNode = (PROC_context*)0;
	while(1){
		//reached the end of the list
		if(!listWalker){
			printk_err("proc that was tried to block wasn't on proc queue, seems like an Ethan problem\n");
			asm("hlt");
		}
		//found the process in the list
		if(listWalker->pid == mypid){
	    
			addCtxToPQ(pq, listWalker);

			if(!lastNode){
				procListHead = listWalker->next;
			}
			else {
				lastNode->next = listWalker->next;
			}

			if(enable_ints)
				asm("STI");
			break;
		}

		lastNode = listWalker;
		listWalker = listWalker->next;
	}
    
	yield();
}

void PROC_block_on_or_timeout(ProcessQueue *pq, int timeout_secs){
	if(!pq){
		printk_err("tried to block current process, but the process queue has not been initialized\n");
		return;
	}

	int mypid = curProc->pid;

	PROC_context *listWalker = procListHead, *lastNode = (PROC_context*)0;
	while(1){
		//reached the end of the list
		if(!listWalker){
			printk_err("proc that was tried to block wasn't on proc queue, seems like an Ethan problem\n");
			asm("hlt");
		}
		//found the process in the list
		if(listWalker->pid == mypid){
	    
			addCtxToPQ(pq, listWalker);

			if(!lastNode){
				procListHead = listWalker->next;
			}
			else {
				lastNode->next = listWalker->next;
			}

			break;
		}

		lastNode = listWalker;
		listWalker = listWalker->next;
	}

	/* register rtc timeout */
	set_timeout_for_process(pq, mypid, timeout_secs);
	
	yield();	
}

void PROC_unblock_all(ProcessQueue *pq){
	if(!pq->head){
		return;
	}
	if(!procListHead){
		procListHead = pq->head;
	}
	else {
		PROC_context *procPtr = procListHead;
		while(procPtr->next) procPtr = procPtr->next;
		procPtr->next = pq->head;
	}
	pq->head = (PROC_context*)0;
}

void PROC_unblock_head(ProcessQueue *pq){
	if(!pq->head){
		/* printk_warn("Tried to unblock head of empty process queue\n"); */
		return;
	}
	if(!procListHead){
		procListHead = pq->head;
	}
	else {
		PROC_context *procPtr = procListHead;
		while(procPtr->next) procPtr = procPtr->next;
		procPtr->next = pq->head;
	}

	//pop the unblocked head off of the process queue
	PROC_context *unblocked = pq->head;
	pq->head = pq->head->next;

	//unlink the rest of the process queue from the unblocked process
	unblocked->next = (PROC_context*)0;
}

void PROC_unblock(ProcessQueue *pq, int pid_to_unblock){
	if(!pq->head){
		/* printk_warn("Tried to unblock head of empty process queue\n"); */
		return;
	}

	/* find the process with specified pid */
	PROC_context *cur = pq->head, *last = NULL;
	while(1){
		if(!cur){
			printk_err("attempted to unblock process %d, but it wasn't on the process queue\n", pid_to_unblock);
			return;
		}
		if(cur->pid == pid_to_unblock){
			if(last){
				last->next = cur->next;
			}
			else {
				pq->head = cur->next;
			}
			cur->next = NULL;

		        clear_timeouts_for_pid(cur->pid);
			break;
		}
		last = cur;
		cur = cur->next;
	}

	/* add process to the process list */
	if(!procListHead){
		procListHead = cur;
	}
	else {
		PROC_context *procPtr = procListHead;
		while(procPtr->next) procPtr = procPtr->next;
		procPtr->next = cur;
	}
}

void PROC_init_queue(ProcessQueue *pq){
	pq->head = (PROC_context*)0;
}
