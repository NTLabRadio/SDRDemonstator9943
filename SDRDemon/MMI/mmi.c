/************************************************************************************************************
 *
 *  $Copyright © 2012-2014 CML Systems $
 *
 *  $Header: mmi.c  Revision:1.23.1.2  18 February 2014 11:03:34  ilewis $
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

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "ctype.h"
#include "stdlib.h"

#ifdef TARGET_DMR
#ifdef DEBUG_CHANGE_INC_FROM_INITIAL_SDR_PROJECT
  #include "..\DMR\mmi_dmr.h"
	#include "..\DMR\radio.h"
#else
	#include "mmi_dmr.h"
	#include "radio.h"
#endif
  #include "mmi_defines.h"  
#else
#ifdef TARGET_PMR
  #include "mmi_pmr.h"
  #include "mmi_defines.h"
  #include "radio_pmr.h"
#endif
#endif

// This is an external reference so we can jump to the script environment.
extern void Script_Main (void);

// DAVED - Declarations moved from the header files.
//static
MMODE MMI_SavedMode;
MMODE MMI_Mode;
MSTATE MMI_State = MSTATE_WAITING;

//Global data
char DisplayTop[17]   ="                ";
char DisplayBottom[17]="                ";
char DisplayChanged = TRUE;
signed int CurrentChannel = 0, CurrentVol = MAX_VOLUME>>1, CurrentSetupItem = 0;
// Set to maximum volume for now.
//signed int CurrentChannel = 0, CurrentVol = MAX_VOLUME, CurrentSetupItem = 0;

extern char Type2Temp[17];			//string for typed in value


int LCD_Word, backlight;
long int backlight_timer = 0;

char BatteryVoltage[5]="4.50";

// Bundle the two routines for MMI operation into one routine which makes for one clear call from the DMR
// code.
void MMI_Main ( void )
{
	//MMI_Mode = MMODE_GUI;  //HACK FOR SDR3 first build. *IPL
	CheckRadioMessages();   //check for messages from the main radio application
	ProcessKeys();
	UpdateDisplay();
	BatteryCheck();
  
	if (MMI_State != MSTATE_TX)
		ReadRSSI();

	if (MMI_Mode == MMODE_GUI)
	{
		GPIO_ResetBits(LCD, BACKLIGHT); //back light off
		// Call to CML GUI
#ifdef	CML_GUI
		Script_Main();
#endif
	}
}

void MMI_Init ( void )
{

  GPIO_InitTypeDef GPIO_Init_Struct;
  ADC_InitTypeDef ADC_Init_Struct;

  // All the initialisation here is the extra required for the MMI that is not initialised in the SDR init routine.
  // Ideally we should move all this initialisation into the SDR initialisation routine.
  SystemCoreClockUpdate ();

#if	0
  //Port A Timer before remapping - DEBUG ONLY
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //switch clock on
  GPIO_Init_Struct.GPIO_Pin  = GPIO_Pin_8;
  GPIO_Init_Struct.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init_Struct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_Init_Struct);                        //call init function
#endif

#define BATTERY_IN GPIO_Pin_0
#define LOCK_DETECT GPIO_Pin_1
  //init port b for ADC and lock detect
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); //switch clock on
  GPIO_Init_Struct.GPIO_Pin  = BATTERY_IN;
  GPIO_Init_Struct.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init_Struct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_Init_Struct);                        //call init function
  GPIO_Init_Struct.GPIO_Pin  = LOCK_DETECT;
  GPIO_Init_Struct.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init_Struct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_Init_Struct);                        //call init function

  //Port C - Keypad
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); //switch clock on
  //Rows are outputs
  GPIO_Init_Struct.GPIO_Pin  = ROWS;                         //set parameters in struct
  GPIO_Init_Struct.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init_Struct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(KEYPAD, &GPIO_Init_Struct);                        //call init function
  //Columns are inputs. so is PTT
  GPIO_Init_Struct.GPIO_Pin  = COLUMNS | PTT_IO;                //set parameters in struct
  GPIO_Init_Struct.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(KEYPAD, &GPIO_Init_Struct);                        //call init function

  //Port D - LCD
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE); //switch clock on
  //All outputs - need to change when we read back from LCD
  GPIO_Init_Struct.GPIO_Pin  = GPIO_Pin_All; //set parameters in struct
  GPIO_Init_Struct.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init_Struct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(LCD, &GPIO_Init_Struct);                        //call init function

#if	0
 // This is already initialised in the SDR init routine.
  //Port E - Misc / debug
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE); //switch clock on
  //All outputs - may need to change if we want to read. Also need to config PE9 as oscillator.
  GPIO_Init_Struct.GPIO_Pin  = GPIO_Pin_All; //set parameters in struct
  GPIO_Init_Struct.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init_Struct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(MISC_IO, &GPIO_Init_Struct);                        //call init function
#endif

  GPIO_Init_Struct.GPIO_Pin  = CHARGE_PUMP_CLK;
  GPIO_Init_Struct.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init_Struct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(MISC_IO, &GPIO_Init_Struct);                        //call init function


  RCC_ADCCLKConfig(RCC_PCLK2_Div8);                     //set clock prescaler for ADC
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); //switch clock on

  ADC_DeInit(ADC1);

  ADC_Init_Struct.ADC_Mode = ADC_Mode_Independent;      //fill in init strucyure
  ADC_Init_Struct.ADC_ScanConvMode = DISABLE;
  ADC_Init_Struct.ADC_ContinuousConvMode = ENABLE;
  ADC_Init_Struct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_Init_Struct.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_Init_Struct.ADC_NbrOfChannel = 1;
  ADC_Init(ADC1, &ADC_Init_Struct);                     //call init function

  ADC_Cmd(ADC1, ENABLE);                                //turn ADC on.

  ADC_RegularChannelConfig(ADC1, 8, 1, ADC_SampleTime_239Cycles5);      //set mux and sample time

  ADC_ResetCalibration(ADC1);                    // Enable ADC1 reset calibaration register
  while(ADC_GetResetCalibrationStatus(ADC1));   // Check the end of ADC1 reset calibration register
  ADC_StartCalibration(ADC1);                   // Start ADC1 calibaration
  while(ADC_GetCalibrationStatus(ADC1));        // Check the end of ADC1 calibration

  ADC_SoftwareStartConvCmd(ADC1, ENABLE);       //start conversions

  TIM1_Init_MMI();

  LCDInit();

  //HelloWorld();

  // Initialising the MMI is the first thing we do to give the user a confidence that something is
  // happening from power up.. We set the MMI into init mode until we get an indication from
  // the DMR firmware that the CMX's are codeloaded and initialised.
  MMI_Mode = MMODE_INIT;
  MMI_SavedMode = MMI_Mode;
  // Load the strings so that we have something to display when we call UpdateDisplay
  // for the first time.
  SetChannel();
  //SetVol();
  UpdateStrings();
}

int temp = 0x1234;

void LCDInit(void)
{
    ClearCounter();
    while (ReadCounter() < 40000);        //wait 40ms
    LCDWriteInstruction(SET_FUNCTION | EIGHT_BIT | TWO_LINES | SMALL_FONT, FALSE);

    ClearCounter();
    while (ReadCounter() < 4100);        //wait 4.1ms
    LCDWriteInstruction(SET_FUNCTION | EIGHT_BIT | TWO_LINES | SMALL_FONT, FALSE);

    ClearCounter();
    while (ReadCounter() < 100);        //wait 100us
    LCDWriteInstruction(SET_FUNCTION | EIGHT_BIT | TWO_LINES | SMALL_FONT, FALSE);

    ClearCounter();
    while (ReadCounter() < 100);        //wait 100us
    LCDWriteInstruction(SET_FUNCTION | EIGHT_BIT | TWO_LINES | SMALL_FONT, FALSE);

    LCDWriteInstruction(DISPLAY_ON_OFF | DISPLAY_ON | CURSOR_OFF | BLINK_OFF, TRUE);

    LCDWriteInstruction(CLEAR_DISPLAY, TRUE);

    LCDWriteInstruction(ENTRY_MODE_SET | ENTRY_MODE_INC | ENTRY_MODE_NO_SHIFT, TRUE);

    return;
}

void HelloWorld(void)
{
  LCDWriteData('H');
  LCDWriteData('e');
  LCDWriteData('l');
  LCDWriteData('l');
  LCDWriteData('o');
  LCDWriteData(' ');
  LCDWriteData('W');
  LCDWriteData('o');
  LCDWriteData('r');
  LCDWriteData('l');
  LCDWriteData('d');

}

void FlashLEDs(void)
{
  static short unsigned int i = 0;

  i++;

  if (i==0)
  {
    GPIO_SetBits(MISC_IO, RED_LED);            //Red On
    //GPIO_ResetBits(MISC_IO,GREEN_LED) ;        //Green Off
    //GPIO_SetBits(LCD, BACKLIGHT);

    //TIM1_Init();
  }

  if (i == 32768)
  {
    //GPIO_SetBits(MISC_IO, GREEN_LED);            //Green On
    GPIO_ResetBits(MISC_IO,RED_LED) ;         //Red Off
    //GPIO_ResetBits(LCD, BACKLIGHT);

        //TIM1_Init();
  }

  if (TIM1->CNT > 50)
      GPIO_ResetBits(MISC_IO,GREEN_LED) ;        //Green Off
  if (TIM1->CNT < 50)
      GPIO_SetBits(MISC_IO, GREEN_LED);            //Green On

  BatteryCheck();


}

void TIM1_Init_MMI(void)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef TIM1_OCInitStruct;

  //AFIO->MAPR = AFIO_MAPR_TIM1_REMAP_FULLREMAP; //tim1 ch1 to pe9
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_FullRemap_TIM1, ENABLE);

  /* --------------------------- System Clocks Configuration -----------------*/
  /* TIM1 clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

  /* Compute the prescaler value */
  // The timer is clocked at 1MHz (1uS period).
  // Note the timer clock for TIM1 is 168MHz.
  //PrescalerValue = (uint16_t) ((SystemCoreClock) / 1000000) - 1;

  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 100;
  TIM_TimeBaseStructure.TIM_Prescaler = (uint16_t) ((SystemCoreClock) / 1000000) - 1;
  //TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);


    TIM1_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM1_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM1_OCInitStruct.TIM_OutputNState = TIM_OutputNState_Enable;
    TIM1_OCInitStruct.TIM_Pulse = TIM_TimeBaseStructure.TIM_Period>>1;
    TIM1_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM1_OCInitStruct.TIM_OCNPolarity = TIM_OCNPolarity_High;
    TIM1_OCInitStruct.TIM_OCIdleState = TIM_OCIdleState_Set;
    TIM1_OCInitStruct.TIM_OCNIdleState = TIM_OCNIdleState_Set;

    // Initial duty cycle equals 0%. Value can range from zero to 1000.


   TIM_OC1Init(TIM1, &TIM1_OCInitStruct);

  // Ensures an update event is generated.
  //TIM_UpdateDisableConfig(TIM1, DISABLE);

  // TIM1 set into master mode - update event is selected as trigger output (TRGO)
  //TIM_SelectOutputTrigger(TIM1,TIM_TRGOSource_Update);

  /* Enable TIM1 Preload register on ARR */
  //TIM_ARRPreloadConfig(TIM1, ENABLE);




  TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);



   /* TIM1 enable counter */
  TIM_Cmd(TIM1, ENABLE);

  TIM_CtrlPWMOutputs(TIM1, ENABLE);


}


/****************************************************/
//Function KeyToChar()
//Takes a 16-bit int with each bit corresponding to a key on the keypad
// and returns the corresponding ASCII char value, for 0-9.
/****************************************************/
char KeyToChar(int Keys)
{
	switch(Keys)
	{
	case KEY_0: return '0';
	case KEY_1: return '1';
	case KEY_2: return '2';
	case KEY_3: return '3';
	case KEY_4: return '4';
	case KEY_5: return '5';
	case KEY_6: return '6';
	case KEY_7: return '7';
	case KEY_8: return '8';
	case KEY_9: return '9';
	}
	return 0;
}



/****************************************************/
//Function UpdateDisplay()
//Writes the strings to the LCD display (h/w) or terminal (windows)
/****************************************************/
void UpdateDisplay( void )
{
    int i, PadSpaces;

    if (DisplayChanged)         //write both strings to LCD
    {
        LCDWriteInstruction(RETURN_HOME, TRUE);             //put cursor to top left

        PadSpaces = FALSE;

        for (i=0 ; i<16 ; i++)
        {
            if (DisplayTop[i] == 0)
              PadSpaces = TRUE;
            if (PadSpaces == TRUE)
              DisplayTop[i] = ' ';
            LCDWriteData(DisplayTop[i]);              //write next character. Cursor auto increments
        }

        LCDWriteInstruction(SET_DISPRAM_ADX | 0x40, TRUE);  //put cursor to top right

        PadSpaces = FALSE;

        for (i=0 ; i<16 ; i++)
        {
            if (DisplayBottom[i] == 0)
              PadSpaces = TRUE;
            if (PadSpaces == TRUE)
              DisplayBottom[i] = ' ';
            LCDWriteData(DisplayBottom[i]);              //write next character. Cursor auto increments
        }

        DisplayChanged = FALSE;
    }
    else return;
}

/****************************************************/
//Channel handling functions.
/****************************************************/
void ChannelUp(void)
{
	CurrentChannel++;
	SetChannel();	//bounds checking done in SetChannel()
	return;
}

void ChannelDown(void)
{
	CurrentChannel--;
	SetChannel();	//bounds checking done in SetChannel()
	return;
}

void SetChannel(void)	//make void
{
	if (CurrentChannel > MAX_CHANNEL) CurrentChannel = MIN_CHANNEL;	//Wrap from top to bottom
	if (CurrentChannel < MIN_CHANNEL) CurrentChannel = MAX_CHANNEL;	//Wrap from bottom to top

	//Program the synth
	Radio_SkyWorks(Channels[CurrentChannel].frac1, Channels[CurrentChannel].frac2, CBUS_INTERFACE_2);

	return;
}

/****************************************************/
//Audio Volume handling functions.
/****************************************************/
void VolumeUp( void )
{
	CurrentVol++;
	SetVol();	//bounds checking done in SetVol()
	return;
}

void VolumeDown( void )
{
	CurrentVol--;
	SetVol();	//bounds checking done in SetVol()
	return;
}


/****************************************************/
//Key handling functions.
/****************************************************/
#define KEY_NOT_PRESSED 0
#define KEY_PRESSED 1
//#define BACKLIGHT_TIME 0x0008ffff
#define BACKLIGHT_TIME 0x0001cccc

//Inputs (columns) have internal pull-ups.
//We drive all rows low, and check to see if any columns are low.
//If any are low, then a key must be pressed, so we scan to find out which key as follows:
//Drive all rows high, then each one low in turn. Read the columns each time.
//If you find a column that's low, then that row/column combination is the key that's pressed.
//We return a value which identifies they key, and change state, so that next time we just check for no keys pressed.
//The 45us settling delay is require as a switch which is half-pressed can have a relatively high impedance,
//so the capacitance on the input takes this long to charge.

short unsigned int Keypad(void)
{
	int i,rows,columns,count;
        static int state = KEY_NOT_PRESSED;

        if (backlight_timer == 0)
        {
          backlight = 0;
          GPIO_ResetBits(LCD, BACKLIGHT); //backlight off
        }
        else
          backlight_timer--;

        GPIO_ResetBits(KEYPAD, ROWS);   //all rows normally driven low

        switch(state)
        {
        case KEY_NOT_PRESSED:           //check if any key is pressed. If something is, scan to find out which one.
          if( (GPIO_ReadInputData(KEYPAD) & COLUMNS) != COLUMNS )       //if a switch is pressed, one will be low
          {                                                             //won't be picked up on first pass (no settling time)
                                                                        //but will be next time.
            for (i=0 ; i<4 ; i++)               //scan through keypad
            {
                //drive 1 output low, 3 high
                rows = 0x0001 << i;             //select asseted row
                GPIO_SetBits(KEYPAD, ROWS);     //set bottom 4
                GPIO_ResetBits(KEYPAD, rows);   //clear one
                //for (j=0 ; j<350 ; j++);
                count = ReadCounter() + 45;       //let everything settle 45us
                while (ReadCounter() < count);

                //read inputs, invert, mask, shift gives 1,2,4,8 for key pressed, 0 if no key
                columns = ((~GPIO_ReadInputData(KEYPAD)) & COLUMNS) >> 4;

                //left shift by i*4 gives values as per table in mmi_defines.h
                if (columns != 0)
                {
                  state = KEY_PRESSED;                  //stop looking until it's released

                  backlight = BACKLIGHT;                //set variable which is used by LCD routines, which are called when a key is pressed.
                  backlight_timer = BACKLIGHT_TIME;     //set up timer
                  return columns << (i*4);
                }
            }
            return 0;   //only get here if we find nothing during the scan.
          }
          else          //all columns high, means no switch pressed
            return 0;

        case KEY_PRESSED:       //If key has been released, revert to KEY_NOT_PRESSED state.
            if( (GPIO_ReadInputData(KEYPAD) & COLUMNS) == COLUMNS )       //if no switch is pressed, all will be high
              state = KEY_NOT_PRESSED;

            return 0;

        default:
            state = KEY_NOT_PRESSED;
            return 0;
        }
}

//return the state of the PTT key. TRUE if PTT is pressed, FALSE if not
//PTT key has an internal ARM pullup, switch connects to ground.
short unsigned int ReadPTT(void)
{
  int count;

  if (GPIO_ReadInputData(KEYPAD) & PTT_IO)      //key not pressed
    return FALSE;

  else                                  //key pressed
  {
    count = ReadCounter() + 200;       //let everything settle 200us
    while (ReadCounter() < count);

    if (GPIO_ReadInputData(KEYPAD) & PTT_IO)      //check again
      return FALSE;
    else
      return TRUE;
  }
}

//Write byte with Register Select (RS) high
void LCDWriteData (char data)
{
    while(LCDReadBusy()){}

  LCD_Word = RS | backlight;                //RS high, r/w low
  GPIO_Write(LCD, LCD_Word);

  //wait 40ns

    LCDWriteByte(data);

  return;
}

//Write byte with Register Select (RS) low
void LCDWriteInstruction (char data, char check_busy)
{
  if (check_busy)
  {
    while(LCDReadBusy()){}
  }

  LCD_Word = 0 | backlight;                 //RS low, r/w low
  GPIO_Write(LCD, LCD_Word);

  //wait 40ns

  LCDWriteByte(data);

  return;
}

void LCDWriteByte(char data)
{
  LCD_Word |= EN;               //enable high
  LCD_Word |= data;             //data valid
  GPIO_Write(LCD, LCD_Word);
  //wait 230ns (setup / EN cycle time)

  LCD_Word &= ~EN;              //enable low (strobe in)
  GPIO_Write(LCD, LCD_Word);
  //wait 10ns (hold)
  //data, r/w, rs allowed to change
  //wait 260ns before driving EN low again.

  return;
}

int LCDReadBusy(void)
{
  int Busy;

  GPIOD->CRL = 0x44444444;       //Set bits 0-7 to input with pullup/pulldown

  LCD_Word =  RW | backlight;           //RS low, r/w high
  GPIO_Write(LCD, LCD_Word);
  //wait 40ns

  LCD_Word |= EN;               //enable high
  GPIO_Write(LCD, LCD_Word);
  //wait 120ns (read delay)

  Busy = (GPIO_ReadInputData(LCD) & BUSY);      //read data
  //wait 210ns (cycle time - read delay)

  LCD_Word &= ~EN;              //enable low (strobe in)
  GPIO_Write(LCD, LCD_Word);

   GPIOD->CRL = 0x11111111;   //Set bits 0-7 to push pull outputs

  //wait 10ns before changing RS, R/W
  return Busy;
}

void BatteryCheck (void)
{
	 long int ADCReading;
	 static long int OldADCReading = 4500000;      //nominal
	 char BattTemp[8];

	if(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) != RESET);  // If an ADC conversion has completed
	  {
	    ADCReading = ADC_GetConversionValue(ADC1);           //read it

	    ADCReading = ADCReading * 1604;       //scale to give uV (scale factor derived empirically)

	    if ( (ADCReading + 15000 < OldADCReading) || (ADCReading - 15000 > OldADCReading))  //if the voltage has changed by > 0.015V
	    {                                                                                   //then update it.
	      OldADCReading = ADCReading;
	      sprintf(BattTemp,"%d",ADCReading);	//Convert to a string

	      BatteryVoltage[0] = BattTemp[0];    //and manipulate to give string in volts. Avoids floating point maths.
	      BatteryVoltage[1] = '.';
	      BatteryVoltage[2] = BattTemp[1];
	      BatteryVoltage[3] = BattTemp[2];
	      UpdateStrings();
	    }
	  }

}
