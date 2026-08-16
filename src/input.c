#include "input.h"

volatile enum Mode mode = MODE_AUTO;

static void pca_timer_restart(void);
static void pca_timer_stop(void);
static enum Mode cycle_mode(void);

void input_init(void)
{
    GPIO_InitTypeDef        GPIO_InitStructure;

    GPIO_InitStructure.Pin  = GPIO_Pin_3;
    GPIO_InitStructure.Mode = GPIO_HighZ;
    GPIO_Inilize(GPIO_P3, &GPIO_InitStructure);

    EXTI_InitTypeDef        EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Mode = EXT_MODE_RiseFall;
    EXTI_InitStructure.EXTI_Polity = PolityLow;
    EXTI_InitStructure.EXTI_Interrupt = ENABLE;
    Ext_Inilize(EXT_INT1, &EXTI_InitStructure);

    /* Configure PCA0 as a 16-bit software timer. */
    PCA_InitTypeDef         PCA_InitStructure;
    PCA_InitStructure.PCA_IoUse          = PCA_P12_P11_P10_P37; // doesn't matter when only timer mode is used
    PCA_InitStructure.PCA_Clock          = PCA_Clock_12T;
    PCA_InitStructure.PCA_Mode           = PCA_Mode_SoftTimer;
    PCA_InitStructure.PCA_PWM_Wide       = PCA_PWM_8bit; /* Unused */
    PCA_InitStructure.PCA_Interrupt_Mode = ENABLE;
    PCA_InitStructure.PCA_Polity         = PolityLow;
    PCA_InitStructure.PCA_Value          = PCA_TIMER_TICKS;
    PCA_Init(PCA0, &PCA_InitStructure);
    CR = 0;
    /*
    * Reset and start the shared PCA counter.
    * DISABLE applies only to the counter-overflow interrupt.
    */
    PCA_InitStructure.PCA_Interrupt_Mode = DISABLE;
    PCA_Init(PCA_Counter, &PCA_InitStructure);
}

static enum Mode cycle_mode(void)
{
    mode++;

    if (mode >= MODE_COUNT) {
        mode = MODE_AUTO;
    }

    return mode;
}


void int1_isr (void) __interrupt (INT1_VECTOR)		//进中断时已经清除标志
{
    // rising edge only
    if (BUTTON_PIN)
    {
        pca_timer_restart();
    }
}

void pca_isr(void) __interrupt (PCA_VECTOR)
{
    if (CCF0)
    {
        CCF0 = 0;

        /* Schedule the next interrupt 10 ms after the previous deadline. */
        CCAP0_tmp += PCA_Timer0;
        CCAP0L = (u8)CCAP0_tmp;
        CCAP0H = (u8)(CCAP0_tmp >> 8);
        if (BUTTON_PIN)
        {
            // P32 = !P32;
            cycle_mode();
            pca_timer_stop();
        }
    }
}

static void pca_timer_stop(void)
{
    CR = 0;
}

static void pca_timer_restart(void)
{
    CR = 0;

    CCF0 = 0;
    CF = 0;

    CH = 0;
    CL = 0;

    CCAP0_tmp = PCA_Timer0;
    CCAP0L = (u8)CCAP0_tmp;
    CCAP0H = (u8)(CCAP0_tmp >> 8);

    CR = 1;
}