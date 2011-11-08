
#include <sys/types.h>

unsigned int __errno;

typedef unsigned int FILE;

void *memset(void *dest, int c, size_t n) {
    int i = 0;
    while(i < n)
    {
        *((unsigned char *)dest + i) = (unsigned char)(c&0xFF);
		i++;
    }

    return dest;
}

void *memcpy(void *dest, const void *src, size_t n) {
    while(n>3)
    {
        *((unsigned int *)dest) = *((unsigned int *)src);
        n -= 4;
    }
    while(n)
    {
        *((unsigned char *)dest) = *((unsigned char *)src);
        n--;
    }

    return dest;
}

FILE *stderr;
FILE *stdout;

int fprintf(FILE *stream, const char *format, ...) {
	return 0;
}
