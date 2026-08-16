#ifndef	__INPUT_H
#define	__INPUT_H

#include "config.h"
#include "gpio.h"
#include "exti.h"
#include "pca.h"

void int1_isr (void) __interrupt (INT1_VECTOR);
void pca_isr(void) __interrupt (PCA_VECTOR);

void input_init(void);
void pca_timer_restart(void);
void pca_timer_stop(void);

#define PCA_TIMER_HZ    100UL
#define PCA_TIMER_TICKS ((u16)(MAIN_Fosc / 12UL / PCA_TIMER_HZ)) // PCA_Clock_12T

#define BUTTON_PIN P33

#endif