#include "filter.h"

unsigned int update_filter(KalmanFilter *filter, unsigned int measurement)
{
    unsigned int prior_uncertainty = filter->uncertainty + PROCESS_VARIANCE;
    unsigned int denominator = prior_uncertainty + MEASUREMENT_VARIANCE;
    signed long difference = (signed long)measurement - filter->prediction;
    unsigned long difference_magnitude;
    unsigned long correction;

    if (difference < 0)
        difference_magnitude = (unsigned long)-difference;
    else
        difference_magnitude = (unsigned long)difference;

    correction = (difference_magnitude * prior_uncertainty + denominator / 2) / denominator;
    if (difference < 0)
        filter->prediction -= (unsigned int)correction;
    else
        filter->prediction += (unsigned int)correction;

    filter->uncertainty = (unsigned int)(
        ((unsigned long)prior_uncertainty * MEASUREMENT_VARIANCE + denominator / 2) /
        denominator
    );

    return filter->prediction;
}
