#include "stm8s.h"
#include "stm8s_gpio.h"
#include "stm8s_tim1.h"
#include "stm8s_uart1.h"
#include "pins.h"

void display_run( void );

void initPwm( void )
{
   TIM1_DeInit();

  /* Time Base configuration */
  /*
  TIM1_Period = 99
  TIM1_Prescaler = 159
  TIM1_CounterMode = TIM1_COUNTERMODE_UP
  TIM1_RepetitionCounter = 0
  */

  TIM1_TimeBaseInit(159, TIM1_COUNTERMODE_UP, 99, 0);

  /* Channel 4 Configuration in PWM mode 
	
  TIM1_OCMode = TIM1_OCMODE_PWM2
  TIM1_OutputState = TIM1_OUTPUTSTATE_ENABLE
  TIM1_Pulse = CCR1_Val
  TIM1_OCPolarity = TIM1_OCPOLARITY_LOW
  TIM1_OCNPolarity = TIM1_OCNPOLARITY_HIGH
  TIM1_OCIdleState = TIM1_OCIDLESTATE_SET
  TIM1_OCNIdleState = TIM1_OCIDLESTATE_RESET
  
  */
  /*TIM1_Pulse = CCR4_Val*/
  TIM1_OC4Init(TIM1_OCMODE_PWM2, TIM1_OUTPUTSTATE_ENABLE, 150, TIM1_OCPOLARITY_LOW,
               TIM1_OCIDLESTATE_SET);

  /* TIM1 counter enable */
  TIM1_Cmd(ENABLE);

  /* TIM1 Main Output Enable */
  TIM1_CtrlPWMOutputs(ENABLE);
	TIM1_SetCompare4(0);
	TIM1_SetCounter(0);
	GPIO_Init(GPIOC,GPIO_PIN_4, GPIO_MODE_OUT_PP_LOW_FAST);
}

void main(void)
{
    /* Configure the Fcpu to DIV1*/
    CLK_SYSCLKConfig(CLK_PRESCALER_CPUDIV1);
    
    /* Configure the HSI prescaler to the optimal value */
    CLK_SYSCLKConfig(CLK_PRESCALER_HSIDIV1);
	GPIO_Init(GPIOD, GPIO_PIN_0, GPIO_MODE_OUT_PP_LOW_FAST);
	GPIO_Init(DISP_GPIO_DATA, GPIO_PIN_ALL, GPIO_MODE_OUT_PP_HIGH_FAST);
	GPIO_Init(DISP_GPIO_CTRL, CTRL_PINS, GPIO_MODE_OUT_PP_HIGH_FAST);
	GPIO_WriteHigh(DISP_GPIO_DATA, GPIO_PIN_ALL);
	GPIO_WriteHigh(DISP_GPIO_CTRL, CTRL_PINS);
	
	GPIO_Init(GPIOD, GPIO_PIN_2, GPIO_MODE_OUT_PP_HIGH_FAST); //WARN patched PB2->PD2


  UART1_Init((uint32_t)625000, UART1_WORDLENGTH_8D,UART1_STOPBITS_1, UART1_PARITY_NO,
                   UART1_SYNCMODE_CLOCK_DISABLE, UART1_MODE_RX_ENABLE);
  /* Enable the UART Receive interrupt: this interrupt is generated when the UART
    receive data register is not empty */
  UART1_ITConfig(UART1_IT_RXNE_OR, ENABLE);
  
  UART1_Cmd(ENABLE);
  
    /* Enable general interrupts */
  enableInterrupts();

/* Backlight PWM*/
	initPwm();
  display_run();
  
}

static volatile char buffer[256];
static volatile uint8_t rxIndex = 0;
static volatile uint8_t readIndex = 0;

uint8_t readUartByte(void) {
	uint8_t c;
	while(1) {
		disableInterrupts();
	  if(rxIndex!=readIndex) {
			c = buffer[readIndex++];
			enableInterrupts();
			return c;
	  }
		enableInterrupts();
		wfi();
	}
}
/**
  * @brief  UART2 RX Interrupt routine
  * @param  None
  * @retval None
  */
 INTERRUPT_HANDLER(UART1_RX_IRQHandler, 18)
{

  /* Read one byte from the receive data register and send it back */
	buffer[rxIndex++] = UART1_ReceiveData8();
	if(rxIndex == readIndex) {
		readIndex++;
	}
}

#ifdef USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: assert_param error line source number
  * @retval : None
  */
void assert_failed(u8* file, u32 line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
