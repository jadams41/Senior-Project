#ifndef _RANDOM_H
#define _RANDOM_H

#include <stdint-gcc.h>

void srand(unsigned int seed);

uint16_t rand_u16();
short rand_short();

uint32_t rand_u32();
int rand_int();

uint64_t rand_u64();
int rand_long();

#endif
