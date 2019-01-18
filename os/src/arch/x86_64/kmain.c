#include <stdint-gcc.h>
#include "printk.h"
#include "ps2Driver.h"
#include "idt.h"
#include "serial.h"
#include "parseMultiboot.h"
#include "memoryManager.h"
#include "test.h"
#include "utils.h"
#include "process.h"
#include "snakes.h"
#include "keyboard.h"
#include "blockDeviceDriver.h"
#include "string.h"
#include "kernel_functions.h"

int stupidFunctionDead = 0;
extern PROC_context *curProc;

int kmain(void *multiboot_point, unsigned int multitest){
  //initialize kernel
  bringupKernel(multiboot_point, multitest);
  
  // wait until gdb attaches to continue executing
  int enabled = 0;
  while(!enabled);

  //loop through all kernel threads
  while(1){
      PROC_run();
      asm("hlt");
  }

  return 0;
}
