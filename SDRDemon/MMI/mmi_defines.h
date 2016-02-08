/************************************************************************************************************
 *
 *  $Copyright © 2012-2014 CML Systems $
 *
 *  $Header: mmi_defines.h  Revision:1.15.1.2  18 February 2014 11:07:19  ilewis $
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

#ifndef _MMIDEFS_H
#define _MMIDEFS_H

#define KEYPAD GPIOC
#define ROWS (GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3)
#define COLUMNS (GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7)
#define PTT_IO GPIO_Pin_8

#define KEY_1    0x0001
#define KEY_2    0x0002
#define KEY_3    0x0004
#define KEY_A    0x0008
#define KEY_4    0x0010
#define KEY_5    0x0020
#define KEY_6    0x0040
#define KEY_B    0x0080
#define KEY_7    0x0100
#define KEY_8    0x0200
#define KEY_9    0x0400
#define KEY_C    0x0800
#define KEY_STAR 0x1000
#define KEY_0    0x2000
#define KEY_HASH 0x4000
#define KEY_D    0x8000

#define LCD GPIOD
#define DATA (GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7) 
#define BUSY GPIO_Pin_7
#define RS GPIO_Pin_8
#define RW GPIO_Pin_9
#define EN GPIO_Pin_10
#define BACKLIGHT GPIO_Pin_15

#define MISC_IO GPIOE
#define AUDIO_PA_SHDN GPIO_Pin_0
#define TL1 GPIO_Pin_1
#define TL2 GPIO_Pin_2
#define TL3 GPIO_Pin_3
#define TL4 GPIO_Pin_4
#define TL5 GPIO_Pin_5
#define RED_LED GPIO_Pin_6
#define GREEN_LED GPIO_Pin_7
#define CHARGE_PUMP_CLK GPIO_Pin_9

//LCD commands for SPLC780D controller
#define CLEAR_DISPLAY       0x01

#define RETURN_HOME         0x02

#define ENTRY_MODE_SET      0x04
#define ENTRY_MODE_INC      0x02
#define ENTRY_MODE_DEC      0x00
#define ENTRY_MODE_SHIFT    0x01
#define ENTRY_MODE_NO_SHIFT 0x00

#define DISPLAY_ON_OFF      0x08
#define DISPLAY_ON          0x04
#define DISPLAY_OFF         0x00
#define CURSOR_ON           0x02
#define CURSOR_OFF          0x00
#define BLINK_ON            0x01
#define BLINK_OFF           0x00

#define DISPLAY_SHIFT       0x10
#define SHIFT_ON            0x80
#define SHIFT_OFF           0x00
#define SHIFT_RIGHT         0x40
#define SHIFT_LEFT          0x00

#define SET_FUNCTION        0x20
#define EIGHT_BIT           0x10
#define FOUR_BIT            0x00
#define TWO_LINES           0x08
#define ONE_LINE            0x00
#define BIG_FONT            0x40
#define SMALL_FONT          0x00

#define SET_CGRAM_ADX       0x40

#define SET_DISPRAM_ADX     0x80



typedef enum {

	MMODE_INIT,					// Mode created while the CMX's are code loading and initialising.
	MMODE_RADIO,
	MMODE_SETUP,
	MMODE_GUI			// Allows us to display debug messages.
} MMODE;

typedef enum {
	MSTATE_WAITING,
	MSTATE_RX,
	MSTATE_TX,
	MSTATE_WAITING_DATA,
	MSTATE_RX_DATA_RUN,
	MSTATE_RX_DATA_PAUSE,
	MSTATE_TX_DATA
} MSTATE;

typedef enum {T2_WAITING_FOR_FIRST_KEY, T2_WAITING_FOR_NEXT_KEY} T2STATE;


void MMI_Init(void);

void ServiceModem(void);
void ServiceVocoder(void);

void UpdateStrings(void);
void UpdateDisplay(void);

void ChannelUp(void);
void ChannelDown(void);
void SetChannel(void);
void ProgSynth(int, int, int);

void VolumeUp(void);
void VolumeDown(void);

short unsigned int Keypad(void);
short unsigned int ReadPTT(void);
char KeyToChar(int Keys);

void LCDInit(void);
void LCDWriteData (char data);
void LCDWriteInstruction (char data, char check_busy);
void LCDWriteByte(char data);
int LCDReadBusy(void);
void HelloWorld(void);
void FlashLEDs(void);
void TIM1_Init_MMI(void);

void MMI_Main ( void );

extern MMODE MMI_Mode;
extern MSTATE MMI_State;
extern MMODE MMI_SavedMode;

extern char DisplayTop[17];
extern char DisplayBottom[17];
extern char DisplayChanged;
extern int CurrentChannel;
extern char BatteryVoltage[5];

#endif
