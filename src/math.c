#include "math.h"

#define LN2_Q11 1419UL
#define TIMER_REGRESSION_SHIFT 5

/* Returns -ln(1 - voltage / stable_voltage) in unsigned Q5.11. */
unsigned int transform_voltage_q11(unsigned int voltage, unsigned int stable_voltage)
{
    unsigned int log_q11 = 0;
    unsigned long u_q15;
    unsigned long z_q15;
    unsigned long z_squared_q15;
    unsigned long term_q15;
    unsigned long sum_q15;

    u_q15 = ((unsigned long)(stable_voltage - voltage) << 15) / stable_voltage;

    while (u_q15 < 16384UL)
    {
        u_q15 <<= 1;
        log_q11 += LN2_Q11;
    }

    z_q15 = ((32768UL - u_q15) << 15) / (32768UL + u_q15);
    z_squared_q15 = (z_q15 * z_q15) >> 15;

    term_q15 = z_q15;
    sum_q15 = term_q15;
    term_q15 = (term_q15 * z_squared_q15) >> 15;
    sum_q15 += term_q15 / 3;
    term_q15 = (term_q15 * z_squared_q15) >> 15;
    sum_q15 += term_q15 / 5;
    term_q15 = (term_q15 * z_squared_q15) >> 15;
    sum_q15 += term_q15 / 7;

    return log_q11 + (unsigned int)((2 * sum_q15 + 8) >> 4);
}

static unsigned long divide_q32(unsigned long numerator, unsigned long denominator)
{
    unsigned char i;
    unsigned long quotient = 0;

    if (!denominator)
        return 0;

    if (numerator >= denominator)
        return 0xffffffffUL;

    for (i = 0; i < 32; i++)
    {
        quotient <<= 1;
        if (numerator >= denominator - numerator)
        {
            numerator -= denominator - numerator;
            quotient |= 1;
        }
        else
        {
            numerator <<= 1;
        }
    }

    return quotient;
}

/* Returns the time/transformed-voltage slope in unsigned Q16.16. */
unsigned long calculate_voltage_slope_q16(const unsigned int *time_buffer,
                                          const __data unsigned int *voltage_buffer,
                                          unsigned char buffer_size,
                                          unsigned int stable_voltage)
{
    unsigned char i;
    unsigned long sum_x = 0;
    unsigned long sum_y = 0;
    unsigned long sum_xy = 0;
    unsigned long sum_x_squared = 0;
    unsigned long numerator;
    unsigned long denominator;
    unsigned long left;
    unsigned long right;
    unsigned char point_count = 0;

    for (i = 0; i < buffer_size; i++)
    {
        if (!voltage_buffer[i])
        {
            continue;
        }

        point_count++;

        unsigned int x = transform_voltage_q11(voltage_buffer[i], stable_voltage);
        unsigned int y = time_buffer[i] >> TIMER_REGRESSION_SHIFT;

        sum_x += x;
        sum_y += y;
        sum_xy += (unsigned long)x * y;
        sum_x_squared += (unsigned long)x * x;
    }

    left = (unsigned long)point_count * sum_xy;
    right = sum_x * sum_y;
    numerator = left - right;

    left = (unsigned long)point_count * sum_x_squared;
    right = sum_x * sum_x;
    denominator = left - right;

    /* The input scales require a 2^32 ratio to produce a Q16.16 result. */
    return divide_q32(numerator, denominator);
}
