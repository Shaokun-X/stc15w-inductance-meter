#ifndef __DEBUG_H
#define __DEBUG_H

#include <stdio.h> // IWYU pragma: keep
#include "config.h"

#define DEBUG 1
#if DEBUG
    #define log(...) printf(__VA_ARGS__)
#else
    #define log(...) ((void)0)
#endif

void debug_init(void);
void uart1_isr(void) __interrupt(UART1_VECTOR);

#endif
