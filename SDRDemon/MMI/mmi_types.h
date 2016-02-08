/************************************************************************************************************
 *
 *  $Copyright © 2014 CML Systems $
 *
 *  $Header: mmi_types.h  Revision:1.0.1.2  18 February 2014 11:08:54  ilewis $
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

typedef struct 
{
	char Name[4];
	char frac1;
        char frac2;
} Channel ;

typedef struct
{
	char Name[17];	//string for top line of display
	int EntryType;	//0-read only, 1-up/down select, 2-type in
	int NumValues;  //Number of values in up/down list (only valid for type 1)
	char Values[MAX_SETUP_VALUES][17];	//string values to display
	signed int *CurrentValue; //Value or index, depending on EntryType.
	void (*fp)(int); //function pointer, to act on what the current value
} Setup_TypeDef ;
