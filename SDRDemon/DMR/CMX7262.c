/************************************************************************************************************
 *
 *  $Copyright © 2012-2014 CML Systems $
 *
 *  $Header: CMX7262.c  Revision:1.29  18 February 2014 10:12:13  ddavenport $
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
#include "cmx7262.h"

#ifndef DEBUG_CHANGE_INC_FROM_INITIAL_SDR_PROJECT
 #include "CMX7262_FI-1.h"
#endif

	//static cmxFI_TypeDef cmx7262FI = {CMX7262FI, (uint16_t*)db1_7262, (uint16_t*)db2_7262, DB1_PTR,  DB1_LEN, DB1_CHK_HI, DB1_CHK_LO, DB2_PTR, DB2_LEN, DB2_CHK_HI, DB2_CHK_LO,ACTIVATE_PTR, ACTIVATE_LEN };
        cmxFI_TypeDef *cmx7262FI = (cmxFI_TypeDef*) START_7262;
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

void	CMX7262_Idle (CMX7262_TypeDef *pCmx7262);
void	CMX7262_Encode (CMX7262_TypeDef *pCmx7262);
void	CMX7262_Decode (CMX7262_TypeDef *pCmx7262);
void	CMX7262_MMI_AudioGain (CMX7262_TypeDef *pCmx7262);

uint16_t CMX7262_Transcode (CMX7262_TypeDef *pCmx7262, uint16_t uMode);

void CMX7262_EnableIRQ (CMX7262_TypeDef *pCmx7262, uint16_t uIRQ);
void CMX7262_DisableIRQ (CMX7262_TypeDef *pCmx7262, uint16_t uIRQ);
void CMX7262_Config (CMX7262_TypeDef  *pCmx7262, uint16_t uConfig);
void CMX7262_Routing (CMX7262_TypeDef  *pCmx7262, uint16_t uData);
uint16_t  CMX7262_InitHardware (CMX7262_TypeDef *pCmx7262 );
void  CMX7262_AnalogBlocks (CMX7262_TypeDef *pCmx7262 );
void CMX7262_FlushStatusReg (CMX7262_TypeDef  *pCmx7262);
void CMX7262_AudioPA (CMX7262_TypeDef  *pCmx7262, FunctionalState eState);
void CMX7262_AudioOutputGain (CMX7262_TypeDef  *pCmx7262, uint16_t uGain);
uint16_t CMX7262_ConfigClocks (CMX7262_TypeDef  *pCmx7262);


// This must be the first routine called so that the CBUS interfaced is mapped correctly and
// the FI loaded for the other CMX7262 routines.

uint16_t  CMX7262_Init (CMX7262_TypeDef  *pCmx7262, uint8_t uInterface )
{

	pCmx7262->FI =cmx7262FI;					// Initialise to the FI load definitions above.
	pCmx7262->pFlash = (DMR_Flash_TypeDef *)ADDR_FLASH_PAGE;
	pCmx7262->uInterface = uInterface;
	pCmx7262->uMode = CMX7262_INIT_MODE;
	pCmx7262->uPacketSize = 0;							// How many bytes we read and write.
	pCmx7262->uIRQRequest = 0;							// Control bits, set by IRQ, MMI, SysTick
	pCmx7262->pDataBuffer = NULL;
	pCmx7262->uIRQ_STATUS_REG = 0;					// Shadow register.
	pCmx7262->uIRQ_ENABLE_REG = 0;					// Shadow register.
	pCmx7262->uOutputGain = 0;
	pCmx7262->uError = 0;										// Clear error field.

	// Check the flag in flash, 0 indicates programmed. Read from saved defaults if the flash
	// has been programmed. Otherwise load the standard macro defaults. Defaults are modified
	// and saved from the MMI.
	if(pCmx7262->pFlash->sFlag == 0)
	{
		pCmx7262->sInputGain = pCmx7262->pFlash->sCmx7262InputGain;
	}
	else
	{
		pCmx7262->sInputGain = CMX7262_INPUT_GAIN_DEFAULT;
	}

	if (!SDR_Load_FI(pCmx7262->FI,uInterface))
	{
		#ifdef DEBUG_TEST_LOAD_FI_CMX7262
		GPIO_SetBits(GPIOE, TEST_RED_LED_D8);			// Red LED - Failed of Load_FI
		#endif		
		pCmx7262->uError |= CMX7262_FI_LOAD_ERROR;
		return 0;
	}
	// Initialise clocks, analog blocks, input and output gains and reg done select.
	if (!CMX7262_InitHardware (pCmx7262))
	{
		#ifdef DEBUG_TEST_LOAD_FI_CMX7262
		GPIO_SetBits(GPIOE, TEST_GREEN_LED_D7);		// Green LED - Failed of InitHardware
		#endif		
		pCmx7262->uError |= CMX7262_CONFIG_CLK_ERROR;
		return 0;
	}
	// Set up packet length, hard decision decoding, FEC disabled.
	CMX7262_Config(pCmx7262, THREE_FRAME | HDD | FEC);
	// Clear any bits set in the status register and align the shadow register.
	CMX7262_FlushStatusReg(pCmx7262);
	return 1;

}

// These functions provide a clear interface to the DMR application calling them. They simply read and write
// data to and from the codec FIFOs and extract the interface and packet size from the Cmx7262 data structure.

void CMX7262_RxFIFO  (CMX7262_TypeDef  *pCmx7262, uint8_t *pData)
{
	CBUS_Read8 (CBUS_VOCODER_OUT,pData,pCmx7262->uPacketSize,pCmx7262->uInterface);
}

void CMX7262_TxFIFO  (CMX7262_TypeDef  *pCmx7262, uint8_t *pData)
{
	CBUS_Write8(CBUS_VOCODER_IN,pData,pCmx7262->uPacketSize,pCmx7262->uInterface);
}


void CMX7262_Idle (CMX7262_TypeDef *pCmx7262)
{
	// Setting the codec to IDLE when it is already IDLE sometimes causes a problem with the
	// the CMX7262 FI. This has been verified as a known issue and the problem found.

	if (pCmx7262->uMode != CMX7262_IDLE_MODE)
	{
		// Disable CBUS IRQs before putting the codec into into Idle mode.
		pCmx7262->uIRQ_ENABLE_REG &= ~(IRQ+ODA+IDW+UF);
		CBUS_Write16(IRQ_ENABLE_REG,&pCmx7262->uIRQ_ENABLE_REG,1,pCmx7262->uInterface);

		if (!CMX7262_Transcode(pCmx7262,CMX7262_VCTRL_IDLE))
			pCmx7262->uError |= CMX7262_IDLE_ERROR;
		else
			pCmx7262->uMode = CMX7262_IDLE_MODE;
	}
        
	CMX7262_AudioPA(pCmx7262,DISABLE);      //Audio PA Off

	// The codec could have underflowed or set any of the IRQ flags before the Idle mode change took effect,
	// As we confirm the mode change we read the status register and pick up any other flags such as the
	// underflow flag (which is valid for a starved decoder). Therefore after the idle we clear the flags
	// in the shadow status register. As good measure I have added this to all calls to Idle - rrespective
	// of wether the hardware is already idle or not.
	pCmx7262->uIRQ_STATUS_REG &= ~(IRQ+ODA+IDW+OV+UF);

}


void CMX7262_Encode (CMX7262_TypeDef *pCmx7262)
{
	// PCM samples in through audio port and TWELP out through CBUS - in relation to the CMX7262
	CMX7262_Routing(pCmx7262, SRC_AUDIO | DEST_CBUS);
	// The encoder is started, there will be a packet delay before  we are requested to service it..
	if (!CMX7262_Transcode (pCmx7262,CMX7262_VCTRL_ENCODE))
		pCmx7262->uError |= CMX7262_ENCODE_ERROR;
	else
	{
		// Set the soft copy of the mode before we enable the IRQ because this is used by the IRQ
		// to set the appropriate request flags.
		pCmx7262->uMode = CMX7262_ENCODE_MODE;
		CMX7262_EnableIRQ(pCmx7262, IRQ+ODA);
	}
}


// We no longer enable the IDW interrupt in this routine because the decoder (Radio Rx) is fed samples
// when they become available from the modem. These are triggered by modem IRQs. A general
// note - To start the decoder we manually write the first TWELP packet without checking the IDW
// flag to start decoding. At the start, the decoder generates interrupts at a higher rate to fill its
// pipeline. Normal is 60mS for three frames per packet. If we do not service the interrupts at the
// higher rate then we could see multiple occurrences of IDW before we service.

void CMX7262_Decode (CMX7262_TypeDef *pCmx7262)
{
	// PCM samples out through audio port and TWELP in through CBUS - in relation to the CMX7262.
	CMX7262_Routing(pCmx7262, SRC_CBUS | DEST_AUDIO);
	//Audio PA On.
	CMX7262_AudioPA(pCmx7262,ENABLE);
	// So this routine is taking a long time to execute. About 10mS.
	if(!CMX7262_Transcode(pCmx7262,CMX7262_VCTRL_DECODE))
		pCmx7262->uError |= CMX7262_DECODE_ERROR;
	else
	{
		pCmx7262->uMode = CMX7262_DECODE_MODE;
		CMX7262_EnableIRQ(pCmx7262, IRQ+UF/*+IDW*/);        //enable underflow irq so we know when the call is over.
	}
}


void CMX7262_MMI_AudioGain (CMX7262_TypeDef *pCmx7262)
{
	CBUS_Write16(AOG3,&pCmx7262->uOutputGain,1,pCmx7262->uInterface);
	GPIO_ToggleBits(GPIOE, TEST_GREEN_LED_D7);		// Toggle Green LED
}


// Selects the codec mode and verifies that the mode change has taken
// effect by waiting on the REG done bit in the status register. The software
// copy of the mode is also updated. If the mode change does not take place
// within 15mS, a 0 is returned for failure, otherwise a 1 for success.

uint16_t CMX7262_Transcode (CMX7262_TypeDef *pCmx7262, uint16_t uMode)
{

	uint16_t uData;

	// Set the codec mode.
	CBUS_Write16(VCTRL_REG,(uint16_t *)&uMode,1,pCmx7262->uInterface);
	// Wait until we have confirmation of the mode being set.
	TIM_SetCounter(TIM5,0);
	while (TIM_GetCounter(TIM5) < CMX7262_TRANSCODE_TIMEOUT)
	{
		CBUS_Read16 (IRQ_STATUS_REG,&uData,1,pCmx7262->uInterface);
		pCmx7262->uIRQ_STATUS_REG |= uData;
		if ((pCmx7262->uIRQ_STATUS_REG & REGDONE) == REGDONE)
		{
			// Clear the REGDONE bit in the shadow regsiter.
			pCmx7262->uIRQ_STATUS_REG &= (uint16_t)(~REGDONE);
			return 1;
		}
	}
	// If we get here we have timed out and the mode selection was not successful,
	// so return 0.
	return 0;

}


// This is called from the IRQ. The parameter is void so that the function pointer that
// references this can handle different parameter types.

void CMX7262_IRQ (void *pData)
{

	uint16_t uTemp;
	CMX7262_TypeDef * pCmx7262;

	//GPIO_ToggleBits(GPIOE, TEST_GREEN_LED_D7);		// Toggle Green LED

	pCmx7262 = (CMX7262_TypeDef*)pData;

	// Read the status register into a shaoow register.
	CBUS_Read16 (IRQ_STATUS_REG,&uTemp,1,pCmx7262->uInterface);
	pCmx7262->uIRQ_STATUS_REG |= uTemp;

	// Check the appropriate bits in the shadow register based on the codec mode.
	if(pCmx7262->uMode==CMX7262_ENCODE_MODE)
	{
		if( (pCmx7262->uIRQ_STATUS_REG & ODA) == ODA)
		{
			// Clear the bit in the shadow register.
			pCmx7262->uIRQ_STATUS_REG &= (uint16_t)(~ODA);
			// The control  flag should not be still set, if it is, there is something wrong so set the
			// error flag. This will be picked up by the SysTick handler.
			// Set the flag in the control field to request appropriate action by routines
			// in PendSV.
			if((pCmx7262->uIRQRequest & CMX7262_ODA) == CMX7262_ODA)
				pCmx7262->uError |= CMX7262_ODA_ERROR;
			pCmx7262->uIRQRequest |= CMX7262_ODA;
		}
	}
	
	// Select IRQ flags to check based on the codec mode.
	if(pCmx7262->uMode==CMX7262_DECODE_MODE)
	{
		if( (pCmx7262->uIRQ_STATUS_REG & IDW) == IDW)
		{
			 // Clear the bit in the shadow register.
			 pCmx7262->uIRQ_STATUS_REG &= (uint16_t)(~IDW);
			 // The control  flag should not be still set, if it is, there is something wrong so set the
			 // error flag. This will be picked up by the SysTick handler.
			 // Set the flag in the control field to request appropriate action by routines
			 // in PendSV.
			 if((pCmx7262->uIRQRequest & CMX7262_IDW) == CMX7262_IDW)
				 pCmx7262->uError |= CMX7262_IDW_ERROR;
			 pCmx7262->uIRQRequest |= CMX7262_IDW;
		}

		if( (pCmx7262->uIRQ_STATUS_REG & UF) == UF)
		{
			 // Clear the bit in the shadow register.
			 pCmx7262->uIRQ_STATUS_REG &= (uint16_t)(~UF);
			 // The control  flag should not be still set, if it is, there is something wrong so set the
			 // error flag. This will be picked up by the SysTick handler.
			 // Set the flag in the control field to request appropriate action by routines
			 // in PendSV.
			 //if((pCmx7262->uIRQRequest & UF) == UF)
			 //	 pCmx7262->uError |= CMX7262_UF_ERROR;
			 pCmx7262->uIRQRequest |= UF;
		}
                 
	}

	// If there are errors, set a bit in the error field which will be picked up by the SysTick.
	// Catch a data overflow.
	if( (pCmx7262->uIRQ_STATUS_REG & OV) == OV)
	{
		pCmx7262->uIRQ_STATUS_REG &= (uint16_t)(~OV);
		pCmx7262->uError |= CMX7262_OV_ERROR;
	}
	
	// Catch a data underflow.
	//if( (pCmx7262->uIRQ_STATUS_REG & UF) == UF)
	//{
	//	 pCmx7262->uIRQ_STATUS_REG &= (uint16_t)(~UF);
	//	 pCmx7262->uError |= CMX7262_UF_ERROR;
	//}
}


// Configure the audio paths for the vocoder. There are several options, input and
// output samples can be routed through CBUS FIFOs or the analog ports. The
// parameter passed to this function configures this.
void CMX7262_Routing (CMX7262_TypeDef  *pCmx7262, uint16_t uData)
{
	CBUS_Write16(AUDIO_ROUTING_REG, (uint16_t *)&uData ,1,pCmx7262->uInterface);
}


// This routine sets the packet size (vocoder output frame  size) which is a variable
// used by the codec read and write routines. It also writes directly to the configuration
// register FEC, hard or soft decision data.

void CMX7262_Config (CMX7262_TypeDef  *pCmx7262, uint16_t uConfig)
{
	if((uConfig & FEC_MASK)==FEC)
	{
		// Calculate the size of a packet transfer based on the size of a frame and the number of
		// frames per packet.
		pCmx7262->uPacketSize = TWELP_FEC_HDD_FRAME_SIZE_BYTES * (FRAME_MASK & uConfig);
	}
	else
	{
		// Calculate the size of a packet transfer based on the size of a frame and the number of
		// frames per packet.
		pCmx7262->uPacketSize = TWELP_HDD_FRAME_SIZE_BYTES * (FRAME_MASK & uConfig);
	}
	CBUS_Write16(VCFG_REG,&uConfig,1,pCmx7262->uInterface);

}

// This routine allows the interrupts to be set without over writing previous IRQ settings.

void	CMX7262_EnableIRQ (CMX7262_TypeDef *pCmx7262, uint16_t uIRQ)
{
	// Set the bits in the interrupt enable shadow register, then write to the cbus
	// register in hardware.
	pCmx7262->uIRQ_ENABLE_REG |= uIRQ;
	CBUS_Write16 (IRQ_ENABLE_REG,&pCmx7262->uIRQ_ENABLE_REG,1,pCmx7262->uInterface);
}

void	CMX7262_DisableIRQ (CMX7262_TypeDef *pCmx7262, uint16_t uIRQ)
{
	// Clear the mask bits in the interrupt enable shadow register, then write to the
	// cbus register in hardware.
	pCmx7262->uIRQ_ENABLE_REG &= ~uIRQ;
	CBUS_Write16(IRQ_ENABLE_REG,&pCmx7262->uIRQ_ENABLE_REG,1,pCmx7262->uInterface);
	pCmx7262->uIRQ_STATUS_REG &= ~uIRQ;
}


// Set the audio input gain. Note that the full 16 bit register for input and output is set by the parameter..
void CMX7262_AudioInputGain (CMX7262_TypeDef  *pCmx7262)
{
	uint16_t uData;
	uData = (uint16_t)pCmx7262->sInputGain;
	// Position the gain to ANAIN2
	uData = uData << 8;
	CBUS_Write16(ANAIN_GAIN,&uData,1,pCmx7262->uInterface);
}


// Sets speaker output course gain.
// bit15 selects +6dB of gain.
// bits 0-6 (range limited to 0-59) selects attenuation in steps of -0.8dB, 0 = no attenuation,
// 59 *-0.8 = -47.2dB.
void CMX7262_AudioOutputGain (CMX7262_TypeDef  *pCmx7262, uint16_t uGain)
{
	CBUS_Write16(AOG3,&uGain,1,pCmx7262->uInterface);
}



// Read ths status register and align the shadow register to the hardware register.
void CMX7262_FlushStatusReg (CMX7262_TypeDef  *pCmx7262)
{
	uint16_t uData;
	// Flush by reading the status register.
	CBUS_Read16 (IRQ_STATUS_REG,&uData,1,pCmx7262->uInterface);
	// Align the shadow register to the hardware status register.
	CBUS_Read16 (IRQ_STATUS_REG,(uint16_t *)(&pCmx7262->uIRQ_STATUS_REG),1,pCmx7262->uInterface);
}


// Group all the hardware initialisation into one routine.
uint16_t  CMX7262_InitHardware (CMX7262_TypeDef *pCmx7262 )
{
	uint16_t uData;

	// Configure the clocks
	if(!CMX7262_ConfigClocks(pCmx7262))
		return 0;
	// Configure analog blocks
	CMX7262_AnalogBlocks(pCmx7262);
	// Setup the input and output gains.
	CMX7262_AudioInputGain(pCmx7262);
	CMX7262_AudioOutputGain(pCmx7262,CMX7262_OUPUT_GAIN_DEFAULT);
	// Switch on the PA
	//CMX7262_AudioPA(pCmx7262,ENABLE);     //moved to CMX7262_Decode
	// Enable register write confirmation for VCTRL
	uData = 0x0008;
	CBUS_Write16(REG_DONE_SELECT,&uData,1,pCmx7262->uInterface);
	return 1;

}

void  CMX7262_AnalogBlocks (CMX7262_TypeDef *pCmx7262 )
{
	uint16_t uData;

	// Power up the appropriate analog blocks - Start
	// DAC Pwr, OP Bias, SPKR2, Enable DrvPwr 1&2
	uData = 0x086A;
	CBUS_Write16(ANAOUT_CONFIG,&uData,1,pCmx7262->uInterface);
	// Single ended uses ANAIN2, differential uses ANAIN1.
	// J24 Pins 1 to 2, 3 to 4 and 7 to 8, 9 to 10 need shorting.
	//uData = 0x0802; 	// ANAIN1 - Differential input
	// ADC Pwr, ANA Sw, ANAIN2 Pwr
	uData = 0x0A08;
	CBUS_Write16(ANAIN_CONFIG,&uData,1,pCmx7262->uInterface);
	// Power up the appropriate analog blocks - End

}



// Configure the CMX7262 clocks. Returns 1 for a success, otherwise 0 for a failure.

uint16_t CMX7262_ConfigClocks (CMX7262_TypeDef  *pCmx7262)
{
	uint16_t data;

	// Flush by reading the status register.
	CBUS_Read16 (IRQ_STATUS_REG,&data,1,pCmx7262->uInterface);

	data = 0x210;	// Select program block 1.2
	CBUS_Write16 (VCTRL_REG,&data,1,pCmx7262->uInterface);
	data = 40;			// Set ref clk Divide in Rx or Tx Mode
	CBUS_Write16 (PROG_REG,&data,1,pCmx7262->uInterface);
	if(!CBUS_WaitBitSet16 (IRQ_STATUS_REG, PRG, pCmx7262->uInterface))
		return 0;		// Program fail.
	data = 208;		// Set PLL clk Divide in Rx or Tx Mode
	CBUS_Write16 (PROG_REG,&data,1,pCmx7262->uInterface);
	if(!CBUS_WaitBitSet16 (IRQ_STATUS_REG, PRG, pCmx7262->uInterface))
		return 0;		// Program fail.
	data = 0x41;		// Set Tx/Rx Internal Clock Divide
	CBUS_Write16 (PROG_REG,&data,1,pCmx7262->uInterface);
	if(!CBUS_WaitBitSet16 (IRQ_STATUS_REG, PRG, pCmx7262->uInterface))
		return 0;		// Program fail.
	data = FS_DIV;		// Set I/Q IO Clock Divide
	CBUS_Write16 (PROG_REG,&data,1,pCmx7262->uInterface);
	if(!CBUS_WaitBitSet16 (IRQ_STATUS_REG, PRG, pCmx7262->uInterface))
		return 0;		// Program fail

	data = 0xE10;	// Select program block 1.E
	CBUS_Write16(VCTRL_REG,&data,1,pCmx7262->uInterface);
	data = 0;				// XTAL Driver Enable
	CBUS_Write16(PROG_REG,&data,1,pCmx7262->uInterface);
	if(!CBUS_WaitBitSet16(IRQ_STATUS_REG, PRG, pCmx7262->uInterface))
		return 0;		// Program fail.

	return 1;			// Configuration was a success.

}


// This routine will set (disable) or clear (enable) the PA.
void CMX7262_AudioPA (CMX7262_TypeDef  *pCmx7262, FunctionalState eState)
{
	if (eState == ENABLE)
		GPIO_ResetBits(GPIOE, AUDIO_PA);
	else
		GPIO_SetBits(GPIOE ,AUDIO_PA);
}


