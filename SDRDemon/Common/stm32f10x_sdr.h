/************************************************************************************************************
 *
 *  $Copyright © 2013-2014 CML Systems $
 *
 *  $Header: stm32f10x_sdr.h  Revision:1.9.1.3  02 July 2014 17:52:54  ilewis $
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
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F1_SDR_H
#define __STM32F1_SDR_H

// To include your own C header files, you must wrap the #include directive
// in an extern "C" statement. This header is setup for compilation using
// a C++ complier. I decided to go for the C++ compiler options to future
// proof the development.

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdio.h>
#include "stm32f10x.h"

   
 //Memory Map - DE9943
 //                  Start            Space     Current length
//      ARM Code     0x08000000	      64kbytes  ~29kbytes 
#define START_7262   0x08010000	    //112kbytes ~91kbytes
#define START_7161   0x0802c000     //78kbytes  ~13kbytes (but will get bigger when analogue stuff is added)
//      Settings     0x0803f800        2kbytes   Set to a page size to allow an erase. Currently 32 bytes are used, but will
// grow if we decide to add more settings to flash.

 //Memory Map - DE9944
 //                  Start            Space     Current length
//      ARM Code     0x08000000	      64kbytes  ~29kbytes 
#define START_7141   0x08010000	    //190kbytes ~??kbytes
//      Settings     0x0803f800        2kbytes   Set to a page size to allow an erase. Currently 32 bytes are used, but will
// grow if we decide to add more settings to flash.
   
 typedef enum {
	 CMX7161FI,
	 CMX7262FI,
         CMX7141FI
 } FI_Description_TypeDef;



 // Created so we can use a common function to load function images.
 typedef struct
 {
	 FI_Description_TypeDef type;
	uint16_t *db1_start_address;
 	uint16_t *db2_start_address;
 	uint16_t db1_ptr;
 	uint16_t db1_len;
 	uint16_t db1_chk_hi;
 	uint16_t db1_chk_lo;
 	uint16_t db2_ptr;
 	uint16_t db2_len;
 	uint16_t db2_chk_hi;
 	uint16_t db2_chk_lo;
 	uint16_t activate_ptr;
 	uint16_t activate_len;
 	uint16_t activation_code1;      //only needed for CMX7x3x & CMX7x4x devices
 	uint16_t activation_code2;      //only needed for CMX7x3x & CMX7x4x devices
 } cmxFI_TypeDef;


 typedef struct
   {
  	__IO uint8_t	uFlag;					// Flag set by IRQ
  	void (*pHandler)( void* );				// IRQ service routine
  	void *pParam;								// Parameters passed with the IRQ service routine.
   } CBUS_IRQ_TypeDef;

#define CBUS_NUMBER_OF_INTERFACES		3

// Controls the chip select for the CBUS interfaces and acts as a reference throughout the code
// to minimise confusion.

#define CBUS_INTERFACE_1			0
#define CBUS_INTERFACE_2			1
#define CBUS_INTERFACE_3			2

 /* Maximum Timeout values for flags waiting loops. These timeouts are not based
    on accurate values, they just guarantee that the application will not remain
    stuck if the SPI communication is corrupted.
    You may modify these timeout values depending on CPU frequency and application
    conditions (interrupts routines ...). */

#define CBUS_TIME_OUT         ((uint32_t)0x1000)

#define CBUS_SPI                       		  SPI1
#define CBUS_SPI_CLK                   RCC_APB2Periph_SPI1

#define CBUS_SPI_SCK_PIN               GPIO_Pin_5                  /* PA.05 */
#define CBUS_SPI_SCK_GPIO_PORT         GPIOA                       /* GPIOA */
#define CBUS_SPI_SCK_GPIO_CLK          RCC_APB2Periph_GPIOA
#define CBUS_SPI_SCK_SOURCE            GPIO_PinSource5
#define CBUS_SPI_SCK_AF                GPIO_AF_SPI1

#define CBUS_SPI_MISO_PIN              GPIO_Pin_6                  /* PA.6 */
#define CBUS_SPI_MISO_GPIO_PORT        GPIOA                       /* GPIOA */
#define CBUS_SPI_MISO_GPIO_CLK         RCC_APB2Periph_GPIOA
#define CBUS_SPI_MISO_SOURCE           GPIO_PinSource6
#define CBUS_SPI_MISO_AF               GPIO_AF_SPI1

#define CBUS_SPI_MOSI_PIN              GPIO_Pin_7                  /* PA.7 */
#define CBUS_SPI_MOSI_GPIO_PORT        GPIOA                       /* GPIOA */
#define CBUS_SPI_MOSI_GPIO_CLK         RCC_APB2Periph_GPIOA
#define CBUS_SPI_MOSI_SOURCE           GPIO_PinSource7
#define CBUS_SPI_MOSI_AF               GPIO_AF_SPI1

// GPIO lines mapped as inputs and interrupt sources from CBUS.

#define CBUS_IRQN1_PIN                GPIO_Pin_0
#define CBUS_IRQN1_GPIO_PORT          GPIOA
#define CBUS_IRQN1_GPIO_CLK           RCC_APB2Periph_GPIOA
#define CBUS_IRQN1_EXTI_LINE          EXTI_Line0
#define CBUS_IRQN1_EXTI_PORT_SOURCE   EXTI_PortSourceGPIOA
#define CBUS_IRQN1_EXTI_PIN_SOURCE    EXTI_PinSource0
#define CBUS_IRQN1_EXTI_IRQn          EXTI0_IRQn

#define CBUS_IRQN3_PIN                GPIO_Pin_1                  /* PA.01 */
#define CBUS_IRQN3_GPIO_PORT          GPIOA                       /* GPIOA */
#define CBUS_IRQN3_GPIO_CLK           RCC_APB2Periph_GPIOA
#define CBUS_IRQN3_EXTI_LINE          EXTI_Line1
#define CBUS_IRQN3_EXTI_PORT_SOURCE   EXTI_PortSourceGPIOA
#define CBUS_IRQN3_EXTI_PIN_SOURCE    EXTI_PinSource1
#define CBUS_IRQN3_EXTI_IRQn          EXTI1_IRQn


 // Definitions for IO ports mapped as chip selects (outputs).

#define CBUS_SPI_CS3_PIN                GPIO_Pin_2                  /* PA.02 */
#define CBUS_SPI_CS_GPIO_PORT          GPIOA                       /* GPIOA */
#define CBUS_SPI_CS_GPIO_CLK           RCC_APB2Periph_GPIOA

#define CBUS_SPI_CS2_PIN                GPIO_Pin_3                  /* PA.03 */
#define CBUS_SPI_CS_GPIO_PORT          GPIOA                       /* GPIOA */
#define CBUS_SPI_CS_GPIO_CLK           RCC_APB2Periph_GPIOA

#define CBUS_SPI_CS1_PIN                GPIO_Pin_4                  /* PA.04 */
#define CBUS_SPI_CS_GPIO_PORT          GPIOA                       /* GPIOA */
#define CBUS_SPI_CS_GPIO_CLK           RCC_APB2Periph_GPIOA


// General GPIO output - Port D

#define AUDIO_PA              GPIO_Pin_0                  /* PE.00 */
#define TEST_PORT          GPIOE                       /* GPIOE */
#define TEST_CLK           	RCC_APB2Periph_GPIOE

#define TEST_TL1              GPIO_Pin_1                  /* PE.01 */
#define TEST_PORT          GPIOE                       /* GPIOE */
#define TEST_CLK           	RCC_APB2Periph_GPIOE

#define TEST_TL2              GPIO_Pin_2                  /* PE.02 */
#define TEST_PORT          GPIOE                       /* GPIOE */
#define TEST_CLK           	RCC_APB2Periph_GPIOE

#define TEST_TL3              GPIO_Pin_3                  /* PE.03 */
#define TEST_PORT          GPIOE                       /* GPIOE */
#define TEST_CLK           	RCC_APB2Periph_GPIOE

#define TEST_TL4               GPIO_Pin_4                  /* PE.04 */
#define TEST_PORT          GPIOE                       /* GPIOE */
#define TEST_CLK           	RCC_APB2Periph_GPIOE

#define TEST_TL5               GPIO_Pin_5                  /* PE.05 */
#define TEST_PORT          GPIOE                       /* GPIOE */
#define TEST_CLK           	RCC_APB2Periph_GPIOE

#define TEST_RED_LED_D8             GPIO_Pin_6                  /* PE.06 */
#define TEST_PORT        GPIOE                       /* GPIOE */
#define TEST_CLK           	RCC_APB2Periph_GPIOE

#define TEST_GREEN_LED_D7             GPIO_Pin_7                  /* PE.07 */
#define TEST_PORT        GPIOE                       /* GPIOE */
#define TEST_CLK           	RCC_APB2Periph_GPIOE


 /** @defgroup STM32F4_DISCOVERY_CBUS_Exported_Macros
   * @{
   */
 #define CBUS_CSN1_LOW()       GPIO_ResetBits(CBUS_SPI_CS_GPIO_PORT, CBUS_SPI_CS1_PIN)
 #define CBUS_CSN1_HIGH()      GPIO_SetBits(CBUS_SPI_CS_GPIO_PORT, CBUS_SPI_CS1_PIN)
 #define CBUS_CSN2_LOW()       GPIO_ResetBits(CBUS_SPI_CS_GPIO_PORT, CBUS_SPI_CS2_PIN)
 #define CBUS_CSN2_HIGH()      GPIO_SetBits(CBUS_SPI_CS_GPIO_PORT, CBUS_SPI_CS2_PIN)
 #define CBUS_CSN3_LOW()       GPIO_ResetBits(CBUS_SPI_CS_GPIO_PORT, CBUS_SPI_CS3_PIN)
 #define CBUS_CSN3_HIGH()      GPIO_SetBits(CBUS_SPI_CS_GPIO_PORT, CBUS_SPI_CS3_PIN)
 /**
   * @}
   */

 /* Dummy Byte Send by the SPI Master device in order to generate the Clock to the Slave device */
#define CBUS_DUMMY_BYTE                 ((uint8_t)0x00)

#define FT232_USART_FLAG_TIMEOUT         ((uint32_t)0x1000)

#define FT232_USART                        USART1
#define FT232_USART_CLOCK		RCC_APB2Periph_USART1

#define FT232_USART_TX_PIN               GPIO_Pin_9                  /* PA.09 */
#define FT232_USART_TX_GPIO_PORT         GPIOA                       /* GPIOA */
#define FT232_USART_TX_GPIO_CLK          RCC_APB2Periph_GPIOA
#define FT232_USART_TX_SOURCE            GPIO_PinSource9
#define FT232_USART_TX_AF                GPIO_AF_USART1

#define FT232_USART_RX_PIN              GPIO_Pin_10                  /* PA.10 */
#define FT232_USART_RX_GPIO_PORT        GPIOA                       /* GPIOA */
#define FT232_USART_RX_GPIO_CLK         RCC_APB2Periph_GPIOA
#define FT232_USART_RX_SOURCE           GPIO_PinSource10
#define FT232_USART_RX_AF               GPIO_AF_USART1

#define FT232_USART_CTS_PIN              GPIO_Pin_11                  /* PA.11 */
#define FT232_USART_CTS_GPIO_PORT        GPIOA                       /* GPIOA */
#define FT232_USART_CTS_GPIO_CLK         RCC_APB2Periph_GPIOA
#define FT232_USART_CTS_SOURCE           GPIO_PinSource11
#define FT232_USART_CTS_AF               GPIO_AF_USART1

#define FT232_USART_RTS_PIN              GPIO_Pin_12                  /* PA.12 */
#define FT232_USART_RTS_GPIO_PORT        GPIOA                       /* GPIOA */
#define FT232_USART_RTS_GPIO_CLK         RCC_APB2Periph_GPIOA
#define FT232_USART_RTS_SOURCE           GPIO_PinSource12
#define FT232_USART_RTS_AF               GPIO_AF_USART1


// Address for final page in flash into which the defaults are saved.
#define ADDR_FLASH_PAGE     ((uint32_t)0x0803F800)

typedef struct {
   	signed int sFlag;														// Flag is cleared to 0 to indicate programmed.
   	uint32_t uBoardNumber;											// Board Number
   	uint32_t uVersion;													// Not used
   	signed int sCmx7161FreqControl;							// Parameter used by calls to control.
   	signed int sCmx7161Cbus5E;									// Copy of Mod 2 Output Control $5E
   	signed int sCmx7161CbusB4;									// Copy of Mod 1 Output Course Gain	$B4
   	signed int sCMX994DcOffset;									// Copy of part of DC offset used in CMX994.
   	signed int sCMX994Gain;
   	signed int sCMX994GainIndex;        
    signed int sCmx7262InputGain;								// Codec input gain (Microphone input gain).
}  DMR_Flash_TypeDef ;

typedef struct {
   	signed int sFlag;														// Flag is cleared to 0 to indicate programmed.
   	uint32_t uBoardNumber;											// Board Number
   	uint32_t uVersion;													// Not used
   	signed int sCmx7141FreqControl;							// Parameter used by calls to control.
   	signed int sCmx7141Cbus5E;									// Copy of Mod 2 Output Control $5E
   	signed int sCmx7141CbusB4;									// Copy of Mod 1 Output Course Gain	$B4
   	signed int sCMX994DcOffset;									// Copy of part of DC offset used in CMX994.
   	unsigned int sSquelchThreshold;
    signed int sInputGain;                  		//value to put in b12-b10 of $B1.
    signed int sInputGainIndex;             		//index used by MMI to select this value.        
    //signed int sCmx7262InputGain;							//Codec input gain (Microphone input gain).
}  PMR_Flash_TypeDef ;


//Copied from F4 std periph library
void GPIO_ToggleBits(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

void SDR_Init(void);
uint16_t SDR_Load_FI (cmxFI_TypeDef *pFI, uint8_t uInterface);
uint16_t SDR_Load_FI_7x4x (cmxFI_TypeDef *pFI, uint8_t uInterface);

uint32_t ReadCounter (void);
void ClearCounter(void);
void CBUS_Write16 (uint8_t uAddress, uint16_t *data_ptr, uint16_t uAccesses, uint8_t uInterface);
void CBUS_Read16 (uint8_t uAddress, uint16_t *data_ptr, uint16_t uAccesses, uint8_t uInterface);
void CBUS_Write8 (uint8_t uAddress, uint8_t *data_ptr, uint16_t uAccesses, uint8_t uInterface);
void CBUS_Read8 (uint8_t uAddress, uint8_t *data_ptr, uint16_t uAccesses, uint8_t uInterface);
uint16_t CBUS_WaitBitSet16(uint8_t Address, uint16_t uMask, uint8_t uInterface);
uint16_t	CBUS_WaitBitClr8(uint8_t Address, uint8_t uMask, uint8_t uInterface);
uint8_t CBUS_SendByte(uint8_t byte);
void CBUS_SetCSNLow(uint8_t uMask);
void CBUS_SetCSNHigh(uint8_t uMask);

uint8_t FT232_USART_ReadByte(void);
uint8_t FT232_USART_SendByte(uint8_t byte);

extern CBUS_IRQ_TypeDef CBUS_IRQTable[CBUS_NUMBER_OF_INTERFACES];

// External reference so that the IRQ (stm32f4xx_it) can reference them.
//extern void (*pFuncSDR_CBUS1_IRQ)( void* );
//extern void *pParamSDR_CBUS1_IRQ;
//extern IrqTask_TypeDef task;


#ifdef __cplusplus
}
#endif

#endif /* __STM32F10X_SDR_H */

