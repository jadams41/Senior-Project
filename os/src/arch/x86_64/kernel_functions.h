#ifndef KERNEL_FUNCTIONS
#define KERNEL_FUNCTIONS

typedef struct {
  // information to parse multiboot tags
  void *multiboot_point;
  unsigned int multitest;

  // settings for kernel bringup
  uint8_t enable_keyboard_proc;
} kernel_bringup_config;

/* functions to manage kernel state */
void bringupKernel(kernel_bringup_config *conf);
void runKernelProcesses();

/* functions called inside of a kernel process */
void readBlock(void *params);
void printBlock(void *params);

/* functions to pass as arguments to readBlock */
void readBlock0();
void readBlock32();
void grabKeyboardInput();


#endif
