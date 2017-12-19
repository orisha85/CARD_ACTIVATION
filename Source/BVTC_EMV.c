/******************************************************************
  Filename		     : BVTC_EMV.c
  Project		       : Bevertec 
  Developer Neame  : Amar
  Module           : EMV
  Description	     : This is the implementation of intializing 
                     cards and perform EMV first generation.
******************************************************************/


#include <svc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <logsys.h> 

#include <ctype.h>
#include <svctxo.h>
#include <ascii.h>
#include <aclascii.h>
#include <aclconio.h>
#include <errno.h>
#include <svc_sec.h>

//emv includes
#include <common.h>

#include <cardslot.h>
#include <EMVCWrappers.h>
#include <EMVCStructs.h>
#include <vxemvap.h>
#include <emvlib.h>
#include <estfuncs.h>
#include <mvtfuncs.h>
#include <mvt.h>
#include <est.h>
#include <VXEMVAP_confproto.h>
#include <proto.h>
#include <EMVGenConfig.h>
#include <VXEMVAP_confio.h> //for GEN_VER_SIZE


#include "..\include\EMVCallBacks.h"
#include "..\include\BVTC_EMV.h"
#include "..\include\SaleTransaction.h"
#include "..\include\Bevertec.h"
#include "..\include\Common.h"
#include "..\include\TouchScreen.h"

sEMV_DATA emvdata;		//global emv structure



srAIDList candidateList;
short gshInterac=0;
unsigned char 		ucAllowKeys=0;
char Global_AccNo[ACC_NUM_ARR_LEN+1]={0};
/****emv global variable*****************/
extern short initiateFallback;
extern short	fSigRequired;
extern short	fPerformOfflinePIN;
extern short	fPerformOnlinePIN;
extern short fOnlinePINCancelled;
extern short fOfflinePINCancelled;
extern int inPinTried;
extern unsigned char uchGblPinTryCtr;

//Script processing

char szScript71[256]={0}, szScript72[256] = {0}; 
unsigned short	us71ScriptLen =0, us72ScriptLen =0; 
int	numScripts;;

//unsigned char uchGblPinTryCtr;
extern int iGlobalNumberScripts;
extern unsigned char * pszGlobalIssuerScriptResults;
extern unsigned char *issuerScriptResults;

/*-------------------------------------------------------------------------
Function      :   LoadMVTFunction
Description   :   Load MVT data
Parameters    :   short 
Returns       :		result success or failure

--------------------------------------------------------------------------*/
short LoadMVTFunction()
{
  	
	int iResult = inLoadMVTRec(0);  // Load the MVT REC 1 and read the RFU string 
  if (iResult != VS_SUCCESS) 
  {
    window(1,1,30,20);
	  clrscr();
    LOG_PRINTF(("Error Loading inLoadMVTRec iResult = %d", iResult));	

    write_at("Error Loading",strlen("Error Loading"), 1, 1);
    write_at("MVT record", strlen("MVT record"),1, 2);
    error_tone() ;
    SVC_WAIT((unsigned long)3000);   //VERIXPRTNG: Porting to Verix
    error_tone() ;
  }
	else
		LOG_PRINTF(("inLoadMVTRec iResult = %d", iResult));	

  return iResult ;

}
/*-------------------------------------------------------------------------
Function      :   StartEMV
Description   :   initilizes the smart card reader
Parameters    :   short 
Returns       :		short

--------------------------------------------------------------------------*/
short StartEMV(void)
{
	int iResult = -1;
	LOG_PRINTF(("Welcome to start EMV function"));	

	//this function opends the smart card reader and initialize emv configuration data
	//0=success ,EMV_FAILURE= failure\\\


  vdEMVSetFallbackToMSR(0);
	vSetDefaultFunctionPointers();
  
	iResult = inVXEMVAPSCInit();
	LOG_PRINTF(("inVXEMVAPSCInit iResult = %d", iResult));

	if(iResult != SUCCESS)
	{
		vdPromptManager(iResult); //function to display message on terminal
		return -1;
	}

	//This function is used to open the primary smartcard reader.
	//NOTE:All applications using Vx EMV Module should call inVXEMVAPInitCardslot() API
	//to open the CardSlot, as inVXEMVAPSCInit() API does not open the CardSlot.
	//success=SUCCESS,  = FAILURE

	iResult = inVXEMVAPInitCardslot();
	LOG_PRINTF(("inVXEMVAPInitCardslot iResult = %d",iResult));

	if(iResult != SUCCESS)
		return -1;

	//update capk file according to kernel
	inVXEMVAPKeyRollOver();

	return 0;
}

/*-------------------------------------------------------------------------
Function      :   vSetDefaultFunctionPointers
Description   :   sets the callback functions 
Parameters    :   int -transaction type
Returns       :		int

--------------------------------------------------------------------------*/
void vSetDefaultFunctionPointers(void)
{
  LOG_PRINTF(("Inside vSetDefaultFunctionPointers"));
	inVxEMVAPSetFunctionality(PERFORM_ISS_ACQ_CVM, (void*)&usEMVIssAcqCVM);
	inVxEMVAPSetFunctionality(PERFORM_SIGNATURE, (void *)&usEMVPerformSignature);
	inVxEMVAPSetFunctionality(PERFORM_ONLINE_PIN,(void *)&usEMVPerformOnlinePIN);
	inVxEMVAPSetFunctionality(GET_LAST_TXN_AMT, (void *)&getLastTxnAmt);
	inVxEMVAPSetFunctionality(DISPLAY_ERROR_PROMPT, (void *)&usEMVDisplayErrorPrompt);
	inVxEMVAPSetFunctionality(GET_USER_PIN, (void *)&getUsrPin);

	inVxEMVAPSetFunctionality(IS_PARTIAL_SELECT_ALLOWED, NULL);
	inVxEMVAPSetFunctionality(GET_CAPK_DATA, NULL);
	inVxEMVAPSetFunctionality(GET_TERMINAL_PARAMETER, NULL);

	//ExtAuth6985:
	inVxEMVAPSetFunctionality(Get_EXT_AUTH_DECISION, (void*)&usGetExtAuthDecision);
	//MULTIAVN:
	inVxEMVAPSetFunctionality(GET_TERMINAL_AVN, NULL);

  //for language preferences
  inVxEMVAPSetFunctionality(PERFORM_LANGUAGE_SEL, (void*)&vdSelPreferredLang);
  
}



/*-------------------------------------------------------------------------
Function      :   vdFlushKeyboard
Description   :   flushes the key buffer
Parameters    :   int -transaction type
Returns       :		int

--------------------------------------------------------------------------*/
void vdFlushKeyboard(void)
{
	char key[20] = {0};
  short    handleConsole = -1;   

	read(handleConsole, key, 20);
}

/*-------------------------------------------------------------------------
Function:		inSetTransactionType
Description:	set the transaction type required for pass into the api
Parameters:		int -transaction type
Returns:		int

--------------------------------------------------------------------------*/
int inSetTransactionType(int TxnType)
{
	int type = 0 ;//I

	switch(TxnType)
	{
	case SALEMSGTYPE_CASE:
		type = SALE;
		break;
	case WITHDRAWAL_MSG_TYPE_CASE:
		type = WITHDRAWAL;
		break;
  case BALINQMSGTYPE_CASE:
	  type = BAL_INQUIRY;
	  break;
	case VOIDMSGTYPE_CASE:
		type = VOID;
		break;
	case REFUNDMSGTYPE_CASE:
		type = REFUND;
		break;
	case DEPOSIT_MSG_TYPE_CASE:
		type = DEPOSIT;
		break;
	case TRANSFER_MSG_TYPE_CASE:
		type = TRANSFER;
		break;
	case PIN_VALIDATE_MSG_TYPE_CASE:
		type = PIN_VALIDATE;
		break;
	case PINCHANGEMSGTYPE_CASE:	
		type = PIN_CHANGE;
		break;
	case ACTIVATIONMSGTYPE_CASE:
		type = CARD_ACTIVATE;
		break;
	default:
		type = TRANS_CANCELLED;
		break;
	}
	return (type);
}

/*-------------------------------------------------------------------------
Function:		parseSerialBuf
Description:	To parse the serial number
Parameters:		char pointer to the serial data
Returns:		void

--------------------------------------------------------------------------*/
void parseSerialBuf(char *serialData)
{
	char tempBuff[16] = {0};

	strcpy(tempBuff, strtok(serialData, "-"));
	strcat(tempBuff, strtok(NULL, "-"));
	strcat(tempBuff, strtok(NULL, "-"));

	memset(serialData, '\0', strlen(serialData));
	strcpy(serialData, tempBuff);
}

/*-------------------------------------------------------------------------
Function:		inRunEMVTransaction
Description:	To process an EMV Transaction 
Parameters:		void
Returns:		TRANS_APPROVED, TRANS_DECLINED, OFFLINE_APPROVED, OFFLINE_DECLINED, 
OFFLINE_NOTALLOWED, NONEMV
Notes:
--------------------------------------------------------------------------*/

int inRunEMVTransaction(int TxnType)
{
	unsigned short      result = 0;
	short               terminalRecord = 0;
	byte                transType = 0;
	char                serialBuf[15] = {0};
  char strCompanyName[22]={0};

	inPinTried = 0;					// A flag to differentiate between first pin try and retries
	uchGblPinTryCtr = 0;
	numScripts = 0;
	us71ScriptLen =0;
	us72ScriptLen =0;

	memset(&szScript71,0x00,sizeof(szScript71));
	memset(&szScript72,0x00,sizeof(szScript72));

	window(1,1,21,8);
	
	memset(serialBuf, 0x00 ,sizeof(serialBuf) );
	memset(strCompanyName,0,sizeof(strCompanyName));

	
	if((result = inVXEMVAPTransInit(terminalRecord,inGetPTermID)) != SUCCESS)
	{
		LOG_PRINTF(("inVXEMVAPTransInit inResult =%d", result));
		return (result);
	}

	vdFlushKeyboard();

	while(inVXEMVAPCardPresent() != SUCCESS)
	{
		//please insert card	//card removed error
		LOG_PRINTF(("card not present"));
		return -1;
	}

	transType = inSetTransactionType(TxnType);
	LOG_PRINTF(("transType %d",transType));
	if (transType == TRANS_CANCELLED)
		return (TRANS_CANCELLED);
  window(1,1,30,20);
	clrscr();
	//call display MSG_PLEASE_WAIT or EMV [ROCESSING
	write_at(EMV_PROCESSING,strlen(EMV_PROCESSING),1,8);

	//Indicates the type of financial transaction . RAVI_B3
	usEMVAddTLVToCollxn(0x9c00, (byte *) &transType, 1);

	//Adding device serial number
	memset(serialBuf,0x00,sizeof(serialBuf));
	SVC_INFO_SERLNO(serialBuf);
	parseSerialBuf(serialBuf);			//283-345-325 remove - symbol
	LOG_PRINTF(("serialBuf %s",serialBuf));
	usEMVAddTLVToCollxn(TAG_9F1E_IFD_SER_NUM , (byte *)serialBuf, 8);
	
	if ((result = inVXEMVAPCardInit()) != SUCCESS)
	{
		//if we get chip error or Invalid ATR from library then we should prompt for fallback.
		LOG_PRINTF(("inVXEMVAPCardInit result %d",result));

		//bFallbackAllowed = EMV_TRUE ;

		LOG_PRINTF(("CHIP_ERROR =%d", CHIP_ERROR));
		LOG_PRINTF(("NO_ATR =%d", NO_ATR));
		LOG_PRINTF(("INVALID_ATR =%d", INVALID_ATR));

		if((result == CHIP_ERROR) || (result == NO_ATR)) 
		{   
			// This function initiates the fallback only if the card is still present.
			initiateFallbackTxn();
			LOG_PRINTF(("CHECK 4: validateICCCard "));

			// Check the status for ICC read failure
			
			emvRemoveCard();
			return FALLBACK;
			
		}
		else
		{
			vdPromptManager((unsigned short)result);
			emvRemoveCard();
			//message display "CHIP READ ERROR!"
		}
		return -1;
	}
	LOG_PRINTF(("inVXEMVAPCardInit result %d",result));
	
	return 0;
}


/******************************************************************************
* initiateFallbackTxn
* sets the fallback
* @return void
*******************************************************************************/
void initiateFallbackTxn(void)
{
	initiateFallback = 0;

	//Fallback flag loaded from default MVT record
	if(inGetFallbackAllowedFlag()) 
	{
		if(inVXEMVAPCardPresent() != SUCCESS) //check emv card is present or not in slot
			return;

		// Set chip failure flag
		vdEMVSetICCReadFailure((char)1);

		if(inEMVGetICCReadFailure() == 1)
		{
			LOG_PRINTF(("-- FALLBACK IS SET --"));
			initiateFallback = 1;
		}
		else
		{
			LOG_PRINTF(("-- FALLBACK IS not  SET --"));
			initiateFallback = 0;
		}
	}
	else
	{
		LOG_PRINTF(("-- FALLBACK IS NOT SET --"));
		initiateFallback = 0;
	}
	return;	
}

/******************************************************************************
* amountLongtoString
* Convert amount long to string 
* @return void
*******************************************************************************/
static void amountLongtoString(unsigned long BinaryValue, unsigned char *NumValue)
{
	// Internal data declaration
	unsigned long Bin;
	unsigned long rest;
	unsigned long i;

	Bin = BinaryValue;

	for (i = 0; i < 6; i++)
	{
		// first nibble
		rest = Bin - ((Bin / 10) * 10);
		NumValue [5 - i] = (unsigned char) rest;
		Bin = (Bin / 10);

		// Second nibble
		rest = Bin - ((Bin / 10) * 10);
		NumValue [5 - i] += (unsigned char) (rest << 4);
		Bin = (Bin / 10);
	}
}

/******************************************************************************
* LongToChar
* Convert amount Long To Char 
* @return void
*******************************************************************************/
static void LongToChar(unsigned long value,unsigned char *data)
{
	data[0] = (unsigned char)((value & 0xFF000000) >> 24);
	data[1] = (unsigned char)((value & 0x00FF0000) >> 16);
	data[2] = (unsigned char)((value & 0x0000FF00) >> 8);
	data[3] = (unsigned char)((value & 0x000000FF));
}
/*--------------------------------------------------------------------------
    Function:		ValidateICCCard
    Description:	Function for application selecion handle fallback scenario and copies the data 
                  into transaction structure
    Parameters:	TransactionMsgStruc structure poniter and sEMV_DATA structure pointer
    Returns:		success,fallback or failure
	Notes:          
--------------------------------------------------------------------------*/
//Validating CARD 
int ValidateICCCard(TransactionMsgStruc *ptrTransMsg,sEMV_DATA * emvdata)
{
	int iResult = 0;//I
	short autoFlag = 0;//I
	unsigned short  inDOLen = 0;//I
  char Track2[200] = {0};//I
	unsigned char CardNumber[25]={0};//I
	unsigned char currencyCode[3]={0};
	char    szTrack2Data[40+1]={0};     // in ASCII
	unsigned char btCardholder[50]={0};//I
	unsigned char btTrack2[20]={0}; //I   
	int i=0,j=0;
	unsigned short      aidcount = 0;
	unsigned short      blockAidCount = 0;

  srAIDListStatus srTermAIDListStatus[MAX_AID_LIST] = {0};

	char strTrack2[50] = {0};
	unsigned short      usDefaultRecordRFU1	= 0;
	int inIssuerNumber = -1,inAcquirerNumber= -1;
	unsigned char NumAmount[6] = {0};
	unsigned char TabAmount[4] = {0};
	unsigned long entered_amount = 0;
	unsigned long entered_amount_cash = 0;

  char *ptr = NULL;
  const char delim[2] = "=";
  char exp [EXPTIME_ARR_LEN+1] ={0};
  char tempExpDate[EXPTIME_ARR_LEN+1]={0};//5
  char Temp_Amount[AMOUNT_LEN+1]={0};//

	//byte AccountsType =0;
	fOfflinePINCancelled = 0; 
	fOnlinePINCancelled = 0; 
	initiateFallback = 0;

	//message display "EMV PROCESSING..."
  window(1,1,30,20);
	clrscr();
	write_at(EMV_PROCESSING,strlen(EMV_PROCESSING),1,8);

	// Required for GPO & App selection separation changes
	usDefaultRecordRFU1 = inGetShortRFU1();
	
	autoFlag = inGetAutoSelectApplnFlag();
	LOG_PRINTF(("autoFlag =%d", autoFlag));
  
	// This function selects the EMV application and searches for the PSE file on the
	// card. If the file is found, the application is selected, else explicit option is used.

	iResult = inVXEMVAPSelectApplication(autoFlag, inMenuFunc, vdPromptManager, NULL /*usAmtEntryFunc*/, NULL);
	
		
	LOG_PRINTF(("inVXEMVAPSelectApplication iResult =%d", iResult));
  EnblTouchScreen(DisableTouch);
	//	for GPO & App selection separation changes
	//  Call Perform GPO if Bit 8 of RFU1 field is set
	if((usDefaultRecordRFU1 & 0x0080) && (iResult == SUCCESS))
	{
		LOG_PRINTF(("for GPO & App selection separation changes "));
		iResult = inVXEMVAPPerformGPO();
		LOG_PRINTF(("inVXEMVAPPerformGPO iResult =%d", iResult));
	}
	if(iResult != SUCCESS)
	{
		LOG_PRINTF(("CARD_REMOVED =%u", CARD_REMOVED));
		LOG_PRINTF(("CARD_BLOCKED =%u", CARD_BLOCKED));
		LOG_PRINTF(("INVALID_PARAMETER =%u", INVALID_PARAMETER));
		LOG_PRINTF(("BAD_ICC_RESPONSE =%u", BAD_ICC_RESPONSE));
		LOG_PRINTF(("CANDIDATELIST_EMPTY =%u", CANDIDATELIST_EMPTY));
		LOG_PRINTF(("CHIP_ERROR =%u", CHIP_ERROR));
		LOG_PRINTF(("BAD_DATA_FORMAT =%u", BAD_DATA_FORMAT));
		LOG_PRINTF(("TRANS_CANCELLED =%u", TRANS_CANCELLED));
		LOG_PRINTF(("EASY_ENTRY_APPL =%u", EASY_ENTRY_APPL));
		LOG_PRINTF(("E_EXPLICIT_SELEC_REQD =%u", E_EXPLICIT_SELEC_REQD));
		LOG_PRINTF(("APPL_BLOCKED =%u", APPL_BLOCKED));

		if(iResult == CANDIDATELIST_EMPTY)
		{
			//To find if there are any blocked applications in the card
			aidcount = MAX_AID_LIST;
			blockAidCount = 0;
			usEMVGetAllAIDStatus(srTermAIDListStatus, &aidcount, &blockAidCount);
			if (blockAidCount > 0)	
			{
					iResult = APPL_BLOCKED ;
					LOG_PRINTF(("blockAidCount =%u", blockAidCount));
					LOG_PRINTF(("aidcount =%u", aidcount));
			}
			else
			{
					LOG_PRINTF(("blockAidCount =%u", blockAidCount));
					LOG_PRINTF(("aidcount =%u", aidcount));
			}
      window(1,1,30,20);
	    clrscr();
      write_at("AID NOT FOUND", strlen("AID NOT FOUND"),8, 11);
      write_at("TRANSACTION STOPPED", strlen("TRANSACTION STOPPED"),4, 12);
      SVC_WAIT(2000);
			ClearKbdBuff();
			KBD_FLUSH();
    
      return _FAIL ;
		}

		if((iResult != CARD_BLOCKED) 
			&&(iResult != CARD_REMOVED)
			&&(iResult != TRANS_CANCELLED)
			&&(iResult != INVALID_PARAMETER)
			&&(iResult !=APPL_BLOCKED)
			&&(iResult != APPL_NOT_AVALBL)&&(iResult != CANDIDATELIST_EMPTY)
			)
		
		{
      LOG_PRINTF(("Going to call initiateFallbackTxn1"));
			initiateFallbackTxn();  //fallback flag initialization
			return FALLBACK;
		}

		if (((iResult == CHIP_ERROR) 
			||(iResult == APPL_NOT_AVAILABLE)
			||(iResult == CANDIDATELIST_EMPTY)
			)&& (!candidateList.countAID)
			)
		{
      LOG_PRINTF(("Going to call initiateFallbackTxn2"));
			initiateFallbackTxn();
			return FALLBACK;
		}

		if((iResult == (int)EMV_FAILURE) )
		{
      LOG_PRINTF(("EMV_FAILURE"));
			initiateFallbackTxn();
			return FALLBACK;
		}
    if((iResult == (int)INVALID_PARAMETER) )
		{
      LOG_PRINTF(("INVALID_PARAMETER"));
			initiateFallbackTxn();
			return FALLBACK;
		}
    if((iResult == (int)BAD_ICC_RESPONSE) )
		{
      LOG_PRINTF(("BAD_ICC_RESPONSE"));
			initiateFallbackTxn();
			return FALLBACK;
		}
    if((iResult == (int)CHIP_ERROR) )
		{
      LOG_PRINTF(("CHIP_ERROR"));
			initiateFallbackTxn();
			return FALLBACK;
		}
		if((iResult == (int)BAD_DATA_FORMAT) )
		{
      LOG_PRINTF(("CHIP_ERROR"));
			initiateFallbackTxn();
			return FALLBACK;
		}
    if((iResult == (int)CARD_REMOVED)
      ||(iResult == (int)CARD_BLOCKED)
      ||(iResult == (int)CARD_REMOVED)
      ||(iResult == (int)EASY_ENTRY_APPL)
      ||(iResult == (int)E_EXPLICIT_SELEC_REQD))
		{
      LOG_PRINTF(("INVALID CARD"));
      window(1,1,30,20);
	    clrscr();
      write_at("INVALID CARD", strlen("INVALID CARD"),8, 11);
      SVC_WAIT(1500);
			ClearKbdBuff();
			KBD_FLUSH();
    
      return _FAIL ;
		}
		else
		{
      LOG_PRINTF(("Going to call vdPromptManager"));
			vdPromptManager((unsigned short)iResult);
			window(1,1,30,20);
	    clrscr();
      if(inVXEMVAPCardPresent() == SUCCESS)
			{
				//message display  "REMOVE CARD"
				display_at(3,8,"PLEASE REMOVE YOUR CARD",CLR_LINE);
				while(!inVXEMVAPCardPresent());
			}
			else
			{
				//message display "CARD REMOVED OR FAILURE!"
				display_at(1,8,"CARD REMOVED",CLR_LINE);
				SVC_WAIT(2000);
			}
		}
		return -1;
	}
  else
  {
    LOG_PRINTF(("iResult = success"));
  }

	window(1,1,30,20);
	clrscr();
	display_at(1,8,EMV_PROCESSING,CLR_LINE);
	
	// below function is called to update record in MVT
	// inIssuerNumber Maps to the field "Scheme Reference" in MVT.
	// inAquirerNumber Maps to the field "Issuer Reference" in MVT.

	inVXEMVAPGetCardConfig(inIssuerNumber,inAcquirerNumber);

	//This function obtains the AFL from the DOC and uses it to locate the files
	//associated with the selected card application. This function then reads all
	//appropriate records from these files, extracts the data, and places it in the DOC.
	if((iResult = inVXEMVAPProcessAFL()) != SUCCESS)
	{
		if(	iResult == APPL_BLOCKED
			|| iResult == CARD_BLOCKED
			|| iResult == APPL_NOT_AVALBL //0x6985 is error code for use case not allowed or appl not available
			)
		{
			vdPromptManager(iResult);//amitesh::for returning error code 
      window(1,1,30,20);
	    clrscr();
			if(inVXEMVAPCardPresent() == SUCCESS)
			{

				//message display  "REMOVE CARD"
        LOG_PRINTF(("REMOVE CARD"));
				display_at(3,8,"PLEASE REMOVE YOUR CARD",CLR_LINE);
				while(!inVXEMVAPCardPresent());
			}
			else
			{
				//message display "CARD REMOVED OR FAILURE!"
        LOG_PRINTF(("CARD REMOVED"));
				display_at(1,8,"CARD REMOVED",CLR_LINE);
				SVC_WAIT(200);
			}
			return (-1);
		}
    LOG_PRINTF(("Going to call initiateFallbackTxn"));
		initiateFallbackTxn();
		return FALLBACK;
	}
  else
  {
    LOG_PRINTF(("iResult = success"));
  }

	//Application Card Number
	memset(CardNumber, 0x00, sizeof(CardNumber));
	inDOLen = 0;

	//Api to retrive tag value from collection(here we are retrieving card number)
	usEMVGetTLVFromColxn(TAG_5A_APPL_PAN, (byte *)CardNumber, (unsigned short *) &inDOLen);  
	LOG_PRINTF(("inDOLen=%d", inDOLen));
	if(inDOLen > 0)
	{
	  hex2asc((unsigned char *)ptrTransMsg->CardDetails.acct, CardNumber, inDOLen);
		/*need to check  
		if(enterCashBackAmount() == BTN_CANCEL)
			return -1;
    */
		/*need to handle  if(strlen(trackCardData->cashBackAmount)>0)
		{
			transType=SALE_WITH_CASHBACK;

			usEMVAddTLVToCollxn(0x9c00, (byte *) &transType, 1);
		}*/
	}

	// Get Track 2 Equivalent Data in BCD
	memset(Track2, 0x00, sizeof(Track2));
	inDOLen = 0;
	usEMVGetTLVFromColxn(TAG_57_TRACK2_EQ_DATA, (byte *)btTrack2, (unsigned short *) &inDOLen);  
	LOG_PRINTF(("inDOLen=%d", inDOLen));
	if(inDOLen > 0)
	{
		// If Track 2 Equivalent Data exists
		// Convert it to a NULL-terminated ASCII string
		memset(szTrack2Data, 0x00, sizeof(szTrack2Data));
		hex2asc((BYTE*)szTrack2Data, btTrack2, inDOLen);
		LOG_PRINTF(("szTrack2Data =%s", szTrack2Data));

		for(i=0; i < (2*inDOLen); i++)
		{
			if(szTrack2Data[i] == 'F')
			{
				szTrack2Data[i] = 0;
				break;
			}
			if(szTrack2Data[i] == 'D')     // Field separator
			{
				szTrack2Data[i] = '=';      // Replace with field separator
			}
		}
		// update global Track 2
		if(strlen(szTrack2Data)>0)
		{
			LOG_PRINTF(("corrected szTrack2Data =%s", szTrack2Data));

			strcpy(strTrack2, szTrack2Data);
      ptr = strstr(szTrack2Data,delim);
      for(i =0 ;i<4;i++)
      {
        exp[i] = ptr [i+1];
      }
			//need to change  strcpy(trackCardData->CARDDATA.track,strTrack2);
       strncpy(ptrTransMsg->CardDetails.track,strTrack2,strlen(strTrack2));
			//  strcpy(trackCardData->CARDDATA.acct, strtok(strTrack2, "="));
			 strcpy(ptrTransMsg->CardDetails.acct, strtok(strTrack2, "="));
       strncpy(ptrTransMsg->PrimaryAccNum,ptrTransMsg->CardDetails.acct,strlen(ptrTransMsg->CardDetails.acct));  
       memset(Global_AccNo,0,sizeof(Global_AccNo));
       strncpy(Global_AccNo,ptrTransMsg->PrimaryAccNum,strlen(ptrTransMsg->PrimaryAccNum));
       //formating expiery date data in card swipe 
	    for(i=2,j=0;i<4;i++,j++)
	    {
			    tempExpDate[j]= exp[i];
	    }
	    for(i=0,j=2;i<2;i++,j++)
	    {
			    tempExpDate[j]= exp[i];
	    }
	    strcpy(ptrTransMsg->CardDetails.exp,tempExpDate);
	    sprintf(ptrTransMsg->ExpiryDate,"%s",&ptrTransMsg->CardDetails.exp);
	    if(LOG_STATUS == LOG_ENABLE)
      {
        LOG_PRINTF (("ExpiryDate = %s",ptrTransMsg->ExpiryDate));	
	      
      }
      LOG_PRINTF(("strTrack2 = %s", strTrack2));
			LOG_PRINTF(("strTrack2 acountNo = %s", ptrTransMsg->PrimaryAccNum));
			//  LOG_PRINTF(("strTrack2 acountNo = %s",trackCardData->CARDDATA.acct));
			
			memcpy(emvdata->Track2,szTrack2Data,strlen(szTrack2Data));
      memcpy(ptrTransMsg->Track2Data,szTrack2Data,strlen(szTrack2Data));
			//GLOBAL FUNCTION TO STORE	// updating track2 in global
		}
	}

	memset(btCardholder,0x00,sizeof(btCardholder));
	inDOLen = 0;
	usEMVGetTLVFromColxn(TAG_5F20_CARDHOLDER_NAME, (byte *)btCardholder, (unsigned short *) &inDOLen);  
	LOG_PRINTF(("inDOLen=%d", inDOLen));
	if(inDOLen >0)
	{
		memcpy(emvdata->m_chArrCardHolderName,btCardholder,inDOLen);
    memcpy(ptrTransMsg->CardHldrName,emvdata->m_chArrCardHolderName,inDOLen);
	}
  LOG_PRINTF (("CardHldrName = %s",ptrTransMsg->CardHldrName));
	//set cuurency code and terminal country code
	currencyCode[0]=0x08;
	currencyCode[1]=0x40;
	currencyCode[2]=0x00;
	usEMVUpdateTLVInCollxn(0x5F2A,currencyCode, 2);
	usEMVUpdateTLVInCollxn(0x9F1A,currencyCode, 2); //terminal country code
	
	//set account type according to transaction types
	//AccountsType = 0x00;  //Default Unspecified.
  /* 
 if(!strncmp (trackCardData->cardType,CARD_TYPE_DEBIT, 5))
	{
		//Selecting account type
		AccountsType = debitAccountSelection("Sale");
		if(AccountsType  != 0x04)
		{
			usEMVAddTLVToCollxn (TAG_5F57_ACCOUNT_TYPE, (unsigned char *)&AccountsType, 1);
		}
		else 
			return -1;
	}
	else
		usEMVAddTLVToCollxn (TAG_5F57_ACCOUNT_TYPE, (unsigned char *)&AccountsType, 1);
    */
	//Transaction Amount changes into integer value

 // strcpy(Temp_Amount,ptrTransMsg->Amount );//
	//ConvertAmount(Temp_Amount, &entered_amount);
  if(ptrTransMsg->TrTypeFlag != BALINQMSGTYPE_CASE)
  {
    strcpy(Temp_Amount,ptrTransMsg->Amount );//
    LOG_PRINTF(("Temp_Amount=%s", Temp_Amount));
  }
  else
  {
    strcpy(Temp_Amount,"0.00");//
    LOG_PRINTF(("BALINQMSGTYPE_CASETemp_Amount=%s", Temp_Amount));
  }
	ConvertAmount(Temp_Amount, &entered_amount);
	entered_amount_cash=0;
  /* // 
	if(strlen(trackCardData->cashBackAmount)>0)
	{
		ConvertAmount(trackCardData->cashBackAmount , &entered_amount_cash);
	}
  */
	LongToChar(entered_amount, TabAmount);
	LOG_PRINTF(("TAG_AMOUNT_AUTH_BIN TabAmount[0]= 0x%x,TabAmount[1]= 0x%x,TabAmount[2]= 0x%x,TabAmount[3]= 0x%x",TabAmount[0],TabAmount[1],TabAmount[2],TabAmount[3]));
	usEMVUpdateTLVInCollxn(TAG_81_AMOUNT_AUTH, (byte *) &TabAmount, 4);

	amountLongtoString(entered_amount, NumAmount);
  //ptrTransMsg
  //packData(ptrTransMsg->Amount,NumAmount);
	usEMVUpdateTLVInCollxn(TAG_9F02_AMT_AUTH_NUM, (byte *) NumAmount, 6);

	//other amounts like cashback
	LongToChar(entered_amount_cash, TabAmount);
	usEMVUpdateTLVInCollxn(TAG_9F04_AMT_OTHER_BIN, (byte *) TabAmount, 4);

	amountLongtoString(entered_amount_cash, NumAmount);
	usEMVUpdateTLVInCollxn(TAG_9F03_AMT_OTHER_NUM, (byte *) NumAmount, 6);

 
	return 0;
}

/******************************************************************************
* emvDataAuthentication
* Perform offline data auth and processing 
* @return EMVMODULE_RET
*******************************************************************************/
int emvDataAuthentication(void)
{
	//LOG_PRINTFF((0x8L, "CHECK IN: emvDataAuthentication"));  
	int inResult;

	//This function obtains the relevant cryptographic data from the DOC and uses it to
	//perform SDA (Static Data Authentication), DDA (Dynamic Data Authentication), or
	//CDDA (Combined DDA/AC).
	if((inResult = inVXEMVAPDataAuthentication(NULL)) != SUCCESS)
	{
		if((inResult == CARD_REMOVED))
		{
			inResult = CARD_REMOVED;
			vdPromptManager((unsigned short)inResult);
		}
		else
			vdPromptManager((unsigned short)inResult);

		LOG_PRINTFF((0x8L, "CHECK OUT 1: emvDataAuthentication"));
		return -1;
	}

	LOG_PRINTFF((0x8L, "CHECK OUT 2: emvDataAuthentication"));
	return 0;
}

/******************************************************************************
* emvPRandTRM
* Perform risk management and processing 
* @return EMVMODULE_RET
*******************************************************************************/
int emvPRandTRM(void)
{
	
	int inResult=-1;
  LOG_PRINTFF((0x8L, "CHECK IN: emvPRandTRM"));
	if((inResult = inVXEMVAPProcRisk(NULL)) != SUCCESS)
	{
		if((inResult == CARD_REMOVED) )
		{
			inResult = CARD_REMOVED;
			vdPromptManager((unsigned short)inResult);
		}
		else
			vdPromptManager((unsigned short)inResult);





		LOG_PRINTFF((0x8L, "CHECK OUT 1: emvPRandTRM"));
		return -1;
	}

	LOG_PRINTFF((0x8L, "CHECK OUT 2: emvPRandTRM"));
	return 0;
}

/******************************************************************************
* emvCardholderVerification
* Perform CVM processing 
* @return EMVMODULE_RET
*******************************************************************************/
int emvCardholderVerification(void)
{
	int inResult;
	unsigned char szCVMResults[10+1];
	unsigned char szTempBuf[10+1];
	unsigned short usTagLen;

	byte bCVMList[TAG8E00_SIZE+1];
	byte bCVM = '\0';
	byte bshortCVM = '\0';
	byte bTVR[TAG9500_SIZE];
	srTxnResult srTVRTSI;

	int inNoOfCVM = 0;
	int i, inUnrecognizedCVMPos  = 0, inPos = 0;
	short fUnrecognizedCVMFound = 0;
	short fSetUnrecognizedCVM = 0;

  //Variable declaration for getting TSI value for printing purpose
  unsigned char TSI[_TSI_SIZE+1] = {0};//
  unsigned char TVR[_TVR_SIZE+1] = {0};//
  //unsigned char Card_Type_Buff[CARD_TYPE_SIZE+1] = {0};//
  short len = 0 ;//
  char tempBuff[20] = {0};  //to display the TSI value


	memset(bCVMList, 0x00, sizeof(bCVMList));
	memset(bTVR, 0x00, sizeof(bTVR));
	memset(&srTVRTSI, 0x00, sizeof(srTVRTSI));

	memset(szCVMResults, 0x00, sizeof(szCVMResults));
	memset(szTempBuf, 0x00, sizeof(szTempBuf));

	LOG_PRINTF(("emvCardholderVerification"));

	fPerformOnlinePIN = 0;
	fSigRequired = 0;
	fPerformOfflinePIN = 0;

	vdSetPinParams();//SECURE_PIN_MODULE changes
	inResult = inVXEMVAPSetDisplayPINPrompt(&displayPINPrompt);// OFFLINE PIN ENTRY WILL BE CALLED IF NEEDED		
  LOG_PRINTF(("inVXEMVAPSetDisplayPINPrompt %d",inResult));
	SVC_WAIT(1000);
	inResult = inVXEMVAPVerifyCardholder(NULL);

	LOG_PRINTF(("cardverify inResult =%d [%02X]",inResult , inResult));
  if(open("/dev/crypto", 0) >=0 )
	{
		LOG_PRINTF(("crypto open sucess"));
	}
	else
	{
		LOG_PRINTF(("crypto fail"));
	}
	if(inResult == E_USR_BACKSPACE_KEY_PRESSED)
		inResult = inVXEMVAPVerifyCardholder(NULL);
  
	
	LOG_PRINTF(("cardverify inResult %d",inResult));
	if(inResult != SUCCESS)
	{
		if((inResult == CARD_REMOVED) )
		{
			inResult = CARD_REMOVED;
			vdPromptManager((unsigned short)inResult);
		}
		else
			vdPromptManager((unsigned short)inResult);
		return -1;
	}

	//Get the CVM List
	usEMVGetTLVFromColxn(TAG_8E00_CVM_LIST,(byte*) &bCVMList, &usTagLen);
  LOG_PRINTF(("usTagLen =%d",usTagLen));
	//Get total number of CVMs
	inNoOfCVM = (usTagLen-8) / 2;
  LOG_PRINTF(("inNoOfCVM =%d",inNoOfCVM));
	for(i=0; i < inNoOfCVM; i++)
	{
		bCVM = bCVMList[(2*i)+8];
		bshortCVM = (byte) (bCVM & 0x3f);

		fUnrecognizedCVMFound = fEMVIsItUnrecognizedCVM(bCVM);

		if(fUnrecognizedCVMFound)
			inUnrecognizedCVMPos = i;

		//If Unrecognozed CVM is the only CVM, then dont do anything
		if((fUnrecognizedCVMFound) && (inNoOfCVM == 1))
			break;
    LOG_PRINTF(("bshortCVM =%d",bshortCVM));
		switch(bshortCVM)
		{
		case PT_PINVERIFY:
		case PT_PINVERIFY_N_SIG:
		case ENCIPH_PINVERIFY:
		case ENCIPH_PINVERIFY_N_SIG:
      LOG_PRINTF(("ENCIPH_PINVERIFY_N_SIG"));
			if(fPerformOfflinePIN == VS_TRUE)
				inPos = i;
			break;
		case SIGNATURE:
      LOG_PRINTF(("SIGNATURE"));
			if(fSigRequired == VS_TRUE)
				inPos = i;
			break;
		case ENCIPH_PINVERIFY_ONLINE:
      LOG_PRINTF(("ENCIPH_PINVERIFY_ONLINE"));
			if(fPerformOnlinePIN == VS_TRUE)
				inPos = i;
			break;
		case 7:
		default:
			//Need to add the condition for unrecognozed CVM here
			if((bshortCVM >= 31) && (bshortCVM <= 47))
				fUnrecognizedCVMFound = 0;
			else if((bshortCVM >= 48) && (bshortCVM <= 62))
				fUnrecognizedCVMFound = 0;
			else
			{
				fUnrecognizedCVMFound = 1;
				inUnrecognizedCVMPos = i;
			}

			break;
		}

		//If any of the CVMs has been executed, check if it was succcessful
		if((fPerformOfflinePIN) || (fPerformOnlinePIN) || (fSigRequired))
		{
			usEMVGetTLVFromColxn(TAG_9500_TVR,(byte*) &bTVR, &usTagLen);
      LOG_PRINTF(("any of the CVMs bTVR =%s",bTVR));
			if(!(bTVR[2] & 0x80)) 
			{
				//The CVM was successful,so check if the Unrecognized CVM came before this CVM
				if(fUnrecognizedCVMFound)
				{
					if((inUnrecognizedCVMPos < inPos) ||(inUnrecognizedCVMPos == 0))
					{
						fSetUnrecognizedCVM = 1;
						break;
					}
				}
				else
					break;
			}
		}

	}

	//Unrecognized CVM found, set the TVR bit
	if(fSetUnrecognizedCVM)
	{
    LOG_PRINTF(("fSetUnrecognizedCVM"));
		bTVR[0] |= 0x40; // amit according to test case 01 for offline sda
		//bTVR[2] |= 0x00; // amit according to test case 01 for offline sda
		memcpy(srTVRTSI.stTVR, bTVR, 5);
    LOG_PRINTF(("fSetUnrecognizedCVM stTVR = %s",srTVRTSI.stTVR));
		usEMVGetTLVFromColxn(TAG_9B00_TSI, (byte*)&srTVRTSI.stTSI, &usTagLen);
    LOG_PRINTF(("fSetUnrecognizedCVM stTSI = %s",srTVRTSI.stTSI));
		usEMVSetTxnStatus(&srTVRTSI);
	}
	
	usEMVGetTLVFromColxn(TAG_9F34_CVM_RESULTS,(byte*) &szTempBuf, &usTagLen);
	SVC_HEX_2_DSP((char*)szTempBuf, (char*)szCVMResults, 3);
	LOG_PRINTF(("CVM Results = %s", szCVMResults));

	if((fOfflinePINCancelled) || (fOnlinePINCancelled))
	{
    LOG_PRINTF(("fOfflinePINCancelled) || (fOnlinePINCancelled"));
    window(1,1,30,20);
	  clrscr();
		write_at("TRANSACTION CANCELLED", strlen("TRANSACTION CANCELLED"), 1, 8);
		SVC_WAIT(2000);
		error_tone();

		emvRemoveCard();
		return -1;
	}

  //For getting TSI value 
  usEMVGetTLVFromColxn(TAG_9B00_TSI, (byte*)tempBuff, &len);  
  LOG_PRINTF(("tTSI len =%d", len));
  hex2asc((BYTE*)TSI, (BYTE*)tempBuff, len);
	memcpy(emvdata.TSI,TSI,strlen((char*)TSI));
  LOG_PRINTF(("tTSI =%s", emvdata.TSI));

  //Getting TVR type 
  memset(tempBuff,0,sizeof(tempBuff));
  usEMVGetTLVFromColxn(TAG_9500_TVR, (byte*)tempBuff, &len);  
  LOG_PRINTF(("TAG_9500_TVR len =%d", len));
  hex2asc((BYTE*)TVR, (BYTE*)tempBuff, len);
  memcpy(emvdata.TVR,TVR,strlen((char*)TVR));
  LOG_PRINTF(("TAG_9500_TVR =%s", emvdata.TVR));

  //Getting Card Type
  memset(tempBuff,0,sizeof(tempBuff));
  usEMVGetTLVFromColxn(TAG_9F12_APPL_PRE_NAME, (byte*)tempBuff, &len);  
  LOG_PRINTF(("9F12 len =%d", len));
  LOG_PRINTF(("9F12 tempBuff =%s", tempBuff));
  memcpy(emvdata.CardType,tempBuff,strlen((char*)tempBuff));
  LOG_PRINTF(("CardType =%s", emvdata.CardType));


 
	return 0;
}
/******************************************************************************
* emvFirstGenerateAC
* Perform 1st Gen AC processing 
* @return EMVMODULE_RET
*******************************************************************************/
int emvFirstGenerateAC(void)
{
	int iResult;
	int TerminalDecision;
	unsigned short usResult;
	//unsigned short usRetVal;
	unsigned short usTagLen;   



	byte cryptInfoData;
	//byte buf[2];
	char szRID[EMV_RID_SIZE + 1];   

	LOG_PRINTFF((0x8L, "CHECK IN : emvFirstGenerateAC "));
	memset(szRID, 0x00, sizeof(szRID));

	//TerminalDecision = inGetTerminalDecision();
	TerminalDecision = 0;

	//This function enforces the merchant forced online option and performs TAA to
	//decide the type of cryptogram to request
	if((iResult = inVXEMVAPFirstGenerateAC(TerminalDecision)) != SUCCESS)
	{
		LOG_PRINTF(( "inVXEMVAPFirstGenerateAC iResult =%d %02X", iResult, iResult));

		// Should do a Second Gen AC for AAR
		if(iResult == BAD_DATA_FORMAT)
		{
			usResult = usEMVGetTLVFromColxn(TAG_9F27_CRYPT_INFO_DATA, &cryptInfoData, &usTagLen);
			if(usResult == EMV_SUCCESS)
				if((cryptInfoData & 0xC0) == 0xC0)
				{
					// Should generate AAC before aborting the transaction for AAR
					inVXEMVAPSecondGenerateAC(HOST_DECLINED);
					return OFFLINE_DECLINED;
				}
		}

		// Check for card presence
		if((iResult == CARD_REMOVED))
		{
			iResult = CARD_REMOVED;
			//fCardRemoved = 0;
			vdPromptManager(iResult);
			LOG_PRINTF(( "emvFirstGenerateAC CARD_REMOVED"));
		}
		else
		{
			iResult = EMV_FAILURE;
			vdPromptManager(iResult);
			if(inVXEMVAPCardPresent() == SUCCESS)
				inVXEMVAPRemoveCard(vdPromptRemoveManager);
			SVC_WAIT(2000);
		}

		LOG_PRINTF(("CHECK OUT 1 : emvFirstGenerateAC iResult= %d %02X", iResult, iResult));
		return EMVMODULE_RET_NOT_OK;

	}


	LOG_PRINTF(("CHECK IN : emvFirstGenerateAC 2 "));

	// Make decision based on cryptogram information data returned by ICC
	if ((usResult = usEMVGetTLVFromColxn(TAG_9F27_CRYPT_INFO_DATA, &cryptInfoData, &usTagLen)) != EMV_SUCCESS)
	{
		//LOG_PRINTFF((0x8L, "CHECK OUT 2 : emvFirstGenerateAC iResult= %d", iResult));      

		if(inVXEMVAPCardPresent() == SUCCESS)
			inVXEMVAPRemoveCard(vdPromptRemoveManager); 

		return EMVMODULE_RET_NOT_OK;
	}

	LOG_PRINTF(("CHECK 1: emvFirstGenerateAC cryptInfoData [%02X]", cryptInfoData));

	switch(cryptInfoData & 0xC0)
	{
	case 0xC0: // AAR 
		// Should generate AAC before aborting the transaction for AAR
		//LOG_PRINTFF((0x8L, "-- CALL 2ND GEN AC --"));
		inVXEMVAPSecondGenerateAC(HOST_DECLINED);
		return OFFLINE_DECLINED;

	case AAC: //Declined --- AAC
		LOG_PRINTF(("-- AAC IN FIRST GEN AC --"));
		return OFFLINE_DECLINED ;

	case TC: //Offline Approval --- TC
		LOG_PRINTF(("-- TC IN FIRST GEN AC --"));
		return OFFLINE_APPROVED ;

	case ARQC: //ON_LINE --- ARQC
		LOG_PRINTF(("-- ARQC IN FIRST GEN AC--"));
		return ONLINE_REQST ;
	default:
		return EMVMODULE_RET_NOT_OK;
	} 




}

int  EMVModuleOnlineAuthorization(sEMV_DATA * emvdata)
{	
  int iRetval=-1;
  iRetval = BuildDE55(emvdata);
  LOG_PRINTF(("iRetval [%d]",iRetval));

  return iRetval;
}
/*--------------------------------------------------------------------------
    Function:		emvFirstGenerationFlow
    Description:	Function for first generation emv 
    Parameters:	TransactionMsgStruc structure poniter
    Returns:		success or failure
	Notes:          
--------------------------------------------------------------------------*/
short emvFirstGenerationFlow(TransactionMsgStruc *ptrTransMsg)
{
	short shortRetval;
 
  window(1,1,30,20);
	clrscr ();
	
	//call display MSG_PLEASE_WAIT or EMV [ROCESSING
	write_at(EMV_PROCESSING,strlen(EMV_PROCESSING),1,8);
 
  memset(&emvdata,0,sizeof(sEMV_DATA)); 
  shortRetval= ValidateICCCard(ptrTransMsg,&emvdata);//After changing
	
  LOG_PRINTF(("emvFirstGenerationFlow "));
	
	if(shortRetval == -1)
	{
		LOG_PRINTF(("ValidateICCCard-fails"));
		//emvRemoveCard();
		return BTN_CANCEL;
	}
	else if(shortRetval == FALLBACK)
	{
		LOG_PRINTF(("FALLBACK-occured"));
		PropmptForFallBack();
		window(1,1,30,20);
	  clrscr();
			               
	 
    return FALLBACK;
   
		//if FALLBACK DO swipe processing and pos entry mode is 080
	}
	else
	{
	
		//if(ptrTransMsg->TrTypeFlag == REFUNDMSGTYPE_CASE) 
		//{
		//	
		//	LOG_PRINTF(("%s","REFUND_TRANSACTION"));
		//	usEMVGetTxnStatus(&TVR_TSI);
		//
		//	//This API issues the First Generate AC command to the ICC and obtains the ICC
		//	//certificate (AC).
		//	//  
  //    usEMVGenerateFirstAC(0x00, &flag, &TVR_TSI) ;
		//	return 0 ;
		//}

		if(emvDataAuthentication() == -1)
		{
			LOG_PRINTF(("emvDataAuthentication-failed"));
			//m_bIsAbort = true;
			//resetEMVTxnData();
			emvRemoveCard();
			return BTN_CANCEL;
		}

		if(emvPRandTRM()== -1)
		{
			LOG_PRINTF(("emvPRandTRM-failed"));
			//m_bIsAbort = true;
			//resetEMVTxnData();
			emvRemoveCard();
			return BTN_CANCEL;
		}

		if(emvCardholderVerification()== -1)
		{
			LOG_PRINTF(("emvPRandTRM-failed"));
			//m_bIsAbort = true;
			//resetEMVTxnData();
			emvRemoveCard();
			return BTN_CANCEL;
		}

		shortRetval = emvFirstGenerateAC();
		switch(shortRetval)
		{
		  case ONLINE_REQST:
			LOG_PRINTF(("%s","ONLINE_REQST"));

			if(EMVMODULE_RET_NOT_OK == EMVModuleOnlineAuthorization(&emvdata))
			{
					//resetEMVTxnData();
				  emvRemoveCard();
					return BTN_CANCEL;
				
			}
      LOG_PRINTF(("%s","After ONLINE_REQST"));
      
		  // 	flagManualSwipe = 2;
		// 	EMVFALLBACKFLAG=0;
			//if(EMVMODULE_RET_NOT_OK == emvUseHostDataFlow())
			//{
			//	//resetEMVTxnData();
			//	//return EMVMODULE_RET_NOT_OK;
			//}


			break;
		case OFFLINE_APPROVED:// later implementation on offline upload of data and chargeslip
			if(EMVMODULE_RET_NOT_OK == EMVModuleOnlineAuthorization(&emvdata))
			{
					//resetEMVTxnData();
				emvRemoveCard();
					return BTN_CANCEL;
				
			}
			//need to handle flagManualSwipe = 4;
			break;
		case OFFLINE_DECLINED:
		emvRemoveCard();
			//return; cancel transaction "show declined from chip"
		default:
			//resetEMVTxnData();
			emvRemoveCard();
			//structure memset
			return BTN_CANCEL;
		}
    
	}
 
	return 0;
}
//
int ParseAuthResponseDe55(unsigned char * emv_data,sEMV_AUTH_RESP * sAuthRespn)
{
	int retVal = 0;
	int iLenScript71=0, iLenScript72=0;
	int iOffset = 0x00;
	unsigned short usTag    = 0x00;
	//unsigned short usLen    = 0x00;
	unsigned char * ptr = NULL;
	unsigned char tagByte1 = 0, tagByte2=0,dataByte=0;
	int numTagBytes = 0,numLengthBytes = 0,dataLength =0;
	int lenToParse = 0, i=0;
	//memset(szScript71,0x00,sizeof(szScript71));
	//memset(szScript72,0x00,sizeof(szScript72));
	//us71ScriptLen =0;
	//us72ScriptLen =0;
	/*
	 * TAG_8A_AUTH_RESP_CODE
	 * TAG_91_ISS_AUTH_DATA
	 * TAG_ISUER_SCRPT_TEMPL_71
	 * TAG_ISUER_SCRPT_TEMPL_72
	 *
	 */
	LOG_PRINTF(("ParseAuthResponseDe55"));

	lenToParse = (emv_data[iOffset++] << 8) & 0xFF00;
	lenToParse |= emv_data[iOffset++] & 0x00FF;

	//lenToParse = emv_data;
	

	//LOG_PRINTF(("De55len = %d",emv_data->De55len));
	LOG_PRINTF(("lenToParse = %d",lenToParse));

	ptr  = (emv_data +iOffset) ;

	while(iOffset < lenToParse)
	{
		//CLogger::TraceLog(TRACE_DEBUG,"iOffset = %d ,lenToParse = %d",iOffset,lenToParse);

		tagByte1 = 0;
		tagByte2 = 0;
		dataByte = 0;

		//Get The tag
		tagByte1 = *ptr;

		//EMV specification says, any tag with == 0x1F
		//must be treated as two byte tags.
		// Bit pattern must be 10011111 or greater

		if ((tagByte1 & 0x1F) == 0x1F)
		{
		    ptr++;
		    tagByte2 = *ptr;
		    usTag = (unsigned short) (tagByte1 << 8);
		    usTag += (unsigned short) tagByte2;
		    numTagBytes = 2;
		}
		else
		{
			  usTag = (unsigned short) tagByte1;
		    numTagBytes = 1;
		}

		LOG_PRINTF(("usTag = 0x%x",usTag));

		//Buffer overflow check
		if(iOffset +numTagBytes >lenToParse)
		{
			//CLogger::TraceLog(TRACE_DEBUG,"Buffer Overflow return 1");
			return retVal;
		}

		// Get the Length
		ptr++;
		dataByte = *ptr;
		if (dataByte & 128) // If last bit is set
		{
		    dataLength = 0;
		    numLengthBytes = (short) dataByte & 127;  // b7 - b1 represent the number of subsequent length bytes
		    ptr++;
		    for (i = 0; i < numLengthBytes; i++)
		    {
		        dataLength = (dataLength << 8) + (short) *ptr;
		        ptr++;
		    }
		}
		else // Length field consists of 1 byte max value of 127
		{
		    numLengthBytes = 1;
		    dataLength = (short) *ptr;
		    ptr++;
		}

		//Buffer overflow check
		if((iOffset +numTagBytes+dataLength) >lenToParse)
		{
			//CLogger::TraceLog(TRACE_DEBUG,"Buffer Overflow return 2");
			return retVal;
		}
		switch(usTag)
		{
			case TAG_8A_AUTH_RESP_CODE: /*0x8A*/
			case 0x8a:
				LOG_PRINTF(("TAG_8A_AUTH_RESP_CODE"));
				memcpy(sAuthRespn->AuthResponseCode,ptr,dataLength);
				sAuthRespn->AuthResponseCodeLen = dataLength;
				retVal = 1;
				break;
			case TAG_91_ISS_AUTH_DATA: 			/*0x91*/
			case 0x91:
				LOG_PRINTF(("TAG_91_ISS_AUTH_DATA"));
				memcpy(sAuthRespn->IssuerAuthData,ptr,dataLength);
				sAuthRespn->IssuerAuthDataLen = dataLength;
				retVal = 1;
				break;
			case TAG_ISUER_SCRPT_TEMPL_71: /*0x71*/
			case 0x71:
				//CLogger::TraceLog(TRACE_DEBUG,"TAG_ISUER_SCRPT_TEMPL_71");
				//mayank 29/05/2013 multiple IssuerScript
				iLenScript71 = sAuthRespn->IssuerScript71Len;
				sAuthRespn->IssuerScript71[iLenScript71++] = dataLength & 0xFF;
				memcpy(sAuthRespn->IssuerScript71 + iLenScript71,ptr,dataLength);
				iLenScript71 += dataLength;

				sAuthRespn->IssuerScript71Len = iLenScript71;
				//LOG_PRINTF(("usTag = 0x%x",usTag));
				LOG_PRINTF(("IssuerScript71Len[%d]", sAuthRespn->IssuerScript71Len));

//				memcpy(sAuthRespn->IssuerScript71,ptr,dataLength);
//				sAuthRespn->IssuerScript71Len = dataLength;
				retVal = 1;
				break;
			case TAG_ISUER_SCRPT_TEMPL_72: /*0x72*/
			case 0x72:
				//CLogger::TraceLog(TRACE_DEBUG,"TAG_ISUER_SCRPT_TEMPL_72");

				iLenScript72 = sAuthRespn->IssuerScript72Len;
				sAuthRespn->IssuerScript72[iLenScript72++] = dataLength & 0xFF;
				memcpy(sAuthRespn->IssuerScript72 + iLenScript72,ptr,dataLength);
				iLenScript72 += dataLength;

				sAuthRespn->IssuerScript72Len = iLenScript72;
				LOG_PRINTF(("IssuerScript72Len[%d]", sAuthRespn->IssuerScript72Len));
//				memcpy(sAuthRespn->IssuerScript72,ptr,dataLength);
//				sAuthRespn->IssuerScript72Len = dataLength;
				retVal = 1;
				break;
		};

		//increment the offset
		iOffset += numTagBytes + numLengthBytes + dataLength;
		//CLogger::TraceLog(TRACE_DEBUG,"iOffset = %d",iOffset);

		if(iOffset < lenToParse){
			//increment the ptr
			//CLogger::TraceLog(TRACE_DEBUG,"incrementing pointer");
			ptr += dataLength;
		}
		//CLogger::TraceLog(TRACE_DEBUG,"Searching Next Tag");
	}
	//CLogger::TraceLog(TRACE_DEBUG,"Returning from PARSEDE55");
	LOG_PRINTF(("end ParseAuthResponseDe55"));
	return retVal;
}

///******************************************************************************
//* createScriptBuffers 
//*******************************************************************************/
int createScriptBuffers(byte *scriptBuf, int iLen, short iType)
{
	//byte *ptr;
	//byte data[512] = {0};
#ifdef _TARG_68000
	unsigned int tag = 0; 
	unsigned int for Verix
#else
	unsigned short tag = 0;
#endif
	//short bufLen = 0;
	short bytesRead = 0;
	/*short bytes = 0;*/
	short numScripts = 0;

	bytesRead = 0;
	numScripts = 0;
	us71ScriptLen =0;
	us72ScriptLen =0;
	memset(szScript71,0x00,sizeof(szScript71));
	memset(szScript72,0x00,sizeof(szScript72));

	bytesRead += iLen;

	tag = iType;
	LOG_PRINTF(("createScriptBuffers tag [%02X] LEN %d", tag,iLen));
	SVC_WAIT(200);

	if ((tag == 0x71) /*&& (bytes <= sizeof(data))*/)
	{
		LOG_PRINTF(("createScriptBuffers tag 71 found"));
		szScript71[us71ScriptLen++] = tag ;

		szScript71[us71ScriptLen++] =(byte)iLen;
		numScripts++;
		memcpy((char *)&szScript71[us71ScriptLen],(char *) scriptBuf, iLen);
		us71ScriptLen += iLen;
	}
	else if ((tag == 0x72) /*&& (bytes <= sizeof(data))*/)
	{
		szScript72[us72ScriptLen++] = tag ;

		szScript72[us72ScriptLen++] =(byte)iLen;
		numScripts++;
		LOG_PRINTF(("createScriptBuffers tag 72 found "));
		memcpy((char *)&szScript72[us72ScriptLen],(char *) scriptBuf, iLen); 
		us72ScriptLen += iLen;
	}
	//} while (bytesRead < bufLen);
	return (numScripts);
}


int emvUseHostDataFlow(char *hostRespID)
{
  int  iHostDecision=0;
	int iOut=-1;
	unsigned short tagLen = 0;
	unsigned int uiSecGen = 0;
  byte btCID = 0;//I                 // Cryptogram Identifier


  int iNumScripts = 0;

	//unsigned short usRetVal = EMV_SUCCESS;
	
	byte            tempBuffer[25] = {0};
	char            tempBufferStr[50] = {0};
	unsigned short  len = 0;
	

	srTxnResult srTVRTSI ;
	unsigned char ucAIP[255];
	unsigned	short usAIPLen = 0;
	unsigned char ucTermCap[255];
	unsigned	short usTermCapLen = 0;

	memset(ucAIP, 0x00, sizeof(ucAIP));
	memset(ucTermCap, 0x00, sizeof(ucTermCap));

	LOG_PRINTF(( "CHECK IN : emvUseHostDataFlow "));

	//IsTxnApproved() == true
		//Comparing the response buffer a check for success
		if((strncmp(hostRespID,"00",2) == 0) \
			|| (strncmp(hostRespID,"76",2) == 0) \
			|| (strncmp(hostRespID,"10",2) == 0))		//Success
		{
			iHostDecision = HOST_AUTHORISED;
		}
		else
		{
			iHostDecision = HOST_DECLINED;
		}

	//IsTxnDeclined() == true
		//iHostDecision = HOST_DECLINED;

	//aborted
		//iHostDecision = HOST_DECLINED;

	usEMVGetTxnStatus(&srTVRTSI); //Get TVR & TSI
	usEMVGetTLVFromColxn(TAG_8200_APPL_INTCHG_PROFILE, &ucAIP[0], &usAIPLen); // Get Application Interchange Profile (AIP)
	usEMVGetTLVFromColxn(TAG_9F33_TERM_CAP, &ucTermCap[0], &usTermCapLen); // Get Terminal Capabilities
	
	//Check for failed to connect, check if Offline Data Authentication not performed bit is set in TVR, check if CDA failed bit is not set in TVR, check if the terminal and the card support CDA
	if((iHostDecision == FAILED_TO_CONNECT) 
		&& ((srTVRTSI.stTVR[0] & 0x80) == 0x80) 
		&& ((srTVRTSI.stTVR[0] & 0x04) == 0x00) 
		&& (((ucTermCap[BYTE_2] & 0x08) == 0x08) 
		&& ((ucAIP[BYTE_0] & 0x01) == 0x01)))
	{
		//srTVRTSI.stTVR[0] &= 0x7F; //Reset the offline data authentication not performed bit
		srTVRTSI.stTVR[0] &= 0x7F; //change by amit as test case1 Reset the offline data authentication not performed bit
		usEMVSetTxnStatus(&srTVRTSI);
	}

	//Need to allocate memory to script results else will crash when passed to Toolkit
  LOG_PRINTF(( "CHECK 3 : emvUseHostDataFlow "));

  iNumScripts = iGlobalNumberScripts;
  //LOG_PRINTF(("CHECK 4 : emvUseHostDataFlow iNumScripts =%d", iNumScripts));
	//LOG_PRINTF(("CHECK 4 : emvUseHostDataFlow us71ScriptLen =%d", us71ScriptLen));
	
	//LOG_HEX_PRINTF("**scriptBuf 2ND GEN",szScript71,us71ScriptLen);//APPR

	pszGlobalIssuerScriptResults = (byte *) malloc(iNumScripts * 5);
  memset(pszGlobalIssuerScriptResults, 0x00, (iNumScripts * 5));
  LOG_PRINTF(( "CHECK 5 : emvUseHostDataFlow "));
	//This function obtains the data returned by the host after a transaction has gone
	//online and uses the data to perform the steps required to complete the
	//transaction.

	//memset(szScript71,0x00,sizeof(szScript71));
	//memcpy((byte *)szScript71,"\x71\x0F\x86\x0D\x84\x24\x00\x00\x08\xC8\x9A\xEE\x9F\xB7\x18\x92\x82",17);
	//LOG_PRINTF(("pszGlobalIssuerScriptResults=%d",pszGlobalIssuerScriptResults));
	LOG_HEX_PRINTF("TAG_ISSUER_SCRIPT_72",(byte *)szScript72,us72ScriptLen);
	LOG_HEX_PRINTF("TAG_ISSUER_SCRIPT_71",(byte *)szScript71,us71ScriptLen);



	//get_char();
	uiSecGen = inVXEMVAPUseHostData(iHostDecision, pszGlobalIssuerScriptResults, &iNumScripts,(byte *)szScript71,us71ScriptLen,(byte *)szScript72,us72ScriptLen);
	//uiSecGen = inVXEMVAPUseHostData(iHostDecision, pszGlobalIssuerScriptResults, &iNumScripts,(byte *)"\x71\x0F\x86\x0D\x84\x24\x00\x00\x08\xC8\x9A\xEE\x9F\xB7\x18\x92\x82",17,(byte *)szScript72,us72ScriptLen);
	
	LOG_PRINTF(("uiSecGen=%d",uiSecGen));

	if(0 != uiSecGen)
	{
		free((char*)pszGlobalIssuerScriptResults);	//Free the allocated memory.
		pszGlobalIssuerScriptResults = NULL;
			LOG_PRINTF(("Start emvUseHostDataFlow6"));
		return 3;
	}

	//Store the Issuer script results 
  if(pszGlobalIssuerScriptResults)
  {		
	    LOG_PRINTF(("CHECK 7 : emvUseHostDataFlow "));
      LOG_PRINTF(( "CHECK 7A : pszGlobalIssuerScriptResults [%02X][%02X][%02X][%02X][%02X]", pszGlobalIssuerScriptResults[0], pszGlobalIssuerScriptResults[1], pszGlobalIssuerScriptResults[2], pszGlobalIssuerScriptResults[3], pszGlobalIssuerScriptResults[4]));
      // LOG_PRINTF(("CHECK 7B : ISS_SCRIPT_RES[0] [%02X]", (unsigned short)ISS_SCRIPT_RES[0] ));
	
	    //amitesh ::for rupay Proprietary tag for issuer script result storage
	    usEMVAddTLVToCollxn(TAG_9F5B_ISSUER_SCRIPT_RESULTS , pszGlobalIssuerScriptResults, iNumScripts * 5);
	    LOG_PRINTF(("CHECK 7C : inEMVAPIUseHostDataFlow %d", iNumScripts * 5));
  }
    
	LOG_PRINTF(( "CHECK 8 : emvUseHostDataFlow "));
      free((char*)pszGlobalIssuerScriptResults);	//Free the allocated memory.
        
      //to store EMV data in batch
	usEMVGetTLVFromColxn(TAG_9F26_APPL_CRYPTOGRAM, (byte *) tempBuffer, &len);
	hex2asc((byte *) tempBufferStr, tempBuffer, len);
	LOG_PRINTF(("**************** CHECK 8 : tempBufferStr [%s] ", tempBufferStr));

    if(uiSecGen == SUCCESS)
    {
	      LOG_PRINTF(("CHECK 9 : emvUseHostDataFlow "));
        usEMVGetTLVFromColxn(TAG_9F27_CRYPT_INFO_DATA, &btCID, &tagLen);
        LOG_PRINTF(( "CHECK 9A : emvUseHostDataFlow btCID [%02x]", btCID));
	      switch(btCID & 0xC0)
        {
            case 0:			//Declined --- AAC
			        iOut=0;
			        LOG_PRINTF(( "CHECK 9A : Declined --- AAC "));
			        break;
            case 0x40:		//Offline Approval --- TC
			        iOut =1; 
			        LOG_PRINTF(("CHECK 9B :  Approval --- TC "));
              break;
        }
    }
    
    LOG_PRINTF(("CHECK OUT 10 : emvUseHostDataFlow"));
    return iOut;
}

/*--------------------------------------------------------------------------
    Function:		vdSelPreferredLang
    Description:	Function for fetch the language prefernces data from card
                   and taking input there are multiple languages 
    Parameters:	void
    Returns:		success or failure
	Notes:          
--------------------------------------------------------------------------*/
short vdSelPreferredLang (void)
{
  short          i = 0,j=0;
  unsigned short usRet = 0, usLen = 0;
  unsigned char  vucAux[30] = {0};
 
  short defaultLangFlag = 0;
  short defaultLangIndex = 0;
  char tempBuff[50]={0};
 
  
  LOG_PRINTF(("Inside vdSelPreferredLang"));
 
  usRet = usEMVGetTLVFromColxn (TAG_5F2D_LANG_PREFERENCE, vucAux, &usLen);

  if ((usRet == EMV_SUCCESS) && (usLen > 0) && (vucAux != NULL))
  {
      LOG_PRINTF(("Inside usLen = %d",usLen));
      LOG_PRINTF(("Inside vucAux = %s",vucAux));
      
      //Converts strings in to  Lowercase
      for (i=0; i<usLen ; i++)
 	      vucAux[i]=tolower(vucAux[i]);
      
      for (i = 0,j=0; i < usLen; i += 2,j++)
      {
          LOG_PRINTF(("Inside for  i= %d,j=%d",i,j ));
         
          if (memcmp (&vucAux[i], "en", 2) == 0)
          {
            defaultLangFlag = 1 ;
            
            defaultLangIndex = j ;
            LOG_PRINTF(("EngLish is supported" ));
          }
      }
  }
  if(defaultLangFlag ==1 && usLen ==2)
  {
    LOG_PRINTF(("EngLish is only lang supported" ));
  }
  else
  {
    usRet = LangMenuFunc((char*)vucAux,usLen);
    EnblTouchScreen(DisableTouch);
		
    LOG_PRINTF(("usRet=%d ",usRet));
  
    if (TRANS_CANCELLED != usRet)
    {
      if(defaultLangFlag == 1)
      {
        if(usRet == defaultLangIndex)
        {
       
        }
        else
        {
         
          sprintf(tempBuff,"%s","LANGUAGE IS NOT SUPPORTED");
        }   
      }
      else
      {
         
         sprintf(tempBuff,"%s","LANGUAGE IS NOT SUPPORTED");
         LOG_PRINTF(("text to display =%s ",tempBuff));
       
      }
    }
    else
    {
         return (TRANS_CANCELLED);
    }
  
    if(strcmp(tempBuff,"")!=0)
    {
      clrscr();
      window(1,1,30,20);
      write_at(tempBuff, strlen(tempBuff),4, 10);

      memset(tempBuff,0,sizeof(tempBuff));
      sprintf (tempBuff, "%s","PROCEEDING WITH ENGLISH");
      write_at(tempBuff, strlen(tempBuff),4, 12);
      SVC_WAIT(2000);
    }
  }
  return SUCCESS;
}
/*--------------------------------------------------------------------------
    Function:		LangMenuFunc
    Description:	Function for taking touch input for multiple languages
    Parameters:	char*	languages buffer pointer and int length of data
    Returns:		Input language index
	Notes:          
--------------------------------------------------------------------------*/

int LangMenuFunc(char *labels, int usLen)
{
     short  i = 0;
     short  j = 0;
     char   key;
     char tempBuff[50]={0};
    
    
     LOG_PRINTF(("usLen =%d",usLen));
     clrscr();
     display_at(7,2,"SELECT LANGUAGE",  NO_CLEAR);
     for (i = 0,j=0; i < usLen; i += 2,j++)
     {
          memset(tempBuff,0,sizeof(tempBuff));
          switch(j+1)
	        {
	        case 1:
		        memcpy(tempBuff,&labels[i],2);
            ModifyLangName(tempBuff);
            LOG_PRINTF(("Inside szSupportedlanguage1 = %s",tempBuff));
            display_at(((30-strlen(tempBuff))/2),  (((j+1) * 3) + 3), tempBuff, CLR_EOL);
		        break;
	        case 2:
		        memcpy(tempBuff,&labels[i],2);
            ModifyLangName(tempBuff);
            LOG_PRINTF(("Inside szSupportedlanguage2 = %s",tempBuff));
            display_at(((30-strlen(tempBuff))/2),  (((j+1) * 3) + 3), tempBuff, CLR_EOL);
		        break;
	        case 3:
		        memcpy(tempBuff,&labels[i],2);
            ModifyLangName(tempBuff);
            LOG_PRINTF(("Inside szSupportedlanguage3 = %s",tempBuff));
            display_at(((30-strlen(tempBuff))/2),  (((j+1) * 3) + 3), tempBuff, CLR_EOL);
		        break;
          case 4:
		        memcpy(tempBuff,&labels[i],2);
            ModifyLangName(tempBuff);
            LOG_PRINTF(("Inside szSupportedlanguage4 = %s",tempBuff));
            display_at(((30-strlen(tempBuff))/2),  (((j+1) * 3) + 3), tempBuff, CLR_EOL);
		        break;
	        default:
		        LOG_PRINTF(("Inside default case"));
		        break;
	        }
     }
     LOG_PRINTF(("j =%d",j));
     EnblMultiLangTouchScreen(j);
     while (1)
     {
          key = get_char();
          LOG_PRINTF(("key  == %d",key));

          if (key >= 1 && key <= 4) 
          {    
              LOG_PRINTF(("key >= 1 && key <= 4"));
                // Menu selection
                
              return(key);
          }
          else
                if (key == 27)
                    return (TRANS_CANCELLED);
                else
                    error_tone();
         
     }
   
}
