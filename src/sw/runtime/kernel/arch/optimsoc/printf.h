#ifndef _PRINTF_H_
#define _PRINTF_H_

#define PRINTFBUFFER_SIZE 512
extern char PRINTFBUFFER[PRINTFBUFFER_SIZE]; // Declare a global printf buffer
//int vfnprintf ( char *stream, size_t n, const char *format, va_list arg);


int printf_to_uart(const char *fmt, ...);
int printf_to_sim(const char *fmt, ...);

int putchar_to_uart(int c);
int putchar_to_sim(int c);

int puts_to_uart(const char *str);
int puts_to_sim(const char *str);

// for now...
#undef _UART_H_
#ifdef _UART_H_
#define printf printf_to_uart
#define putchar putchar_to_uart
#define puts puts_to_uart
#else
#define printf printf_to_sim
#undef putchar // was defined by stdlib
#define putchar putchar_to_sim
#define puts puts_to_sim
#endif

#endif
