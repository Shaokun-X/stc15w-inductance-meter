#ifndef	__AUTORANGE_H
#define	__AUTORANGE_H

#define SW_5V P12
#define SW_DISCHARGE P13

enum Range {
    RANGE_BELOW_10U, // outbound
    RANGE_10U_TO_100U, // comparator range
    RANGE_100U_TO_1M,
    RANGE_1M_TO_10M,
    RANGE_10M_TO_100M,
    RANGE_100M_TO_1H,
    RANGE_ABOVE_1H, // outbound
    RANGE_COUNT,
};

#define COMPARATOR_RANGE RANGE_10U_TO_100U
// pessimistic time needed for the capacitor bank to fully discharge to low excitation voltage, in us
#define DISCHARGE_TIME 100
// time needed for the source to charge up the capacitors, in us
#define CHARGE_TIME 100
// delay time before switching 2 MOSFETs on the same path, in us
#define SHOOT_THROUGH_DEADZONE 20

extern volatile enum Range range;

void range_init(void);
enum Range switch_range(enum Range target);
enum Range auto_range(void);

#endif