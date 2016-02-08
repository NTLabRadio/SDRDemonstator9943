/************************************************************************************************************
 *
 *  $Copyright © 2012-2014 CML Systems $
 *
 *  $Header: mainDMR.c  Revision:1.30  18 February 2014 10:14:17  ddavenport $
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

#include <stdio.h>
#include <string.h>

#ifdef DEBUG_CHANGE_INC_FROM_INITIAL_SDR_PROJECT
 #include "..\common\stm32f10x_it.h"
#else
 #include "stm32f10x_it.h"
#endif
#include "mainDMR.h"

#include "mmi_dmr.h"
#ifdef DEBUG_CHANGE_INC_FROM_INITIAL_SDR_PROJECT
 #include "..\MMI\mmi_defines.h"
#else
 #include "mmi_defines.h"
#endif

#ifndef DEBUG_CHANGE_INC_FROM_INITIAL_SDR_PROJECT
 #include "prbs511.h"				// Includes definition for the test pattern
#else
 #define PRBS_LEN (511)
 uint8_t aPRBS[PRBS_LEN] = {0xFF, 0x83, 0xDF, 0x17, 0x32, 0x09, 0x4E, 0xD1, 0xE7, 0xCD, 0x8A, 0x91, 0xC6, 0xD5, 0xC4, 0xC4, 
														0x40, 0x21, 0x18, 0x4E, 0x55, 0x86, 0xF4, 0xDC, 0x8A, 0x15, 0xA7, 0xEC, 0x92, 0xDF, 0x93, 0x53, 
														0x30, 0x18, 0xCA, 0x34, 0xBF, 0xA2, 0xC7, 0x59, 0x67, 0x8F, 0xBA, 0x0D, 0x6D, 0xD8, 0x2D, 0x7D, 
														0x54, 0x0A, 0x57, 0x97, 0x70, 0x39, 0xD2, 0x7A, 0xEA, 0x24, 0x33, 0x85, 0xED, 0x9A, 0x1D, 0xE1, 
														0xFF, 0x07, 0xBE, 0x2E, 0x64, 0x12, 0x9D, 0xA3, 0xCF, 0x9B, 0x15, 0x23, 0x8D, 0xAB, 0x89, 0x88, 
														0x80, 0x42, 0x30, 0x9C, 0xAB, 0x0D, 0xE9, 0xB9, 0x14, 0x2B, 0x4F, 0xD9, 0x25, 0xBF, 0x26, 0xA6, 
														0x60, 0x31, 0x94, 0x69, 0x7F, 0x45, 0x8E, 0xB2, 0xCF, 0x1F, 0x74, 0x1A, 0xDB, 0xB0, 0x5A, 0xFA, 
														0xA8, 0x14, 0xAF, 0x2E, 0xE0, 0x73, 0xA4, 0xF5, 0xD4, 0x48, 0x67, 0x0B, 0xDB, 0x34, 0x3B, 0xC3, 
														0xFE, 0x0F, 0x7C, 0x5C, 0xC8, 0x25, 0x3B, 0x47, 0x9F, 0x36, 0x2A, 0x47, 0x1B, 0x57, 0x13, 0x11, 
														0x00, 0x84, 0x61, 0x39, 0x56, 0x1B, 0xD3, 0x72, 0x28, 0x56, 0x9F, 0xB2, 0x4B, 0x7E, 0x4D, 0x4C, 
														0xC0, 0x63, 0x28, 0xD2, 0xFE, 0x8B, 0x1D, 0x65, 0x9E, 0x3E, 0xE8, 0x35, 0xB7, 0x60, 0xB5, 0xF5, 
														0x50, 0x29, 0x5E, 0x5D, 0xC0, 0xE7, 0x49, 0xEB, 0xA8, 0x90, 0xCE, 0x17, 0xB6, 0x68, 0x77, 0x87, 
														0xFC, 0x1E, 0xF8, 0xB9, 0x90, 0x4A, 0x76, 0x8F, 0x3E, 0x6C, 0x54, 0x8E, 0x36, 0xAE, 0x26, 0x22, 
														0x01, 0x08, 0xC2, 0x72, 0xAC, 0x37, 0xA6, 0xE4, 0x50, 0xAD, 0x3F, 0x64, 0x96, 0xFC, 0x9A, 0x99, 
														0x80, 0xC6, 0x51, 0xA5, 0xFD, 0x16, 0x3A, 0xCB, 0x3C, 0x7D, 0xD0, 0x6B, 0x6E, 0xC1, 0x6B, 0xEA, 
														0xA0, 0x52, 0xBC, 0xBB, 0x81, 0xCE, 0x93, 0xD7, 0x51, 0x21, 0x9C, 0x2F, 0x6C, 0xD0, 0xEF, 0x0F, 
														0xF8, 0x3D, 0xF1, 0x73, 0x20, 0x94, 0xED, 0x1E, 0x7C, 0xD8, 0xA9, 0x1C, 0x6D, 0x5C, 0x4C, 0x44, 
														0x02, 0x11, 0x84, 0xE5, 0x58, 0x6F, 0x4D, 0xC8, 0xA1, 0x5A, 0x7E, 0xC9, 0x2D, 0xF9, 0x35, 0x33, 
														0x01, 0x8C, 0xA3, 0x4B, 0xFA, 0x2C, 0x75, 0x96, 0x78, 0xFB, 0xA0, 0xD6, 0xDD, 0x82, 0xD7, 0xD5, 
														0x40, 0xA5, 0x79, 0x77, 0x03, 0x9D, 0x27, 0xAE, 0xA2, 0x43, 0x38, 0x5E, 0xD9, 0xA1, 0xDE, 0x1F, 
														0xF0, 0x7B, 0xE2, 0xE6, 0x41, 0x29, 0xDA, 0x3C, 0xF9, 0xB1, 0x52, 0x38, 0xDA, 0xB8, 0x98, 0x88, 
														0x04, 0x23, 0x09, 0xCA, 0xB0, 0xDE, 0x9B, 0x91, 0x42, 0xB4, 0xFD, 0x92, 0x5B, 0xF2, 0x6A, 0x66, 
														0x03, 0x19, 0x46, 0x97, 0xF4, 0x58, 0xEB, 0x2C, 0xF1, 0xF7, 0x41, 0xAD, 0xBB, 0x05, 0xAF, 0xAA, 
														0x81, 0x4A, 0xF2, 0xEE, 0x07, 0x3A, 0x4F, 0x5D, 0x44, 0x86, 0x70, 0xBD, 0xB3, 0x43, 0xBC, 0x3F, 
														0xE0, 0xF7, 0xC5, 0xCC, 0x82, 0x53, 0xB4, 0x79, 0xF3, 0x62, 0xA4, 0x71, 0xB5, 0x71, 0x31, 0x10, 
														0x08, 0x46, 0x13, 0x95, 0x61, 0xBD, 0x37, 0x22, 0x85, 0x69, 0xFB, 0x24, 0xB7, 0xE4, 0xD4, 0xCC, 
														0x06, 0x32, 0x8D, 0x2F, 0xE8, 0xB1, 0xD6, 0x59, 0xE3, 0xEE, 0x83, 0x5B, 0x76, 0x0B, 0x5F, 0x55, 
														0x02, 0x95, 0xE5, 0xDC, 0x0E, 0x74, 0x9E, 0xBA, 0x89, 0x0C, 0xE1, 0x7B, 0x66, 0x87, 0x78, 0x7F, 
														0xC1, 0xEF, 0x8B, 0x99, 0x04, 0xA7, 0x68, 0xF3, 0xE6, 0xC5, 0x48, 0xE3, 0x6A, 0xE2, 0x62, 0x20, 
														0x10, 0x8C, 0x27, 0x2A, 0xC3, 0x7A, 0x6E, 0x45, 0x0A, 0xD3, 0xF6, 0x49, 0x6F, 0xC9, 0xA9, 0x98, 
														0x0C, 0x65, 0x1A, 0x5F, 0xD1, 0x63, 0xAC, 0xB3, 0xC7, 0xDD, 0x06, 0xB6, 0xEC, 0x16, 0xBE, 0xAA, 
														0x05, 0x2B, 0xCB, 0xB8, 0x1C, 0xE9, 0x3D, 0x75, 0x12, 0x19, 0xC2, 0xF6, 0xCD, 0x0E, 0xF0};
#endif

//#define DATA_SLOT_IRQ		// Tx modem data based on the modem Slot IRQ.
#define DATA_CODEC_IRQ	// Tx modem data based on the codec IRQ
//#define SPEECH_CODEC_IRQ

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

// DAVED DEBUG - Start

const uint8_t	aModemTestSamples[CMX7262_CODEC_BUFFER_SIZE] = {
		0xD0,0xC0,0xB0,0xA0,0x90,0x80,0x70,0x60,0x50,0x40,0x30,0x20,0x10,0x00,
		0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D
};

// Added 2 packets for good measure.
uint8_t	aTestSamples[CMX7262_CODEC_BUFFER_SIZE * ( NUM_TEST_PACKETS+2)];
__IO uint16_t uAudioTestFrameCount;
__IO uint8_t *pTestSample;

// DAVED DEBUG
uint16_t uPRBSByteCount;;
uint16_t uPRBSBitErrorCount;
uint16_t uPRBSFrameCount;
uint16_t uPRBSOldBitErrorCount;
uint16_t uPRBSOldFrameCount;

// DAVED DEBUG - End
// Sync pattern definition
uint8_t	VOICE_SYNC[CMX7161_DMR_SYNC_SIZE] = {0x7F,0x7D,0x5D,0xD5,0x7D,0xFD};
// Buffer for sync data
uint8_t aSync[CMX7161_DMR_SYNC_SIZE];
// Buffer for payload data.
uint8_t aPayLoad[CMX7262_CODEC_BUFFER_SIZE] = {
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};


// Local function declarations that are not global.
void DMR_Init (void );

void DMR_MMI_Idle_Handler (void);	// Handler for the MMI Idle state.
void DMR_MMI_PTTKey_Handler (void);
void DMR_MMI_StarKey_Handler (void );
void DMR_MMI_Volume (void);
void DMR_MMI_DataOn_Handler (void);
void DMR_MMI_DataOff_Handler (void);

void DMR_ErrorCheck ( void );
uint16_t DMR_CheckSync (uint8_t *pExtractedSync, uint8_t *pExpectedSync );
uint16_t DMR_SlotTestComparison (void );
void DMR_Dummy  (void);


void PRBS_CopyPattern (uint8_t *pOSamples, uint8_t uSize);
uint16_t PRBS_CheckPattern (uint8_t *pISamples, uint8_t uSize);

// Instance of DMR
DMR_TypeDef dmr;

int main(void)
{
	// Initialise ARM on the SDR card, specifically timers, GPIO, CBUS and UART.
	SDR_Init();

	// Call this here so that thr MMI can read the board number from flash during boot up.
	// The pointer to flash is initialised within this routine. Sets up the DMR and MMI request
	// state and initialises the function pointer table for MMI requests.
	DMR_Init();

	MMI_Init();


#if	0
// Debug for testing new counters.
	TIM_SetCounter(TIM4,0);
	while(1)
	{
		if (TIM_GetCounter(TIM4) > (uint16_t)(1000))
		{
			GPIO_ToggleBits(GPIOE, TEST_TL1);
			TIM_SetCounter(TIM4,0);
		}
	}
#endif

	CMX7161_Init(&dmr.Cmx7161,CBUS_INTERFACE_1);

	ClearCounter();
	// We wait for 2 seconds here in a loop while running the MMI. If the * key is pressed, then we jump
	// into MMI mode.
	while (ReadCounter() < 2000000)
	{
		MMI_Main ();
	}
	// We leave the MMI running, so that we can update the user on the progress of code load and
	// initialisation.

	// Initialise an instance of the CMX devices and load the FI.
	CMX7262_Init (&dmr.Cmx7262,CBUS_INTERFACE_3);
	//CMX7161_Init(&dmr.Cmx7161,CBUS_INTERFACE_1);

	// Initialise one of the GPIO lines on the ARM as an IRQ.
	// Then initialise the PendSV and set the System Tick running.
	IRQ_CBUSInit(CBUS_INTERFACE_3);
	IRQ_CBUSInit(CBUS_INTERFACE_1);
	IRQ_SystemInit ();

	// This delay gives the codec time to initialise. The explanation given is the time taken to
   	// initialise all the look up tables and so on.
	ClearCounter();
	while (ReadCounter() < 100000);

	CBUS_IRQTable[CBUS_INTERFACE_3].pHandler = CMX7262_IRQ;
	CBUS_IRQTable[CBUS_INTERFACE_3].uFlag = 0;
	CBUS_IRQTable[CBUS_INTERFACE_3].pParam = &dmr.Cmx7262;

	CBUS_IRQTable[CBUS_INTERFACE_1].pHandler = CMX7161_IRQ;
	CBUS_IRQTable[CBUS_INTERFACE_1].uFlag = 0;
	CBUS_IRQTable[CBUS_INTERFACE_1].pParam = &dmr.Cmx7161;

	// START

	CMX7161_ModemControl (&dmr.Cmx7161,cCMX7161_IDLE);
	CMX7262_Idle (&dmr.Cmx7262);

	// DAVED DEBUG - Set the output pins low for debug
	GPIO_ResetBits(GPIOE, TEST_TL4);
	GPIO_ResetBits(GPIOE, TEST_TL3);
	GPIO_ResetBits(GPIOE, TEST_TL2);
	GPIO_ResetBits(GPIOE, TEST_TL1);

	// Send a message to the MMI indicating initialisation complete.
	dmr.eRadioRequest = MMI_DMR_INIT_COMPLETE;
	dmr.eState = DMR_STATE_IDLE;

	while (1)
	{
		MMI_Main ();
	}

}


// Dummy function that does nothing, fills up blank entries in the function pointer table.
void DMR_Dummy  (void)
{
	return;
}


// The system tick is set to 1mS. All the data tasks and requests to control the modem and codec
// are made through this handler. Fundamentally this allows the MMI to run in the foreground
// and be interrupted as and when required. CBUS IRQ handlers (data requests) set flags which
// are acted upon by this handler. MMI requests set an index into a table of function pointers which this
// routine executes to call the appropriate control. The error flags are also checked by the handler.
// There is also a DMR state which is used by the the handler to schedule the sequencing of codec and
// modem.


void DMR_SysTick (void)
{
  static int16_t sDMRRxLostSyncCnt;				// Counts the number of packets without sync
  static int16_t sDMRRxStartDelayCnt;				// Counts the delay in the Rx startup
	static int16_t sDMRTxStartDelayCnt;				// Counts the delay in the Tx startup

	uint16_t i;

	// Check for any errors.
	DMR_ErrorCheck();

	// Check MMI requests
	// A handler is called from the function pointer table determined by the MMI request..
	(*dmr.pMMI_FunctionRequestTable[dmr.eMMIRequest])();


	switch(dmr.eState)
	{
		case DMR_STATE_TX_WAITING:
	    	// We have introduced a delay in the Tx startup, so that the modem is deliberately
	    	// delayed after the first codec samples arrive. This allows the encoder to slip
	    	// slightly on samples being available in relation to the modem requiring them,
	    	// before it becomes starved. Slip can be introduced by the system tick resolution
	    	// as well as encoder processing time.
			sDMRTxStartDelayCnt--;
			if (sDMRTxStartDelayCnt == 0)
			 {
				// Pre-load the samples before starting the modem.
				CMX7161_TxData (&dmr.Cmx7161,&aPayLoad[0],&VOICE_SYNC[0]);
				CMX7161_ModemControl (&dmr.Cmx7161,cCMX7161_TX_SLOT);
				dmr.eState = DMR_STATE_TX_RUNNING;
			 }
		break;
		case DMR_STATE_DATA_TX_DELAY_MODEM_START:
	    	// We have introduced a delay in the Tx startup, so that the modem is deliberately
	    	// delayed after the first codec samples arrive. This allows the encoder to slip
	    	// slightly on samples being available in relation to the modem requiring them,
	    	// before it becomes starved. Slip can be introduced by the system tick resolution
	    	// as well as encoder processing time.
			sDMRTxStartDelayCnt--;
			if (sDMRTxStartDelayCnt == 0)
			 {
				// Pre-load the samples before starting the modem.
				CMX7161_TxData (&dmr.Cmx7161,&aPayLoad[0],&VOICE_SYNC[0]);
				CMX7161_ModemControl (&dmr.Cmx7161,cCMX7161_TX_SLOT);
				dmr.eState = DMR_STATE_DATA_TX_RUNNING;
			 }
		break;
		case DMR_STATE_RX_WAITING:
			sDMRRxStartDelayCnt--;
	        if (sDMRRxStartDelayCnt == 0)
	        {
	            CMX7262_Decode (&dmr.Cmx7262);
	            CMX7262_TxFIFO(&dmr.Cmx7262,(uint8_t *)&aPayLoad[0]);
	            dmr.eState = DMR_STATE_RX_RUNNING;
	            dmr.eRadioRequest = MMI_DMR_START_RX;
	         }
		break;
		// DAVED DEBUG - Start
		case DMR_STATE_DATA_RX_WAITING:
			sDMRRxStartDelayCnt--;
	        if (sDMRRxStartDelayCnt == 0)
	        {
	        	CMX7262_Decode (&dmr.Cmx7262);
	           // Reset the error count, frame count and pattern count on receipt of the first packet from the Cmx7161
	            uPRBSByteCount = 0;
	            uPRBSFrameCount = 1;
	            uPRBSOldFrameCount = 0;
	            uPRBSOldBitErrorCount  = 0;
	            uPRBSBitErrorCount =  PRBS_CheckPattern(aPayLoad,CMX7262_CODEC_BUFFER_SIZE);
                // Zero out the cpodec samples so the audio does not sound bad.
                for(i=0;i<CMX7262_CODEC_BUFFER_SIZE;i++)
            		aPayLoad[i]=0;
            	CMX7262_TxFIFO(&dmr.Cmx7262,(uint8_t *)&aPayLoad[0]);
            	dmr.eState = DMR_STATE_DATA_RX_RUNNING;
            	dmr.eRadioRequest = MMI_DMR_RX_DATA_RUN;
	         }
		break;

		// DAVED DEBUG - End
		default:
		break;
	}


	// CHECK CMX7262 IRQ REQUESTS (DMR TX)

	if((dmr.Cmx7262.uIRQRequest & CMX7262_ODA) == CMX7262_ODA)
	{
		dmr.Cmx7262.uIRQRequest = dmr.Cmx7262.uIRQRequest & ~CMX7262_ODA;
		switch(dmr.eState)
		{
			// Codec has been started from a MMI request
			case DMR_STATE_TX_WAIT_FOR_ENCODER:
				CMX7262_RxFIFO(&dmr.Cmx7262,(uint8_t *)&aPayLoad[0]);
				// Load the count so that we delay the start of the CMX7161 modem.
				sDMRTxStartDelayCnt = DMR_TX_START_DELAY;
				dmr.eState = DMR_STATE_TX_WAITING;
			break;

			case DMR_STATE_TX_RUNNING:
				CMX7262_RxFIFO(&dmr.Cmx7262,(uint8_t *)&aPayLoad[0]);
				CMX7161_TxData (&dmr.Cmx7161,&aPayLoad[0],&VOICE_SYNC[0]);
				break;

			case DMR_STATE_TX_STOPPING :
				CMX7262_RxFIFO(&dmr.Cmx7262,(uint8_t *)&aPayLoad[0]);
				// Stop the codec. We are safe to do this here because a CMX7262 IRQ has just set the flag.
				CMX7262_Idle (&dmr.Cmx7262);
				// DAVED DEBUG
				// We need a check so that we do not shut the modem down if it is still transmitting.
				CMX7161_ModemControl (&dmr.Cmx7161,cCMX7161_IDLE);
				dmr.eState = DMR_STATE_IDLE;
				break;

			// **************************  Data  Start *********************************/
			/* For normal voice operation the data specific code can be ignored */

			// Codec has been started from an MMI request
			case DMR_STATE_DATA_TX_WAIT_FOR_ENCODER:
				CMX7262_RxFIFO(&dmr.Cmx7262,(uint8_t *)&aPayLoad[0]);
				uPRBSByteCount = 0;
				PRBS_CopyPattern(aPayLoad,CMX7262_CODEC_BUFFER_SIZE);
				// Load the count so that we delay the start of the CMX7161 modem.
				sDMRTxStartDelayCnt = DMR_TX_START_DELAY;
				dmr.eState = DMR_STATE_DATA_TX_DELAY_MODEM_START;
			break;

			case DMR_STATE_DATA_TX_RUNNING:
				CMX7262_RxFIFO(&dmr.Cmx7262,(uint8_t *)&aPayLoad[0]);
				PRBS_CopyPattern(aPayLoad,CMX7262_CODEC_BUFFER_SIZE);
				CMX7161_TxData (&dmr.Cmx7161,&aPayLoad[0],&VOICE_SYNC[0]);
			break;

			case DMR_STATE_DATA_TX_STOPPING :
				CMX7262_RxFIFO(&dmr.Cmx7262,(uint8_t *)&aPayLoad[0]);
				// Stop the codec. We are safe to do this here because a CMX7262 IRQ has just set the flag.
				CMX7262_Idle (&dmr.Cmx7262);
				// We need a check so that we do not shut the modem down if it is still transmitting.
				CMX7161_ModemControl (&dmr.Cmx7161,cCMX7161_IDLE);
				dmr.eState = DMR_STATE_DATA_IDLE;
			break;

			// **************************  Data  End *********************************/

			default:
				break;
		}

	}

    // Once the Rx Modem is running, in slotted mode the Rx FIFO IRQ will always fire
    // irrespective of good data or not. The modem has no knowledge of what it is receiving
    // expect for the initial sync.

    if( (dmr.Cmx7161.uIRQ_STATUS_REG & RXFIFO) == RXFIFO)
	{
		dmr.Cmx7161.uIRQ_STATUS_REG &= (~RXFIFO);
        switch (dmr.eState)
		{
			case DMR_STATE_RX_STARTUP:
                CMX7161_RxData (&dmr.Cmx7161,&aPayLoad[0],&aSync[0]);
                dmr.eState = DMR_STATE_RX_WAITING;
                sDMRRxStartDelayCnt = DMR_RX_START_DELAY;
			break;
			case DMR_STATE_RX_RUNNING:

				// Reset the lost packet count
				sDMRRxLostSyncCnt = DMR_RX_LOST_SYNC_COUNT;
				// Extract the Pay load and Sync from the Rx FIFO
                CMX7161_RxData (&dmr.Cmx7161,&aPayLoad[0],&aSync[0]);

                // Check the sync against an expected pattern.
                if (DMR_CheckSync (&aSync[0],&VOICE_SYNC[0] ) == 0)
                {
                	// We have received a packet without sync, so this is either end of call or a
                	// break in transmission. Decrement the lost packets count and over write
                	// the decoder samples with 0. Then set the DMR state to indicate loss of
                	// packet sync.
                	sDMRRxLostSyncCnt--;
                	for(i=0;i<CMX7262_CODEC_BUFFER_SIZE;i++)
                		aPayLoad[i]=0;
                	dmr.eState = DMR_STATE_RX_RUNNING_NO_SYNC;
                	GPIO_ToggleBits(GPIOE, TEST_TL1);
                }
    			// Write the pay load to the decoder

                CMX7262_TxFIFO(&dmr.Cmx7262,(uint8_t *)&aPayLoad[0]);

            break;
			case DMR_STATE_RX_RUNNING_NO_SYNC:
               // Extract the Pay load and Sync from the Rx FIFO
                CMX7161_RxData (&dmr.Cmx7161,&aPayLoad[0],&aSync[0]);
                // Check the sync against an expected pattern.
                if (DMR_CheckSync (&aSync[0],&VOICE_SYNC[0] )==0)
                {
                	// Overwrite the codec samples because we have a bad packet.
                	for(i=0;i<CMX7262_CODEC_BUFFER_SIZE;i++)
                		aPayLoad[i]=0;
                	sDMRRxLostSyncCnt--;
                	// With successive lost packets, end the call, send a stop receiving message to the MMI,
                	// stop the decoder and set the DMR state machine to Idle.
                	if (sDMRRxLostSyncCnt <= 0)
                	{
                		dmr.eRadioRequest = MMI_DMR_RX_STOP;
                		CMX7262_Idle (&dmr.Cmx7262);
                		// DAVED DEBUG - Start
                		// Disabling the IRQ and setting the modem to Idle clear any problems with the FIFO
                		// transfer problem in Rx. I believe this to be a problem with the CMX7161 FI since it has
                		// been seen in the script environment. Ordinarily we do not need to set the modem to idle
                		// here. It is sufficient to re-issue the receive command to reset the receiver.
                		CMX7161_DisableIRQ (&dmr.Cmx7161,0xFFFF);
                		CMX7161_ModemControl (&dmr.Cmx7161,cCMX7161_IDLE);
                		// DAVED DEBUG - End
                		dmr.eState = DMR_STATE_IDLE;
                		break;
                	}
                }
                else
                {
                	// If we regain packet sync move the DMR state back to running.
                	dmr.eState = DMR_STATE_RX_RUNNING;
                }
                // Write the pay load to the decoder
                CMX7262_TxFIFO(&dmr.Cmx7262,(uint8_t *)&aPayLoad[0]);
			break;

			// **************************  Data  Start *********************************/
			/* For normal voice operation the data specific code can be ignored */

			case DMR_STATE_DATA_RX_STARTUP:
                CMX7161_RxData (&dmr.Cmx7161,&aPayLoad[0],&aSync[0]);
                dmr.eState = DMR_STATE_DATA_RX_WAITING;
                sDMRRxStartDelayCnt = DMR_RX_START_DELAY;
			break;

			case DMR_STATE_DATA_RX_RUNNING:

				sDMRRxLostSyncCnt = DMR_DEBUG_FIFO_LOST_SYNC;
				uPRBSFrameCount++;
				// Extract the Pay load and Sync from the Rx FIFO
				CMX7161_RxData (&dmr.Cmx7161,&aPayLoad[0],&aSync[0]);
                uPRBSBitErrorCount =  uPRBSBitErrorCount + PRBS_CheckPattern(aPayLoad,CMX7262_CODEC_BUFFER_SIZE);
                if (DMR_CheckSync (&aSync[0],&VOICE_SYNC[0] ) == 0)
                {
                    // We have received a packet without sync, we decrement the lost
                	// packet count. More than 20 cosequtive lost syncs we have either a very bad
                	// RF path or we have the FIFO issue.
                    sDMRRxLostSyncCnt--;
                    dmr.eState = DMR_STATE_DATA_RX_RUNNING_NO_SYNC;
                }
                // Zero out the cpodec samples so the audio does not sound bad.
                for(i=0;i<CMX7262_CODEC_BUFFER_SIZE;i++)
            		aPayLoad[i]=0;
            	CMX7262_TxFIFO(&dmr.Cmx7262,(uint8_t *)&aPayLoad[0]);
            break;

            // I introduced this state to capture the FIFO issue and indicate with a Red LED. If we have
            // the FIFO issue then we see the first 19 bytes Ok, which includes the sync, then the frames
            // become offset and the sync is embedded in the wrong position. Therefore after 21 frames
            // we will have seen 20 consecutive sync losses. For bad RF or modem performance we see initial sync
            // and good frames which slowly degrade with bit errors, so the Red LED should not light for
            // this condition.

			case DMR_STATE_DATA_RX_RUNNING_NO_SYNC:
				 sDMRRxLostSyncCnt--;
				uPRBSFrameCount++;
				// Extract the Pay load and Sync from the Rx FIFO
				CMX7161_RxData (&dmr.Cmx7161,&aPayLoad[0],&aSync[0]);
                uPRBSBitErrorCount =  uPRBSBitErrorCount + PRBS_CheckPattern(aPayLoad,CMX7262_CODEC_BUFFER_SIZE);
                if (DMR_CheckSync (&aSync[0],&VOICE_SYNC[0] ) == 1)
                {
                	// We have received a good sync, so reset the dmr state
                    dmr.eState = DMR_STATE_DATA_RX_RUNNING;
                }
                else
                {
                	if ( (sDMRRxLostSyncCnt <= 0) && uPRBSFrameCount < (DMR_DEBUG_FIFO_LOST_SYNC+2))
                	{
                		// Light the RED LED for more than 20 consequtive sync losses.
                		GPIO_SetBits(GPIOE, TEST_RED_LED_D8);
                	}
                }
                // Zero out the cpodec samples so the audio does not sound bad.
                for(i=0;i<CMX7262_CODEC_BUFFER_SIZE;i++)
            		aPayLoad[i]=0;
            	CMX7262_TxFIFO(&dmr.Cmx7262,(uint8_t *)&aPayLoad[0]);
            break;

            // **************************  Data  End  *********************************/

			default:
			break;
		}
	}

}


void DMR_Init (void )
{

	dmr.pFlash = (DMR_Flash_TypeDef *)ADDR_FLASH_PAGE;
	dmr.uBoardNumber = dmr.pFlash->uBoardNumber;

	dmr.eState = DMR_STATE_INIT;				// State of DMR radio process.
	dmr.eMMIRequest = DMR_MMI_IDLE;			// Requests from the MMI to DMR

	// Initialise table of function pointers to handle requests from the MMI.

	dmr.pMMI_FunctionRequestTable[DMR_MMI_IDLE] =   DMR_MMI_Idle_Handler;
	dmr.pMMI_FunctionRequestTable[DMR_MMI_DATA_OFF] =  DMR_MMI_DataOff_Handler;;
	dmr.pMMI_FunctionRequestTable[DMR_MMI_DATA_ON] =  DMR_MMI_DataOn_Handler;
	dmr.pMMI_FunctionRequestTable[DMR_MMI_PTT] = DMR_MMI_PTTKey_Handler;
	dmr.pMMI_FunctionRequestTable[DMR_MMI_VOLUME] =  DMR_MMI_Volume;
	dmr.pMMI_FunctionRequestTable[DMR_MMI_STAR] =  DMR_MMI_StarKey_Handler;

}


// This handles the scenario when the MMI is Idle. If we were previously in Tx, then we need to stop
// the  modem, but we only stop when the modem and encoder are up and running. The encoder is stopped
// on packet boundaries to prevent overflows. We could probably stop the encoder immediately, but this is
// something I have not yet tested. The Rx can be stopped immediately, so we stop in startup and running
// DMR Rx states. Note that when we are in Rx the MMI will in general be idle, hence the program counter
// comes through this section.

void DMR_MMI_Idle_Handler (void)
{

		switch (dmr.eState)
		{
		case DMR_STATE_IDLE: //WE COME HERE WHEN THERE'S AN AUDIO GLITCH
			GPIO_ResetBits(GPIOE, TEST_GREEN_LED_D7);		// Switch the Green LED off.
			CMX7161_RxFIFOSetup (&dmr.Cmx7161);
			// Enable FIFO Rx IRQ.
			CMX7161_EnableIRQ (&dmr.Cmx7161,(IRQ | RXFIFO));
			// Enable all frame sync's.
			CMX7161_ModemOptions(&dmr.Cmx7161,0x03FF);
			CMX7161_ModemControl (&dmr.Cmx7161,cCMX7161_RX_SLOT);
			dmr.eState = DMR_STATE_RX_STARTUP;
			break;
		case DMR_STATE_TX_RUNNING:
			dmr.eState = DMR_STATE_TX_STOPPING;
			break;

		// **************************  Data  Start *********************************/
		/* For normal voice operation the data specific code can be ignored */

		case DMR_STATE_DATA_IDLE:
			CMX7161_RxFIFOSetup (&dmr.Cmx7161);
			// Enable FIFO Rx IRQ.
			CMX7161_EnableIRQ (&dmr.Cmx7161,(IRQ | RXFIFO));
			// Enable all frame sync's.
			CMX7161_ModemOptions(&dmr.Cmx7161,0x03FF);
			CMX7161_ModemControl (&dmr.Cmx7161,cCMX7161_RX_SLOT);
			dmr.eState = DMR_STATE_DATA_RX_STARTUP;
			break;
		case DMR_STATE_DATA_TX_RUNNING:
			dmr.eState = DMR_STATE_DATA_TX_STOPPING;
			break;

		case DMR_STATE_DATA_RX_PAUSE:
			// The first PTT press pauses Rx data mode, so when the PTT is released we change
			// state so that we can handle the second PTT press which will stop Rx Data mode.
			dmr.eRadioRequest = MMI_DMR_RX_DATA_PAUSE;
			dmr.eState = DMR_STATE_DATA_RX_IDLE;
		break;
		case DMR_STATE_DATA_RX_EXIT:
			// After the release of the second PTT press we move the radio back to idle
			dmr.eRadioRequest = MMI_DMR_RX_DATA_STOP;
			dmr.eState = DMR_STATE_DATA_IDLE;
			break;

		// **************************  Data  End *********************************/

		default:
			break;

	}
}


// While the PTT is pressed, the MMI request will be set to DMR_MMI_PTT. If the previous state was DMR
// idle then we know to start the encoder so that it can start to collect the first packet of speech samples
// from the codec.
// We first start the encoder. The modem is started later based on  the first encoder IRQ which
// indicates a packet of encoded samples are available. In actual fact the encoder IRQ triggers a count
// so that the modem is started several ticks later. Generally this gives us some margin between samples
// being available and the modem needing them.
// We test for DMR_STATE_RX_STARTUP because the receive may never have got beyond startup if it has
// not received a sync.
// The Rx can be stopped immediately, so we stop in startup and running DMR Rx states.

void DMR_MMI_PTTKey_Handler (void)
{

	switch (dmr.eState)
	{
		case DMR_STATE_IDLE:
		case DMR_STATE_RX_STARTUP:
			// DAVED DEBUG - Start
			GPIO_ResetBits(GPIOE, TEST_TL4);
			GPIO_ResetBits(GPIOE, TEST_TL3);
			GPIO_ResetBits(GPIOE, TEST_TL2);
			GPIO_ResetBits(GPIOE, TEST_TL1);
			// DAVED DEBUG - End

			CMX7262_Idle (&dmr.Cmx7262);
			CMX7161_DisableIRQ (&dmr.Cmx7161,0xFFFF);
			CMX7161_ModemControl (&dmr.Cmx7161,cCMX7161_IDLE);
			// Start the encoder to gather speech samples before we start the modem (Cmx7161).
			CMX7262_Encode (&dmr.Cmx7262);
			dmr.eState =  DMR_STATE_TX_WAIT_FOR_ENCODER;
			break;

		case DMR_STATE_RX_RUNNING:
			CMX7262_Idle (&dmr.Cmx7262);
			CMX7161_DisableIRQ (&dmr.Cmx7161,0xFFFF);
			CMX7161_ModemControl (&dmr.Cmx7161,cCMX7161_IDLE);
			dmr.eState = DMR_STATE_IDLE;
			break;


		// **************************  Data  Start *********************************/
		/* For normal voice operation the data specific code can be ignored */

		case DMR_STATE_DATA_RX_RUNNING:
		case DMR_STATE_DATA_RX_RUNNING_NO_SYNC:
			// On the first press of the PTT in Rx data the codec and modem are stopped
			// We also instruct the MMI to show stopped so the user can read results.
        	CMX7262_Idle (&dmr.Cmx7262);
        	// DAVED DEBUG - Start
        	// Disabling the IRQ and setting the modem to Idle clear any problems with the FIFO
        	// transfer problem in Rx. I believe this to be a problem with the CMX7161 FI since it has
        	// been seen in the script environment. Ordinarily we do not need to set the modem to idle
        	// here. It is sufficient to re-issue the receive command to reset the receiver.
        	CMX7161_DisableIRQ (&dmr.Cmx7161,0xFFFF);
        	CMX7161_ModemControl (&dmr.Cmx7161,cCMX7161_IDLE);
			// Switch to the stop state ready for the second PTT press.
			dmr.eState = DMR_STATE_DATA_RX_PAUSE;
			break;

		case DMR_STATE_DATA_RX_IDLE:
			// On the second press of the PTT move the radio to exit state which will be picked
			// up by the idle handler as the PTT is released. We also clear the RED LED should
			// it have been lit during the test.
			GPIO_ResetBits(GPIOE, TEST_RED_LED_D8);
			dmr.eState = DMR_STATE_DATA_RX_EXIT;
		break;

		case DMR_STATE_DATA_IDLE:
		case DMR_STATE_DATA_RX_STARTUP:
			// DAVED DEBUG - Start
			GPIO_ResetBits(GPIOE, TEST_TL4);
			GPIO_ResetBits(GPIOE, TEST_TL3);
			GPIO_ResetBits(GPIOE, TEST_TL2);
			GPIO_ResetBits(GPIOE, TEST_TL1);
			// DAVED DEBUG - End

			CMX7262_Idle (&dmr.Cmx7262);
			CMX7161_DisableIRQ (&dmr.Cmx7161,0xFFFF);
			CMX7161_ModemControl (&dmr.Cmx7161,cCMX7161_IDLE);
			// Start the encoder to gather speech samples before we start the modem (Cmx7161).
			CMX7262_Encode (&dmr.Cmx7262);
			dmr.eState =  DMR_STATE_DATA_TX_WAIT_FOR_ENCODER;
			break;

		// **************************  Data  End  *********************************/


		default:
		break;
	}

}

// Calls, starts and runs the Rx slot test from an MMI request. The routine checks for state of the DMR
// application being Idle If so, this means we need to start the Rx slot test by initialising and starting the
// Rx modem. The DMR application state is changed to Rx startup to reflect this.
void DMR_MMI_DataOn_Handler (void)
{
	dmr.eState = DMR_STATE_DATA_IDLE;
}

void DMR_MMI_DataOff_Handler (void)
{
	dmr.eState = DMR_STATE_IDLE;
}


// Handles an MMI request to change the audio output level (speaker) on the CMX7262.
void DMR_MMI_Volume (void)
{
	CMX7262_MMI_AudioGain (&dmr.Cmx7262);
	// Clear the MMI request once the volume request has been action so this can be fed back
	// to the MMI as complete.
	dmr.eMMIRequest = DMR_MMI_IDLE;
}

// This handles an MMI request triggered by a press of the star (*) key. We shut everything
// down and move the DMR state into IDLE.

void DMR_MMI_StarKey_Handler (void )
{

	switch (dmr.eState)
	{
		case DMR_STATE_RX_STARTUP:
		case DMR_STATE_RX_RUNNING:
			CMX7262_Idle (&dmr.Cmx7262);
			CMX7161_DisableIRQ (&dmr.Cmx7161,0xFFFF);
			CMX7161_ModemControl (&dmr.Cmx7161,cCMX7161_IDLE);
			dmr.eState = DMR_STATE_IDLE;
		break;
		case DMR_STATE_TX_RUNNING:
			dmr.eState = DMR_STATE_TX_STOPPING;
		break;

		case DMR_STATE_DATA_RX_STARTUP:
		case DMR_STATE_DATA_RX_RUNNING:
			CMX7262_Idle (&dmr.Cmx7262);
			CMX7161_DisableIRQ (&dmr.Cmx7161,0xFFFF);
			CMX7161_ModemControl (&dmr.Cmx7161,cCMX7161_IDLE);
			dmr.eState = DMR_STATE_DATA_IDLE;
		break;
		case DMR_STATE_DATA_TX_RUNNING:
			dmr.eState = DMR_STATE_DATA_TX_STOPPING;
		break;

		default:
		break;
	}


}

// Checks the extracted sync with the expected sync and returns 1 for a match otherwise 0.
uint16_t DMR_CheckSync (uint8_t *pExtractedSync, uint8_t *pExpectedSync )
{
	uint16_t 	i;
	for (i=0;i<CMX7161_DMR_SYNC_SIZE;i++)
	{
		if(pExtractedSync[i] != pExpectedSync[i])
			return 0;
	}
	return 1;
}

// Return the number of miss matches.
uint16_t DMR_SlotTestComparison (void )
{
	uint16_t 	i;
	uint16_t uError;

	uError = 0;

	for (i=0;i< CMX7262_CODEC_BUFFER_SIZE;i++)
	{
		if(aModemTestSamples[i] != aTestSamples[i])
			uError++;
	}
	// Return 1 for good data match
	return uError;
}

// Write to flash at an address. The flash is written in words and the size if the number of words.

uint16_t DMR_WriteFlash (DMR_TypeDef *pDMR)
{

	FLASH_Status status;
	uint16_t uError = 0;

	/* Unlock the Flash to enable the flash control register access *************/
	  FLASH_Unlock();

	  /* Clear pending flags (if any) */
	  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_WRPRTERR |
			  FLASH_FLAG_PGERR);


	  FLASH_ErasePage((uint32_t)pDMR->pFlash);

	  // Clear the flag to 0 so that initialisation code reads the power up defaults from flash.
	  status = FLASH_ProgramWord((uint32_t)(&pDMR->pFlash->sFlag),0);
	  if(status != FLASH_COMPLETE)
		  uError = 1;

	  status = FLASH_ProgramWord((uint32_t)(&pDMR->pFlash->uBoardNumber),pDMR->uBoardNumber);
	  if(status != FLASH_COMPLETE)
		  uError = 1;

	  status = FLASH_ProgramWord((uint32_t)(&pDMR->pFlash->sCmx7161FreqControl),pDMR->Cmx7161.sFreqControl);
	  if(status != FLASH_COMPLETE)
		  uError = 1;

	  status = FLASH_ProgramWord((uint32_t)(&pDMR->pFlash->sCmx7161Cbus5E),pDMR->Cmx7161.sCbus5E);
	  if(status != FLASH_COMPLETE)
		  uError = 1;

	  status = FLASH_ProgramWord((uint32_t)(&pDMR->pFlash->sCmx7161CbusB4),pDMR->Cmx7161.sCbusB4);
	  if(status != FLASH_COMPLETE)
		  uError = 1;

	  status = FLASH_ProgramWord((uint32_t)(&pDMR->pFlash->sCMX994DcOffset),pDMR->Cmx7161.sCMX994DcOffset);
	  if(status != FLASH_COMPLETE)
		  uError = 1;
	  
          status = FLASH_ProgramWord((uint32_t)(&pDMR->pFlash->sCMX994Gain),pDMR->Cmx7161.sCMX994Gain);
	  if(status != FLASH_COMPLETE)
		  uError = 1;

          status = FLASH_ProgramWord((uint32_t)(&pDMR->pFlash->sCMX994GainIndex),pDMR->Cmx7161.sCMX994GainIndex);
	  if(status != FLASH_COMPLETE)
		  uError = 1;          
	  
          status = FLASH_ProgramWord((uint32_t)(&pDMR->pFlash->sCmx7262InputGain),pDMR->Cmx7262.sInputGain);
	  if(status != FLASH_COMPLETE)
		  uError = 1;

	  /* Lock the Flash to disable the flash control register access (recommended
	     to protect the FLASH memory against possible unwanted operation) *********/
	  FLASH_Lock();

	  return uError;

}

// Group the error checking into one place and called by the System Tick.
void DMR_ErrorCheck ( void )
{
	// For any errors light the Red LED
		if(dmr.Cmx7262.uError || dmr.Cmx7161.uError)
		{
			GPIO_SetBits(GPIOE, TEST_RED_LED_D8);		// Red LED.
			// Clear the error flags so they are not called over and over.
			dmr.Cmx7161.uError = 0;
			dmr.Cmx7262.uError = 0;
		}
}


// DAVED DEBUG - CURRENTLY NOT USED

uint16_t DMR_PendSV (void)
{
	return 1;
}


// Copy samples from the PRBS pattern for a given size.
void PRBS_CopyPattern (uint8_t *pOSamples, uint8_t uSize)
{

	uint16_t i;
	for (i=0;i<uSize;i++)
	{
		*pOSamples++ = aPRBS[uPRBSByteCount++];
		if(uPRBSByteCount>=PRBS_LEN)
			uPRBSByteCount = 0;
	}

}


// Check the pay load, go through each byte and check for bit errors. Return the
// number of bit errors found.
uint16_t PRBS_CheckPattern (uint8_t *pISamples, uint8_t uSize)
{

	uint16_t i,j;
	uint16_t uTemp1;
	uint16_t uOnesCount = 0;

	for (i=0;i<uSize;i++)
	{

			uTemp1 = (*pISamples++ ^ aPRBS[uPRBSByteCount++]);
			// Check for any bit errors
			if (uTemp1)
			{
				// Count the bit errors
				for (j=0;j<8;j++)
				{
					if (uTemp1 & 1)
						uOnesCount++;
					uTemp1 = uTemp1 >> 1;
				}
			}

		if(uPRBSByteCount>=PRBS_LEN)
			uPRBSByteCount = 0;
	}

	return uOnesCount;

}

// Slows down the rate of change for frame count
uint16_t PRBS_ReadFrameCount (void)
{
	  if (uPRBSFrameCount >= (uPRBSOldFrameCount + 10))
	   {
		  uPRBSOldFrameCount = uPRBSFrameCount;
	   }
	  return uPRBSOldFrameCount;
}

// Slows down the rate of change for bit error count.
uint16_t PRBS_ReadBitErrorCount (uint16_t uActual)
{
	  //if (uPRBSBitErrorCount >= (uPRBSOldBitErrorCount  + 10))
	   //{
		//  uPRBSOldBitErrorCount  = uPRBSBitErrorCount;
	  // }
	  //if (uActual)
		  return uPRBSBitErrorCount;
	 //else
	//	  return  uPRBSOldBitErrorCount;
}


