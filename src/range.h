#ifndef	__AUTORANGE_H
#define	__AUTORANGE_H

enum Range {
    RANGE_BELOW_10U, // outbound
    RANGE_10U_TO_100U, // comparator range
    RANGE_100U_TO_1M,
    RANGE_1M_TO_10M,
    RANGE_10M_TO_1H,
    RANGE_ABOVE_1H, // outbound
    RANGE_COUNT,
};

extern volatile enum Range range;

void range_init(void);
enum Range switch_range(enum Range target);
enum Range auto_range(void);

#endif