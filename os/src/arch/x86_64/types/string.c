#include <stdint-gcc.h>
#include <stddef.h>

void memset(void *dst, uint8_t c, size_t n){
    int i = 0;
    char *walker = (char*)dst;
    while(i++ < n){
        *walker++ = c;
    }
}
