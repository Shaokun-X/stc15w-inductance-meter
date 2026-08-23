#include "debug.h"
#include "gpio.h"
#include "usart.h"

void debug_init(void)
{
#if DEBUG
    GPIO_INIT(P3, GPIO_Pin_0 | GPIO_Pin_1, GPIO_PullUp);
    /* The interrupt must be enabled so it can clear the TX busy flag. */
    USART1_TIMER2_INIT(115200UL, ENABLE, ENABLE, PriorityLow,
                       UART1_SW_P30_P31, DISABLE);
#endif
}

void uart1_isr (void) __interrupt (UART1_VECTOR)
{
	if(RI)
	{
		RI = 0;
		if(COM1.B_RX_OK == 0)
		{
			if(COM1.RX_Cnt >= COM_RX1_Lenth)	COM1.RX_Cnt = 0;
			RX1_Buffer[COM1.RX_Cnt++] = SBUF;
			COM1.RX_TimeOut = TimeOutSet1;
		}
	}

	if(TI)
	{
		TI = 0;

		#if(UART_QUEUE_MODE == 1)//判断是否使用队列模式
		if(COM1.TX_read != COM1.TX_write)
		{
		 	SBUF = TX1_Buffer[COM1.TX_read];
			if(++COM1.TX_read >= COM_TX1_Lenth)		COM1.TX_read = 0;
		}
		else	COM1.B_TX_busy = 0;
        #else
        COM1.B_TX_busy = 0;//使用阻塞方式发送直接清除繁忙标志
        #endif
	}
}
