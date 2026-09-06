#ifndef __Q16_H
#define __Q16_H

/* Signed Q16.16 fixed-point value. */
typedef signed int q16_t;

#define Q16_MAX ((q16_t)0x7fffffffL)
#define Q16_MIN ((q16_t)(-0x7fffffffL - 1L))

q16_t q16_from_signed(signed long value);
q16_t q16_from_unsigned(unsigned long value);

q16_t q16_add(q16_t left, q16_t right);
q16_t q16_subtract(q16_t left, q16_t right);
q16_t q16_multiply(q16_t left, q16_t right);
q16_t q16_divide(q16_t dividend, q16_t divisor);

#endif
