#include "debug.h"

void debug_init(void)
{
#if DEBUG
    GPIO_InitTypeDef        GPIO_InitStructure;
    GPIO_InitStructure.Pin  = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.Mode = GPIO_PullUp;
    GPIO_Inilize(GPIO_P3, &GPIO_InitStructure);

    COMx_InitDefine         UART_InitStructure;
    UART_InitStructure.UART_Mode = UART_8bit_BRTx;
    UART_InitStructure.UART_BRT_Use = BRT_Timer2;
    UART_InitStructure.UART_BaudRate = 115200UL;
    UART_InitStructure.UART_RxEnable = ENABLE;
    UART_InitStructure.UART_P_SW = UART1_SW_P30_P31;
    UART_InitStructure.UART_Interrupt = ENABLE;   // uart interrupt has to be turned on otherwise the busy flag is never cleared
    UART_InitStructure.UART_Polity = PolityHigh;      // PolityLow,PolityHigh
    USART_Configuration(USART1, &UART_InitStructure);
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