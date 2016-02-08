/************************************************************************************************************
 *
 *  $Copyright © 2012-2014 CML Systems $
 *
 *  $Header: mainDMR.h  Revision:1.24  18 February 2014 10:16:44  ddavenport $
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

#ifndef _MAINDMR_H
#define _MAINDMR_H

#ifdef DEBUG_CHANGE_INC_FROM_INITIAL_SDR_PROJECT
 #include "..\common\stm32f10x_sdr.h"
#else
 #include "stm32f10x_sdr.h"
#endif
#include "cmx7262.h"
#include "cmx7161.h"

#define	MAX_DMR_MMI_FUNCTIONS	10

#define DMR_RX_START_DELAY						1
#define DMR_TX_START_DELAY						3		// The number of ticks between reading the codec and starting the modem in Tx
#define DMR_RX_LOST_SYNC_COUNT		2		// Number of lost syncs before we stop the Rx and drop the call.
#define DMR_SYNC_BIT_ERRORS					5		// Number of permitted bit errors in the sync

#define DMR_DEBUG_FIFO_LOST_SYNC	10

// Definitions of requests made by the MMI to the DMR application.
typedef enum {
	DMR_MMI_IDLE,
	DMR_MMI_DATA_ON,
	DMR_MMI_DATA_OFF,
	DMR_MMI_PTT,
	DMR_MMI_VOLUME,
	DMR_MMI_STAR
} dmrMMIRequest;

// Definitions of requests made by the DMR application to the MMI.
typedef enum {
    MMI_DMR_NO_MESSAGE,
	MMI_DMR_INIT_COMPLETE,
    MMI_DMR_START_RX,
	MMI_DMR_RX_STOP,
	MMI_DMR_RX_DATA_STOP,
	MMI_DMR_RX_DATA_RUN,
	MMI_DMR_RX_DATA_PAUSE
} dmrRadioRequest;

// Status of the radio and codec.
typedef enum {
	DMR_STATE_INIT,
	DMR_STATE_IDLE,
	DMR_STATE_TX_WAIT_FOR_ENCODER,
	DMR_STATE_TX_WAITING,
	DMR_STATE_TX_RUNNING	,	// Codec and modem running.
	DMR_STATE_TX_STOPPING,		// Request from MMI to stop, but codec and modem still running.
	DMR_STATE_RX_STARTUP,		// The modem Rx has started, but the decoder still needs starting.
	DMR_STATE_RX_WAITING,        //We have received a packet from the modem, but are waiting to start the codec to give us some 'slack'
    DMR_STATE_RX_RUNNING,		// Codec and modem running.
    DMR_STATE_RX_RUNNING_NO_SYNC,	// Lost a packet while in running mode, prepare to stop the call but fill the decoder samples with 0.

    // The states below are part of the test data mode, so under normal voice operation are
    // not in the code flow.

	DMR_STATE_DATA_TX_WAIT_FOR_ENCODER,
	DMR_STATE_DATA_TX_DELAY_MODEM_START,
	DMR_STATE_DATA_TX_RUNNING	,	// Codec and modem running.
	DMR_STATE_DATA_TX_STOPPING,		// Request from MMI to stop, but codec and modem still running.

    DMR_STATE_DATA_IDLE,
	DMR_STATE_DATA_RX_STARTUP,
	DMR_STATE_DATA_RX_WAITING,
	DMR_STATE_DATA_RX_RUNNING,					// Receive and check PRBS packets
	DMR_STATE_DATA_RX_RUNNING_NO_SYNC,
	DMR_STATE_DATA_RX_PAUSE,
	DMR_STATE_DATA_RX_IDLE,
	DMR_STATE_DATA_RX_EXIT
} dmrState;


// Data type for DMR
typedef struct
{

	uint32_t	uBoardNumber;
	// Pointer to area in flash where the defaults are stored.
	DMR_Flash_TypeDef	 *pFlash;

	// Messages for MMI derived from DMR code.
	char 	aMessage[17];

	// State of the DMR application  based around the System Tick IRQ.
	__IO dmrState		eState;

	// dmrMMIRequest is used as an index into the MMI_FunctionRequestTable.
	__IO dmrMMIRequest		eMMIRequest;
	void (*pMMI_FunctionRequestTable[MAX_DMR_MMI_FUNCTIONS])(void);
        
        __IO dmrRadioRequest    eRadioRequest;  //message from Radio to MMI

	CMX7161_TypeDef Cmx7161;
	CMX7262_TypeDef Cmx7262;

} DMR_TypeDef;

#include "mmi_dmr.h"
#ifdef DEBUG_CHANGE_INC_FROM_INITIAL_SDR_PROJECT
 #include "..\MMI\mmi_defines.h"
#else
 #include "mmi_defines.h"
#endif

// Test buffer to verify standalone operation.
//#define 	NUM_TEST_PACKETS 	100
#define 	NUM_TEST_PACKETS 	50

void DMR_SysTick (void);
uint16_t  DMR_PendSV (void);
uint16_t DMR_WriteFlash (DMR_TypeDef *pDMR);


uint16_t PRBS_ReadFrameCount (void);
uint16_t PRBS_ReadBitErrorCount (uint16_t uActual);

#endif
