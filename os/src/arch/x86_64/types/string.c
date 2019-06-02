#include <stdint-gcc.h>
#include <stddef.h>
#include "drivers/memory/memoryManager.h"

void *memset(void *dst, uint8_t c, size_t n){
    int i = 0;
    char *walker = (char*)dst;
    while(i++ < n){
        *walker++ = c;
    }

    return dst;
}

void *memcpy(void *dest, const void *src, size_t n){
    uint8_t *src_byte_arr = (uint8_t*)src;
    uint8_t *dest_byte_arr = (uint8_t*)dest;
    int i;

    for(i = 0; i < n; i++){
	dest_byte_arr[i] = src_byte_arr[i];
    }

    return dest;
}

size_t strlen(const char *s){
    uint64_t len = 0;

    while(s[len] != 0)
	len += 1;

    return len;
}

char *strcpy(char *dest, const char *src){
    uint64_t i = 0;

    do {
	dest[i] = src[i];
    } while(src[i++] != 0);

    return dest;
}

char *strncpy(char *dest, const char *src, int n){
    while(n-- > 0){
        *dest = *src;
        dest++;
        src++;
    }

    return dest;
}


int strcmp(const char *s1, const char *s2){
    int i;
    
    for (i = 0; ; i++){
        if (s1[i] != s2[i]){
            return s1[i] < s2[i] ? -1 : 1;
        }

        if (s1[i] == '\0'){
            return 0;
        }
    }
}

const char *strchr(const char *s, int c){
    char *walker = (char*)s;

    while(*walker != 0){
	if(*walker == c){
	    return walker;
	}
	
        walker += 1;
    }

    return NULL;
}

char *strdup(const char *s){
    int len = strlen(s), i;

    char *dup = (char*)kmalloc(sizeof(char) * (len + 1));

    for(i = 0; i < len; i++){
	dup[i] = s[i];
    }

    dup[len] = 0;

    return dup;
}
