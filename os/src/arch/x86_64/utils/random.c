#include <stdint-gcc.h>

/* extern uint16_t get_rand_short(); */
/* extern uint32_t get_rand_int(); */
/* extern uint64_t get_rand_long(); */

static unsigned long int next = 1;
 
static int rand( void ) // RAND_MAX assumed to be 32767
{
    next = next * 1103515245 + 12345;
    return (unsigned int)(next / 65536) % 32768;
}
 
void srand( unsigned int seed )
{
    next = seed;
}

static uint16_t get_rand_short(){
        int random = rand();
	return (uint16_t)random;
}

static uint32_t get_rand_int(){
        int random = rand();
	return (uint32_t)random;
}

static uint64_t get_rand_long(){
        int random = rand();
	int random2 = rand();

	uint64_t aggregate = random;
	aggregate += random2;

	return aggregate;
}

uint16_t rand_u16(){
	return get_rand_short();
}

short rand_short(){
	uint16_t urand = get_rand_short();
	return (short)urand;
}

uint32_t rand_u32(){
	return get_rand_int();
}

int rand_int(){
	uint32_t urand = get_rand_int();
	return (int)urand;
}

uint64_t rand_u64(){
	return get_rand_long();
}

int rand_long(){
	uint64_t urand = get_rand_long();
	return (long)urand;
}
