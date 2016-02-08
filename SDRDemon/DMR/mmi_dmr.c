/************************************************************************************************************
 *
 *  $Copyright © 2013-2014 CML Systems $
 *
 *  $Header: mmi_dmr.c  Revision:1.8.1.3  18 February 2014 10:20:29  ilewis $
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

#include "stdlib.h"
#include "mmi_dmr.h"
#ifdef DEBUG_CHANGE_INC_FROM_INITIAL_SDR_PROJECT
 #include "..\MMI\mmi_defines.h"
#else
 #include "mmi_defines.h"
#endif

extern signed int CurrentSetupItem;
extern signed int CurrentVol;

T2STATE Type2State = T2_WAITING_FOR_FIRST_KEY;

char RSSI[5];
// DAVED DEBUG - Start
char ERRCNT[5];
char FRMCNT[5];
// DAVED DEBUG - End

Channel Channels[MAX_CHANNEL+1] =
{
	{"D01", 0xC1, 0x55},
	{"D02", 0xC4, 0x00},
	{"D03", 0xC6, 0xAB},
	{"D04", 0xC9, 0x55},
	{"D05", 0xCC, 0x00},
	{"D06", 0xCE, 0xAB},
	{"D07", 0xD1, 0x55},
	{"D08", 0xD4, 0x00}
};


extern DMR_TypeDef dmr;

// These vars are directly related to the variables in the pointer references in the Setup array
// below.
signed int varTxPattern = 0;
signed int varPa = 0;
signed int varRxMode = 0;
signed int varReadDcOffsetI = 0;
signed int varReadDcOffsetQ = 0;
signed int varReadAgc = 0;
signed int varAudioTest = 0;
signed int varRxSlotTest = 0;
signed int varSavedStatus = 1;	// Set to 1 because at power up the configurables will not have changed.
signed int varDummy = 0;
//signed int varCMX994Gain = 0;

Setup_TypeDef SetupArray[MAX_SETUP_ENTRY+1] =
{
	{"Tx Mode",		1, 6, "Off", "PRBS", "00", "01","10","11", &varTxPattern, &tx_pattern_handler},
	{"PA",		1, 3, "Off", "LO Pwr", "HI Pwr ", " ", " "," ",&varPa, &pa_handler},
	{"Rx Mode",		1, 4, "Off", "Setup", "Eye", "Data","","", &varRxMode, &rx_mode_handler},
	{"Freq Control",		2, 0, " ", " ", " ", " "," "," ",&dmr.Cmx7161.sFreqControl,&freq_control_handler},
	{"Tx Mod Deviation",		2, 0, " ", " ", " ", " "," "," ",&dmr.Cmx7161.sCbus5E, &CBUS_5E_handler},
	{"Tx Mod Gain",		2, 0, " ", " ", " ", " "," "," ",&dmr.Cmx7161.sCbusB4, &CBUS_B4_handler},
	{"CMX994 DC Offset",		2, 0, " ", " ", " ", " "," "," ",&dmr.Cmx7161.sCMX994DcOffset, &CMX994_DC_Offset_handler},
	{"CMX994 Gain",		1, 4, "-60dB", "-42dB", "-24dB", "0dB"," "," ", &dmr.Cmx7161.sCMX994GainIndex, &CMX994_Gain_handler},
	{"Mic Input Gain",		2, 0, " ", " ", " ", " "," "," ",&dmr.Cmx7262.sInputGain, &MicInputGain_handler},
	{"Keypad Test",		2, 0, " ", " ", " ", " ", " ", " ", &varDummy, &dummy},
	{"Save?",		1, 2, "Not Saved ", "Saved ", " ", " "," "," ", &varSavedStatus, &Save_Handler},
	{"CMX7161 Version",		0, 0, " ", " ", " ", " "," "," ", &dmr.Cmx7161.FIVersion, &dummy},
	{"CMX7262 Version",		0, 0, " ", " ", " ", " "," "," ", &dmr.Cmx7262.FIVersion, &dummy}
	// DAVED - Removed test mode for CML release at version 3v4.
    //{"Test Mode",		1, 2, "Off ", "On ", " ", " "," "," ", &varRxSlotTest, &TestMode_Handler}
};

void CheckRadioMessages(void)
{
  switch (dmr.eRadioRequest)
  {
  case MMI_DMR_NO_MESSAGE:      //no message, so nothing to do
  break;

  case MMI_DMR_INIT_COMPLETE:
	  // Now that the system is setup and the CMX's are initialised, change the MMI into radio mode.
	  MMI_Mode = MMODE_RADIO;
	  dmr.eRadioRequest = MMI_DMR_NO_MESSAGE;     //and clear off the message
	  break;

  case MMI_DMR_START_RX:        //Going in to Rx,
    MMI_State = MSTATE_RX;                      // so change MMI state
    UpdateStrings();                            // update the display strings
    dmr.eRadioRequest = MMI_DMR_NO_MESSAGE;     //and clear off the message
    break;

  case MMI_DMR_RX_STOP:         //going out of Rx
     MMI_State = MSTATE_WAITING;                // so change MMI state
     UpdateStrings();                           // update the display strings
     dmr.eRadioRequest = MMI_DMR_NO_MESSAGE;    //and clear off the message
  break;

   // **************************  Data  Start *********************************/
   /* For normal voice operation the data specific code can be ignored */

  case MMI_DMR_RX_DATA_STOP:         //going out of Rx
	  MMI_State = MSTATE_WAITING_DATA;                // so change MMI state
	  UpdateStrings();                           // update the display strings
	  dmr.eRadioRequest = MMI_DMR_NO_MESSAGE;    //and clear off the message
  break;

  case MMI_DMR_RX_DATA_RUN:
    MMI_State = MSTATE_RX_DATA_RUN;
    UpdateStrings();
  break;
  case MMI_DMR_RX_DATA_PAUSE:
      MMI_State = MSTATE_RX_DATA_PAUSE;
      UpdateStrings();
  break;

  // ****************************  Data  End *********************************/

  default:
  break;
  }
}

/****************************************************/
//Function ProcessKeys()
//Main bit of MMI
/****************************************************/
char Type2Temp[17];			//string for typed in value
char tempchar;				//single char for poking in here
int Type2TempIdx;			//Index to string
int Type2FirstTime = TRUE;	        //flag to reset it

void ProcessKeys( void )
{
	static int OldKeys, OldPTT;
	int NewKeys, PTT, temp;

	NewKeys=Keypad();			//read the keypad
	PTT = ReadPTT();

	if (NewKeys == 0)
		OldKeys = NewKeys;	//don't process key releases

	if ((NewKeys != OldKeys) || (PTT != OldPTT))		//if something's changed since last time
	{
		OldKeys = NewKeys;		//remember it
		OldPTT = PTT;

		if (MMI_Mode == MMODE_RADIO)
		{
			switch (MMI_State)
			{
                case MSTATE_WAITING:

				if (NewKeys & KEY_STAR)
				{
					//change to setup mode
					MMI_Mode = MMODE_SETUP;
					dmr.eMMIRequest = DMR_MMI_STAR;;
					break;
				}
				else if (PTT)
				{
					//change to Tx
					dmr.eMMIRequest = DMR_MMI_PTT;
					MMI_State = MSTATE_TX;
					break;
				}
				else if (NewKeys & KEY_A)
					ChannelUp();
				else if (NewKeys & KEY_B)
					ChannelDown();
				else if (NewKeys & KEY_C)
					VolumeUp();
				else if (NewKeys & KEY_D)
					VolumeDown();
			break;

            case MSTATE_RX:
				if (NewKeys & KEY_C)
					VolumeUp();
				else if (NewKeys & KEY_D)
					VolumeDown();
			break;

            case MSTATE_TX:
            	if (!PTT)
				{
					//change to Waiting
					MMI_State = MSTATE_WAITING;
					// Send Idle request for release of PTT
					dmr.eMMIRequest = DMR_MMI_IDLE;
					break;
				}
			break;

			// **************************  Data  Start *********************************/
			/* For normal voice operation the data specific code can be ignored */

            case MSTATE_WAITING_DATA:
            	if (NewKeys & KEY_STAR)
            	{
            		//change to setup mode
            		MMI_Mode = MMODE_SETUP;
            		dmr.eMMIRequest = DMR_MMI_STAR;;
            		break;
            	}
            	else if (PTT)
            	{
            		//change to Tx
            		dmr.eMMIRequest = DMR_MMI_PTT;
            		MMI_State = MSTATE_TX_DATA;
            		break;
            	}
            	else if (NewKeys & KEY_A)
            		ChannelUp();
            	else if (NewKeys & KEY_B)
            		ChannelDown();
            break;

            // The PTT key is used during the Rx Data test to pause the test and set
			// the radio back to data idle.
            case MSTATE_RX_DATA_RUN:
            case MSTATE_RX_DATA_PAUSE:
            	if (PTT)
    				dmr.eMMIRequest = DMR_MMI_PTT;
    			else
    				dmr.eMMIRequest = DMR_MMI_IDLE;
    			break;

            case MSTATE_TX_DATA:
 				if (!PTT)
 				{
 					//change to Waiting
 					MMI_State = MSTATE_WAITING_DATA;
 					// Send Idle request for release of PTT
 					dmr.eMMIRequest = DMR_MMI_IDLE;
 					break;
 				}
 			break;

    		// *************************** Data  End ********************************/

			}//end of switch (MMI_State)
		}//end of if (MMI_Mode == RADIO_MODE)

		else if (MMI_Mode == MMODE_SETUP)	//Must be setup mode
		{
			//These keys always valid in setup mode
			if (NewKeys & KEY_STAR)
			{
				//change to radio mode
				MMI_Mode = MMODE_RADIO;
				Type2State = (T2_WAITING_FOR_FIRST_KEY);
				dmr.eMMIRequest = DMR_MMI_IDLE;
			}

			if (NewKeys & KEY_A)
			{
				CurrentSetupItem++;
				if (CurrentSetupItem > MAX_SETUP_ENTRY)
					CurrentSetupItem = 0;
				Type2State = (T2_WAITING_FOR_FIRST_KEY);
			}
			if (NewKeys & KEY_B)
			{
				CurrentSetupItem--;
				if (CurrentSetupItem <  MIN_SETUP_ENTRY)
					CurrentSetupItem = MAX_SETUP_ENTRY;
				Type2State = (T2_WAITING_FOR_FIRST_KEY);
			}

			//Now check the type of the item to see how we handle other keys
			if (SetupArray[CurrentSetupItem].EntryType == 0) //read only - do nothing. All handled in UpdateStrings()
			{}

			else if (SetupArray[CurrentSetupItem].EntryType == 1) //scroll through menu type
			{
				if (NewKeys & KEY_C)
				{
					(*SetupArray[CurrentSetupItem].CurrentValue)++;		//increment the pointer into the menu

					temp = *SetupArray[CurrentSetupItem].CurrentValue;	//remember it in a shorter variable name!

					if (temp >= SetupArray[CurrentSetupItem].NumValues)	//if it's too big
						*SetupArray[CurrentSetupItem].CurrentValue = 0;	//wrap to 0.

					SetupArray[CurrentSetupItem].fp(CurrentSetupItem);	//call the associated function
				}

				else if (NewKeys & KEY_D)
				{
					(*SetupArray[CurrentSetupItem].CurrentValue)--;		//decrement it

					temp = *SetupArray[CurrentSetupItem].CurrentValue;	//remember it in a shorter variable name!

					if (temp < 0)	//if it's too small
						*SetupArray[CurrentSetupItem].CurrentValue = SetupArray[CurrentSetupItem].NumValues-1;	//wrap to max.

					SetupArray[CurrentSetupItem].fp(CurrentSetupItem);	//call the associated function
				}
			}//end of type 1

			else if (SetupArray[CurrentSetupItem].EntryType == 2) //keypad entry value type
			{
				switch(Type2State)
				{
				case T2_WAITING_FOR_FIRST_KEY:
					//itoa(SetupArray[CurrentSetupItem].CurrentValue,Type2Temp,10);
					sprintf(Type2Temp, "%d", *(SetupArray[CurrentSetupItem].CurrentValue));

					if ((tempchar = KeyToChar(NewKeys)) != 0) //A number key was pressed so...
					{
						Type2TempIdx = 0;						//reset the pointer
						Type2Temp[Type2TempIdx++] = tempchar;	//put the first character in, increment pointer
						Type2Temp[Type2TempIdx] = 0;			//null teminator, DON'T INCREMENT pointer so it gets overwritten next time around.
						Type2State = (T2_WAITING_FOR_NEXT_KEY);	//do this next time
					}
				break;

				case T2_WAITING_FOR_NEXT_KEY:
					if (NewKeys == KEY_HASH)	//used as 'enter'
					{
						// DAVED - Commented out as we show the confirmation after we have checked its range.
						//Type2Temp[Type2TempIdx++] = '#';	//display # as confirmation
						Type2Temp[Type2TempIdx] = 0;		//null teminator.
						*SetupArray[CurrentSetupItem].CurrentValue = atoi(Type2Temp);
						SetupArray[CurrentSetupItem].fp(CurrentSetupItem);	//call the associated function
						// DAVED  - Once the value has been range checked we add the # (Hash) confirmation.
						sprintf(Type2Temp, "%d", *SetupArray[CurrentSetupItem].CurrentValue);
						Type2Temp[Type2TempIdx++] = '#';	//display # as confirmation
						Type2Temp[Type2TempIdx] = 0;		//null teminator.

						Type2State = (T2_WAITING_FOR_FIRST_KEY);	//do this next time
					}
					else if ((tempchar = KeyToChar(NewKeys)) != 0) //A number key was pressed so...
					{
						Type2Temp[Type2TempIdx++] = tempchar;	//put the character in, increment pointer
						Type2Temp[Type2TempIdx] = 0;			//null teminator, DON'T INCREMENT pointer so it gets overwritten next time around.
					}
					if (Type2TempIdx >= 16)	//prevent typing past the end of the array.
						Type2TempIdx = 15;

				break;
				}
			}//end of type 2

		}//end of else //must be setup mode		
		else if (MMI_Mode == MMODE_INIT)			// Mode created so we can steal the MMI output for debug messages.
		{
			// While we are in debug mode, key presses are ignored, except for the a press of the
			// * key which returns it to the previous mode.
			if (NewKeys & KEY_STAR)
			{
#ifdef CML_GUI
				//change to script mode
				MMI_Mode = MMODE_GUI;
#endif
			}
		}


	UpdateStrings();	//we know a key has been pressed, so we probably need to change the display.

	}//end of if (NewKeys != OldKeys)

	//Maybe need to check here if mode has changed, and call UpdateStrings() if so
	//(for going in/out of Rx, as that's not controlled by keys)
}

/****************************************************/
//Function UpdateStrings()
//Changes the two strings representing the top and bottom
// lines of the LCD to reflect the current state.
/****************************************************/
void UpdateStrings(void)
{
	//char strptr[17];

	switch(MMI_Mode)
	{
	case MMODE_RADIO:
		//Do top line - just the name of the state
		switch (MMI_State)
		{
		case MSTATE_WAITING:
			sprintf(DisplayTop,"Waiting    %sV",BatteryVoltage);
			//do the bottom line - channel and volume
			sprintf(&DisplayBottom[0],"Ch:%s    Vol:%d",Channels[CurrentChannel].Name,CurrentVol);
		break;
		case MSTATE_RX:
			sprintf(DisplayTop,"Rx      RSSI:%s",RSSI);
			//do the bottom line - channel and volume
			sprintf(&DisplayBottom[0],"Ch:%s    Vol:%d",Channels[CurrentChannel].Name,CurrentVol);
		break;
		case MSTATE_TX:
			sprintf(DisplayTop,"Transmit   %sV",BatteryVoltage);
			//do the bottom line - channel and volume
			sprintf(&DisplayBottom[0],"Ch:%s    Vol:%d",Channels[CurrentChannel].Name,CurrentVol);
		break;

		// **************************  Data  Start *********************************/
		/* For normal voice operation the data specific code can be ignored */

		case MSTATE_TX_DATA:
			sprintf(DisplayTop,"TxTest     %sV",BatteryVoltage);
			//do the bottom line - channel and volume
			sprintf(&DisplayBottom[0],"Ch:%s    Vol:%d",Channels[CurrentChannel].Name,CurrentVol);
		break;
		case MSTATE_WAITING_DATA:
			sprintf(DisplayTop,"IdleTest   %sV",BatteryVoltage);
			//do the bottom line - channel and volume
			sprintf(&DisplayBottom[0],"Ch:%s    Vol:%d",Channels[CurrentChannel].Name,CurrentVol);
		break;
		case MSTATE_RX_DATA_RUN:
			sprintf(DisplayTop,"RxTest  RSSI:%s",RSSI);
			sprintf(ERRCNT,"%d",PRBS_ReadBitErrorCount(0));
			sprintf(FRMCNT,"%d",PRBS_ReadFrameCount());
			sprintf(DisplayBottom,"E:%s  F:%s",ERRCNT,FRMCNT);
		break;
		case MSTATE_RX_DATA_PAUSE:
			sprintf(DisplayTop,"RxTest  Paused");
			sprintf(ERRCNT,"%d",PRBS_ReadBitErrorCount(1));
			sprintf(FRMCNT,"%d",PRBS_ReadFrameCount());
			sprintf(DisplayBottom,"E:%s  F:%s",ERRCNT,FRMCNT);
		break;

		// **************************  Data  End * *********************************/

		}

	break;

	case MMODE_SETUP:
		//Display name of setup item on top line
		sprintf(DisplayTop,"%s",SetupArray[CurrentSetupItem].Name);

		//Bottom line depends on item type
		switch (SetupArray[CurrentSetupItem].EntryType)
		{
		case 0:	//read only
			SetupArray[CurrentSetupItem].fp(CurrentSetupItem);		//call the handler function to do the read
			//itoa(SetupArray[CurrentSetupItem].CurrentValue,strptr,10);	//convert value to string
			//sprintf(DisplayBottom,"%s",strptr);				//Update display string
                        sprintf(DisplayBottom,"%d",*(SetupArray[CurrentSetupItem].CurrentValue));
		break;

		case 1:	//type 1 is scroll through list, so use current value as index to array of strings
			sprintf(DisplayBottom,"%s",SetupArray[CurrentSetupItem].Values[*(SetupArray[CurrentSetupItem].CurrentValue)]);
		break;

		case 2: //type 2 is keypad entry, so display edit string
			sprintf(DisplayBottom,"%s",Type2Temp);	//Update display string
		break;
		}
	break;

	// We are in this mode while the CMX devices are in the process of code loading and initialisation
	case MMODE_INIT:
		// Overwrite the standard top display message of "Save" on the display to display the board number as well.
		//sprintf(DisplayTop,"Init Board      "); %d .. ",dmr.pFlash->uBoardNumber);
                sprintf(DisplayTop,INIT_STRING);
		sprintf(DisplayBottom,VERSION);
	break;
	case MMODE_GUI:
		sprintf(DisplayTop,"GUI Mode        ");// %d",dmr.pFlash->uBoardNumber);
	break;

	}

	DisplayChanged = TRUE;				//Flag to tell the main loop to write to the lcd

	return;
}


const uint16_t uVolumeTable[] = {
		0x003B,	// 00
		0x003B,	// 01
		0x003B,	// 02
		0x0038,	// 03
		0x0034,	// 04
		0x803B,	// 05
		0x8038, // 06
		0x8034,	// 07
		0x8030,	// 08
		0x802C, // 09
		0x0028,	// 10
		0x8024, // 11
		0x8020, // 12
		0x801C,	// 13
		0x8018,	// 14
		0x8014,	// 15
		0x8010, // 16
		0x800C,	// 17
		0x8008,	// 18
		0x8004,	// 19
		0x8000	// 20
};

void SetVol( void )
{
	if (CurrentVol > MAX_VOLUME) CurrentVol = MAX_VOLUME;	//Saturate at top
	if (CurrentVol < MIN_VOLUME) CurrentVol = MIN_VOLUME;	//Saturate at bottom

	// There are 59 steps of attenuation (0.8dB per step), plus one
	// 6dB step in hardware. There are 20 volume steps available in the MMI. From
	// this we have derived the setting below. Note that we keep the 6dB step in place.
	// and rely on the -47.2dB to adjust volume. Maximum MMI volume is 20 and
	// minimum attenuation on hardware is 0dB.
	dmr.Cmx7262.uOutputGain = uVolumeTable[CurrentVol];
	// Only set the flag once there is a valid volume value.

	dmr.eMMIRequest = DMR_MMI_VOLUME;
	while (dmr.eMMIRequest == DMR_MMI_VOLUME);
	return;
}

void ReadRSSI(void)
{
  int16_t temp;
  static int16_t old_reading;
  temp = CMX7161_ReadRSSI(&dmr.Cmx7161);
  if ((temp+2 < old_reading) || (temp-2 > old_reading))
  {
    old_reading = temp;
    // These two lines extend the signed bit of the 12 bit RSSI value.
    temp = temp << 4;
    temp = temp >> 4;
    sprintf(RSSI,"%d",temp);
    UpdateStrings();
  }
}

/****************************************************/
//Setup table handler functions
/****************************************************/
void tx_pattern_handler(int index )	//index points to row in setup array
{
	//setup tx pattern

	//lookup the thing we're supposed to do
	//N.B. The case values must match the order of the strings in the array....
	switch(*SetupArray[index].CurrentValue)
    {
    case 0:
    	CMX7161_ModemControl (&dmr.Cmx7161, cCMX7161_IDLE);
    break;
    case 1:
    	CMX7161_ModemControl (&dmr.Cmx7161, cCMX7161_TX_PRBS);
    break;
    case 2:
    	// Setup mode Tx Pattern 5F
    	CMX7161_ModemControl (&dmr.Cmx7161,cCMX7161_TX_PATTERN_00);
	break;
    case 3:
    	// Setup mode Tx Pattern 55 55 55 55 FF FF FF FF
    	CMX7161_ModemControl (&dmr.Cmx7161,cCMX7161_TX_PATTERN_01);
    break;
    case 4:
    	// Setup mode Tx Pattern 55
    	CMX7161_ModemControl (&dmr.Cmx7161,cCMX7161_TX_PATTERN_10);
	break;
    case 5:
    	// Setup mode Tx Pattern 7F 7D 5D D5 7D FD
       	CMX7161_ModemControl (&dmr.Cmx7161,cCMX7161_TX_PATTERN_11);
 	break;
    }
}

void pa_handler(int index)	//index points to row in setup array
{
	//lookup the thing we're supposed to do
	//N.B. The case values must match the order of the strings in the array....
	switch(*SetupArray[index].CurrentValue)
    {
    	case 0:
    	{
    		CMX7161_PA_Off (&dmr.Cmx7161);
    		break;
    	}
    	case 1:
    	{
    		CMX7161_PA_Low (&dmr.Cmx7161);
    		break;
    	}
    	case 2:
    	{
    		CMX7161_PA_High (&dmr.Cmx7161);
    		break;
    	}
    }

}

void freq_control_handler (int index)
{
	// The value is ranged checked against the max value the frequency
	// control can accept (AUXDAC3). We do nothing for out of range values.
	if(*SetupArray[index].CurrentValue>CMX7161_FREQ_CONTROL_MAX)
	{
		// Limit to maximum value.
		*SetupArray[index].CurrentValue = CMX7161_FREQ_CONTROL_MAX;
	}
	dmr.Cmx7161.sFreqControl = *SetupArray[index].CurrentValue;
	// Clearing varSavedStatus shows that the parameters have now changed and are not saved when
	// viewing the Save menu item.
	varSavedStatus = 0;
	CMX7161_FreqControl (&dmr.Cmx7161);
}

void CBUS_5E_handler(int index)
{
	if(*SetupArray[index].CurrentValue>65535)
	{
		// Limit to maximum value.
		*SetupArray[index].CurrentValue = 65535;
	}
	// Clearing varSavedStatus shows that the parameters have now changed and are not saved when
	// viewing the Save menu item.
	varSavedStatus = 0;
	dmr.Cmx7161.sCbus5E = *SetupArray[index].CurrentValue;
	CMX7161_Cbus_5E (&dmr.Cmx7161);
}

void CBUS_B4_handler(int index)
{
	if(*SetupArray[index].CurrentValue>65535)
	{
		// Limit to maximum value.
		*SetupArray[index].CurrentValue = 65535;
	}
	// Clearing varSavedStatus shows that the parameters have now changed and are not saved when
	// viewing the Save menu item.
	varSavedStatus = 0;
	dmr.Cmx7161.sCbusB4 = *SetupArray[index].CurrentValue;
	CMX7161_Cbus_B4 (&dmr.Cmx7161);
}


void rx_mode_handler(int index)
{
	switch(*SetupArray[index].CurrentValue)
    {
    case 0:
    	CMX7161_ModemControl (&dmr.Cmx7161,cCMX7161_IDLE);
    break;
    case 1:
    	CMX7161_ModemControl (&dmr.Cmx7161,cCMX7161_RX_SETUP);
    break;
    case 2:
    	CMX7161_ModemControl (&dmr.Cmx7161,cCMX7161_RX_EYE);
    break;
    case 3:
    	CMX7161_ModemControl (&dmr.Cmx7161,cCMX7161_RX_DATA);
    break;
    }

}	// read current value, switch rx eye on or off
void dummy(int index){}	//do nothing


void CMX994_DC_Offset_handler (int index)
{
	if(*SetupArray[index].CurrentValue>0xFF)
	{
		// Limit to maximum value.
		*SetupArray[index].CurrentValue = 0xFF;
	}
	// Clearing varSavedStatus shows that the parameters have now changed and are not saved when
	// viewing the Save menu item.
	varSavedStatus = 0;
	dmr.Cmx7161.sCMX994DcOffset = *SetupArray[index].CurrentValue;
	CMX7161_CMX994_DcOffset (&dmr.Cmx7161);

}

void CMX994_Gain_handler (int index)
{
    switch(*SetupArray[index].CurrentValue)
    {
        case 0: //-60dB
            dmr.Cmx7161.sCMX994Gain = 0x37;
        break;
        case 1: //-42dB
             dmr.Cmx7161.sCMX994Gain = 0x07;
        break;
        case 2: //-24dB
             dmr.Cmx7161.sCMX994Gain = 0x04;
        break;
        case 3: //0dB
             dmr.Cmx7161.sCMX994Gain = 0x00;    //I think this is the correct value for this *IPL
        break;
    }
    CMX7161_CMX994_Gain (&dmr.Cmx7161);
    varSavedStatus = 0;
}

void MicInputGain_handler (int index)
{
	if(*SetupArray[index].CurrentValue>7)
	{
		// Limit to maximum value.
		*SetupArray[index].CurrentValue = 7;
	}
	// Clearing varSavedStatus shows that the parameters have now changed and are not saved when
	// viewing the Save menu item.
	varSavedStatus = 0;
	dmr.Cmx7262.sInputGain = *SetupArray[index].CurrentValue;
	CMX7262_AudioInputGain (&dmr.Cmx7262);
}

void Save_Handler (int index)
{
	switch(*SetupArray[index].CurrentValue)
	    {
	    case 0:
	    	// This fiddles the MMI to always show saved for this menu option. The only
	    	// time when the status is shown as not saved is when the parameters have
	    	// been changed.
	    	varSavedStatus = 1;
	    break;
	    case 1:
	    	// Write the configurable's  to flash.
	    	DMR_WriteFlash(&dmr);
	    	// Update the status to show we have saved the configurable's.
	    	varSavedStatus = 1;
	    	//CMX7161_WriteFlash(&dmr.Cmx7161);
	    break;
          }
    return;
}

void TestMode_Handler (int index)
{
	switch(*SetupArray[index].CurrentValue)
	{
	    case 0:
	    	MMI_State = MSTATE_WAITING;
	    	dmr.eMMIRequest = DMR_MMI_DATA_OFF;
	    break;
	    case 1:
	    	MMI_State = MSTATE_WAITING_DATA;
	    	dmr.eMMIRequest = DMR_MMI_DATA_ON;
	    break;
    }
	return;
}
