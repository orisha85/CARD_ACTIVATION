/*****************************************************************************************
  Filename		     : EMVCallBacks.c
  Project		       : Bevertec 
  Developer Neame  : Amar
  Module           : EMV
  Description	     : This is the implementation of EMV for verification method like offline-
                     online pin ,signature and build tags which are send to host
******************************************************************/



#include <svc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <logsys.h>
//#include <eoslog.h>

//emv includes
#include <VXEMVAP_confproto.h>
#include <vxemvapdef.h>
#include <cardslot.h>
#include <EMVCWrappers.h>
#include <EMVCStructs.h>

#include <vxemvap.h>
#include <emvlib.h>
#include <estfuncs.h>
#include <mvtfuncs.h>
#include <mvt.h>
#include <est.h>
#include <proto.h>
#include <Applidl.h>
#include <aclconio.h>


#include "..\include\Bevertec.h"
#include "..\include\BVTC_EMV.h"
#include "..\include\TouchScreen.h"
#include "..\Include\IPP_Key_injection.h"
#include "..\include\EMVCallBacks.h"


/****Emv global defnition********/
extern char Global_AccNo[ACC_NUM_ARR_LEN+1];//  
extern short PinFlag ;
extern sEMV_DATA emvdata;		//global emv structure


short	fSigRequired;
short	fPerformOfflinePIN;
short	fPerformOnlinePIN;
short fOnlinePINCancelled;
short fOfflinePINCancelled;
short initiateFallback;
int inPinTried;
unsigned char uchGblPinTryCtr;
int iGlobalNumberScripts = 0;
byte * pszGlobalIssuerScriptResults;

byte	*issuerScriptResults;
int inGblNoChipAttempts = 0;



#define MESSAGE_SIZE 40
// 
void displayAt(int x,int y,const char* m,int o) 
{
	// If message string is present
	if (strlen(m) > 0)
		display_at((unsigned int) x, (unsigned int) y, (char *) m, (unsigned int) o);
	// otherwise displays null
	else 
	{
		if((o == CLR_EOL) || (o == CLR_LINE)) 
			display_at((unsigned int) x, (unsigned int) y, " ", (unsigned int) o);
	}
	return;
}

/*****************************************************************************************
 <Name>vdPromptManager</Name>
 <Purpose> The EMV Module uses this function when the application has to display a
	message to warn or inform the user </Purpose>
 <Input>status returned by EMV initialization function</INPUT>
 <Date>17-March-2014</Date>
 <DeveloperName>Anil Kumar</DeveloperName>
 *****************************************************************************************/
void vdPromptManager(unsigned short usCondition)
{
  char szMessage[MESSAGE_SIZE + 1]= {0};//I

	//LOG_PRINTFF((0x8L, "vdPromptManager Condition = %X", usCondition));	
	memset(szMessage, 0, sizeof(szMessage));
	switch (usCondition)
	{
	case TRANS_APPROVED:
		// Approved message
		strcpy(szMessage, "APPROVED");
		break;
	case TRANS_DECLINED:
		// Declined message
		strcpy(szMessage, "EMV DECLINED");
		break;
	case OFFLINE_APPROVED:
		// Offline approved
		strcpy(szMessage, "OFFLINE APPROVED");
		break;
	case OFFLINE_DECLINED:
		// Offline declined
		strcpy(szMessage, "OFFLINE DECLINED");
		break;
	case OFFLINE_NOTALLOWED:
		// Offline not allowed
		strcpy(szMessage, "NOT ACCEPTED");
		break;
		//    case NONEMV:
		//    	 // Non EMV message
		//strcpy(szMessage, "NON EMV");
		//        break;
	case FAILURE:
		// Failure message
		strcpy(szMessage, "FAILURE");
		break;
	case EMV_CHIP_ERROR:
		// EMV Chip error message
		strcpy(szMessage, "CHIP ERROR");
		break;
	case CARD_REMOVED:
		// EMV Card removed
		strcpy(szMessage, "CARD REMOVED");
		break;
	case CARD_BLOCKED:
		// Card is blocked
		strcpy(szMessage, "CARD BLOCKED");
		break;
	case APPL_BLOCKED:
		// EMV App is blocked
		strcpy(szMessage, "APPLICATION BLOCKED");
		break;
	case CANDIDATELIST_EMPTY:
		// Candidate list is empty
		strcpy(szMessage, "Use Not Satisfied");
		break;
	case CHIP_ERROR:
		// EMV chip error
		strcpy(szMessage, "CHIP ERROR");
		break;
	case BAD_DATA_FORMAT:
		// Bad data format
		strcpy(szMessage, "BAD DATA FORMAT");
		break;
		//case APPL_NOT_AVAILABLE:
		// App not available
		//strcpy(szMessage, "APPL NOT AVAILABLE");
		//break;
	case TRANS_CANCELLED:
		// Transaction cancelled
		strcpy(szMessage, "TRANSACTION CANCELLED");
		break;
	case EASY_ENTRY_APPL:
		// Easy entry app
		strcpy(szMessage, "EASY ENTRY APPL");
		break;
	case ICC_DATA_MISSING:
		// Chip data missing
		strcpy(szMessage, "ICC DATA MISSING");
		break;
	case CONFIG_FILE_NOT_FOUND:
		// Config file missing
		strcpy(szMessage, "CFG FILE NOT PRESENT");
		break;
	case FAILED_TO_CONNECT:
		// Failed to connect
		strcpy(szMessage, "FAILED TO CONNECT");
		break;
	case SELECT_FAILED:
		// Select failed
		strcpy(szMessage, "SELECTION FAILED");
		break;
	case USE_CHIP:
		// Use chip reader
		strcpy(szMessage, "USE CHIP READER!");
		break;
	case REMOVE_CARD:
		// Remove card
		strcpy(szMessage, "REMOVE CARD");
		break;
	case BAD_ICC_RESPONSE:
		// Bad chip response
		strcpy(szMessage, "BAD ICC RESPONSE");
		break;
	case EMV_FAILURE:
		// EMV Failure
		strcpy(szMessage, "EMV FAILURE");
		break;
	case USE_MAG_CARD:
		// Use swipe reader
		strcpy(szMessage, "USE MAG STRIPE");
		break;
	case TAG_NOTFOUND:
		// Tag not found
		strcpy(szMessage, "TAG NOT FOUND");
		break;
	case INVALID_PARAMETER:
		// Invalid parameter
		strcpy(szMessage, "INVALID PARAMETER");
		break;
	case TAG_NOT_SUPPORTED:
		// Tag not supported
		strcpy(szMessage, "TAG NOT SUPPORTED");
		break;
	case TAG_ALREADY_PRESENT:
		// EMV tag already supported` 
		strcpy(szMessage, "TAG ALREADY PRESENT");
		break;
	case INVALID_CAPK_FILE:
		// Invalid CAPK file
		strcpy(szMessage, "INVALID CAPK FILE");
		break ;
	case INVALID_ATR:
		// Invalid ATR
		strcpy(szMessage, "INVALID ATR");
		break ;

	case 0x6985://0x6985 is error code for use case not allowed or appl not available
		strcpy(szMessage, "EMV APP NOT AVAIL");
		break;

	case E_INVALID_LENGTH:

	case 0x6400://State of non-volatile memory unchanged.
	case 0x6300://State of non-volatile memory changed. No information given.
	case 0x63C0://EMV FAILURE
	case 0x63C1://EMV FAILURE
	case 0x63C2://EMV FAILURE
	case 0x63C3://EMV FAILURE
	case 0x6A83://Wrong parameter(s) P1-P2. Record not found.
	case 0x6A88://Wrong parameter(s) P1-P2. Referenced data not found.
	case 0x6700://Process aborted due to a wrong length. No further action.
	case 0x6E00://Class not supported.
	case 0x6D00://Process aborted due to an invalid or not supported instructioncode. No further action.
		// EMV Failure
		strcpy(szMessage, "FAILURE");
		break;
	default:
		// Transaction declined
		strcpy(szMessage, "EMV DECLINED");
		break;
	}

	EMVDisplayFunc(szMessage);
	SVC_WAIT(1000);
	LOG_PRINTF(("Exit vdPromptManager Msg= %s",szMessage));
}
/*--------------------------------------------------------------------------
    Function      :		EMVDisplayFunc
    Description   :	Display the input string on the screen
    Parameters    :	char*	languages buffer pointer
    Returns       :		void
	 
--------------------------------------------------------------------------*/
void EMVDisplayFunc(char *strRespMsg)
{
	//display complany name on top
	// need to check   displayCompanyName();
  if(strcmp(strRespMsg,"")!=0)
  {
    window(1,1,30,20);
	  clrscr();
	  write_at(strRespMsg,strlen(strRespMsg),1,8);
    SVC_WAIT(2000);
  }
}
/*--------------------------------------------------------------------------
    Function      :		inGetPTermID
    Description   :	  Fetch the terminal id into input buffer 
    Parameters    :	  char*	buffer pointer
    Returns       :		int
	 
--------------------------------------------------------------------------*/
int inGetPTermID(char *ptid)
{
	char    tempPTID [EMV_TERM_ID_SIZE + 2] = {0};

	SVC_INFO_PTID (tempPTID);
	strncpy (ptid, tempPTID+1, EMV_TERM_ID_SIZE);

	ptid[EMV_TERM_ID_SIZE+1] = '\0';

	return (EMV_SUCCESS);
}

EMVResult usEMVIssAcqCVM(unsigned short issacq, unsigned short *code)
{
	return (E_CVM_NOT_PERFORMED);
}
/*--------------------------------------------------------------------------
    Function      :		usEMVPerformSignature
    Description   :	  Called when the Verification method is signature
    Parameters    :	  void
    Returns       :		unsigned short
	 
--------------------------------------------------------------------------*/
unsigned short usEMVPerformSignature(void)
{
	fSigRequired  = 1;
	fPerformOfflinePIN = 0;
	fPerformOnlinePIN = 0;
  LOG_PRINTF(("usEMVPerformSignature"));
  strcpy(emvdata.VerificationMethod,"Sig:");//Added to print the verification on the reciept  19-08-2016
	return (EMV_SUCCESS);
}

EMVResult getLastTxnAmt(Ulong *amt)
{
	*amt = 0L;
	return (EMV_SUCCESS);
}
/*--------------------------------------------------------------------------
    Function      :		usEMVDisplayErrorPrompt
    Description   :	  Display appropriate message on the screen according to 
                      the error id
    Parameters    :	  unsigned short
    Returns       :		void 
	 
--------------------------------------------------------------------------*/
void usEMVDisplayErrorPrompt(unsigned short errorID)
{
	char szMessage[MESSAGE_SIZE+1];
	memset(szMessage, 0x00, sizeof(szMessage));

	LOG_PRINTF(("errorID =%02x",errorID));

	//LOG_PRINTF(("usEMVDisplayErrorPrompt errorID=%d", errorID));
	switch (errorID)
	{
	case E_CAPK_FILE_NOT_FOUND:
		// Bug 21719 - Message change
		//strcpy(szMessage, "CAPK FILE NOT FOUND");
		break;
	case E_INVALID_CAPK:
		strcpy(szMessage, "INVALID CAPK");
		break;
	case E_CAPK_FILE_EXPIRED:
		strcpy(szMessage, "CAPK EXPIRED");
		break;
	case E_NO_CAPK_DATA:
		strcpy(szMessage, "CAPK DATA MISSING!");
		break;	
	case E_ICC_BLOCKED:
		strcpy(szMessage, "ICC BLOCKED");
		break;
	case E_PIN_REQD:
		strcpy(szMessage, "PIN REQUIRED");
		break;
	case E_LAST_PIN_TRY:
		strcpy(szMessage, "LAST PIN TRY");
		break;
	case E_PIN_TRY_LT_EXCEED:
		strcpy(szMessage, "PIN TRY LT EXCEED");
		break;
	case E_USR_ABORT:
	case E_USR_PIN_CANCELLED:
		strcpy(szMessage, "PIN CANCELLED");
		break;
	case E_USR_PIN_BYPASSED:
		strcpy(szMessage, "PIN Bypassed");
		break;
	case E_PIN_BLOCKED:
		strcpy(szMessage, "PIN is Blocked");
		break;
	case E_APP_EXPIRED:
		strcpy(szMessage, "APP EXPIRED");
		break;
	case EMV_PIN_SESSION_IN_PROGRESS:
		// Bug 21719 - Message change
		//strcpy(szMessage, "EMV PIN SESSION");
		break;
	case EMV_PIN_SESSION_COMPLETE:
		// Bug 21719 - Message change
		//strcpy(szMessage, "PIN SESSION COMPLETE");
		break;
	case E_INVALID_PIN:
		strcpy(szMessage, "WRONG PIN");
		break;
	default:
		strcpy(szMessage, "EMV FAILURE");
		break;
	}

	//If Offline PIN cancelled do not display any message as Transaction is to be aborted and card to be removed
	if(fOfflinePINCancelled)
		return;
  if(errorID == EMV_PIN_SESSION_IN_PROGRESS)
  {
    EMVDisplayFunc("PIN Processing..");
    SVC_WAIT(1000);
  }

	//LOG_PRINTF(("usEMVDisplayErrorPrompt szMessage=%s", szMessage));
	if((errorID != EMV_PIN_SESSION_IN_PROGRESS) || (errorID != EMV_PIN_SESSION_COMPLETE))	
	{
		EMVDisplayFunc(szMessage);
		SVC_WAIT(1000);
		LOG_PRINTF(("Exit vdPromptManager Msg= %s",szMessage));	
	}
	
}
/*--------------------------------------------------------------------------
    Function      :		getUsrPin
    Description   :	  Callback function invoked when the Verification method 
                      is offline PIN
    Parameters    :	  unsigned char * 
    Returns       :		short
	 
--------------------------------------------------------------------------*/
short getUsrPin(unsigned char *pin)
{

	LOG_PRINTF(("%s","****getUsrPin"));
	LOG_PRINTF(("*fOnlinePINCancelled[%d]",fOnlinePINCancelled));
	
	if(fOnlinePINCancelled)
	{
		LOG_PRINTF(("%s","**getUsrPin**E_USR_PIN_CANCELLED"));
		fOfflinePINCancelled = 1;
		return E_USR_PIN_CANCELLED;
	}
  PinFlag = 1 ;//Added to print the verification on the reciept  19-08-2016
  strcpy(emvdata.VerificationMethod,"PIN OFFLINE");//Added to print the verification on the reciept  19-08-2016
	LOG_PRINTF(("%s","**getUsrPin**E_PERFORM_SECURE_PIN"));

	return(E_PERFORM_SECURE_PIN);  
}


unsigned short usGetExtAuthDecision(unsigned short usStatusWord)
{
	if (usStatusWord == 0x6985) //0x6985 is error code for use case not allowed or appl not available
	{
		vdPromptManager(usStatusWord);
		return EMV_FAILURE;
	}
	return EMV_SUCCESS;
}
/*--------------------------------------------------------------------------
    Function      :		usEMVPerformOnlinePIN
    Description   :	  Callback function invoked when the Verification method 
                      is online PIN
    Parameters    :	  void
    Returns       :		unsigned short
	 
--------------------------------------------------------------------------*/
unsigned short usEMVPerformOnlinePIN(void)
{
	int iRetVal;
	//unsigned short usResult;
	//char strPINPad[3] = {0};
 
	LOG_PRINTF(("%s","Inside usEMVPerformOnlinePIN"));
	//To abort the transaction when the cancel key is pressed during PIN entry
	if(fOfflinePINCancelled)
	{
		LOG_PRINTF(("%s","E_USR_PIN_CANCELLED returned"));
		fOnlinePINCancelled = 1;
		return E_USR_PIN_CANCELLED;
	}

	fSigRequired  = 0;
	fPerformOnlinePIN = 1;
  
  //****Integarating Online Pin verification module ********/////
  LOG_PRINTF(("Global_AccNo = %s",Global_AccNo));
  iRetVal = PinPrompt(Global_AccNo);//
  if(iRetVal != _SUCCESS)
  {
    return ((unsigned short)_FAIL);
  }
  
	
  PinFlag = 1 ;//Added to print the verification on the reciept  19-08-2016
  strcpy(emvdata.VerificationMethod,"PIN ONLINE");//Added to print the verification on the reciept  19-08-2016

	LOG_PRINTF(("%s","before emv GetOnlinePin"));
	return (EMV_SUCCESS);
}
/*--------------------------------------------------------------------------
    Function      :		dispStrAtRight
    Description   :	  The function displays the message on the right side of 
                      the screen
    Parameters    :	  char pointer to the message to be displayed,int y 
                      co-ordinates and int flag for Clear display option
    Returns       :		void
	 
--------------------------------------------------------------------------*/
void dispStrAtRight(char* pchInStr,int iYCord,int iClearDp) 
{
	int iXCord=0,
		x1=0,
		x2=0,
		y1=0,
		y2=0;

	// Get current window position
	wherewin(&x1, &y1, &x2, &y2);

	// Calculate the Col index based on message width
	iXCord = x2 - x1 - strlen(pchInStr) + 2;

	// Display the justified message
	display_at(iXCord, iYCord, pchInStr, iClearDp);
	return;
}

void sortApplnsListOnPriority(char **labels, int inNumOfLabels, EMV_NEW_LABEL srNewMenuArray[])
{
	int i = 0, j = 0;

	EMV_NEW_LABEL srTempLabelStruct={0};

	LOG_PRINTF(("In start sortApplnsListOnPriority"));

	for(i=0; i < inNumOfLabels; i++)
	{
		strncpy(srNewMenuArray[i].szLabel, labels[i], MAX_LABEL_LEN);
		srNewMenuArray[i].inOrgLabelPos = i;
		/*
		This will work based on the assumption that the candidate list and the label array 
		passed to the application match one to one
		*/
	//need to ckecka 	srNewMenuArray[i].inAppPriority = candidateList.arrAIDList[i].appPriority & 0x0F; 
	}

	for(i=0; i < inNumOfLabels-1; i++)
	{
		for(j=0; j < inNumOfLabels-1-i;j++)
		{
			if(srNewMenuArray[j].inAppPriority > srNewMenuArray[j+1].inAppPriority) 
			{
				memcpy((void*)&srTempLabelStruct, (void*)&srNewMenuArray[j+1], sizeof(EMV_NEW_LABEL));
				memcpy((void*)&srNewMenuArray[j+1], (void*)&srNewMenuArray[j], sizeof(EMV_NEW_LABEL));
				memcpy((void*)&srNewMenuArray[j], (void*)&srTempLabelStruct, sizeof(EMV_NEW_LABEL));
			}
		}
	}

	LOG_PRINTF(("In End sortApplnsListOnPriority"));
	return;
}

/*--------------------------------------------------------------------------
    Function      :		inMenuFunc
    Description   :	  The function displays multiple application available in 
                      the card and provide an option to select the application
                      using touch event 
    Parameters    :	  char double pointer to the applications name to be displayed,
                      int total no of applications 
    Returns       :		void	 
--------------------------------------------------------------------------*/
int inMenuFunc(char **labels, int numLabels)
{
     short  i = 0;
     short  j = 0, k = 0;
     char   nextKey = 0;
     char   prevKey = 0;
     char   menuKeys[4] = {0};
     char   key;
     short shDispModFlag=0;
     short ucAllowKeys =0;//for  touch eanble
     // short ucAllowKeys = 1;//for key

   
     LOG_PRINTF(("numLabels =%d",numLabels));
     
   
     
     while (1)
     {
          clrscr();
          memset(menuKeys, 0x00, 4);
          nextKey = 0;
          prevKey = 0;

          j = 0;
          while (j < 4)
          {
              displayAt(7,2,"SELECT APPLICATION",  NO_CLEAR);
              if ((i + j) >= numLabels)
                   break;
              else
              {
                   for (k=0; (labels[i + j][k] == 0x00);(labels[i + j][k] = 0x20),k++);
                  
                   if(ucAllowKeys == 1)
                        menuKeys[j] = 49;
                   else
                        menuKeys[j] = 1;
                   if (shDispModFlag)
                   {
                        
                        LOG_PRINTF(("App selection"));
                        dispStrAtRight(labels[i + j], ((j * 1) + 1), CLR_EOL);
                   }
                   else
                   {
                        EnblMultiAppTouchScreen(i+j+1);

                        displayAt(((30-strlen(labels[i + j]))/2),  (((j+1) * 3) + 3), labels[i + j], CLR_EOL);
                        LOG_PRINTF(("labels[%d]  = %s",i+j,labels[i + j]));
                   }
              }
              j++;
          }

          // No. of labels on screen is now j
          if ((i + j) < numLabels)
              nextKey = 1;

          if (i)
              prevKey = 1;

          if (nextKey)         
          {
              //Support For multiple Application.
              
              if( ucAllowKeys  == 1 )
                   displayAt(25,19,"NEXT", NO_CLEAR);
              else if (shDispModFlag)
                   displayAt(25,19,"NEXT", NO_CLEAR);
              else
                   displayAt(25,19,"NEXT",  NO_CLEAR);
             
             
              LOG_PRINTF(("next key"));
              EnblMultiAppTouchScreen(5);
              
          }

          if (prevKey)
          {
              if (shDispModFlag)
                   displayAt(1, 19, "PREV", NO_CLEAR);
              else
                   displayAt(1, 19, "PREV", NO_CLEAR);

              LOG_PRINTF(("prev key"));
              EnblMultiAppTouchScreen(i+j+1);
              
          }

          key = get_char();
          LOG_PRINTF(("key  == %d",key));
          
          //Support For multiple Application.

          if( ucAllowKeys  == 1 )
          {
              LOG_PRINTF(("ucAllowKeys  == 1"));
              //Kishor_S1 added on 5-1-10 for "#" And "*" keys implementation    
              if((key == 109) || (key == 35)) //UP
                   key=99;
              if((key == 108) || (key == 42)) //DOWN
                   key = 100;
          }
          else if( ucAllowKeys  == 0 )
          {
             if((key == 6)) //UP
                   key=99;
              if((key == 5)) //DOWN
                   key = 100;
          }
          if (key == 100)
          {   
              LOG_PRINTF(("key == 100"));
              // Next
              if (nextKey)
                   i += 4;
              else
                   error_tone();
          }
          else if (key == 99)
          {    
              LOG_PRINTF(("key == 99"));
              // Previous
                if (prevKey)
                    i -= 4;
                else
                    error_tone();
          }
          else if( ucAllowKeys  == 1 )          // for ONTIME id 22518 T_gangadhar_s1  24/April/08  support to work all function keys for all Vx terminals
          {
              if (key >= 49 && key <= 52) 
              {   
                  LOG_PRINTF(("key >= 49 && key <= 52"));
                  // Menu selection
                    if (menuKeys[key - 49])
                        return(i + (key - 49) + 1);
                    else
                        error_tone();
              }
              else
                    if (key == 27)
                        return (TRANS_CANCELLED);
                    else
                        error_tone();
          }
          else if( ucAllowKeys == 0)
          {
              if (key >= 1 && key <= 4) 
              {    
                  LOG_PRINTF(("key >= 1 && key <= 4"));
                    // Menu selection
                    if (menuKeys[key - 1])
                        return(i + (key - 1) + 1);
                    else
                        error_tone();
              }
              else
                    if (key == 27)
                        return (TRANS_CANCELLED);
                    else
                        error_tone();
          }
     }
}

/*--------------------------------------------------------------------------
    Function      :		emvRemoveCard
    Description   :	  The function remove terminate the card slot and perform 
                      and cold and warm reset and display the appropriate message
    Parameters    :	  void
    Returns       :		int
--------------------------------------------------------------------------*/
int emvRemoveCard(void)
{
	int inResult=0, inRes =-1;
  if (!inVXEMVAPCardPresent())
	{
		inResult = Terminate_CardSlot(CUSTOMER_CARD, SWITCH_OFF_CARD);
    if (inResult == CARDSLOT_SUCCESS)
		{
			inResult = Reset_CardSlot (CUSTOMER_CARD, RESET_COLD);		
			if (inResult != CARDSLOT_SUCCESS)				
				inResult = Reset_CardSlot (CUSTOMER_CARD, RESET_WARM);					
		}
		//clrscr();
		LOG_PRINTF((" bef inVXEMVAPRemoveCard "));
		inRes =inVXEMVAPRemoveCard(vdPromptRemoveManager);
		LOG_PRINTF((" aftr inVXEMVAPRemoveCard RES %d",inRes));
		SVC_WAIT(200);

		vdEMVSetICCReadFailure(0);
	}
  else
  {
    LOG_PRINTF((" Card is present "));
  }
	return 0;
}

/*--------------------------------------------------------------------------
Function:		hex2asc
Description:	
Parameters:		BYTE *, BYTE *, int
Returns:		
Notes:          
--------------------------------------------------------------------------*/
void hex2asc(unsigned char *outp, unsigned char *inp, int length)
{
	short i = 0;

	for(i=0;i<length;i++)
	{
		if((inp[i] >> 4) > 9)
			outp[i*2] = (unsigned char) ((inp[i]>>4) + 'A' - 10);
		else
			outp[i*2] = (unsigned char) ((inp[i]>>4) + '0');

		if((inp[i] & 0x0f) > 9)
			outp[i*2+1] = (unsigned char) ((inp[i] & 0x0f) + 'A' - 10);
		else
			outp[i*2+1] = (unsigned char) ((inp[i] & 0x0f) + '0');
	}
	outp[i*2] = 0;

	return;
}
/*--------------------------------------------------------------------------
    Function      :		vdPromptRemoveManager
    Description   :	  display the message card removed
    Parameters    :	  unsigned short
    Returns       :		void
--------------------------------------------------------------------------*/
void vdPromptRemoveManager(unsigned short test)
{
	if (!inVXEMVAPCardPresent())
	{
		vdPromptManager(CARD_REMOVED);
	}
}
/*--------------------------------------------------------------------------
    Function      :		vdSetPinParams
    Description   :	  sets the pin parameters of offline pin like min-max 
                      digits ,display character ,default character and timeouts
    Parameters    :	  void
    Returns       :		void
--------------------------------------------------------------------------*/
void vdSetPinParams(void) //SECURE_PIN_MODULE changes
{
	//short	shRetVal				=	0;

    srPINParams psParams;
    
    psParams.ucMin = 4;
    psParams.ucMax = 12;
    psParams.ucEchoChar = '*';
    psParams.ucDefChar = ' ';
    psParams.scDspLine = 6;  
    psParams.scDspCol = -1;
    psParams.ucOption = 0x0A; 


  psParams.ulFirstKeyTimeOut = 10000;

	psParams.ulInterCharTimeOut = 5000;

	psParams.ulWaitTime = 120000;
  
  psParams.abortOnPINEntryTimeOut = 1;

	// To support PIN BYPASS
/*  psParams.ucPINBypassKey = ENTER_KEY_PINBYPASS;
	
	psParams.ucSubPINBypass = EMV_TRUE;	*/ 

	// To disable PIN Bypass
	psParams.ucPINBypassKey = 0x00;
	
	psParams.ucSubPINBypass = EMV_FALSE ;	 

  //Use this API to set the PIN parameters.
#ifdef __thumb
   usEMVSetPinParams(&psParams);
#endif

}

/*--------------------------------------------------------------------------
    Function      :		displayPINPrompt
    Description   :	  display the Enter pin prompt for Offline pin
    Parameters    :	  void
    Returns       :		void
--------------------------------------------------------------------------*/
void displayPINPrompt(void)
{	

	unsigned short usLen;
	
  window(1,1,30,20);
  clrscr();
	
	LOG_PRINTF(("displayPINPrompt****"));
	clrscr();
	if(!inPinTried) // This is the first Pin try
		usEMVGetTLVFromColxn(TAG_9F17_PIN_TRY_COUNTER, &uchGblPinTryCtr, &usLen);

	LOG_PRINTF(("uchGblPinTryCtr =%02x",uchGblPinTryCtr));

	if (inPinTried)
	{
		LOG_PRINTF(("WRONG PIN!"));
		display_at(1,4,"WRONG PIN!", CLR_LINE);
		//diplay message "WRONG PIN!"
	}

	if (uchGblPinTryCtr == 1)
	{
		LOG_PRINTF(("LAST PIN TRY!"));
		display_at(1,4,"LAST PIN TRY!", CLR_LINE);
		//diplay message "LAST PIN TRY!"
	}
  LOG_PRINTF(("uchGblPinTryCtr =%02x",uchGblPinTryCtr));
	display_at(1,4,"ENTER PIN:",CLR_LINE);
	//window(15,4,21,4);

}

/******************************************************************************
* fEMVIsItUnrecognizedCVM
*  
* @return true/false
*******************************************************************************/
short fEMVIsItUnrecognizedCVM(byte bCVM)
{
	if(((bCVM & 0x3F) == 0x3F) || ((bCVM & 0xF0) == 0x60) || ((bCVM & 0x2F) == 0x2F)) 
		return 1;
	else
		return 0;    
}

/*--------------------------------------------------------------------------
    Function      :		ConvertAmount
    Description   :	  converts the decimat amount into unsigned long amount
    Parameters    :	  input char pointer to the decimal amount and the pointer 
                      to the unsigned long output amount
    Returns       :		int
--------------------------------------------------------------------------*/
int ConvertAmount(char *EnteredAmount ,unsigned long * ulAmount)
{
	char * pch = NULL;
	unsigned long Amount = 0;
	unsigned long Amountdecimal  = 0;

	pch = strtok(EnteredAmount, ".,");
	if (NULL == pch)
		return -1;

	Amount = atoi(pch);

	pch = strtok(NULL,"\\");
	if (NULL == pch)
		return -1;

	Amountdecimal = atoi(pch);

	*ulAmount = Amount*100 + Amountdecimal;
	LOG_PRINTF(("*ulAmount [%u]",*ulAmount));

	return 0;
}
/*--------------------------------------------------------------------------
    Function      :		PropmptForFallBack
    Description   :	  propmts for fallback if there any ICC read failure 
    Parameters    :	  void
    Returns       :		void
--------------------------------------------------------------------------*/
void PropmptForFallBack(void)
{
	int inResult = 0;
  LOG_PRINTF(("PropmptForFallBack"));
	if(inVXEMVAPCardPresent() != SUCCESS)
  {
    LOG_PRINTF(("inVXEMVAPCardPresent() != SUCCESS"));
		return;
  }

	if(inEMVGetICCReadFailure() == 1)
	{
		//display message--"FALLBACK :USE SWIPE",	
    LOG_PRINTF(("inEMVGetICCReadFailure() == 1"));
		SVC_WAIT(2000);
		inResult = Terminate_CardSlot(CUSTOMER_CARD, SWITCH_OFF_CARD);
		if (inResult == CARDSLOT_SUCCESS)
		{
			inResult = Reset_CardSlot (CUSTOMER_CARD, RESET_COLD);		
			if (inResult != CARDSLOT_SUCCESS)				
				inResult = Reset_CardSlot (CUSTOMER_CARD, RESET_WARM);					
		}
    LOG_PRINTF(("inResult =  %d",inResult));
		vdEMVSetICCReadFailure(0);
	}
  else
  {
    LOG_PRINTF(("inEMVGetICCReadFailure() == 0"));
  }
}
/*--------------------------------------------------------------------------
    Function      :		iCorrect_track2_buffer
    Description   :	  corrects the track2 data
    Parameters    :	  char pointer to the track2 buffer and length of track2 buffer
    Returns       :		int
--------------------------------------------------------------------------*/

static int iCorrect_track2_buffer(char * track2,int track2Length)
{
	int i=0;
	int j = 0;
	char tmptrack2[100]= {0};//I
	char strTrack2[50] = {0};
	int iOffset = 0;

	int inputTrack2len = strlen(track2);
	memset(tmptrack2,0x00,100);
	while (i < inputTrack2len) {
		if (('B' == track2[i]) || ('b' == track2[i])) {
			track2[i++] = ';';
			tmptrack2[j++] = ';';

		}
		else if (('D' == track2[i]) || ('d' == track2[i])) {
			track2[i++] = '=';
			tmptrack2[j++] = '=';

		}
		else if (('F' == track2[i])||('f' == track2[i])) {
			track2[i++] = '?';
			tmptrack2[j++] = '?';
		}
		else{
			tmptrack2[j] = track2[i];
			i++;
			j++;
		}
	}


	if(tmptrack2[j-1] != '?')
	{
		LOG_PRINTF(("Appendin ?"));
		strcat(tmptrack2,"?");
		j++;
	}
	LOG_PRINTF(("tmptrack2 = %s",tmptrack2));

	strcpy(strTrack2, tmptrack2);
//	strcpy(trackCardData->acountNo, strtok(strTrack2, "="));
	LOG_PRINTF(("strTrack2 = %s", strTrack2));
	//LOG_PRINTF(("strTrack2 acountNo = %s", trackCardData->acountNo));

	LOG_PRINTF(("j = %d",j));

	memset(track2,0x00,128);
	if(tmptrack2[0] != ';')
	{
		track2[0] = ';';
		iOffset++;
		j++;
	}
	track2Length = j;
	memcpy(track2+iOffset,tmptrack2,track2Length);

	LOG_PRINTF(("track2 = %s",track2));
	LOG_PRINTF(("track2Length = %d",track2Length));

	return track2Length;
}


/*--------------------------------------------------------------------------
    Function      :		emvGetTagLenValue
    Description   :	  This function Build Tags to send to Host
    Parameters    :	  byte pointer,tag value ,pointer to the EMV data structure
    Returns       :		int
--------------------------------------------------------------------------*/

static int emvGetTagLenValue (const unsigned long tag, byte * outVal, sEMV_DATA * emv_data)
{
	int locOffset = 0;
  byte tempBuffer[255]= {0};
	unsigned short len = 0;
	char tmpData[50]= {0}; 
	char Buff[50]= {0};

	memset(tempBuffer, 0x00, sizeof(tempBuffer));
	memset(Buff, 0x00, sizeof(Buff));

	usEMVGetTLVFromColxn((Ushort)tag, tempBuffer, &len);
  
	LOG_PRINTF(("tag:0x%x",tag));
	LOG_PRINTF(("len:%d",len));
	//char *dest ,char *src ,unsigned short bcdlen
	bcd2a(Buff,(char*)tempBuffer,len);
	LOG_PRINTF(("value:[%s]",Buff));
	//LOG_HEX_PRINTF("tag val",(unsigned char*)tempBuffer,len);

	//amitesh :: in case TLV having zero length
	if(len <= 0)
		return locOffset;

	// Tag 4f - Application Identifier (AID)-ICC
	if (TAG_4F_AID == tag )
	{
		memset(tmpData,0x00,sizeof(tmpData)); 	
		len = (len < 25)?len:25;
		memcpy(tmpData,tempBuffer,len);


		//bcd2a
		bcd2a(( char *)emv_data->ApplicationId,( char *)tmpData,len);

		LOG_PRINTF(("ApplicationId:%s",emv_data->ApplicationId));
	}

	// Tag 57 - Track 2 Equivalent Data
	if (TAG_57_TRACK2_EQ_DATA == tag )
	{
		memset(tmpData,0x00,50);
		memcpy(tmpData,tempBuffer,len);
		bcd2a((char *)emv_data->Track2,(char *)tmpData, len);	
		LOG_PRINTF(("Before Track2:%s",emv_data->Track2));
		iCorrect_track2_buffer(emv_data->Track2,strlen(emv_data->Track2));
    
		LOG_PRINTF(("After  Track2:%s",emv_data->Track2));
		emv_data->Track2Len = strlen(emv_data->Track2);
	}

	//Added Cardholder name
	if (TAG_5F20_CARDHOLDER_NAME == tag )
	{
		memcpy(emv_data->m_chArrCardHolderName,tempBuffer,len);
		LOG_PRINTF(("CardHolder name:%s",emv_data->m_chArrCardHolderName));
	}

	if ((tag & 0x00ff) == 0)
	{
		LOG_PRINTF(("%s","1 BYTE ALIGNEMNT"));
		outVal[locOffset++] = (tag >> 8)& 0xff;
	} 
  else 
  {
		LOG_PRINTF(("%s","2 BYTE ALIGNEMNT"));
		outVal[locOffset++] = (tag >> 8)& 0xff;
		outVal[locOffset++] = (tag & 0xff);
	}
  LOG_PRINTF(("%s","After ALIGNEMNT"));
	
  // add length
	outVal[locOffset++] = len & 0xff;
 
  // copy data
	memcpy((byte *) &outVal[locOffset], (byte *) tempBuffer, len);
  locOffset += len;	

	return locOffset;
}

/*----------------------------------------------------------------------------------
    Function      :		BuildDE55
    Description   :	  This function get fetch the informaion according defined set
    Parameters    :	  pointer to the EMV data structure
    Returns       :		int
------------------------------------------------------------------------------------*/
int BuildDE55( sEMV_DATA * emv_data)
{
  static const unsigned long RequiredTags[] = {
		          TAG_5F2A_TRANS_CURCY_CODE,
              TAG_ISUER_SCRPT_TEMPL_71,
              TAG_ISUER_SCRPT_TEMPL_72,
              TAG_8200_APPL_INTCHG_PROFILE,
              TAG_84_DF_NAME,
              TAG_91_ISS_AUTH_DATA,
              TAG_9500_TVR,
              TAG_9A_TRAN_DATE,
              TAG_9C00_TRAN_TYPE,
              TAG_9F02_AMT_AUTH_NUM,
              TAG_9F03_AMT_OTHER_NUM,
              TAG_9F09_TERM_VER_NUM,
              TAG_9F10_ISSUER_APP_DATA,
              TAG_9F1A_TERM_COUNTY_CODE,
              TAG_9F1E_IFD_SER_NUM,
              TAG_9F26_APPL_CRYPTOGRAM,
              TAG_9F27_CRYPT_INFO_DATA,
              TAG_9F33_TERM_CAP,
              TAG_9F34_CVM_RESULTS,
              TAG_9F35_TERM_TYPE,
              TAG_9F36_ATC,
              TAG_9F37_UNPRED_NUM,
              TAG_9F41_TRANS_SEQ_COUNTER,
              TAG_9F53_TERM_CAT_CODE,
              TAG_5F34_APPL_PAN_SEQNUM,
          
	};

	int ioffSet = 0;
	byte szEMVDE55Buff[MAX_DE55_SIZE]={0};//I
 	int count = 0;//I

	LOG_PRINTF(("BuildDE55"));
	memset(szEMVDE55Buff, 0x00, MAX_DE55_SIZE);

	
	for(count = 0; count < sizeof(RequiredTags) / sizeof(RequiredTags[0]); count++)
	{
		ioffSet += emvGetTagLenValue (RequiredTags[count], &szEMVDE55Buff[ioffSet], emv_data);
	     
  }
 
  LOG_PRINTF(("Going to convert hex to ascci"));
	if (0)
	{
		memcpy(emv_data->DE55,(char *)szEMVDE55Buff,ioffSet);
		emv_data->De55len = ioffSet;
		LOG_PRINTF(("A M E X EMV DE55 in Hex"));
	} 
  else 
  {
		bcd2a((char *)emv_data->DE55,(char *)szEMVDE55Buff,ioffSet);
		emv_data->De55len = ioffSet*2;//strlen(emv_data->DE55);
		LOG_PRINTF(("EMV DE55 in ascii offset =%d",ioffSet));
  }
	LOG_PRINTF(("DE55-::%s",emv_data->DE55));

	return ioffSet;
}


/******************************************************************************
* getNextRawTLVData
*******************************************************************************/
short getNextRawTLVData(unsigned short *tag, byte *data, const byte *buffer)
{
	short bytesRead = 0;
	byte *ptr = NULL;
	byte tagByte1 = '0';
	byte tagByte2 = '0';
	// byte dataByte = '0';
	short numTagBytes = 0;
	short dataLength = 0;
	short numLengthBytes = 0;
	short i = 0;
	ptr = (byte *)buffer;
	tagByte1 = *ptr;
	// EMV specification says, any tag with == 0x1F (31) must be treated as two byte tags.
	if ((tagByte1 & 31) == 31) // Bit pattern must be 10011111 or greater
	{
		ptr++;
		LOG_PRINTF(("tagByte1 & 31  2 bytes"));

		tagByte2 = *ptr;
		*tag = (short) ((tagByte1 << 8) + tagByte2);
		numTagBytes = 2;
	}
	else
	{
		LOG_PRINTF(("tagByte1 & 31  1 bytes"));

		*tag = (short) tagByte1;
		numTagBytes = 1;
	}

	ptr++;
	numLengthBytes = 1;
	dataLength = *ptr;

	bytesRead = numTagBytes + numLengthBytes + dataLength;
	LOG_PRINTF(("bytesRead [%d] ", bytesRead));

	ptr = (byte *)buffer;
	for (i = 0; i < bytesRead; i++)
	{
		data[i] = *ptr;
		ptr++;
	}
	return (bytesRead);
}





