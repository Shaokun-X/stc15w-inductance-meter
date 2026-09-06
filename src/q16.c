#include "q16.h"

#define Q16_UNSIGNED_SIGN_BIT 0x80000000UL
#define Q16_FRACTION_BITS 16

static unsigned long q16_magnitude(q16_t value)
{
    if (value < 0)
        return (unsigned long)(-(value + 1)) + 1UL;

    return (unsigned long)value;
}

static q16_t q16_from_magnitude(unsigned long magnitude, unsigned char negative)
{
    unsigned long limit = negative ? Q16_UNSIGNED_SIGN_BIT : 0x7fffffffUL;

    if (magnitude >= limit)
        return negative ? Q16_MIN : Q16_MAX;

    return negative ? -(q16_t)magnitude : (q16_t)magnitude;
}

static unsigned char q16_add_magnitude(unsigned long *value,
                                       unsigned long addend,
                                       unsigned long limit)
{
    if (addend > limit - *value)
        return 0;

    *value += addend;
    return 1;
}

q16_t q16_from_signed(signed long value)
{
    if (value > 32767L)
        return Q16_MAX;
    if (value < -32768L)
        return Q16_MIN;

    return (q16_t)(value * 65536L);
}

q16_t q16_from_unsigned(unsigned long value)
{
    if (value > 32767UL)
        return Q16_MAX;

    return (q16_t)(value * 65536UL);
}

q16_t q16_add(q16_t left, q16_t right)
{
    if (right > 0 && left > Q16_MAX - right)
        return Q16_MAX;
    if (right < 0 && left < Q16_MIN - right)
        return Q16_MIN;

    return left + right;
}

q16_t q16_subtract(q16_t left, q16_t right)
{
    if (right > 0 && left < Q16_MIN + right)
        return Q16_MIN;
    if (right < 0 && left > Q16_MAX + right)
        return Q16_MAX;

    return left - right;
}

q16_t q16_multiply(q16_t left, q16_t right)
{
    unsigned long left_magnitude = q16_magnitude(left);
    unsigned long right_magnitude = q16_magnitude(right);
    unsigned long left_high = left_magnitude >> Q16_FRACTION_BITS;
    unsigned long right_high = right_magnitude >> Q16_FRACTION_BITS;
    unsigned long left_low = left_magnitude & 0xffffUL;
    unsigned long right_low = right_magnitude & 0xffffUL;
    unsigned char negative = (left < 0) != (right < 0);
    unsigned long limit = negative ? Q16_UNSIGNED_SIGN_BIT : 0x7fffffffUL;
    unsigned long high_product = left_high * right_high;
    unsigned long result;

    if (high_product > (limit >> Q16_FRACTION_BITS))
        return negative ? Q16_MIN : Q16_MAX;

    result = high_product << Q16_FRACTION_BITS;
    if (!q16_add_magnitude(&result, left_high * right_low, limit) ||
        !q16_add_magnitude(&result, left_low * right_high, limit) ||
        !q16_add_magnitude(&result, (left_low * right_low) >> Q16_FRACTION_BITS, limit))
        return negative ? Q16_MIN : Q16_MAX;

    return q16_from_magnitude(result, negative);
}

q16_t q16_divide(q16_t dividend, q16_t divisor)
{
    unsigned long dividend_magnitude = q16_magnitude(dividend);
    unsigned long divisor_magnitude = q16_magnitude(divisor);
    unsigned char negative = (dividend < 0) != (divisor < 0);
    unsigned long limit = negative ? Q16_UNSIGNED_SIGN_BIT : 0x7fffffffUL;
    unsigned long integer_part;
    unsigned long remainder;
    unsigned long fraction = 0;
    unsigned long result;
    unsigned int i;

    if (!divisor_magnitude)
        return dividend < 0 ? Q16_MIN : Q16_MAX;

    integer_part = dividend_magnitude / divisor_magnitude;
    if (integer_part > (limit >> Q16_FRACTION_BITS))
        return negative ? Q16_MIN : Q16_MAX;

    remainder = dividend_magnitude % divisor_magnitude;

    for (i = 0; i < Q16_FRACTION_BITS; i++)
    {
        fraction <<= 1;
        if (remainder >= divisor_magnitude - remainder)
        {
            remainder -= divisor_magnitude - remainder;
            fraction |= 1UL;
        }
        else
        {
            remainder <<= 1;
        }
    }

    result = (integer_part << Q16_FRACTION_BITS) | fraction;
    if (result > limit)
        return negative ? Q16_MIN : Q16_MAX;

    return q16_from_magnitude(result, negative);
}
