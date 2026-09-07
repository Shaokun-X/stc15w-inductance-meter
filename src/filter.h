#ifndef __FILTER_H
#define __FILTER_H

// Should be initialized with the first measurement, and the uncertainty for that measurement
typedef struct {
    unsigned int uncertainty;
    unsigned int prediction;
} KalmanFilter;

#define PROCESS_VARIANCE 10000U
#define MEASUREMENT_VARIANCE 20000U
#define DEFAULT_INITIAL_UNCERTAINTY 5000U

unsigned int update_filter(KalmanFilter* filter, unsigned int measurement);


#endif
