/************************************************************************************************************
 *
 *  $Copyright © 2012-2014 CML Systems $
 *
 *  $Header: cmx7262.h  Revision:1.21  18 February 2014 10:13:11  ddavenport $
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

#ifndef _CMX7262_H
#define _CMX7262_H

// CBUS register defintions as per the CMX7262

#define	FREQ_CONTROL				0x52
#define	VCFG_REG							0x55
#define	SIGNAL_CONTROL			0x58
#define	AUDIO_ROUTING_REG	0x5D
#define	PROG_REG							0x6A	//  DSPIO_SW_WR9
#define	REG_DONE_SELECT		0x69
#define	VCTRL_REG   						0x6B	// DSPIO_SW_WR0
#define	IRQ_ENABLE_REG			0x6C
#define	DSPIO_SW_RD0					0x70
#define	IRQ_STATUS_REG      		0x7E	// DSPIO_CBUS_STATUS

// Real CBUS registers that the host can access, with special meanings

#define	CBUS_AUDIO_IN				0x49	// CBUS_TXWRITE16 - for writing 16-bit LPCM samples into CMX7262
#define	CBUS_VOCODER_IN		0x48	// CBUS_TXWRITE8 - for writing byte-wide coded data into CMX7262
#define	CBUS_TXLEVEL					0x4B
#define	CBUS_AUDIO_OUT			0x4D	// CBUS_RXREAD16 - for reading 16-bit LPCM samples from CMX7262
#define	CBUS_VOCODER_OUT	0x4C	// CBUS_RXREAD8 - for reading byte-wide coded data from CMX7262
#define	CBUS_RXLEVEL					0x4F
#define	GPIO_CONTROL					0x64
#define	PEAK_LEVEL						0x71
#define	NOISE_CTRL						0x53

#define	ANAIN_CONFIG					0xB0
#define	ANAIN_GAIN							0xB1
#define	ANAOUT_CONFIG				0xB3
#define	AOG3										0xB6		// Speaker output course gain.


// Bit references in Status (0x7E) and IRQ Enable Register (0x6C)
#define	ODA											(1<<3)
#define	IDW											(1<<8)
#define	OV												(1<<10)			// Overflow
#define	UF												(1<<11)			// Underflow
#define	REGDONE								(1<<13)
#define	PRG											(1<<14)
#define	IRQ											(1<<15)

// Bit references for configuration of vocoder (VCFG - $55)
#define	HDD											(1<<5)
#define	FEC											(1<<4)
#define	ONE_FRAME						1
#define	TWO_FRAME						2
#define	THREE_FRAME					3
#define	FOUR_FRAME						0

// Masks to extract the configuration settings above.
#define FRAME_MASK		0x0003
#define FEC_MASK		FEC

// I have reduced this to 3 to grab back much need RAM for dual boot mode.
//#define MAX_FRAMES_PER_ENCDEC	3
// TWELP frame size hard decision.
#define TWELP_HDD_FRAME_SIZE_BYTES		6
// TWELP frame size hard decision with FEC.
#define TWELP_FEC_HDD_FRAME_SIZE_BYTES		9

// Size of buffer to hold one packet of encoded samples (3 frame, hard decision, FEC).
#define	CMX7262_CODEC_BUFFER_SIZE	27

// Clock setup
#define	FS_DIV										78 				// Setup a 8kHz sample rate for the audio converters


// Definitions for the VCTRL register 6B (Main control register)
#define	CMX7262_VCTRL_IDLE				0x0000
#define	CMX7262_VCTRL_DECODE		0x0001
#define	CMX7262_VCTRL_ENCODE		0x0002
#define	CMX7262_PASSTHRU					0x0004
#define	CMX7262_VCTRL_TEST				0x0007


// CMX7262 Vocoder Modes
// These modes are required so that we can set the appropriate request
// flag in the IRQ and to prevent multiple idles. Multiple idles is a known
// issue with the CMX7262 FI.

typedef enum {
	CMX7262_INIT_MODE = 0,
	CMX7262_IDLE_MODE = 1,
	CMX7262_DECODE_MODE = 2,
	CMX7262_ENCODE_MODE =3,
	CMX7262_PASSTHRU_MODE = 4,
	CMX7262_TEST_MODE = 5
} cmx7262Mode;

// References for audio source and destinations in relation to
// CBUS and analog ports (Audio)..

#define	DEST_CBUS		0x1
#define	DEST_AUDIO	0x2
#define	SRC_CBUS		(0x1)<<4
#define	SRC_AUDIO		(0x2)<<4

// Number of functions in function pointer table.
#define CMX7262_MAX_CONTROL_FUNCTIONS	10

// IRQ flag bit definitions.
#define	CMX7262_ODA								(1<<3)
#define	CMX7262_IDW								(1<<4)

// CMX7262 errors use the upper 16 bits.

#define	CMX7262_FI_LOAD_ERROR					(0x00010000)		// FI failed to load correctly.
#define	CMX7262_CONFIG_CLK_ERROR			(0x00020000)		// Failed to initialise the config clocks.
#define	CMX7262_ENCODE_ERROR					(0x00040000)		// Encode mode setup failed.
#define	CMX7262_DECODE_ERROR					(0x00080000)		// Decoder mode setup failed.
#define	CMX7262_IDLE_ERROR							(0x00100000)		// Idle mode setup failed.
#define	CMX7262_ODA_ERROR								(0x00200000)		// Multiple ODA flags.
#define	CMX7262_IDW_ERROR								(0x00400000)		// Multiple IDW flags.
#define	CMX7262_OV_ERROR								(0x00800000)
#define	CMX7262_UF_ERROR								(0x01000000)

#define	CMX7262_TRANSCODE_TIMEOUT		6000//1500	// 15mS with a 10uS tick. Used when we set up codec modes.

// Data type for an instance of the CMX7262.
typedef struct
{
	cmxFI_TypeDef* FI;									// Function image parameters used during loading.
	DMR_Flash_TypeDef	 *pFlash;						// Pointer to the area of flash for storage of defaults.

	cmx7262Mode uMode;							// Codec mode.
	uint8_t	uInterface;										// CBUS interface that the instance maps too.
	__IO uint16_t uIRQ_STATUS_REG;		// Shadow register for IRQ status reads.
	uint16_t uIRQ_ENABLE_REG;				// Shadow register for the IRQ enable.
	uint16_t uPacketSize;								// Size of vocoded speech packets in bytes written to and
																			// from the CMX7262. Packets are multiples of TWELP
																			// frames and their size depending on the type of encoding,
																			// FEC etc.
	__IO uint8_t *pDataBuffer;						// Defined as volatile because it is updated by an IRQ.
	__IO uint32_t uIRQRequest;					// Bit field for CMX7262 IRQ requests.
	__IO uint32_t uError;								// Bit field to indicate any errors.
	uint16_t uOutputGain;								// Parameter used by calls to control.
	signed int FIVersion;

	// Note the "signed int" definitions below to match up with the pointer types within the MMI definitions.
	signed int sInputGain;							// Parameter used by calls to control.

} CMX7262_TypeDef;


#define CMX7262_INPUT_GAIN_DEFAULT		0x0000
#define CMX7262_OUPUT_GAIN_DEFAULT	0x8000				// Default gain.

uint16_t  CMX7262_Init (CMX7262_TypeDef  *pCmx7262, uint8_t uInterface );
void CMX7262_IRQ (void *pData);

void CMX7262_Decode (CMX7262_TypeDef *pCmx7262);
void CMX7262_Encode (CMX7262_TypeDef *pCmx7262);
void CMX7262_Idle (CMX7262_TypeDef *pCmx7262);

void	CMX7262_EnableIRQ (CMX7262_TypeDef *pCmx7262, uint16_t uIRQ);

void CMX7262_TxFIFO  (CMX7262_TypeDef  *pCmx7262, uint8_t *pData);
void CMX7262_RxFIFO  (CMX7262_TypeDef  *pCmx7262, uint8_t *pData);

void CMX7262_MMI_AudioGain (CMX7262_TypeDef *pCmx7262);
void CMX7262_AudioInputGain (CMX7262_TypeDef  *pCmx7262);

extern cmxFI_TypeDef *cmx7262FI;

#endif
