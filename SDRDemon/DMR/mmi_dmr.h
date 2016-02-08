/************************************************************************************************************
 *
 *  $Copyright © 2013-2014 CML Systems $
 *
 *  $Header: mmi_dmr.h  Revision:1.8.1.2  18 February 2014 10:38:28  ilewis $
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

#ifndef _MMI_DMR_H
#define _MMI_DMR_H

#include "mainDMR.h"

#define INIT_STRING "CML DMR Demo"
#define	VERSION	"Ver ES994334"

#define MIN_CHANNEL 0
#define MAX_CHANNEL 7//24

#define MAX_SETUP_VALUES 6
#define MIN_SETUP_ENTRY 0
#define MAX_SETUP_ENTRY 12

#define MAX_VOLUME 20
#define MIN_VOLUME 0

#ifdef DEBUG_CHANGE_INC_FROM_INITIAL_SDR_PROJECT
 #include "..\MMI\mmi_types.h"
#else
 #include "mmi_types.h"
#endif

#ifdef DEBUG_DEFINE_TRUE_AS_1
	#define TRUE 	(1)
	#define FALSE (0)	
#endif

extern Channel Channels[MAX_CHANNEL+1];
extern Setup_TypeDef SetupArray[MAX_SETUP_ENTRY+1];

void CheckRadioMessages(void);
void ProcessKeys(void);
void SetVol(void);
void tx_pattern_handler(int index);
void pa_handler(int index);
void rx_mode_handler(int index);
void freq_control_handler (int index);
void CBUS_5E_handler(int index);
void CBUS_B4_handler(int index);
void CMX994_DC_Offset_handler (int index);
void CMX994_Gain_handler (int index);
void MicInputGain_handler (int index);
void dummy(int index);
void Save_Handler (int index);
void TestMode_Handler (int index);
void BatteryCheck (void);
void ReadRSSI(void);

#endif
