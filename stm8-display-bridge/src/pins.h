/* TIMERS
TIM1	Output CH4	TIM1_CH4	PC4
*/

/* USART
UART1	Asynchronous	UART2_RX	PD6
UART1	Asynchronous	UART2_TX	PD5
*/
/*
DISPLAY
*/


#define DISP_GPIO_DATA GPIOB
#define DISP_GPIO_CTRL GPIOA

#define NRST GPIO_PIN_3	
#define CD   GPIO_PIN_4	
#define WR   GPIO_PIN_5	
#define RD   GPIO_PIN_6	
#define D7   GPIO_PIN_7	
#define D6   GPIO_PIN_6	
#define D5   GPIO_PIN_5	
#define D4   GPIO_PIN_4	
#define D3   GPIO_PIN_3	
#define D2   GPIO_PIN_2	
#define D1   GPIO_PIN_1	
#define D0   GPIO_PIN_0	

#define CTRL_PINS   (NRST | CD | WR |RD)

#define SetRD0() GPIO_WriteLow(DISP_GPIO_CTRL, RD)
#define SetRD1() GPIO_WriteHigh(DISP_GPIO_CTRL, RD)

#define SetWR0() GPIO_WriteLow(DISP_GPIO_CTRL, WR)
#define SetWR1() GPIO_WriteHigh(DISP_GPIO_CTRL, WR)

#define SetCD0() GPIO_WriteLow(DISP_GPIO_CTRL, CD)
#define SetCD1() GPIO_WriteHigh(DISP_GPIO_CTRL, CD)

#define SetRESET0() GPIO_WriteLow(DISP_GPIO_CTRL, NRST)
#define SetRESET1() GPIO_WriteHigh(DISP_GPIO_CTRL, NRST)

