/************************************************************************************************************
 *
 *  $Copyright © 2012-2014 CML Systems $
 *
 *  $Header: stm32f10x_it.c  Revision:1.9.1.4  02 July 2014 17:53:09  ilewis $
 *  $Label: ES994334 $
 *
 *  THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 *  WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 *  TIME. AS A RESULT, CML MICROSYSYEMS SHALL NOT BE HELD LIABLE FOR ANY
 *  DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 *  FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 *  CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 *************************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "stm32f10x_sdr.h"
    
#ifdef TARGET_DMR
 #ifdef DEBUG_CHANGE_INC_FROM_INITIAL_SDR_PROJECT
  #include "..\DMR\mainDMR.h"
 #else
  #include "mainDMR.h"
 #endif
#else 
 #ifdef TARGET_PMR
 #include "mainPMR.h" 
 #endif
#endif
 
#ifndef DEBUG_CHANGE_INC_FROM_INITIAL_SDR_PROJECT
#include "hdw_io.h"             //added for uart rx buffer and associated variables. *IPL
#endif

// Hardware declarations for CBUS interrupt interface.
GPIO_TypeDef* CBUS_IRQ_PORT[CBUS_NUMBER_OF_INTERFACES] = {CBUS_IRQN1_GPIO_PORT,0,CBUS_IRQN3_GPIO_PORT};
const uint16_t CBUS_IRQ_PIN[CBUS_NUMBER_OF_INTERFACES] = {CBUS_IRQN1_PIN,0,CBUS_IRQN3_PIN};
const uint32_t CBUS_IRQ_CLK[CBUS_NUMBER_OF_INTERFACES] = {CBUS_IRQN1_GPIO_CLK,0,CBUS_IRQN3_GPIO_CLK};
const uint16_t CBUS_IRQ_EXTI_LINE[CBUS_NUMBER_OF_INTERFACES] = {CBUS_IRQN1_EXTI_LINE,0,CBUS_IRQN3_EXTI_LINE};
const uint8_t CBUS_IRQ_PORT_SOURCE[CBUS_NUMBER_OF_INTERFACES] = {CBUS_IRQN1_EXTI_PORT_SOURCE,0,CBUS_IRQN3_EXTI_PORT_SOURCE};
const uint8_t CBUS_IRQ_PIN_SOURCE[CBUS_NUMBER_OF_INTERFACES] = {CBUS_IRQN1_EXTI_PIN_SOURCE,0,CBUS_IRQN3_EXTI_PIN_SOURCE};
const uint8_t CBUS_IRQn[CBUS_NUMBER_OF_INTERFACES] = {CBUS_IRQN1_EXTI_IRQn,0,CBUS_IRQN3_EXTI_IRQn};

// Local function prototypes

void PendSV_Priority (uint8_t uPriority);
void SysTick_Priority (uint8_t uPriority);

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */

//extern uint16_t RxEncoderSamples (void);
#ifdef TARGET_DMR
void PendSV_Handler(void)
{
	DMR_PendSV();
}
#else
#ifdef TARGET_PMR
void PendSV_Handler(void)
{
	GPIO_SetBits(GPIOE, TEST_TL2);
	PMR_PendSV();
	GPIO_ResetBits(GPIOE, TEST_TL2);
}
#endif
#endif
//extern void DMR_SysTick (void);

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */

#ifdef TARGET_DMR
void SysTick_Handler(void)
{
	DMR_SysTick ();
}
#else
#ifdef TARGET_PMR
void SysTick_Handler(void)
{
	GPIO_ToggleBits(GPIOE, TEST_TL3);
	PMR_SysTick ();
}
#endif
#endif
/******************************************************************************/
/*                 STM32Fxxx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32fxxx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @brief  This function handles EXTI0_IRQ Handler.
  * @param  None
  * @retval None
  */

/**
  * @brief  This function handles EXTI0_IRQ Handler.
  * @param  None
  * @retval None
  */
void EXTI0_IRQHandler(void)
{

  // Calls a function via a pointer and passes a pointer to a parameter. Since this IRQ is common
  // to several projects we can map this IRQ to different handlers as required.
  //( *pFuncSDR_CBUS1_IRQ)(pParamSDR_CBUS1_IRQ);

  (*CBUS_IRQTable[CBUS_INTERFACE_1].pHandler)(CBUS_IRQTable[CBUS_INTERFACE_1].pParam);

  // Clear the EXTI line pending bit
  EXTI_ClearITPendingBit(CBUS_IRQN1_EXTI_LINE);

}


void EXTI1_IRQHandler(void)
{

  // Calls a function via a pointer and passes a pointer to a parameter. Since this IRQ is common
  // to several projects we can map this IRQ to different handlers as required.
  //( *pFuncSDR_CBUS1_IRQ)(pParamSDR_CBUS1_IRQ);

  (*CBUS_IRQTable[CBUS_INTERFACE_3].pHandler)(CBUS_IRQTable[CBUS_INTERFACE_3].pParam);

  // Clear the EXTI line pending bit
  EXTI_ClearITPendingBit(CBUS_IRQN3_EXTI_LINE);

}



/**
  * @brief  This function handles EXTI15_10_IRQ Handler.
  * @param  None
  * @retval None
  */
void OTG_FS_WKUP_IRQHandler(void)
{

}

/**
  * @brief  This function handles OTG_HS Handler.
  * @param  None
  * @retval None
  */
void OTG_FS_IRQHandler(void)
{

}

// Initialises the system tick and sets the priority for PendSV and the
// system tick. There is no initialisation for PendSV. Now we have
// multiple interrupts the priority and pre-emption grouping is set up.

void IRQ_SystemInit (void)
{
	RCC_ClocksTypeDef RCC_Clocks;
	// SysTick end of count event each 1mS. We do not need this for the script application
	// but it is used for the DMR implementation.
	RCC_GetClocksFreq(&RCC_Clocks);
	SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);
	// Ensure that we have pre-emption priority and sub-priority enabled
	// by enabling the priority grouping. Upper 4 bits are pre-emption and
	// lower 4 bits are sub-priority.
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	// The interrupt priorities are set so that the PendSV has the highest
	// priority followed by the CBUS IRQs. The PendSV is set to the higher
	// priority because it clears flags in the control field. If the CBUS IRQ
	// pre-empted PendSV writing to the control field and then set a flag
	// in the control field, then CBUS IRQ the flag could get cleared and
	// therefore missed as the context returned to PendSV.

	// PendSV is no longer used in this design, but I decided to leave it in after spending
	// so much time working out how to use and configure it.

	PendSV_Priority(0x11);
	SysTick_Priority(0xFF);
}

// This routine sets up the GPIO pins for the interrupts from the PDSP6
// and connects them to the interrupt controller of the ARM. It does not
// initialise the interrupts on the PDSP6. The parameter is the CBUS
// interface reference.

void IRQ_CBUSInit(uint8_t uInterface)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  EXTI_InitTypeDef EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  // We subtract 1 because the interface references (starting from 1) are stored in an array
  // starting with offset 0..
  // uInterface--;

  /* Enable the BUTTON Clock */
  RCC_APB1PeriphClockCmd(CBUS_IRQ_CLK[uInterface], ENABLE);

  /* Configure Button pin as input */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  //GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Pin = CBUS_IRQ_PIN[uInterface];
  GPIO_Init(CBUS_IRQ_PORT[uInterface], &GPIO_InitStructure);

  GPIO_EXTILineConfig(CBUS_IRQ_PORT_SOURCE[uInterface], CBUS_IRQ_PIN_SOURCE[uInterface]);

  /* Configure Button EXTI line */
  EXTI_InitStructure.EXTI_Line = CBUS_IRQ_EXTI_LINE[uInterface];
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  // Enable and set the CBUS Interrupt priority
  NVIC_InitStructure.NVIC_IRQChannel = CBUS_IRQn[uInterface];
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0xFF;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0xFF;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

  // Enable peripheral interrupts.
  NVIC_Init(&NVIC_InitStructure);

}

// This specifically sets the priority of the PendSV interrupt. It sets the field
// PRI_14 in the System Handler Priority Register. Note that if priority grouping
// is enabled then the parameter is an amalgam of the pre-emption and sub
// priority bits.
void PendSV_Priority (uint8_t uPriority)
{
	SCB->SHP[10] = uPriority;
}


// This sets the priority of the SysTick. It has previously been set by SysTick_Config,
// using generic routines, but this short routine allows us to write the priority directly.
void SysTick_Priority (uint8_t uPriority)
{
	SCB->SHP[11] = uPriority;
}

// Set or clear the PendSV bit in the Interrupt control state register by using SET or RESET.

void PendSV (FlagStatus State)
{
	if (State == SET)
	{
		SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
	}
	else
	{
		SCB->ICSR |= SCB_ICSR_PENDSVCLR_Msk;
	}
}

void USART1_IRQHandler(void)
{
#ifdef CML_GUI
  UART_rx_buffer[UART_rb_inptr++] = (USART1->DR);       //read byte from USART1, put into circular buffer
  if (UART_rb_inptr == UART_RX_BUFFER_SIZE)             //check for buffer wrap.
    UART_rb_inptr = 0; 
  UART_rb_count++;                                      //increment count
  return;
#endif
}
