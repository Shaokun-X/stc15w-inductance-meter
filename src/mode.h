#ifndef	__MODE_H
#define	__MODE_H

#include "config.h"

enum Mode {
    MODE_AUTO,
    MODE_10U_TO_100U,
    MODE_100U_TO_1M,
    MODE_1M_TO_10M,
    MODE_10M_TO_1H,
    MODE_COUNT,
};

extern volatile enum Mode mode;

void mode_init(void);

void int1_isr (void) __interrupt (INT1_VECTOR);
void pca_isr(void) __interrupt (PCA_VECTOR);

#endif