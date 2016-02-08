/************************************************************************************************************
 *
 *  $Copyright © 2013-2014 CML Systems $
 *
 *  $Header: cmx7161.h  Revision:1.14  18 February 2014 10:10:59  ddavenport $
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

#ifndef _CMX7161_H
#define _CMX7161_H

// Comment out to disable base band testing.
//#define BASE_BAND_TEST


// BS Sourced Voice Sync Pattern
#define CMX7161_DMR_SYNC_SIZE		6
#define CMX7161_HARD_DATA_PACKET_LENGTH	33
#define CMX7161_HARD_DATA_RX_FIFO_CONTROL		( CMX7161_HARD_DATA_PACKET_LENGTH<<8)


#define CMX7161_FIFO_STATUS					0x47
#define CMX7161_TXFIFO_BYTE					0x48
#define CMX7161_RXFIFO_BYTE					0x4C
#define CMX7161_FIFO_CONTROL               0x50
#define CMX7161_AUXADC1_CONTROL            0x51
#define CMX7161_AUXADC2_CONTROL            0x52
#define CMX7161_AUXADC3_CONTROL            0x53
#define CMX7161_AUXADC4_CONTROL            0x54
#define CMX7161_AUXADC1_THRESHOLDS         0x55
#define CMX7161_AUXADC2_THRESHOLDS         0x56
#define CMX7161_AUXADC3_THRESHOLDS         0x57
#define CMX7161_AUXADC4_THRESHOLDS         0x58
#define CMX7161_AUXDAC1_CONTROL            0x59
#define CMX7161_AUXDAC2_CONTROL            0x5A
#define CMX7161_AUXDAC3_CONTROL            0x5B
#define CMX7161_AUXDAC4_CONTROL            0x5C
#define CMX7161_IINPUT_CONTROL             0x5D
//#define CMX7161_QINPUT_CONTROL            0x5E
#define CMX7161_MOD1_CONTROL               0x5F
#define CMX7161_MOD2_CONTROL               0x60
#define CMX7161_SW_WR17                    0x61
#define CMX7161_SSP_THRU_CONTROL           0x62
#define CMX7161_SSP_THRU_WRITE             0x63
#define CMX7161_GPIO_CONTROL               0x64
#define CMX7161_SW_WR21                    0x65
#define CMX7161_SW_WR22                    0x66
#define CMX7161_SW_WR23                    0x66
#define CMX7161_TONE                       0x68
#define CMX7161_MODEM_OPTIONS              0x69

#define CMX7161_PROGRAMMING                0x6A
#define CMX7161_MODEM_CONTROL              0x6B
#define CMX7161_IRQ_ENABLE                 0x6C

#define CMX7161_RSSI                					  0x77

//;****************************************************************
//              HOST READ REGISTERS
//****************************************************************

#define CMX7161_SSP_THRU_READ		0x78

#define CMX7161_STATUS_REG                  0x7E
#define CMX7161_DAC_CH1_GAIN				0xB4
#define CMX7161_DAC_CH2_GAIN				0xB5

// Bit references in Status (0x7E) and IRQ Enable Register (0x6C)
#define	RXFIFO									(1<<6)
#define	TXFIFO										(1<<7)
#define	SLOT0										(1<<8)
#define	SLOT1										(1<<9)
#define	SYNC										(1<<11)
#define	PROG										(1<<13)
#define	MODE										(1<<14)

#define CMX7161_MODEM_CONTROL_TIMEOUT			1500		// 15mS with a 10uS tick. Used for modem control changes.

#define CMX7161_MAX_CONTROL_FUNCTIONS	20	// Maximum number of control functions.

// Enumeration of all settings for the CMX7161 in transmit.
typedef enum {
	cCMX7161_IDLE = 0,							// Clear must have a value of 0 so that we know that there is no request to the CMX7161.
	cCMX7161_RX_SETUP = 0x0001,
	cCMX7161_RX_EYE = 0x0002,
	cCMX7161_RX_DATA = 0x0003,
	cCMX7161_RX_SLOT = 0x0F0F,
	cCMX7161_TX_PRBS  =  0x000A,
	cCMX7161_TX_SLOT_TEST = 0x000B,
	cCMX7161_TX_PATTERN_00 = 0x0009,	// Setup mode Tx Pattern		5F
	cCMX7161_TX_PATTERN_01 = 0x0019,	// Setup mode Tx Pattern		55 55 55 55 FF FF FF FF
	cCMX7161_TX_PATTERN_10 = 0x0029,	// Setup mode Tx Pattern		55
	cCMX7161_TX_PATTERN_11 = 0x0039,		// Setup mode Tx Pattern		7F 7D 5D D5 7D FD
	cCMX7161_TX_SLOT = 0x0B0F					// Standard Tx mode
} cmx7161Control;


// Bit definitons for errors on the CMX7161.
#define	CMX7161_FI_LOAD_ERROR				(1<<0)		// FI failed to load correctly.
#define	CMX7161_TX_MODEM_CONTROL_ERROR		(0x00000002)		// <Modem control failed to complete.
#define	CMX7161_RX_MODEM_CONTROL_ERROR		(0x00000004)		// <Modem control failed to complete.
#define	CMX7161_CMX994_RX_ERROR		(0x00000008)		// Error configuring the CMX994 for Rx.
#define	CMX7161_CMX994_TX_ERROR		(0x00000010)		// Error configuring the CMX994 for Tx.
#define	CMX7161_FLASH_PROGRAM_ERROR		(0x00000020)		// Error during saving data to flash.
#define	CMX7161_RAMDAC_MODE_ERROR		(0x00000040)		// Changing into program RAMDAC mode
#define	CMX7161_RAMDAC_PROG_ERROR		(0x00000080)		// Failed programming RAMDAC

// Defaults and maximum values for the CMX7161.
#define	CMX7161_FREQ_CONTROL_MAX	700
#define	CMX7161_FREQ_CONTROL_DEFAULT	494		// Default value for AuxDac3 input to VCTCXO
#define	CMX7161_AUXDAC2_DEFAULT		0x81EE
#define	CMX7161_CBUS_5E_DEFAULT		0x2E00			// // Tx Mod Deviation
#define	CMX7161_CBUS_B4_DEFAULT		0x40
#define	CMX7161_CMX994_DC_OFFSET_DEFAULT		0x00
#define	CMX7161_CMX994_DC_OFFSET_MAX		0xFF
#define	CMX7161_CMX994_GAIN_DEFAULT	0x37
#define	CMX7161_CMX994_GAIN_INDEX_DEFAULT	0

// Data type for an instance of the CMX7161.
typedef struct
{
	cmxFI_TypeDef* FI;								// Function image parameters used during loading.
	DMR_Flash_TypeDef	 *pFlash;					// Pointer to the area of flash for storage of defaults.

	__IO uint16_t uIRQ_STATUS_REG;			// Shadow reg for IRQ status reads.
	uint16_t uIRQ_ENABLE_REG;				// Shadow register for the IRQ enable.
	uint8_t	uInterface;									// CBUS interface that the instance maps too.

	uint8_t aSlot[CMX7161_HARD_DATA_PACKET_LENGTH];

	// Note the "signed int" definitions below to match up with the pointer types within the MMI definitions.
	signed int sFreqControl;							// Parameter used by calls to control.
	signed int sCbus5E;								// Copy of Mod 2 Output Control $5E
	signed int sCbusB4;								// Copy of Mod 1 Output Course Gain	$B4
	signed int sCMX994DcOffset;				// Copy of part of DC offset used in CMX994.
	signed int sCMX994Gain;
	signed int sCMX994GainIndex;
	uint32_t uError;
	signed int FIVersion;
} CMX7161_TypeDef;


uint16_t  CMX7161_Init( CMX7161_TypeDef *pCmx7161, uint8_t uInterface );

void CMX7161_Main (CMX7161_TypeDef  *pCmx7161);
void CMX7161_WriteFlash (CMX7161_TypeDef *pCmx7161);
void CMX7161_IRQ (void *pData);

void CMX7161_TxConfig (CMX7161_TypeDef *pCmx7161);
void CMX7161_RxConfig (CMX7161_TypeDef *pCmx7161);

void CMX7161_PA_Low (CMX7161_TypeDef  *pCmx7161);
void CMX7161_PA_High (CMX7161_TypeDef  *pCmx7161);
void CMX7161_PA_Off (CMX7161_TypeDef  *pCmx7161);

uint16_t CMX7161_ModemControl (CMX7161_TypeDef *pCmx7161, cmx7161Control eControl );

void CMX7161_TxData (CMX7161_TypeDef *pCmx7161, uint8_t *pPayLoad, uint8_t *pSync);
void CMX7161_RxData (CMX7161_TypeDef *pCmx7161, uint8_t *pData, uint8_t *pSync);
void CMX7161_RxFIFOSetup (CMX7161_TypeDef *pCmx7161);
void CMX7161_EnableIRQ (CMX7161_TypeDef *pCmx7161, uint16_t uIRQ);
void CMX7161_DisableIRQ (CMX7161_TypeDef *pCmx7161, uint16_t uIRQ);
void CMX7161_ModemOptions (CMX7161_TypeDef *pCmx7161, uint16_t uModemOptions);
uint16_t CMX7161_ReadRSSI (CMX7161_TypeDef *pCmx7161);

void CMX7161_CMX994_DcOffset (CMX7161_TypeDef  *pCmx7161);
void CMX7161_CMX994_Gain (CMX7161_TypeDef  *pCmx7161);
void CMX7161_Cbus_5E (CMX7161_TypeDef  *pCmx7161);
void CMX7161_Cbus_B4 (CMX7161_TypeDef  *pCmx7161);
void CMX7161_FreqControl (CMX7161_TypeDef  *pCmx7161);

uint16_t CMX7161_ReadStatusReg (CMX7161_TypeDef *pCmx7161, uint16_t uMask);

uint16_t CMX7161_ProgramTxRamp (CMX7161_TypeDef  *pCmx7161);

// DAVED DEBUG - Added temporarily
void CMX7161_FlushStatusReg (CMX7161_TypeDef  *pCmx7161);

extern cmxFI_TypeDef *cmx7161FI;

#endif
