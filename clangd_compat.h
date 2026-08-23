#ifndef CLANGD_COMPAT_H
#define CLANGD_COMPAT_H

/* Approximate SDCC extensions for Clang-based editor tooling only. */
#ifdef __clang__
/* Avoid Clang's hosted-program signature requirement for firmware main. */
#define main sdcc_main
#define __sfr volatile unsigned char
#define __sbit volatile unsigned char
#define __bit unsigned char
#define __data
#define __xdata
#define __code
#define __at(address)
#define __interrupt(vector) \
    __attribute__((diagnose_if((vector) < 0, "invalid interrupt vector", "error")))
#endif

#endif
