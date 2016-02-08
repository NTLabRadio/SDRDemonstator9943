/************************************************************************************************************
 *
 *  $Copyright © 2013-2014 CML Systems $
 *
 *  $Header: cmx7161.c  Revision:1.22  18 February 2014 09:59:40  ddavenport $
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

#ifdef DEBUG_CHANGE_INC_FROM_INITIAL_SDR_PROJECT
 #include "..\common\stm32f10x_sdr.h"
#else
 #include "stm32f10x_sdr.h"
#endif
#include "cmx7161.h"
#include "radio.h"

#ifndef DEBUG_CHANGE_INC_FROM_INITIAL_SDR_PROJECT
 #include "CMX7161_FI-1.h"
#endif
	//static cmxFI_TypeDef cmx7161FI = {CMX7161FI, (uint16_t*)db1_7161, (uint16_t*)db2_7161, DB1_PTR,  DB1_LEN, DB1_CHK_HI, DB1_CHK_LO, DB2_PTR, DB2_LEN, DB2_CHK_HI, DB2_CHK_LO,ACTIVATE_PTR, ACTIVATE_LEN };
        cmxFI_TypeDef *cmx7161FI = (cmxFI_TypeDef*) START_7161;
#undef db1
#undef db2
#undef DB1_PTR
#undef DB1_LEN
#undef DB1_CHK_LO
#undef DB1_CHK_HI
#undef DB2_PTR
#undef DB2_LEN
#undef DB2_CHK_LO
#undef DB2_CHK_HI
#undef ACTIVATE_PTR
#undef ACTIVATE_LEN
#undef U16

#if	0
// Defaults definition in flash located at the start of the last page boundary. The defaults below
// are what is loaded into flash when we do the inital flash of the code. However, the user has
// or will have the option to over write these.
#pragma location = 0x0803F800
const struct Cmx7161Flash_TypeDef	Cmx7161Flash =
	{
	 0,
	 CMX7161_FREQ_CONTROL_DEFAULT,
	 CMX7161_CBUS_5E_DEFAULT,
	 CMX7161_CBUS_B4_DEFAULT,
	 CMX7161_CMX994_DC_OFFSET_DEFAULT
	};
#endif

#if	0
// Tx Ramp Profile
const uint16_t aTxRamp[] =
{
		0,2,345,370,385,395,405,415,
		425,435,445,453,461,467,473,479,
		485,491,496,501,506,511,516,521,
		525,529,533,537,541,545,549,553,
		557,561,565,569,572,575,578,581,
		584,587,590,593,596,599,602,605,
		608,611,614,617,620,623,626,629,
		632,635,638,641,644,647,650,653
};
#endif

// Tx Ramp Profile
const uint16_t aTxRamp[] =
{
		0,2,455,460,465,470,475,480,
		486,494,502,510,516,522,528,534,
		540,545,550,555,560,565,570,574,
		578,582,586,590,594,598,602,606,
		610,614,618,622,625,628,631,634,
		637,640,643,646,649,652,655,658,
		661,664,667,670,673,676,679,682,
		685,688,691,694,697,700,703,703
};

// Function handlers for controlling the CMX7161.
void  CMX7161_EnableAuxDAC (CMX7161_TypeDef *pCmx7161);
void CMX7161_FreqControl (CMX7161_TypeDef  *pCmx7161);
void CMX7161_Cbus_5E (CMX7161_TypeDef  *pCmx7161);
void CMX7161_Cbus_B4 (CMX7161_TypeDef  *pCmx7161);
void CMX7161_RxSetup (CMX7161_TypeDef  *pCmx7161);

void CMX7161_PeripheralConfig (CMX7161_TypeDef *pCmx7161);
void CMX7161_CMX994_DcOffset (CMX7161_TypeDef  *pCmx7161);
void CMX7161_Off (CMX7161_TypeDef  *pCmx7161);
void CMX7161_Dummy  (CMX7161_TypeDef  *pCmx7161);


void CMX7161_FlushStatusReg (CMX7161_TypeDef  *pCmx7161);

// In relation to the CMX7161 this must be the first routine called so that the cbus
// interface can be mapped  and the parameters for the FI code load. Other routines
// set the other variables as required.
extern signed int var1;

uint16_t  CMX7161_Init( CMX7161_TypeDef *pCmx7161, uint8_t uInterface )
{

	pCmx7161->FI =cmx7161FI;					// Initialise to the FI load definitions above.
	pCmx7161->pFlash = (DMR_Flash_TypeDef *)ADDR_FLASH_PAGE;
	pCmx7161->uIRQ_STATUS_REG = 0;		// Shadow of CBUS status register
	pCmx7161->uIRQ_ENABLE_REG = 0;
	pCmx7161->uInterface = uInterface;
	pCmx7161->uError = 0;

	// Check the flag in flash, 0 indicates programmed. Read from saved defaults if the flash
	// has been programmed. Otherwise load the standard macro defaults. Defaults are modified
	// and saved from the MMI.
	if(pCmx7161->pFlash->sFlag == 0)
	{
		pCmx7161->sFreqControl = pCmx7161->pFlash->sCmx7161FreqControl;				// Default value for AuxDac3 input to VCTCXO
		pCmx7161->sCbus5E = pCmx7161->pFlash->sCmx7161Cbus5E;
		pCmx7161->sCbusB4 = pCmx7161->pFlash->sCmx7161CbusB4;
		pCmx7161->sCMX994DcOffset = pCmx7161->pFlash->sCMX994DcOffset;
		pCmx7161->sCMX994Gain = pCmx7161->pFlash->sCMX994Gain;
		pCmx7161->sCMX994GainIndex = pCmx7161->pFlash->sCMX994GainIndex;                
	}
	else
	{
		pCmx7161->sFreqControl = CMX7161_FREQ_CONTROL_DEFAULT;				// Default value for AuxDac3 input to VCTCXO
		pCmx7161->sCbus5E = CMX7161_CBUS_5E_DEFAULT;
		pCmx7161->sCbusB4 = CMX7161_CBUS_B4_DEFAULT;
		pCmx7161->sCMX994DcOffset = CMX7161_CMX994_DC_OFFSET_DEFAULT;
		pCmx7161->sCMX994Gain = CMX7161_CMX994_GAIN_DEFAULT;
		pCmx7161->sCMX994GainIndex = CMX7161_CMX994_GAIN_INDEX_DEFAULT;
	}

	if (SDR_Load_FI(pCmx7161->FI,uInterface))
	{
		#ifdef DEBUG_TEST_LOAD_FI_CMX7161
		GPIO_SetBits(GPIOE, TEST_GREEN_LED_D7);		// Green LED - Success.
		#endif
	}
	else
	{
		#ifdef DEBUG_TEST_LOAD_FI_CMX7161
		GPIO_SetBits(GPIOE, TEST_RED_LED_D8);			// Red LED - Failed.
		#endif
		pCmx7161->uError |= CMX7161_FI_LOAD_ERROR;
		return 0;
	}

	// DAVED DEBUG - Skyworks is called temporarily here so that we do not have to program, the channel
	// using the MMI. Note that Skywors is also hard coded with channel 0 values so the parameters passed
	// ar not important for this debug code.
	//Radio_SkyWorks (0,0,CBUS_INTERFACE_2);

	//Setup the VCTXCO control volts and AUIXDAC3 and Aux\Dac1.
	CMX7161_EnableAuxDAC (pCmx7161);
	CMX7161_FlushStatusReg (pCmx7161);
	CMX7161_ModemControl (pCmx7161,cCMX7161_IDLE);

	CMX7161_ProgramTxRamp (pCmx7161);
	Radio_CMX994Config(pCmx7161);
	CMX7161_PeripheralConfig (pCmx7161);
	CMX7161_ModemControl (pCmx7161,cCMX7161_IDLE);

	return 1;
}

// Dummy function that does nothing, fills up blank entries in the function pointer table.
void CMX7161_Dummy  (CMX7161_TypeDef  *pCmx7161)
{
	return;
}


uint16_t CMX7161_ModemControl (CMX7161_TypeDef *pCmx7161, cmx7161Control eControl )
{
	uint16_t	uData;


	uData = eControl;
	CBUS_Write16(CMX7161_MODEM_CONTROL,&uData,1,pCmx7161->uInterface);

	if(CMX7161_ReadStatusReg (pCmx7161,MODE) == 0)
	{
		// if we get here we have timed out and the mode selection was not successful,
		// so return 0. Set the error flag.
		pCmx7161->uError |= CMX7161_RX_MODEM_CONTROL_ERROR;
		return 0;
	}
	return 1;

}


void CMX7161_RxFIFOSetup (CMX7161_TypeDef *pCmx7161)
{
	uint16_t	uData;
	uint16_t	uFIFOControl;
	uData = 0;
	CBUS_Write16(CMX7161_FIFO_STATUS,&uData,1,pCmx7161->uInterface);
	uFIFOControl = CMX7161_HARD_DATA_RX_FIFO_CONTROL;
	uFIFOControl = uFIFOControl | 0x8080;
	CBUS_Write16(CMX7161_FIFO_CONTROL,&uFIFOControl,1,pCmx7161->uInterface);
	uFIFOControl = uFIFOControl ^ 0x8080;

}

// Combines the sync (48bits) with the data (216bits) into a slot of data. Currently the slot is
// put together in a buffer and then written as one to the Tx FIFO.

void CMX7161_TxData (CMX7161_TypeDef *pCmx7161, uint8_t *pPayLoad, uint8_t *pSync)
{
	uint16_t	i;
	for(i=0;i<13;i++)
	{
		pCmx7161->aSlot[i] = *pPayLoad++;
	}

	// Insert first nibble of the sync.
	pCmx7161->aSlot[13] = ((*pPayLoad & 0xF0) |  ((*pSync >> 4) & 0x0F));
	for(i=14;i<((CMX7161_DMR_SYNC_SIZE-1)+14);i++)
	{
		pCmx7161->aSlot[i] = (*pSync << 4) & 0xF0;
		pSync++;
		pCmx7161->aSlot[i] = pCmx7161->aSlot[i] | ((*pSync >> 4) & 0x0F);
	}
	// Insert last nibble of the sync.
	pCmx7161->aSlot[19] = ((*pPayLoad++ & 0x0F) | ((*pSync << 4) &  0xF0));

	for(i=20;i<(20+13);i++)
	{
		pCmx7161->aSlot[i] = *pPayLoad++;
	}

	// Write the slot to the Tx FIFO.
	CBUS_Write8(CMX7161_TXFIFO_BYTE,(uint8_t *)pCmx7161->aSlot,CMX7161_HARD_DATA_PACKET_LENGTH,pCmx7161->uInterface);

}

// Extracts the sync (48bits) and data (216bits) into two separate buffers whose
// addresses are passed as parameters

void CMX7161_RxData (CMX7161_TypeDef *pCmx7161, uint8_t *pData, uint8_t *pSync)
{
	uint16_t	i;

	CBUS_Read8(CMX7161_RXFIFO_BYTE,(uint8_t *)pCmx7161->aSlot,CMX7161_HARD_DATA_PACKET_LENGTH,pCmx7161->uInterface);

	// Re-arm the RX FIFO IRQ
	i = CMX7161_HARD_DATA_RX_FIFO_CONTROL;
	CBUS_Write16(CMX7161_FIFO_CONTROL,&i,1,pCmx7161->uInterface);

	// Extract the first nibble of the sync.
	*pSync = (pCmx7161->aSlot[13] & 0x0F) << 4;
	for(i=14;i<((CMX7161_DMR_SYNC_SIZE-1)+14);i++)
	{
		*pSync = *pSync | ((pCmx7161->aSlot[i] & 0xF0) >> 4);
		pSync++;
		*pSync = (pCmx7161->aSlot[i] & 0x0F) << 4;
	}
	// Extract the last nibble of the sync.
	*pSync = *pSync | ((pCmx7161->aSlot[19] & 0xF0) >> 4);

	for(i=0;i<13;i++)
	{
		*pData++ = pCmx7161->aSlot[i];
	}

	// Extract the first part of the nibble.
	*pData = (pCmx7161->aSlot[13] & 0xF0);

	// Extract the last part of the nibble.
	*pData = *pData | (pCmx7161->aSlot[19] & 0x0F);

	// Increment to the next byte.
	pData++;

	for(i=20;i<(20+13);i++)
	{
		*pData++ = pCmx7161->aSlot[i];
	}

}


void	CMX7161_EnableIRQ (CMX7161_TypeDef *pCmx7161, uint16_t uIRQ)
{
	// Set the bits in the interrupt enable shadow register, then write to the Cbus
	// register in hardware.
	pCmx7161->uIRQ_ENABLE_REG |= uIRQ;
	CBUS_Write16 (CMX7161_IRQ_ENABLE,&pCmx7161->uIRQ_ENABLE_REG,1,pCmx7161->uInterface);
}

void	CMX7161_DisableIRQ (CMX7161_TypeDef *pCmx7161, uint16_t uIRQ)
{
	// Clear the mask bits in the interrupt enable shadow register, then write to the
	// cbus register in hardware.
	pCmx7161->uIRQ_ENABLE_REG &= ~uIRQ;
	CBUS_Write16(CMX7161_IRQ_ENABLE,&pCmx7161->uIRQ_ENABLE_REG,1,pCmx7161->uInterface);
	pCmx7161->uIRQ_STATUS_REG &= ~uIRQ;
}



void CMX7161_ModemOptions (CMX7161_TypeDef *pCmx7161, uint16_t uModemOptions)
{
	CBUS_Write16 (CMX7161_MODEM_OPTIONS,&uModemOptions,1,pCmx7161->uInterface);
}

uint16_t CMX7161_ReadRSSI (CMX7161_TypeDef *pCmx7161)
{
  uint16_t uData;
  CBUS_Read16(CMX7161_RSSI,&uData,1, pCmx7161-> uInterface);
  return uData & 0x0fff;
}

void CMX7161_FreqControl (CMX7161_TypeDef  *pCmx7161)
{
	uint16_t uData;
	uData = 0x8000 | ((uint16_t)pCmx7161->sFreqControl);
	// DAVED - Early board support. AuxDac 2 and 3 are swapped.
//	CBUS_Write16(CMX7161_AUXDAC3_CONTROL,&uData,1,pCmx7161->uInterface);
     CBUS_Write16(CMX7161_AUXDAC2_CONTROL,&uData,1,pCmx7161->uInterface);    //DACS swapped on Rev B PCB
	
        GPIO_ToggleBits(GPIOE, TEST_GREEN_LED_D7);		// Toggle Green LED
}

void CMX7161_Cbus_5E (CMX7161_TypeDef  *pCmx7161)
{
	uint16_t uData;
	uData = (uint16_t )pCmx7161->sCbus5E;
	CBUS_Write16(0x5E,&uData,1,pCmx7161->uInterface);
	GPIO_ToggleBits(GPIOE, TEST_GREEN_LED_D7);		// Toggle Green LED
}

void CMX7161_Cbus_B4 (CMX7161_TypeDef  *pCmx7161)
{
	uint16_t uData;
	uData = (uint16_t)pCmx7161->sCbusB4;
	CBUS_Write16(0xB4,&uData,1,pCmx7161->uInterface);
	GPIO_ToggleBits(GPIOE, TEST_GREEN_LED_D7);		// Toggle Green LED
}

#ifndef DEBUG_REM_MISSING_FUNCS_FROM_INITIAL_SDR_PROJECT
void CMX7161_RxSetup (CMX7161_TypeDef  *pCmx7161)
{
	uint16_t uData;
	CMX7161_RxConfig (pCmx7161);
	// Set modem into Rx Data mode.
	uData = 0x0001;
	CBUS_Write16(CMX7161_MODEM_CONTROL,&uData,1,pCmx7161->uInterface);
	GPIO_ToggleBits(GPIOE, TEST_GREEN_LED_D7);		// Toggle Green LED
}
#endif

void CMX7161_CMX994_DcOffset (CMX7161_TypeDef  *pCmx7161)
{
	Radio_CMX994Config(pCmx7161);
	GPIO_ToggleBits(GPIOE, TEST_GREEN_LED_D7);		// Toggle Green LED
}

void CMX7161_CMX994_Gain (CMX7161_TypeDef  *pCmx7161)
{
	Radio_CMX994Config(pCmx7161);
}

void CMX7161_PA_Low (CMX7161_TypeDef  *pCmx7161)
{
	uint16_t uData;
	uData = 0x8200;		// Switch the PA on
	CBUS_Write16(CMX7161_AUXDAC1_CONTROL,&uData,1,pCmx7161->uInterface);
	GPIO_ToggleBits(GPIOE, TEST_GREEN_LED_D7);		// Toggle Green LED
}

void CMX7161_PA_High (CMX7161_TypeDef  *pCmx7161)
{
	uint16_t uData;
	uData = 0x82A0;		// Switch the PA on
	CBUS_Write16(CMX7161_AUXDAC1_CONTROL,&uData,1,pCmx7161->uInterface);
	GPIO_ToggleBits(GPIOE, TEST_GREEN_LED_D7);		// Toggle Green LED
}

void CMX7161_PA_Off (CMX7161_TypeDef  *pCmx7161)
{
	uint16_t uData;
	uData = 0x8000;		// Switch the PA off
	CBUS_Write16(CMX7161_AUXDAC1_CONTROL,&uData,1,pCmx7161->uInterface);
	GPIO_ToggleBits(GPIOE, TEST_GREEN_LED_D7);		// Toggle Green LED
}

// Put the CMX7161 into Deep Sleep
void CMX7161_Off (CMX7161_TypeDef  *pCmx7161)
{
	uint16_t uData;
	uData = 0x8000;		// Switch the PA off
	CBUS_Write16(CMX7161_AUXDAC1_CONTROL,&uData,1,pCmx7161->uInterface);
	// and then put it into sleep mode.
	uData = 0x0000;
	CBUS_Write16(CMX7161_MODEM_CONTROL,&uData,1,pCmx7161->uInterface);
	GPIO_ToggleBits(GPIOE, TEST_GREEN_LED_D7);		// Toggle Green LED
}


void  CMX7161_EnableAuxDAC (CMX7161_TypeDef *pCmx7161)
{

	uint16_t	uData;
	// Flush by reading the status register.
	CBUS_Read16 (CMX7161_STATUS_REG,&uData,1,pCmx7161->uInterface);
	uData = 0x8000;		// Enable AUXDAC1 with 0 value
	CBUS_Write16(CMX7161_AUXDAC1_CONTROL,&uData,1,pCmx7161->uInterface);
	// VCTXCO Control Volts, AUXDAC3
	uData = 0x8000 | (uint16_t)(pCmx7161->sFreqControl);
	// DAVED - Early board support. AuxDac 2 and 3 are swapped.
	//	CBUS_Write16(CMX7161_AUXDAC3_CONTROL,&uData,1,pCmx7161->uInterface);
    CBUS_Write16(CMX7161_AUXDAC2_CONTROL,&uData,1,pCmx7161->uInterface);    //dacs swapped on rev B cards

}


void CMX7161_PeripheralConfig (CMX7161_TypeDef *pCmx7161)
{
	uint16_t	uData;

	uData = (uint16_t)pCmx7161->sCbusB4;		// Tx Mod Gain
	CBUS_Write16(CMX7161_DAC_CH1_GAIN,&uData,1,pCmx7161->uInterface);
	uData = 0x00;		// Values used on board 2 with 7161-1.x.x.7 FI
	CBUS_Write16(CMX7161_DAC_CH2_GAIN,&uData,1,pCmx7161->uInterface);
	uData = 0x0B00;		// Values used on board 2 with 7161-1.x.x.7 FI
	CBUS_Write16(0x5D,&uData,1,pCmx7161->uInterface);
	uData = (uint16_t)pCmx7161->sCbus5E;		// Tx Deviation
	CBUS_Write16(0x5E,&uData,1,pCmx7161->uInterface);

}

// Provides a common function for the CMX7161 to read the status register correctly. We have dedicated timer
// TIM4 for timeout protection. This timer should not be used for anything else within the code. Bear in mind
// that interrupts on the CMX7161 could read the status register, which are copied into the shadow register.
// Hence completion of the function below is based on the contents of the status register. This has tripped me
// up quite a few times.

uint16_t	CMX7161_ReadStatusReg (CMX7161_TypeDef *pCmx7161, uint16_t uMask)
{
	uint16_t uData;

	TIM_SetCounter(TIM4,0);
	while (TIM_GetCounter(TIM4) < CMX7161_MODEM_CONTROL_TIMEOUT)
	{
		CBUS_Read16 (CMX7161_STATUS_REG,&uData,1,pCmx7161->uInterface);
		pCmx7161->uIRQ_STATUS_REG |= uData;
		if ((pCmx7161->uIRQ_STATUS_REG & uMask) == uMask)
		{
			// Clear the bit in the shadow register.
			pCmx7161->uIRQ_STATUS_REG &= (uint16_t)(~uMask);
			return 1;
		}
	}
	return 0;
}


// Read ths status register and align the shadow register to the hardware register.
void CMX7161_FlushStatusReg (CMX7161_TypeDef  *pCmx7161)
{
	uint16_t uData;
	// Flush by reading the status register.
	CBUS_Read16 (CMX7161_STATUS_REG,&uData,1,pCmx7161->uInterface);
	// Align the shadow register to the hardware status register.
	CBUS_Read16 (CMX7161_STATUS_REG,(uint16_t *)(&pCmx7161->uIRQ_STATUS_REG),1,pCmx7161->uInterface);
}


uint16_t CMX7161_ProgramTxRamp (CMX7161_TypeDef  *pCmx7161)
{
	uint16_t i;
	uint16_t uData;
	// Change RAMDAC scan rate.
	uData = 0;
	CBUS_Write16(CMX7161_MODEM_CONTROL,&uData,1,pCmx7161->uInterface);
	uData = 0x0100;
	CBUS_Write16(CMX7161_PROGRAMMING,&uData,1,pCmx7161->uInterface);
	// Start writing new ramp at P.0.1
	uData = 0x0100;
	CBUS_Write16(CMX7161_MODEM_CONTROL,&uData,1,pCmx7161->uInterface);

	if(CMX7161_ReadStatusReg (pCmx7161,MODE) == 0)
	{
		// if we get here we have timed out and the mode selection was not successful,
		// so return 0. Set the error flag.
		pCmx7161->uError |= CMX7161_RAMDAC_MODE_ERROR;
		return 0;
	}

	for (i=0;i<64;i++)
	{
		CBUS_Write16(CMX7161_PROGRAMMING,(uint16_t *)&aTxRamp[i],1,pCmx7161->uInterface);
		if(CMX7161_ReadStatusReg (pCmx7161,PROG) == 0)
			{
				// if we get here we have timed out and the mode selection was not successful,
				// so return 0. Set the error flag.
				pCmx7161->uError |= CMX7161_RAMDAC_PROG_ERROR;
				return 0;
                        }
        }

	// Timing Delay Ramp
	uData = 0x0220;
	CBUS_Write16(CMX7161_MODEM_CONTROL,&uData,1,pCmx7161->uInterface);

	if(CMX7161_ReadStatusReg (pCmx7161,MODE) == 0)
	{
		// if we get here we have timed out and the mode selection was not successful,
		// so return 0. Set the error flag.
		pCmx7161->uError |= CMX7161_RAMDAC_MODE_ERROR;
	}
	uData = 0x3600;
	CBUS_Write16(CMX7161_PROGRAMMING,&uData,1,pCmx7161->uInterface);
	if(CMX7161_ReadStatusReg (pCmx7161,PROG) == 0)
	{
		// if we get here we have timed out and the program was not successful,
		// so return 0. Set the error flag.
		pCmx7161->uError |= CMX7161_RAMDAC_PROG_ERROR;
	}
	uData = 0x01B0;
	CBUS_Write16(CMX7161_PROGRAMMING,&uData,1,pCmx7161->uInterface);
	if(CMX7161_ReadStatusReg (pCmx7161,PROG) == 0)
	{
		// if we get here we have timed out and the program was not successful,
		// so return 0. Set the error flag.
		pCmx7161->uError |= CMX7161_RAMDAC_PROG_ERROR;
	}

	return 1;
}

// This is called from the IRQ. The parameter is void so that the function pointer that
// references this can handle different parameter types.

void CMX7161_IRQ (void *pData)
{

	uint16_t uTemp;
	CMX7161_TypeDef * pCmx7161;

	pCmx7161 = (CMX7161_TypeDef*)pData;

	// Read the status register into a shaoow register.
	CBUS_Read16 (CMX7161_STATUS_REG,&uTemp,1,pCmx7161->uInterface);
	uTemp = uTemp & 0x7FFF;	// clear bit 15..
	pCmx7161->uIRQ_STATUS_REG |= uTemp;

}
