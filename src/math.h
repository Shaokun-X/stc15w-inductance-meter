#ifndef __MATH_H
#define __MATH_H

unsigned int transform_voltage_q11(unsigned int voltage, unsigned int stable_voltage);
unsigned long calculate_voltage_slope_q16(const unsigned int *time_buffer,
                                          const __data unsigned int *voltage_buffer,
                                          unsigned char point_count,
                                          unsigned int stable_voltage);

#endif
