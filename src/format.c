#include "format.h"

#include "display.h"

static char buffer[16];

static char *append_number(char *destination, unsigned long value)
{
    unsigned long divisor = 1;

    while (value / divisor >= 10)
    {
        divisor *= 10;
    }
    do
    {
        *destination++ = '0' + (value / divisor) % 10;
        divisor /= 10;
    } while (divisor != 0);

    return destination;
}

static char *append_fraction(char *destination, unsigned int value)
{
    *destination++ = '0' + value / 100;
    *destination++ = '0' + value / 10 % 10;
    *destination++ = '0' + value % 10;
    return destination;
}

const char *format_inductance(unsigned long microhenries)
{
    unsigned long whole;
    unsigned int fraction;
    char *end = buffer;

    if (microhenries < 1000UL)
    {
        end = append_number(end, microhenries);
        *end++ = ' ';
        *end++ = DISPLAY_CHAR_MU;
        *end++ = 'H';
    }
    else if (microhenries < 1000000UL)
    {
        end = append_number(end, microhenries / 1000UL);
        *end++ = '.';
        end = append_fraction(end, microhenries % 1000UL);
        *end++ = ' ';
        *end++ = 'm';
        *end++ = 'H';
    }
    else
    {
        whole = microhenries / 1000000UL;
        fraction = (microhenries % 1000000UL + 500UL) / 1000UL;
        if (fraction == 1000)
        {
            whole++;
            fraction = 0;
        }

        end = append_number(end, whole);
        *end++ = '.';
        end = append_fraction(end, fraction);
        *end++ = ' ';
        *end++ = 'H';
    }

    *end = '\0';
    return buffer;
}
