/************************************************************************************************************
 *
 *  $Copyright © 2013-2014 CML Systems $
 *
 *  $Header: radio.c  Revision:1.11  18 February 2014 10:43:20  ddavenport $
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

//uint8_t frac2[] = {0x55,0x00,0xAB,0x55,0x00, 0xAB, 0x55, 0x00, 0xAB};
//uint8_t frac1[] = {0xC1,0xC4,0xC6,0xC9,0xCC, 0xCE, 0xD1, 0xD4, 0xAA};

// This is a seperate piece of hardware located on CBUS interface 2.
// Skyworks SKY72300_4 bit address, 12 bit data is the word format
void Radio_SkyWorks (uint8_t frac1, uint8_t frac2, uint16_t uInterface)
{
		uint16_t	uData;

		// PhaseDet/Cp cntrl of skyworks 12 bit data
		// Address=6, data=0x008 (250uA CP current), data=0x012 (562.5uA) and data=0x018 (750uA)
		uData = 0x04;
		CBUS_Write8(0x60, (uint8_t *)&uData, 1, uInterface);
		// PowerDown/MUX Cntrl of skyworks 12 bit data
		// Address=7, data=0x010 (Disables AUX side of chip)
		uData = 0x10;
		CBUS_Write8(0x72, (uint8_t *)&uData, 1, uInterface);
		// Ref Freq Div of skyworks 12 bit data
		// Address=5, data=0x021
		uData = 0x21;
		CBUS_Write8(0x50, (uint8_t *)&uData, 1, uInterface);
		// Main Divider of skyworks 12 bit data
		// Address=0, data=frac0
		uData = 0x3D;
		CBUS_Write8(0x00, (uint8_t *)&uData, 1, uInterface);
		// Main_Div_LSB of skyworks 12 bit data
		// Address=2, data=frac2
		// DAVED DEBUG
		//uData = 0x55;		// Channel 1
		uData = frac2;
		CBUS_Write8(0x20, (uint8_t *)&uData, 1, uInterface);
		// Main_Div_MSB of skyworks 12 bit data
		// Address=1, data=frac1a and 1b
		// DAVED DEBUG
		//uData = 0xC1;		// Channel 1
		uData = frac1;
		CBUS_Write8(0x13, (uint8_t *)&uData, 1, uInterface);

}

// Returns 1 for successfull configuration, or 0 for failure.

uint16_t Radio_CMX994Config (CMX7161_TypeDef *pCmx7161 )
{
	uint16_t uData;
	// Disable PLL - 80h to use ext LO and PLL
	// General Cntrl Reg, interrupt when accepted, but SPI incomplete
	uData = 0x80;
	CBUS_Write16(RADIO_SPI_DATA,&uData,1,pCmx7161-> uInterface);
	uData = 0x0111;
	CBUS_Write16(RADIO_SPI_CON,&uData,1, pCmx7161-> uInterface);
	if(CMX7161_ReadStatusReg (pCmx7161,RADIO_SPI_DONE_BIT) == 0)
	{
		pCmx7161->uError = pCmx7161->uError | CMX7161_CMX994_RX_ERROR;
		return 0;
	}
	// VCO Control Reg
	uData = 0x10;
	CBUS_Write16(RADIO_SPI_DATA,&uData,1, pCmx7161-> uInterface);
	uData = 0x0125;
	CBUS_Write16(RADIO_SPI_CON,&uData,1, pCmx7161-> uInterface);
	if(CMX7161_ReadStatusReg (pCmx7161,RADIO_SPI_DONE_BIT) == 0)
	{
		pCmx7161->uError = pCmx7161->uError | CMX7161_CMX994_RX_ERROR;
		return 0;
	}
	// 10 is div by 2 and max BW, 08 is div by 2 and mid BW
	// Rx Cntrl Reg
	//uData = 0x10;
	uData = 0x08;
	CBUS_Write16(RADIO_SPI_DATA,&uData,1, pCmx7161-> uInterface);
	uData = 0x0112;
	CBUS_Write16(RADIO_SPI_CON,&uData,1, pCmx7161-> uInterface);
	if(CMX7161_ReadStatusReg (pCmx7161,RADIO_SPI_DONE_BIT) == 0)
	{
		pCmx7161->uError = pCmx7161->uError | CMX7161_CMX994_RX_ERROR;
		return 0;
	}
	// 00 = Max gain in 100R mode and 08 = max gain in 50R mode
	// Rx gain Reg
	//uData = 0x00;
	uData = (uint16_t)pCmx7161->sCMX994Gain;
        CBUS_Write16(RADIO_SPI_DATA,&uData,1, pCmx7161-> uInterface);
	uData = 0x0116;
	CBUS_Write16(RADIO_SPI_CON,&uData,1, pCmx7161-> uInterface);
	if(CMX7161_ReadStatusReg (pCmx7161,RADIO_SPI_DONE_BIT) == 0)
	{
		pCmx7161->uError = pCmx7161->uError | CMX7161_CMX994_RX_ERROR;
		return 0;
	}
	// Rx DC offset Reg
	//data = 0xA0;
	uData = (uint16_t)pCmx7161->sCMX994DcOffset;
	CBUS_Write16(RADIO_SPI_DATA,&uData,1, pCmx7161-> uInterface);
	uData = 0x0113;
	CBUS_Write16(RADIO_SPI_CON,&uData,1, pCmx7161-> uInterface);
	if(CMX7161_ReadStatusReg (pCmx7161,RADIO_SPI_DONE_BIT) == 0)
	{
		pCmx7161->uError = pCmx7161->uError | CMX7161_CMX994_RX_ERROR;
		return 0;
	}
	// 3F for best IM performance
	// IM Reg
	//uData = 0x00;
	uData = 0x3F;
	CBUS_Write16(RADIO_SPI_DATA,&uData,1, pCmx7161-> uInterface);
	uData = 0x0114;
	CBUS_Write16(RADIO_SPI_CON,&uData,1, pCmx7161-> uInterface);
	if(CMX7161_ReadStatusReg (pCmx7161,RADIO_SPI_DONE_BIT) == 0)
	{
		pCmx7161->uError = pCmx7161->uError | CMX7161_CMX994_RX_ERROR;
		return 0;
	}
	return 1;
}

