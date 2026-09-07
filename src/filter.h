#ifndef __FILTER_H
#define __FILTER_H

// Should be initialized with the first measurement, and the uncertainty for that measurement
typedef struct {
    unsigned int uncertainty;
    unsigned int prediction;
} KalmanFilter;

// Since inductance usually doesn't change dramatically, the measure variance should be the main
// contributor. Also the initial measurement should have a low uncertainty.
#define PROCESS_VARIANCE (1U << 8)
#define MEASUREMENT_VARIANCE (50U << 8)
#define DEFAULT_INITIAL_UNCERTAINTY (1U << 8)

unsigned int update_filter(KalmanFilter* filter, unsigned int measurement);


#endif
