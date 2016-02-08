/************************************************************************************************************
 *
 *  $Copyright © 2013-2014 CML Systems $
 *
 *  $Header: radio.h  Revision:1.6  18 February 2014 10:43:44  ddavenport $
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

#ifndef _RADIO_H
#define _RADIO_H


#define RADIO_SPI_DONE_BIT			(1<<5)

#define RADIO_SPI_DATA						0x63
#define RADIO_SPI_CON						0x62
#define RADIO_SPI_RD							0x78


void Radio_SkyWorks (uint8_t frac1, uint8_t frac2, uint16_t uInterface);
uint16_t Radio_CMX994Config (CMX7161_TypeDef *pCmx7161 );

#endif
