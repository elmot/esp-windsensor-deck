/**
  ******************************************************************************
  * @file    stm8s_tim1.c
  * @author  MCD Application Team
  * @version V2.3.0
  * @date    16-June-2017
  * @brief   This file contains all the functions for the TIM1 peripheral.
   ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm8s_tim1.h"

/** @addtogroup STM8S_StdPeriph_Driver
  * @{
  */
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void TI1_Config(uint8_t TIM1_ICPolarity, uint8_t TIM1_ICSelection,
                       uint8_t TIM1_ICFilter);

static void TI2_Config(uint8_t TIM1_ICPolarity, uint8_t TIM1_ICSelection,
                       uint8_t TIM1_ICFilter);

static void TI3_Config(uint8_t TIM1_ICPolarity, uint8_t TIM1_ICSelection,
                       uint8_t TIM1_ICFilter);

static void TI4_Config(uint8_t TIM1_ICPolarity, uint8_t TIM1_ICSelection,
                       uint8_t TIM1_ICFilter);

/**
  * @addtogroup TIM1_Public_Functions
  * @{
  */

/**
  * @brief  Deinitializes the TIM1 peripheral registers to their default reset values.
  * @param  None
  * @retval None
  */
void TIM1_DeInit(void) {
    TIM1->CR1 = TIM1_CR1_RESET_VALUE;
    TIM1->CR2 = TIM1_CR2_RESET_VALUE;
    TIM1->SMCR = TIM1_SMCR_RESET_VALUE;
    TIM1->ETR = TIM1_ETR_RESET_VALUE;
    TIM1->IER = TIM1_IER_RESET_VALUE;
    TIM1->SR2 = TIM1_SR2_RESET_VALUE;
    /* Disable channels */
    TIM1->CCER1 = TIM1_CCER1_RESET_VALUE;
    TIM1->CCER2 = TIM1_CCER2_RESET_VALUE;
    /* Configure channels as inputs: it is necessary if lock level is equal to 2 or 3 */
    TIM1->CCMR1 = 0x01;
    TIM1->CCMR2 = 0x01;
    TIM1->CCMR3 = 0x01;
    TIM1->CCMR4 = 0x01;
    /* Then reset channel registers: it also works if lock level is equal to 2 or 3 */
    TIM1->CCER1 = TIM1_CCER1_RESET_VALUE;
    TIM1->CCER2 = TIM1_CCER2_RESET_VALUE;
    TIM1->CCMR1 = TIM1_CCMR1_RESET_VALUE;
    TIM1->CCMR2 = TIM1_CCMR2_RESET_VALUE;
    TIM1->CCMR3 = TIM1_CCMR3_RESET_VALUE;
    TIM1->CCMR4 = TIM1_CCMR4_RESET_VALUE;
    TIM1->CNTRH = TIM1_CNTRH_RESET_VALUE;
    TIM1->CNTRL = TIM1_CNTRL_RESET_VALUE;
    TIM1->PSCRH = TIM1_PSCRH_RESET_VALUE;
    TIM1->PSCRL = TIM1_PSCRL_RESET_VALUE;
    TIM1->ARRH = TIM1_ARRH_RESET_VALUE;
    TIM1->ARRL = TIM1_ARRL_RESET_VALUE;
    TIM1->CCR1H = TIM1_CCR1H_RESET_VALUE;
    TIM1->CCR1L = TIM1_CCR1L_RESET_VALUE;
    TIM1->CCR2H = TIM1_CCR2H_RESET_VALUE;
    TIM1->CCR2L = TIM1_CCR2L_RESET_VALUE;
    TIM1->CCR3H = TIM1_CCR3H_RESET_VALUE;
    TIM1->CCR3L = TIM1_CCR3L_RESET_VALUE;
    TIM1->CCR4H = TIM1_CCR4H_RESET_VALUE;
    TIM1->CCR4L = TIM1_CCR4L_RESET_VALUE;
    TIM1->OISR = TIM1_OISR_RESET_VALUE;
    TIM1->EGR = 0x01; /* TIM1_EGR_UG */
    TIM1->DTR = TIM1_DTR_RESET_VALUE;
    TIM1->BKR = TIM1_BKR_RESET_VALUE;
    TIM1->RCR = TIM1_RCR_RESET_VALUE;
    TIM1->SR1 = TIM1_SR1_RESET_VALUE;
}

/**
  * @brief  Initializes the TIM1 Time Base Unit according to the specified parameters.
  * @param  TIM1_Prescaler specifies the Prescaler value.
  * @param  TIM1_CounterMode specifies the counter mode  from @ref TIM1_CounterMode_TypeDef .
  * @param  TIM1_Period specifies the Period value.
  * @param  TIM1_RepetitionCounter specifies the Repetition counter value
  * @retval None
  */
void TIM1_TimeBaseInit(uint16_t TIM1_Prescaler,
                       TIM1_CounterMode_TypeDef TIM1_CounterMode,
                       uint16_t TIM1_Period,
                       uint8_t TIM1_RepetitionCounter) {
    /* Check parameters */
    assert_param(IS_TIM1_COUNTER_MODE_OK(TIM1_CounterMode));

    /* Set the Autoreload value */
    TIM1->ARRH = (uint8_t) (TIM1_Period >> 8);
    TIM1->ARRL = (uint8_t) (TIM1_Period);

    /* Set the Prescaler value */
    TIM1->PSCRH = (uint8_t) (TIM1_Prescaler >> 8);
    TIM1->PSCRL = (uint8_t) (TIM1_Prescaler);

    /* Select the Counter Mode */
    TIM1->CR1 = (uint8_t) ((uint8_t) (TIM1->CR1 & (uint8_t) (~(TIM1_CR1_CMS | TIM1_CR1_DIR)))
                           | (uint8_t) (TIM1_CounterMode));

    /* Set the Repetition Counter value */
    TIM1->RCR = TIM1_RepetitionCounter;
}

/**
  * @brief  Enables or disables the TIM1 peripheral.
  * @param  NewState new state of the TIM1 peripheral.
  *         This parameter can be ENABLE or DISABLE.
  * @retval None
  */
void TIM1_Cmd(FunctionalState NewState) {
    /* Check the parameters */
    assert_param(IS_FUNCTIONALSTATE_OK(NewState));

    /* set or Reset the CEN Bit */
    if (NewState != DISABLE) {
        TIM1->CR1 |= TIM1_CR1_CEN;
    } else {
        TIM1->CR1 &= (uint8_t) (~TIM1_CR1_CEN);
    }
}

/**
  * @brief  Sets the TIM1 Counter Register value.
  * @param   Counter specifies the Counter register new value.
  * This parameter is between 0x0000 and 0xFFFF.
  * @retval None
  */
void TIM1_SetCounter(uint16_t Counter)
{
    /* Set the Counter Register value */
    TIM1->CNTRH = (uint8_t)(Counter >> 8);
    TIM1->CNTRL = (uint8_t)(Counter);
}


/**
  * @brief  Sets the TIM1 Capture Compare4 Register value.
  * @param   Compare4 specifies the Capture Compare4 register new value.
  * This parameter is between 0x0000 and 0xFFFF.
  * @retval None
  */
void TIM1_SetCompare4(uint16_t Compare4) {
    /* Set the Capture Compare4 Register value */
    TIM1->CCR4H = (uint8_t) (Compare4 >> 8);
    TIM1->CCR4L = (uint8_t) (Compare4);
}

/**
  * @brief  Initializes the TIM1 Channel4 according to the specified parameters.
  * @param  TIM1_OCMode specifies the Output Compare mode  from
  *         @ref TIM1_OCMode_TypeDef.
  * @param  TIM1_OutputState specifies the Output State
  *         from @ref TIM1_OutputState_TypeDef.
  * @param  TIM1_Pulse specifies the Pulse width  value.
  * @param  TIM1_OCPolarity specifies the Output Compare Polarity
  *         from @ref TIM1_OCPolarity_TypeDef.
  * @param  TIM1_OCIdleState specifies the Output Compare Idle State
  *         from @ref TIM1_OCIdleState_TypeDef.
  * @retval None
  */
void TIM1_OC4Init(TIM1_OCMode_TypeDef TIM1_OCMode,
                  TIM1_OutputState_TypeDef TIM1_OutputState,
                  uint16_t TIM1_Pulse,
                  TIM1_OCPolarity_TypeDef TIM1_OCPolarity,
                  TIM1_OCIdleState_TypeDef TIM1_OCIdleState)
{
    /* Check the parameters */
    assert_param(IS_TIM1_OC_MODE_OK(TIM1_OCMode));
    assert_param(IS_TIM1_OUTPUT_STATE_OK(TIM1_OutputState));
    assert_param(IS_TIM1_OC_POLARITY_OK(TIM1_OCPolarity));
    assert_param(IS_TIM1_OCIDLE_STATE_OK(TIM1_OCIdleState));

    /* Disable the Channel 4: Reset the CCE Bit */
    TIM1->CCER2 &= (uint8_t)(~(TIM1_CCER2_CC4E | TIM1_CCER2_CC4P));
    /* Set the Output State  &  the Output Polarity */
    TIM1->CCER2 |= (uint8_t)((uint8_t)(TIM1_OutputState & TIM1_CCER2_CC4E ) |
                             (uint8_t)(TIM1_OCPolarity  & TIM1_CCER2_CC4P ));

    /* Reset the Output Compare Bit  and Set the Output Compare Mode */
    TIM1->CCMR4 = (uint8_t)((uint8_t)(TIM1->CCMR4 & (uint8_t)(~TIM1_CCMR_OCM)) |
                            TIM1_OCMode);

    /* Set the Output Idle state */
    if (TIM1_OCIdleState != TIM1_OCIDLESTATE_RESET)
    {
        TIM1->OISR |= (uint8_t)(~TIM1_CCER2_CC4P);
    }
    else
    {
        TIM1->OISR &= (uint8_t)(~TIM1_OISR_OIS4);
    }

    /* Set the Pulse value */
    TIM1->CCR4H = (uint8_t)(TIM1_Pulse >> 8);
    TIM1->CCR4L = (uint8_t)(TIM1_Pulse);
}
/**
  * @brief  Enables or disables the TIM1 peripheral Main Outputs.
  * @param  NewState new state of the TIM1 peripheral.
  *         This parameter can be ENABLE or DISABLE.
  * @retval None
  */
void TIM1_CtrlPWMOutputs(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONALSTATE_OK(NewState));

    /* Set or Reset the MOE Bit */

    if (NewState != DISABLE)
    {
        TIM1->BKR |= TIM1_BKR_MOE;
    }
    else
    {
        TIM1->BKR &= (uint8_t)(~TIM1_BKR_MOE);
    }
}


/**
  * @}
  */

/**
  * @}
  */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
