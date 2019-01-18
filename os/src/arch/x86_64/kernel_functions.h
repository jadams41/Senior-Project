#ifndef KERNEL_FUNCTIONS
#define KERNEL_FUNCTIONS

/* functions to manage kernel state */
void bringupKernel(void *multiboot_point, unsigned int multitest);

/* functions called inside of a kernel process */
void readBlock(void *params);
void printBlock(void *params);

/* functions to pass as arguments to readBlock */
void readBlock0();
void readBlock32();
void grabKeyboardInput();


#endif
