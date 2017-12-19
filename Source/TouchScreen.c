/************************************************************************************************
  Filename      :  TouchScreen.C
  Project				:  Bevertec
  Description   :  This is the implementation of Touch Screen features.
**************************************************************************************************/

#include <ctype.h>
#include <ascii.h>
#include <aclascii.h>
#include <aclconio.h>
#include <errno.h>
#include <svc_sec.h>

//Include file for ACT2000

#include <time.h>
#include <formater.h>

#include "..\include\Appevent.h"
#include "..\include\TouchScreen.h"
#include "..\include\Common.h"
extern short handleConsole;

// This structure maps areas to a key value that will be returned when touching that 
// particular area.  Key will be returned keyboard events.
// Application is responsible for drawing the key areas idenfitied in the array
//The key event for transaction,supervisor and merchant button



//maps areas for Main Menu screen 
static tkm_t touchMainMenuScreen[] =
{
	26,  30, 99,  103, eTransaction,
	137, 30, 210, 103, eMerchent,
	80,  165,154, 238, eSuperviser

};
//maps areas for Transaction Methord screen 
static tkm_t touchTransMethordScreen[] =
{
	26,  30,  99,  103, eCardActivation,       
	137, 30,  210, 103, ePinChange,
	//80,  165, 154, 238, eLoyality,
	15,  270, 61,  310, eTrMethordBack
};

//maps areas for Transaction Type screen 
static tkm_t touchTransTypeScreen[] =
{
	26,  30, 99,   103,  eSale,							//Left Top
	137, 30, 210,  103,  eBalanceInquiry,				//Right Top
	26,  150,99,   224,  eVoid,							//Left Bottom
	136, 150, 209, 224,  eRefund,						//Right Bottom
	15,  270, 61,  310,  eTrTypeBack  
};

static tkm_t Agency_touchTransTypeScreen[] =
{
	26,  30, 99,  103, eSale,
	137, 30, 210, 103, eVoid,
	80,  165,154, 238, eRefund,
	15,  270, 61,  310,  eTrTypeBack
	//26,  30, 99,   103,  eWithdrawal,
	//137, 30, 210,  103,  eBalanceInquiry,
	//26,  150,99,   224,  eDeposit,
	//136, 150, 209, 224,  eTransfer,
	//15,  270, 61,  310,  eTrTypeBack,
	//173, 270, 220,  310, eTrTypeNext
};

static tkm_t Agency_touchTransType1Screen[] =
{
	26,  30, 99,  103, eVoid,
	15,  270, 61, 310, eTrType1Back
};
//maps areas for Account Type screen used for Debit card 
static tkm_t touchAccTypeScreen[] =
{
	26,  30, 99,  103,   eChequeAccount,
	137, 30, 210, 103,	 eSavingAccount,
	80,  165,154, 238,	 eCreditAccount,
	15,  270, 61,  310,  eAccTypeBack

};
//maps areas for Agency Bank Account Type screen used for Debit card
static tkm_t Agency_touchAccTypeScreen[] =
{
	26,  30, 99,  103,   eChequeAccount,
	137, 30, 210, 103,	 eSavingAccount,
	80,  165,154, 238,	 eOtherAccount,
	15,  270, 61,  310,  eAccTypeBack
	
};
static tkm_t Agency_touch_Sav_Oth_AccScreen[] =
{
	26,  30, 99,  103,   eCheToSavAccount,
	137, 30, 210, 103,	 eCheToOthAccount,
	//80,  165,154, 238,	 
	15,  270, 61,  310,  eAccTypeBack
	
};
static tkm_t Agency_touch_Che_Oth_AccScreen[] =
{
	26,  30, 99,  103,   eSavToCheAccount,
	137, 30, 210, 103,	 eSavToOthAccount,
	//80,  165,154, 238,	 
	15,  270, 61,  310,  eAccTypeBack
	
};
static tkm_t Agency_touch_ToAccTypeScreen[] =
{
	26,  30, 99,  103,   eOthToCheAccount,
	137, 30, 210, 103,	 eOthToSavAccount,
	80,  165,154, 238,	 eOthToOthAccount,
	15,  270, 61,  310,  eAccTypeBack
	
};





tkm_t defaultsetting[] =
{
       0,  0,  0, 0, 0
       

};





//maps areas for Merchanr First screen 
static tkm_t touchMerchFirstScreen[] =
{
	26,  30,  99,   103, eLogOn,
	137, 30,  210,  103, eSattlement,
	26,  149, 99,   224, eTransactionDetail,		//Agency Bank
	//26,  149, 99,   224, ecopy,
	136, 149, 209,  224, eTotalReport,
	15,  270, 61,   310, eMerchant1Back,
	//173, 270, 220,  310, eMerchant1Next
	
};

//maps areas for Merchant Second screen 
static tkm_t touchMerchSecScreen[] =
{
	26,  30,  99,  103,  eTransactionDetail,
	137, 30,  210, 103,  eKeyExchange,
	15,  270, 61,  310,  eMerchant2Back
};

//maps areas for Supervisor first screen 
/* Commented for Aggency bank
static tkm_t touchSuperFirstScreen[] =
{
	26,  30, 99,   103, ePassword,
	137, 30, 210,  103, econfiguration,
	26,  149, 99,  224, eDownload,
	136, 149, 209, 224, eKeyDownload,
	15,  270, 61,  310, eSuper1Back,
	173, 270, 220, 310, eSuper1Next
};
*/
//Agency Bank 
static tkm_t touchSuperFirstScreen[] =
{
	26,  30, 99,   103, ePassword,
	137, 30, 210,  103, econfiguration,
	26,  149, 99,  224, ePINValidate,
	136, 149, 209, 224, eLogout,
	//136, 149, 209, 224, eKeyDownload,
	15,  270, 61,  310, eSuper1Back,
	//173, 270, 220, 310, eSuper1Next
};
	//maps areas for Supervisor Second screen 
static tkm_t touchSuperSecScreen[] =
{
	26,  30, 99,  103, eHelpDesk,
	15,  270, 61, 310, eSuper2Back
};

//maps areas for Cash Back screen 
static tkm_t touchCashBackScreen[] =
{
	26,  149, 99,  224, eCashBackYes,
	136, 149, 209, 224, eCashBackNo,
	15,  270, 61,  310, eCashBack 
	
};

static tkm_t touchConfigkScreen[] =
{
		17,24,72,80,COMPNAME,
		89,24,143,80,HOST_IP,
		156,24,211,80,HOST_PORT,
		18,108,72,163,MERCHANT_ID,
		89,107,143,163,MERCHANT_NAME,
		157,107,211,163,TERMINAL_ID,
		17,185,71,242,SUPER_PASSWORD,
		15,  270, 61,  310,CONGIF_BACK
	
};

static tkm_t touchCreditTransTypeScreen[] =
{
  26,  30, 99,  103,   eSale,
	137, 30, 210, 103,	 eVoid,
	80,  165,154, 238,	 eRefund,
  15,  270, 61,  310,  eTrTypeBack  
};


static tkm_t touchAppSelectionScreen1[] =
{
	  100,  72,  200,  104, App1,
	
};
static tkm_t touchAppSelectionScreen2[] =
{
    100,  72,  200,  104, App1,
	100, 120,  200, 152, App2,
};
static tkm_t touchAppSelectionScreen3[] =
{
  100,  72,  200,  104, App1,
	100, 120,  200, 152, App2,
	100,  168, 200, 200, App3,
};
static tkm_t touchAppSelectionScreen4[] =
{
  100,  72,  200,  104, App1,
	100, 120,  200, 152, App2,
	100,  168, 200, 200, App3,
	100,  216, 200,  248, App4,
	
};


static tkm_t touchAppSelectionScreenNext[] =
{
  100,  72,  200,  104, App1,
	100, 120,  200, 152, App2,
	100,  168, 200, 200, App3,
	100,  216, 200,  248, App4,
	//220,  288, 235,  304, AppNext
  180,  288, 239,  319, AppNext
};
static tkm_t touchAppSelection5ScreenPrev[] =
{
  100,  72,  200,  104, App1,
	100, 120,  200, 152, App2,
	100,  168, 200, 200, App3,
	100,  216, 200,  248, App4,
	1,  288, 56,  304, AppPrev
};
static tkm_t touchAppSelection4ScreenPrev[] =
{
 100,  72,  200,  104, App1,
	100, 120,  200, 152, App2,
	100,  168, 200, 200, App3,
	
	1,  288, 56,  304, AppPrev
};
static tkm_t touchAppSelection3ScreenPrev[] =
{
  100,  72,  200,  104, App1,
	100, 120,  200, 152, App2,
	1,  288, 56,  304, AppPrev
};
static tkm_t touchAppSelection2ScreenPrev[] =
{
  100,  72,  200,  104, App1,
	1,  288, 56,  304, AppPrev
};

// setting Image on screen
static tkm_t touchLangSelectionScreen1[] =
{
	  100,  72,  200,  104, App1,
	
};
static tkm_t touchLangSelectionScreen2[] =
{
  100,  72,  200,  104, App1,
	100, 120,  200, 152, App2,
};
static tkm_t touchLangSelectionScreen3[] =
{
  100,  72,  200,  104, App1,
	100, 120,  200, 152, App2,
	100,  168, 200, 200, App3,
};
static tkm_t touchLangSelectionScreen4[] =
{
  100,  72,  200,  104, App1,
	100, 120,  200, 152, App2,
	100,  168, 200, 200, App3,
	100,  216, 200,  248, App4,
	
};
int SetImage(char *ImageName)
{
	window(1,1,30,20);
	clrscr();
	set_display_coordinate_mode (PIXEL_MODE);
	put_BMP_at (1, 1, ImageName);
	set_display_coordinate_mode (CHARACTER_MODE);
	return 0;
}

void DisplayMsgOnScreen(char *msg,short x,short y)
{
		write_at(msg,strlen(msg),x,y);
}

// This Function  maps areas to a key value that will be returned when touching that 
// particular area.  Key will be returned keyboard events.
/****************************************************************************************************************
*	Function Name : EnblTouchScreen																											                          *
*	Purpose		    : Enable touch screens for variious icons in the transaction ,merchant and the super visor menu *
* Input					: int index																							                                        *
*	Output		    : void																																		*
*****************************************************************************************************************/
void EnblTouchScreen(int Key)   // set up keymap and display prompt
{

	switch(Key)
	{
		case  mainMenuS:
			set_touchscreen_keymap((tkm_t *)touchMainMenuScreen, sizeof(touchMainMenuScreen)/sizeof(tkm_t));
			break;
		case  TransMethord:
			set_touchscreen_keymap((tkm_t *)touchTransMethordScreen, sizeof(touchTransMethordScreen)/sizeof(tkm_t));
			break;
		case  TransType:
			//set_touchscreen_keymap((tkm_t *)touchTransTypeScreen, sizeof(touchTransTypeScreen)/sizeof(tkm_t));
			set_touchscreen_keymap((tkm_t *)Agency_touchTransTypeScreen, sizeof(Agency_touchTransTypeScreen)/sizeof(tkm_t));	//Agency Bank
			break;
		case  TransType1:
			//set_touchscreen_keymap((tkm_t *)touchTransTypeScreen, sizeof(touchTransTypeScreen)/sizeof(tkm_t));
			set_touchscreen_keymap((tkm_t *)Agency_touchTransType1Screen, sizeof(Agency_touchTransTypeScreen)/sizeof(tkm_t));	//Agency Bank
			
			break;
		case  AccountType:
			set_touchscreen_keymap((tkm_t *)touchAccTypeScreen, sizeof(touchAccTypeScreen)/sizeof(tkm_t));
			break;
		case Agency_AccountType:	
			set_touchscreen_keymap((tkm_t *)Agency_touchAccTypeScreen, sizeof(Agency_touchAccTypeScreen)/sizeof(tkm_t));
			break;
		case Agency_Sav_Oth_AccountType:	
			set_touchscreen_keymap((tkm_t *)Agency_touch_Sav_Oth_AccScreen, sizeof(Agency_touchAccTypeScreen)/sizeof(tkm_t));
			break;
		case Agency_Che_Oth_AccountType:	
			set_touchscreen_keymap((tkm_t *)Agency_touch_Che_Oth_AccScreen, sizeof(Agency_touchAccTypeScreen)/sizeof(tkm_t));
		break;
			case Agency_ToAccountType:	
			set_touchscreen_keymap((tkm_t *)Agency_touch_ToAccTypeScreen, sizeof(Agency_touchAccTypeScreen)/sizeof(tkm_t));
			break;
		case  MerchentMenu1:
			set_touchscreen_keymap((tkm_t *)touchMerchFirstScreen, sizeof(touchMerchFirstScreen)/sizeof(tkm_t));
			break;
		case  MerchentMenu2:
			set_touchscreen_keymap((tkm_t *)touchMerchSecScreen, sizeof(touchMerchSecScreen)/sizeof(tkm_t));
			break;
		case  SupervisorMenu1:
			set_touchscreen_keymap((tkm_t *)touchSuperFirstScreen, sizeof(touchSuperFirstScreen)/sizeof(tkm_t));
			break;
		case  SupervisorMenu2:
			set_touchscreen_keymap((tkm_t *)touchSuperSecScreen, sizeof(touchSuperSecScreen)/sizeof(tkm_t));
			break;
		case  CashBack:
			set_touchscreen_keymap((tkm_t *)touchCashBackScreen, sizeof(touchCashBackScreen)/sizeof(tkm_t));
			break;
		case ConfigMenu:
				set_touchscreen_keymap((tkm_t *)touchConfigkScreen, sizeof(touchConfigkScreen)/sizeof(tkm_t));
			break;
    case CreditTransMenu:
				set_touchscreen_keymap((tkm_t *)touchCreditTransTypeScreen, sizeof(touchCreditTransTypeScreen)/sizeof(tkm_t));
			break;
    case DisableTouch:
       set_touchscreen_keymap((tkm_t *)defaultsetting, sizeof(defaultsetting)/sizeof(tkm_t));  
   	break;
    default :
      LOG_PRINTF(("Default lang selected"));
      break;
	} 
}

/****************************************************************************************************************
*	Function Name : EnblMultiAppTouchScreen																											                  *
*	Purpose		    : Enable touch screens for multiple application in EMV card                                     *
* Input					: int index																							                                        *
*	Output		    : void 																																		                      *
*****************************************************************************************************************/
void EnblMultiAppTouchScreen(int Key)   // set up keymap and display prompt
{
	switch(Key)
	{
		case  MultiApp1:
			set_touchscreen_keymap((tkm_t *)touchAppSelectionScreen1, sizeof(touchAppSelectionScreen1)/sizeof(tkm_t));
			break;
		case  MultiApp2:
			set_touchscreen_keymap((tkm_t *)touchAppSelectionScreen2, sizeof(touchAppSelectionScreen2)/sizeof(tkm_t));
			break;
		case  MultiApp3:
			set_touchscreen_keymap((tkm_t *)touchAppSelectionScreen3, sizeof(touchAppSelectionScreen3)/sizeof(tkm_t));
			break;
		case  MultiApp4:
			set_touchscreen_keymap((tkm_t *)touchAppSelectionScreen4, sizeof(touchAppSelectionScreen4)/sizeof(tkm_t));
			break;
    case  MultiAppNext:
			set_touchscreen_keymap((tkm_t *)touchAppSelectionScreenNext, sizeof(touchAppSelectionScreenNext)/sizeof(tkm_t));
			break;
    case  MultiAppPrev1:
			set_touchscreen_keymap((tkm_t *)touchAppSelection2ScreenPrev, sizeof(touchAppSelection2ScreenPrev)/sizeof(tkm_t));
			break;
		case  MultiAppPrev2:
			set_touchscreen_keymap((tkm_t *)touchAppSelection3ScreenPrev, sizeof(touchAppSelection3ScreenPrev)/sizeof(tkm_t));
			break;
		case  MultiAppPrev3:
			set_touchscreen_keymap((tkm_t *)touchAppSelection4ScreenPrev, sizeof(touchAppSelection4ScreenPrev)/sizeof(tkm_t));
			break;
		case  MultiAppPrev4:
			set_touchscreen_keymap((tkm_t *)touchAppSelection5ScreenPrev, sizeof(touchAppSelection5ScreenPrev)/sizeof(tkm_t));
			break;
   
    default :
      LOG_PRINTF(("Default lang selected"));
      break;
	} 
}

/****************************************************************************************************************
*	Function Name : EnblMultiLangTouchScreen																										                  *
*	Purpose		    : Enable touch screens for multiple language in EMV card                                        *
* Input					: int index																							                                        *
*	Output		    : void 																																		                      *
*****************************************************************************************************************/
void EnblMultiLangTouchScreen(int Key)   // set up keymap and display prompt
{
 /// EnblTouchScreen(DisableTouch);
		
	switch(Key)
	{
		case  MultiLang1:
			set_touchscreen_keymap((tkm_t *)touchLangSelectionScreen1, sizeof(touchLangSelectionScreen1)/sizeof(tkm_t));
			break;
		case  MultiLang2:
			set_touchscreen_keymap((tkm_t *)touchLangSelectionScreen2, sizeof(touchLangSelectionScreen2)/sizeof(tkm_t));
			break;
		case  MultiLang3:
			set_touchscreen_keymap((tkm_t *)touchLangSelectionScreen3, sizeof(touchLangSelectionScreen3)/sizeof(tkm_t));
			break;
		case  MultiLang4:
			set_touchscreen_keymap((tkm_t *)touchLangSelectionScreen4, sizeof(touchLangSelectionScreen4)/sizeof(tkm_t));
			break;
      
    default :
      LOG_PRINTF(("Default case selected"));
      break;
	} 
}
/****************************************************************************************************************
*	Function Name : ModifyLangName																				           						                  *
*	Purpose		    : char * code name for language                                                                 *
* Input					: int index																							                                        *
*	Output		    : void 																																		                      *
*****************************************************************************************************************/
void ModifyLangName(char * buff)   // set up keymap and display prompt
{
	if((!strcmp(buff,"es")))
  {
    strcpy(buff,"SPANISH");
  
  }
  else if((!strcmp(buff,"de")))
  {
    strcpy(buff,"GERMAN");
  }
  else if((!strcmp(buff,"fr")))
  {
    strcpy(buff,"FRENCH");
  }
}
