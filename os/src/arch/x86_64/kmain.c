#include <stdint-gcc.h>
#include "utils/kernel_functions.h"

//todo: figure out what these args correspond to
int kmain(void *multiboot_point, unsigned int multitest){
  kernel_bringup_config config;
  config.multiboot_point = multiboot_point;
  config.multitest = multitest;
  config.enable_keyboard_proc = 1;

  //initialize kernel
  bringupKernel(&config);
  
  // wait until gdb attaches to continue executing
  int enabled = 0;
  while(!enabled);

  //loop through all kernel threads
  runKernelProcesses();

  return 0;
}
