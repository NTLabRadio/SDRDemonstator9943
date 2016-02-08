/************************************************************************************************************
 *
 *  $Copyright © 2012-2014 CML Systems $
 *
 *  $Header: stm32f10x_sdr.c  Revision:1.15.1.3  02 July 2014 17:53:28  ilewis $
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

// Includes

#include "stm32f10x_sdr.h"
#include "stdlib.h"

#ifdef TARGET_DMR
 #ifdef DEBUG_CHANGE_INC_FROM_INITIAL_SDR_PROJECT
	#include "..\DMR\mainDMR.h"
 #else
  #include "mainDMR.h"
 #endif
extern DMR_TypeDef dmr;
#else 
 #ifdef TARGET_PMR
 #include "mainPMR.h" 
extern PMR_TypeDef pmr;
 #endif
#endif


// Local function prototypes.

void TIM2_Init (void);
void TIM3_Init (void);
// Timer for CMX7161 timeouts.
void TIM4_Init (void);
// Timer for CMX7262 timeouts.
void TIM5_Init(void);

void FT232_USART_Init (void);
uint32_t FT232_USART_TIMEOUT_UserCallback(void);

void CBUS_Init (void);
uint32_t CBUS_TIMEOUT_UserCallback(void);

void PortE_Init (void);
void SDR_Dummy_IRQ (void *pData);

void PendSV_Priority (uint8_t uPriority);
void SysTick_Priority (uint8_t uPriority);


// Table for CBUS interrupt handlers.
CBUS_IRQ_TypeDef CBUS_IRQTable[CBUS_NUMBER_OF_INTERFACES];

// Global within the scope of this file so that all the init routines can configure GPIO.
static GPIO_InitTypeDef  GPIO_InitStructure;

__IO uint32_t  FT232_USARTTimeout = FT232_USART_FLAG_TIMEOUT;
__IO uint32_t  CBUSTimeout = CBUS_TIME_OUT;

// Collects all the hardware initialisation for the SDR card, GPIO, Timers, USART, CBUS
// into one function call which can be made from either the script or standalone DMR
// application.

void SDR_Init( void )
{

  // Initialise to a dummy function to start with. This is a table of function pointers
  // which in our case point to the interrupt handlers.
  CBUS_IRQTable[CBUS_INTERFACE_1].pHandler = SDR_Dummy_IRQ;
  CBUS_IRQTable[CBUS_INTERFACE_2].pHandler = SDR_Dummy_IRQ;

   // Initialise 2 * 16 bit timers to replicate the 32 bit Hyperstone timer used for script delays.
  TIM2_Init();
  TIM3_Init();
  // Timers for Transcode and Modem Mode timeouts.
  TIM4_Init();
  TIM5_Init();

  // Initialise the CBUS interface.
  CBUS_Init();

  // Initialise the FTDI USART interface.
  FT232_USART_Init();

  // Enable Test port (GPIOE) for the LEDs and test loop connections.
  PortE_Init();
  GPIO_SetBits(GPIOE ,AUDIO_PA);

}

// Provides initialisation for the debug port, which has the two LEDs and five test loops
// and the audio PA attached.

void PortE_Init (void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  // Enable Test port (GPIOE) for the LEDs and test loop connections.
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
  GPIO_InitStructure.GPIO_Pin = TEST_RED_LED_D8 | TEST_GREEN_LED_D7 | TEST_TL1 | TEST_TL2 | TEST_TL3 | TEST_TL4 | TEST_TL5 | AUDIO_PA;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(TEST_PORT, &GPIO_InitStructure);
}

#define	LOAD_FI_END	0

// This function is designed to load an FI defined in a FI header file down the CBUS onto
// a PDSP6. The FI is split up into data blocks - each with a header. The parameter is
// a reference to the CBUS interface.

#ifdef TARGET_DMR	
//CMX7161 and CMX7262 use this style of bootloader
uint16_t SDR_Load_FI (cmxFI_TypeDef *pFI, uint8_t uInterface )
{
	uint16_t	*pData;
	uint16_t 	start_code;
	uint16_t 	length;
	uint16_t	data;
	uint16_t	state;

	uint16_t	uTxFIFOCount;
        
        char tempstr[5];        //used to comvers the version from hex to decimal

	uTxFIFOCount = 0;
	data = 0;
	state = 0;
	// Write a general reset to the pDSP6
	CBUS_Write8(1, (uint8_t *)&data, 0, uInterface);
	// Wait for a second.
	ClearCounter();
	while (ReadCounter() < 1000000);

	// Read the FIFO output level. It should be 3
	CBUS_Read8 (0x4F,(uint8_t*)&data,1,uInterface);

	// Read the 3 device check words.
	if (data == 3)
	{
		CBUS_Read16 (0x4D,&data,1,uInterface);
		CBUS_Read16 (0x4D,&data,1,uInterface);
		CBUS_Read16 (0x4D,&data,1,uInterface);
	}
	else
	{
		// If there are no device check words, we return a failure.
		return 0;
	}
	// Initialise the data pointer to the start of the FI image.
	pData = pFI->db1_start_address;
	while (1)
	{
			switch (state)
			{
				case 0:

					// For each of the data blocks set the start codes and length from the definitions in
					// the FI header file. The address of pData is used to identify the current data block.
					if (pData==pFI->db1_start_address)
					{
						start_code = pFI->db1_ptr;
						length = pFI->db1_len;
					}
					else if (pData==pFI->db2_start_address)
					{
						start_code =  pFI->db2_ptr;
						length = pFI->db2_len;
					}
					else if (pData==LOAD_FI_END)
					{
						start_code = pFI->activate_ptr;
						length = pFI->activate_len;
					}
					else
					{
						// If the data pointer setting is not recognised then we have an error condition.
						// So return with an error code.
						return 0;
					}
					state++;
					break;

				case 1:

					CBUS_Write16(0x49, &length,1,uInterface);
					CBUS_Write16(0x49, &start_code,1,uInterface);

					// At this point, if the length is 0 then we have reached the end of the data blocks and the
					// FI should be loaded, so return 1.
					if (length == 0)
					{
						// Check the FI has programmed by checking the programming bit - protect the loop
						// with a 1 second time out. Return 1 for success or 0 for failure..

						ClearCounter();
						while (ReadCounter()<1000000)
						{                                                  
							CBUS_Read16 (0x7E,&data,1,uInterface);
							if (data & 0x4000 && (pFI->type==CMX7262FI))
							{
								//read version code
                                                                CBUS_Read16 (0x4D,&data,1,uInterface);  //device type; ignore
                                                                CBUS_Read16 (0x4D,&data,1,uInterface);  //version (in hex) e.g. version 1.0.0.0 will be 0x1000
                                                                sprintf(tempstr,"%X",data);             //convert to string "1000"                                                  
                                                                dmr.Cmx7262.FIVersion = atoi(tempstr);  //convert to decimal 1000 for display
  
                                                                return 1;
							}
							if (data & 0x4000 && (pFI->type==CMX7161FI))
							{
								//read version code
                                                                CBUS_Read16 (0x4D,&data,1,uInterface);
                                                                CBUS_Read16 (0x4D,&data,1,uInterface);
                                                                sprintf(tempstr,"%X",data);                                                                
                                                                dmr.Cmx7161.FIVersion = atoi(tempstr);
                                                                return 1;
							}
						}
						// Program Failure.
						return 0;
					}
					else
					{
						// Move to next state to programme the target device with
						// the function image.
						state++;
					}

	 				break;

				case 2:
					// There is a 128 deep FIFO on the CBUS interface which allows us to write
					// 128 words before checking the level register (0x4B). Check the level
					// register if the Tx FIFO count is 128. If the level does not reach zero in 1 second
					// we return an error code.
					if (uTxFIFOCount >= 128)
					{
						if (!CBUS_WaitBitClr8(0x4B,0xFF,uInterface))
							// Input FIFO level failure.
							return 0;
						uTxFIFOCount = 0;
					}
					CBUS_Write16(0x49,pData++,1,uInterface);
					length--;
					uTxFIFOCount++;

					// When the length is 0 we will be at the end of the current data block, so we
					// then work out which data block it is and set the data pointer to the next data
					// block or to END. If length is not 0 then there is still more data block to output.

					if (length == 0)
					{
						// If we are at the end of block 1 set the data pointer to the start of block 2.
						// Check if we are at the end of Block 1. If we are ....
						if (pData==(pFI->db1_start_address+pFI->db1_len))
						{
							// Read the level register but doing nothing with it.
							CBUS_Read8 (0x4F,(uint8_t *)&data,1,uInterface);
							// We are at the end of block 1, so read the checksums for the block and
							// return a failure if they are wrong.
							CBUS_Read16 (0x4D,&data,1,uInterface);
							if (data != pFI->db1_chk_hi)
								return 0;
							CBUS_Read16 (0x4D,&data,1,uInterface);
							if(data != pFI->db1_chk_lo)
								return 0;
							pData = pFI->db2_start_address;
						}
						// If we are at the end of block 2 set the data pointer to END.
						else if (pData==(pFI->db2_start_address+pFI->db2_len))
						{
							// Read the level register but doing nothing with it.
							CBUS_Read8 (0x4F,(uint8_t *)&data,1,uInterface);
							// We are at the end of block 2, so read the checksums for the block and
							// return a failure if they are wrong.
							CBUS_Read16 (0x4D,&data,1,uInterface);
							if (data != pFI->db2_chk_hi)
								return 0;
							CBUS_Read16 (0x4D,&data,1,uInterface);
							if(data != pFI->db2_chk_lo)
								return 0;
							pData = LOAD_FI_END;
						}
						state=0;
					}
					else
						state=2;
					break;
			}
		}

}
#else
#ifdef TARGET_PMR
//CMX7141 needs a different style of bootload
uint16_t wait_prog_bit(uint16_t uInterface, uint32_t timeout);

uint16_t SDR_Load_FI_7x4x (cmxFI_TypeDef *pFI, uint8_t uInterface )
{
	uint16_t	*pData;
	uint16_t 	start_code;
	uint16_t 	length;
	uint16_t	data;
	uint16_t	state;
	uint16_t	bootmode;
	//uint16_t	uTxFIFOCount;
        
        char tempstr[5];        //used to convert the version from hex to decimal

	//uTxFIFOCount = 0;
	data = 0;
	state = 0;
        bootmode = 1;   //cbus boot
	// Write a general reset to the pDSP6
	CBUS_Write8(1, (uint8_t *)&data, 0, uInterface);
	// Wait for a second.
	//ClearCounter();
	//while (ReadCounter() < 1000000);
        
        if (wait_prog_bit(uInterface, 1000000) == 0) return 0;

	// Initialise the data pointer to the start of the FI image.
	pData = pFI->db1_start_address;
	while (1)
	{
			switch (state)
			{
				case 0:

					// For each of the data blocks set the start codes and length from the definitions in
					// the FI header file. The address of pData is used to identify the current data block.
					if (pData==pFI->db1_start_address)
					{
						start_code = pFI->db1_ptr;
						length = pFI->db1_len;
					}
					else if (pData==pFI->db2_start_address)
					{
						start_code =  pFI->db2_ptr;
						length = pFI->db2_len;
					}
					else if (pData==LOAD_FI_END)
					{
						start_code = pFI->activate_ptr;
						length = pFI->activate_len;
					}
					else
					{
						// If the data pointer setting is not recognised then we have an error condition.
						// So return with an error code.
						return 0;
					}
					state++;
					break;

				case 1:
					CBUS_Write16(0xB6, &start_code,1,uInterface);
					CBUS_Write16(0xB7, &length,1,uInterface);                                        
                                        //if (wait_prog_bit(uInterface, 1000000) == 0) return 0;

                                        CBUS_Write16(0xC8, &bootmode, 1, uInterface); 					
                                        
                                        // At this point, if the length is 0 then we have reached the end of the data blocks and the
					// FI should be loaded, so read version, do checksums, activate and return
					if (length == 0)
					{
                                          if (wait_prog_bit(uInterface, 1000000) == 0) return 0;                                                                                   
                                          //read version code
                                          CBUS_Read16 (0xC9,&data,1,uInterface);  //version (in hex) e.g. version 1.0.0.0 will be 0x1000
                                          sprintf(tempstr,"%X",data);             //convert to string "1000"                                                  
                                          pmr.Cmx7141.FIVersion = atoi(tempstr);  //convert to decimal 1000 for display
					  //check the checksums	
                                          CBUS_Read16 (0xB8,&data,1,uInterface);
                                          if (data != pFI->db1_chk_hi)
                                                  return 0;
                                          
                                          CBUS_Read16 (0xB9,&data,1,uInterface);
                                          if (data != pFI->db1_chk_lo)
                                                  return 0;                                          

                                          CBUS_Read16 (0xA9,&data,1,uInterface);
                                          if (data != pFI->db2_chk_hi)
                                                  return 0;
                                          
                                          CBUS_Read16 (0xAA,&data,1,uInterface);
                                          if (data != pFI->db2_chk_lo)
                                                  return 0; 
                                         
                                          //load activation codes
                                          CBUS_Write16(0xC8,&pFI->activation_code1,1,uInterface); 
                                          if (wait_prog_bit(uInterface, 100000) == 0) return 0;
                                         
                                          CBUS_Write16(0xC8,&pFI->activation_code2,1,uInterface); 
                                          if (wait_prog_bit(uInterface, 100000) == 0) return 0;        
                                          
                                          //check for successful activation
                                          CBUS_Read16 (0xB8,&data,1,uInterface);
                                          if (data == 0xDEAD)
                                                  return 0;                                          
          
                                          return 1;

					}
					else
					{
						// Move to next state to programme the target device with
						// the function image.
						state++;
					}

	 				break;

				case 2:
                                        if (wait_prog_bit(uInterface, 100000) == 0) return 0;
                                        CBUS_Write16(0xC8,pData++,1,uInterface);   			                                        
                                        length--;

					// When the length is 0 we will be at the end of the current data block, so we
					// then work out which data block it is and set the data pointer to the next data
					// block or to END. If length is not 0 then there is still more data block to output.

					if (length == 0)
					{
						// If we are at the end of block 1 set the data pointer to the start of block 2.
						// Check if we are at the end of Block 1. If we are ....
						if (pData==(pFI->db1_start_address+pFI->db1_len))
						{
							pData = pFI->db2_start_address;
						}
						// If we are at the end of block 2 set the data pointer to END.
						else if (pData==(pFI->db2_start_address+pFI->db2_len))
						{
							pData = LOAD_FI_END;
						}
						state=0;
					}
					else
						state=2;
					break;

			}
		}

}

//wait specified number of us for prog bit ($C6 b0)
//return 1 if bit set, 0 for timed out
uint16_t wait_prog_bit(uint16_t uInterface, uint32_t timeout)
{
  uint16_t data;
  
  ClearCounter();
  while (ReadCounter()<timeout)
  {                                                  
    CBUS_Read16 (0xC6,&data,1,uInterface);
    if (data & 0x0001) return 1;
  }
  return 0;  
}

#endif
#endif


// Delay Routines - Start

void TIM_ResetSMCR(TIM_TypeDef* TIMx)
{
   /* Select the Slave Mode */
  TIMx->SMCR = 0;
}

// Delay Routines - Start

// Timer 2 clocks Timer 3. Timer 2 is setup with a 1uS clock which wraps at 65536.
// When timer 2 wraps, it provides one tick of timer 3. So timer 3 is being clocked
// every 65.5mS (approx 15.2Hz). The combination of these timers create a 32 bit
// hardware counter being ticked at 1uS. These are used for delays in the code.
// The reality is a counter which wraps at 1hr 11mins.

/**
  * @brief  Configures the TIM Peripheral.
  * @param  None
  * @retval None
  */
void TIM2_Init(void)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

  /* --------------------------- System Clocks Configuration -----------------*/
  /* TIM2 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  /* Compute the prescaler value */
  // The timer is clocked at 1MHz (1uS period).
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 65535;
  TIM_TimeBaseStructure.TIM_Prescaler = (uint16_t) ((SystemCoreClock) / 1000000) - 1;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

  // Ensures an update event is generated.
  TIM_UpdateDisableConfig(TIM2, DISABLE);

  // TIM2 set into master mode - update event is selected as trigger output (TRGO)
  TIM_SelectOutputTrigger(TIM2,TIM_TRGOSource_Update);

  /* Enable TIM2 Preload register on ARR */
  TIM_ARRPreloadConfig(TIM2, ENABLE);

   /* TIM2 enable counter */
  TIM_Cmd(TIM2, ENABLE);

}

void TIM3_Init(void)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

  /* --------------------------- System Clocks Configuration -----------------*/
  /* TIM3 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  // The end event from Timer 2 acts as the pre-scaler count for Timer 3.
  TIM_TimeBaseStructure.TIM_Period = 65535;
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

   /* Enable TIM3 Preload register on ARR */
  TIM_ARRPreloadConfig(TIM3, ENABLE);

  // Set SMCR register
  // Selects internal trigger for TIM3 from TIM2 and sets TIM3
  // slave controller into external clock mode.
  TIM_ITRxExternalClockConfig(TIM3,TIM_TS_ITR1);

  /* TIM3 enable counter */
  TIM_Cmd(TIM3, ENABLE);
}

/**
  * @brief  Configures the TIM Peripheral.
  * @param  None
  * @retval None
  */
// Since there are 8 timers on the Cortex M3, this timer is dedicated to monitoring
// timeouts of bits on the CMX7161 staus register.

void TIM4_Init(void)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

  /* --------------------------- System Clocks Configuration -----------------*/
  /* TIM4 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

  /* Compute the prescaler value */
  // The timer is clocked at 10kHz (100uS period).
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 65535;
  TIM_TimeBaseStructure.TIM_Prescaler = (uint16_t) ((SystemCoreClock) / 100000) - 1;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

   /* Enable TIM4 Preload register on ARR */
  TIM_ARRPreloadConfig(TIM4, ENABLE);

   /* TIM4 enable counter */
  TIM_Cmd(TIM4, ENABLE);

}

/**
  * @brief  Configures the TIM Peripheral.
  * @param  None
  * @retval None
  */

// Since there are 8 timers on the Cortex M3, this timer is dedicated to monitoring
// timeouts of bits on the CMX7262 staus register.

void TIM5_Init(void)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

  /* --------------------------- System Clocks Configuration -----------------*/
  /* TIM5 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

  /* Compute the prescaler value */
  // The timer is clocked at 100kHz (10uS period).
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 65535;
  TIM_TimeBaseStructure.TIM_Prescaler = (uint16_t) ((SystemCoreClock) / 100000) - 1;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);

   /* Enable TIM5 Preload register on ARR */
  TIM_ARRPreloadConfig(TIM5, ENABLE);

   /* TIM5 enable counter */
  TIM_Cmd(TIM5, ENABLE);

}

// Delay Routines - End

// This routine reads back two timers and combines the result into one 32 bit value
// ticked at 1uS. Using ReadCounter and ClearCounter the combination of these timers
// create a 32 bit hardware counter being ticked at 1uS. These are used for delays in the
// code. The reality is a counter which wraps at 1hr 11mins.

uint32_t ReadCounter (void)
{
	uint32_t uCountResult = 0;
	uCountResult = TIM_GetCounter(TIM3) << 16;
	uCountResult = uCountResult | TIM_GetCounter(TIM2);
	return uCountResult;
}


void ClearCounter(void)
{
	TIM_SetCounter(TIM2,0);
	TIM_SetCounter(TIM3,0);
}

// Delay Routines - End


// CBUS Routines - Start

/**
  * @brief  Initializes the low level interface used to drive the CBUS
  * @param  None
  * @retval None
  */
void CBUS_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;

  /* Enable the SPI periph */
  RCC_APB2PeriphClockCmd(CBUS_SPI_CLK, ENABLE);

  /* Enable SCK, MOSI and MISO GPIO clocks */
  RCC_APB2PeriphClockCmd(CBUS_SPI_SCK_GPIO_CLK | CBUS_SPI_MISO_GPIO_CLK | CBUS_SPI_MOSI_GPIO_CLK, ENABLE);

  /* Enable CS  GPIO clock */
  RCC_APB2PeriphClockCmd(CBUS_SPI_CS_GPIO_CLK, ENABLE);

  GPIO_InitStructure.GPIO_Pin = CBUS_SPI_SCK_PIN | CBUS_SPI_MOSI_PIN | CBUS_SPI_MISO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(CBUS_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);
  
  
  GPIO_InitStructure.GPIO_Pin = CBUS_SPI_MISO_PIN;  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(CBUS_SPI_SCK_GPIO_PORT, &GPIO_InitStructure); 
  
  /* SPI SCK pin configuration */
  //GPIO_InitStructure.GPIO_Pin = CBUS_SPI_SCK_PIN;
  //GPIO_Init(CBUS_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

  /* SPI  MOSI pin configuration */
  //GPIO_InitStructure.GPIO_Pin =  CBUS_SPI_MOSI_PIN;
  //GPIO_Init(CBUS_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

  /* SPI MISO pin configuration */
  //GPIO_InitStructure.GPIO_Pin = CBUS_SPI_MISO_PIN;
  //GPIO_Init(CBUS_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

  /* SPI configuration -------------------------------------------------------*/
  SPI_I2S_DeInit(CBUS_SPI);
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  // The BaudRatePrescalar below sets the CBUS clock speed.
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;		// 8MHz approximately.
  //SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;	// 4MHz approximately.
  //SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;	// 1MHz approximately.
  //SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;	// 500kHz approximately.
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_Init(CBUS_SPI, &SPI_InitStructure);

  /* Enable SPI1  */
  SPI_Cmd(CBUS_SPI, ENABLE);

  // DAVED
  // On the discovery board, the chip select for the accelerometer
  // is located on Port E. Therefore there will be no conflict with using
  // chip selects on port A. We configure three chip selects for the three
  // CBUS interfaces.

  /* Configure GPIO PINs for CBUS Chip Selects */
  GPIO_InitStructure.GPIO_Pin = CBUS_SPI_CS1_PIN | CBUS_SPI_CS2_PIN | CBUS_SPI_CS3_PIN;
  //GPIO_InitStructure.GPIO_Pin = CBUS_SPI_CS1_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  //GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  //GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
  GPIO_Init(CBUS_SPI_CS_GPIO_PORT, &GPIO_InitStructure);

  /* Deselect : Chip Select high */
  GPIO_SetBits(CBUS_SPI_CS_GPIO_PORT, CBUS_SPI_CS1_PIN);
  GPIO_SetBits(CBUS_SPI_CS_GPIO_PORT, CBUS_SPI_CS2_PIN);
  GPIO_SetBits(CBUS_SPI_CS_GPIO_PORT, CBUS_SPI_CS3_PIN);

  /* Configure GPIO PINs for CBUS Status \ IRQs */
  GPIO_InitStructure.GPIO_Pin = CBUS_IRQN1_PIN | CBUS_IRQN3_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  //GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  //GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_Init(CBUS_SPI_CS_GPIO_PORT, &GPIO_InitStructure);

}


/**
  * @brief  Writes one byte to the CBUS.
  * @param  pBuffer : pointer to the buffer  containing the data to be written to the CBUS.
  * @param  WriteAddr : CBUS's internal address to write to.
  * @param  NumByteToWrite: Number of bytes to write.
  * @retval None
  */

void CBUS_Write16(uint8_t uAddress, uint16_t *data_ptr, uint16_t uAccesses, uint8_t uInterface)
{
	uint16_t uAccessCount;

	// An IRQ routine includes a CBUS access to clear the status register. Therefore
	// we do not want a none IRQ CBUS accesses to be disturbed. Otherwise there
	// could be problems with the chip select lines directing MISO\MOSI to the wrong
	// CBUS interface. Therefore, we disable interrupts for the duration of the CBUS
	// access.

	__disable_irq();


	/* Set chip select Low at the start of the transmission */
	CBUS_SetCSNLow(uInterface);

	/* Send the Address of the register */
	CBUS_SendByte(uAddress);

	for (uAccessCount=0; uAccessCount < uAccesses; uAccessCount++)
	{
		/* Send the data that will be written into the device (MSB First) */
		CBUS_SendByte((*data_ptr & 0xFF00)>>8);
		CBUS_SendByte(*data_ptr & 0x00FF);
		data_ptr++;
	}

	/* Set chip select High at the end of the transmission */
	CBUS_SetCSNHigh(uInterface);

	__enable_irq();

}


void CBUS_Read16 (uint8_t uAddress, uint16_t *data_ptr, uint16_t uAccesses, uint8_t uInterface)
{
	uint16_t uAccessCount;

	*data_ptr=0;

	// An IRQ routine includes a CBUS access to clear the status register. Therefore
	// we do not want a none IRQ CBUS accesses to be disturbed. Otherwise there
	// could be problems with the chip select lines directing MISO\MOSI to the wrong
	// CBUS interface. Therefore, we disable interrupts for the duration of the CBUS
	// access.

	__disable_irq();

	/* Set chip select Low at the start of the transmission */

	CBUS_SetCSNLow(uInterface);

	// Setup the CBUS address to read from.
    CBUS_SendByte(uAddress);

	for (uAccessCount = 0; uAccessCount < uAccesses; uAccessCount++)
	{
		*data_ptr=0;
		 // Send dummy byte (0x00) to generate the SPI clock to CBUS (Slave device).
		 // This reads the first byte for 2 byte accesses.
		 *data_ptr = (CBUS_SendByte(CBUS_DUMMY_BYTE) << 8);

		 // Send dummy byte (0x00) to generate the SPI clock to CBUS (Slave device).
		 // This reads the second byte (for 2 byte accesses) or the first byte for single byte
		 // reads.
		 *data_ptr |= CBUS_SendByte(CBUS_DUMMY_BYTE);
		data_ptr++;
	}

	 /* Set chip select High at the end of the transmission */
	CBUS_SetCSNHigh(uInterface);

	__enable_irq();

}

// Supports byte wide CBUS streaming and efficient packing of bytes.

void CBUS_Write8(uint8_t uAddress, uint8_t *data_ptr, uint16_t uAccesses, uint8_t uInterface)
{
	uint16_t uAccessCount;

	// An IRQ routine includes a CBUS access to clear the status register. Therefore
	// we do not want a none IRQ CBUS accesses to be disturbed. Otherwise there
	// could be problems with the chip select lines directing MISO\MOSI to the wrong
	// CBUS interface. Therefore, we disable interrupts for the duration of the CBUS
	// access.

	__disable_irq();

	/* Set chip select Low at the start of the transmission */
	CBUS_SetCSNLow(uInterface);

	/* Send the Address of the register */
	CBUS_SendByte(uAddress);

	for (uAccessCount=0; uAccessCount < uAccesses; uAccessCount++)
	{
		/* Send the data that will be written into the device  */
		CBUS_SendByte(*data_ptr);
		data_ptr++;
	}

	/* Set chip select High at the end of the transmission */
	CBUS_SetCSNHigh(uInterface);

	__enable_irq();

}


// Supports byte wide CBUS streaming and efficient packing of bytes.

void CBUS_Read8 (uint8_t uAddress, uint8_t *data_ptr, uint16_t uAccesses, uint8_t uInterface)
{
	uint16_t uAccessCount;

	*data_ptr=0;

	// An IRQ routine includes a CBUS access to clear the status register. Therefore
	// we do not want a none IRQ CBUS accesses to be disturbed. Otherwise there
	// could be problems with the chip select lines directing MISO\MOSI to the wrong
	// CBUS interface. Therefore, we disable interrupts for the duration of the CBUS
	// access.

	__disable_irq();

	/* Set chip select Low at the start of the transmission */
	CBUS_SetCSNLow(uInterface);

	// Setup the CBUS address to read from.
    CBUS_SendByte(uAddress);

	for (uAccessCount = 0; uAccessCount < uAccesses; uAccessCount++)
	{
		// Send dummy byte (0x00) to generate the SPI clock to CBUS (Slave device).
		// This reads the first byte for single byte  reads.
		*data_ptr = CBUS_SendByte(CBUS_DUMMY_BYTE);
		data_ptr++;
	}

	 /* Set chip select High at the end of the transmission */
	CBUS_SetCSNHigh(uInterface);

	__enable_irq();

}



// Continually read a CBUS register until the bits in the mask are set or the 1 second
// timeout expires. Returns 1 when the mask is matched. Otherwise 0 for a timeout.

uint16_t	CBUS_WaitBitSet16 (uint8_t Address, uint16_t uMask, uint8_t uInterface)
{
	uint16_t data;

	ClearCounter();
	while (ReadCounter()<1000000)
	{
		data = 0;
		CBUS_Read16 (Address,&data,1,uInterface);
		if (uMask == (data & uMask))
		{
			return data;
		}
	}
	return 0;
}

// Continually read a CBUS register until the bits in the mask are clear or the 1 second
// timeout expires. Returns 1 when the bits in the mask are clear. Otherwise 0 for a timeout.

uint16_t	CBUS_WaitBitClr8(uint8_t Address, uint8_t uMask, uint8_t uInterface)
{
	uint8_t data;

	ClearCounter();
	// Timeout set at 1 second.
	while (ReadCounter()<1000000)
	{
		data = 0;
		CBUS_Read8 (Address,&data,1,uInterface);
		if ((data & ~uMask)==0)
		{
			return 1;
		}
	}
	return 0;
}


/**
  * @brief  Sends a Byte through the SPI interface and return the Byte received
  *         from the SPI bus.
  * @param  Byte : Byte send.
  * @retval The received byte value
  */
uint8_t CBUS_SendByte(uint8_t byte)
{
  /* Loop while DR register in not emplty */
  CBUSTimeout = CBUS_TIME_OUT;
  while (SPI_I2S_GetFlagStatus(CBUS_SPI, SPI_I2S_FLAG_TXE) == RESET)
  {
    if((CBUSTimeout--) == 0) return CBUS_TIMEOUT_UserCallback();
  }

  /* Send a Byte through the SPI peripheral */
  SPI_I2S_SendData(CBUS_SPI, byte);

   /* Wait to receive a Byte */
  CBUSTimeout = CBUS_TIME_OUT;
  while (SPI_I2S_GetFlagStatus(CBUS_SPI, SPI_I2S_FLAG_RXNE) == RESET)
  {
    if((CBUSTimeout--) == 0) return CBUS_TIMEOUT_UserCallback();
  }

  /* Return the Byte read from the SPI bus */
  return (uint8_t)SPI_I2S_ReceiveData(CBUS_SPI);
}

/*
 * Set the CSN bit low based on the mask.
 */

void CBUS_SetCSNLow(uint8_t uMask)
{
	if((uMask & 0x03) == 0)
	{
		CBUS_CSN1_LOW();
	}
	else if ((uMask & 0x03) == 1)
	{
		CBUS_CSN2_LOW();
	}
	else if ((uMask & 0x03) == 2)
	{
		CBUS_CSN3_LOW();
	}

	// If none of the above are true we do not touch the CSN (chip select) lines.
}

/*
 * Set the CSN bit high based on the mask.
 */

void CBUS_SetCSNHigh(uint8_t uMask)
{
	if((uMask & 0x03) == 0)
	{
		CBUS_CSN1_HIGH();
	}
	else if ((uMask & 0x03) == 1)
	{
		CBUS_CSN2_HIGH();
	}
	else if ((uMask & 0x03) == 2)
	{
		CBUS_CSN3_HIGH();
	}

	// If none of the above are true we do not touch the CSN (chip select) lines.

}


/**
  * @brief  Basic management of the timeout situation.
  * @param  None.
  * @retval None.
  */
uint32_t CBUS_TIMEOUT_UserCallback(void)
{
	//GPIO_SetBits(GPIOD,GPIO_Pin_14);		// Light the Red LED on a CBUS failure.
	// Delaty the toggle by 0.5s
	//Delay(50);
	return 0;
}

// CBUS Routines - End


// USART Routines - Start

/**
  * @brief  Function to configure software to drive the FT232RL (FTDI UART) chip.
  * @param  None
  * @retval None
  */

void FT232_USART_Init(void)
{
	USART_InitTypeDef USART_InitStructure;

	// Enable the FT232 USART periph
	RCC_APB2PeriphClockCmd(FT232_USART_CLOCK, ENABLE);

	// Enable TX, RX, CTS and RTS clocks
	RCC_APB2PeriphClockCmd(FT232_USART_TX_GPIO_CLK | FT232_USART_RX_GPIO_CLK | FT232_USART_CTS_GPIO_CLK | FT232_USART_RTS_GPIO_CLK, ENABLE);

	// Map the USART interface to the IO pins on port A (Alternative function)
	//GPIO_PinAFConfig(FT232_USART_TX_GPIO_PORT,FT232_USART_TX_SOURCE,FT232_USART_TX_AF);
	//GPIO_PinAFConfig(FT232_USART_RX_GPIO_PORT,FT232_USART_RX_SOURCE,FT232_USART_RX_AF);
	//GPIO_PinAFConfig(FT232_USART_CTS_GPIO_PORT,FT232_USART_CTS_SOURCE,FT232_USART_CTS_AF);
	//GPIO_PinAFConfig(FT232_USART_RTS_GPIO_PORT,FT232_USART_RTS_SOURCE,FT232_USART_RTS_AF);

	GPIO_InitStructure.GPIO_Pin =  FT232_USART_RX_PIN | FT232_USART_CTS_PIN;
        //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
        
        //GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        //GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(FT232_USART_TX_GPIO_PORT, &GPIO_InitStructure);
        


        GPIO_InitStructure.GPIO_Pin =  FT232_USART_TX_PIN | FT232_USART_RTS_PIN | GPIO_Pin_8;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
        GPIO_Init(FT232_USART_TX_GPIO_PORT, &GPIO_InitStructure);
        
	/* TX pin configuration */
	//GPIO_InitStructure.GPIO_Pin = FT232_USART_TX_PIN ;
	//GPIO_Init(FT232_USART_TX_GPIO_PORT, &GPIO_InitStructure);

	/* RX pin configuration */
	//GPIO_InitStructure.GPIO_Pin = FT232_USART_RX_PIN ;
	//GPIO_Init(FT232_USART_RX_GPIO_PORT, &GPIO_InitStructure);

	/* CTS pin configuration */
	//GPIO_InitStructure.GPIO_Pin = FT232_USART_CTS_PIN ;
	//GPIO_Init(FT232_USART_CTS_GPIO_PORT, &GPIO_InitStructure);

	/* RTS pin configuration */
	//GPIO_InitStructure.GPIO_Pin = FT232_USART_RTS_PIN ;
	//GPIO_Init(FT232_USART_RTS_GPIO_PORT, &GPIO_InitStructure);

	/* USART configuration -------------------------------------------------------*/
	USART_DeInit(FT232_USART);
	/* USART_InitStruct members default value */
	USART_InitStructure.USART_BaudRate =1250000;// 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//USART_HardwareFlowControl_RTS_CTS;//
	USART_Init(FT232_USART, &USART_InitStructure);
	/* Enable FT232 USART  */
	USART_Cmd(FT232_USART, ENABLE);

}


void FT232_USART_Write(uint8_t* pBuffer, uint16_t NumByteToWrite)
{
  /* Send the data that will be written into the device (MSB First) */
  while(NumByteToWrite >= 0x01)
  {
    FT232_USART_SendByte(*pBuffer);
    NumByteToWrite--;
    pBuffer++;
  }
}


void FT232_USART_Read(uint8_t* pBuffer,  uint16_t NumByteToRead)
{
  /* Receive the data that will be read from USART */
  while(NumByteToRead > 0x00)
  {
     *pBuffer = FT232_USART_ReadByte();
    NumByteToRead--;
    pBuffer++;
  }
}


/**
  *
  * When the transmit buffer is empty a byte is written to the USART.
  * Returns 0 on a timeout.
  *
  */

uint8_t FT232_USART_SendByte(uint8_t byte)
{
  /* Loop while DR register in not emplty */
  FT232_USARTTimeout = FT232_USART_FLAG_TIMEOUT;
  while (USART_GetFlagStatus(FT232_USART, USART_FLAG_TXE) == RESET)
  {
    if((FT232_USARTTimeout--) == 0) return (uint8_t)FT232_USART_TIMEOUT_UserCallback();
  }
  /* Send a Byte through the USART peripheral */
  USART_SendData(FT232_USART, byte);
  return 1;
}



uint8_t FT232_USART_ReadByte(void)
{
/* Wait to receive a Byte */
FT232_USARTTimeout = FT232_USART_FLAG_TIMEOUT;
while (USART_GetFlagStatus(FT232_USART,USART_FLAG_RXNE) == RESET)
{
 if((CBUSTimeout--) == 0) return (uint8_t)FT232_USART_TIMEOUT_UserCallback();
}
/* Return the Byte read from the SPI bus */
return (uint8_t)USART_ReceiveData(FT232_USART);
}

/**
  * @brief  Toggles the specified GPIO pins..
  * @param  GPIOx: where x can be (A..I) to select the GPIO peripheral.
  * @param  GPIO_Pin: Specifies the pins to be toggled.
  * @retval None
  */
void GPIO_ToggleBits(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
  /* Check the parameters */
  assert_param(IS_GPIO_ALL_PERIPH(GPIOx));

  GPIOx->ODR ^= GPIO_Pin;
}

/**
  * @brief  Basic management of the timeout situation.
  * @param  None.
  * @retval None.
  */
uint32_t FT232_USART_TIMEOUT_UserCallback(void)
{
	GPIO_ToggleBits(GPIOD,GPIO_Pin_12);
	// Delaty the toggle by 0.5s
	//Delay(50);
	return 0;
}




// USART Routines - End


// A dummy IRQ that we initialise the IRQ function pointer too, so if the IRQ gets accidently fired
// the code will at least action an IRQ.
void	SDR_Dummy_IRQ (void *pData)
{
}
