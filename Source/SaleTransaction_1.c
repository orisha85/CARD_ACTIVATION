/***********************************************************************************************
  Filename		     : SaleTransaction.c
  Project		       : Bevertec 
  Developer Neame  : Amar
  Module           : Sale transaction 
  Description	     : This is the implementation of Swipe or insert card and along with the sale 
                     transaction processing
 ***********************************************************************************************/




#include <svc_sec.h>
#include <string.h>
#include <aclconio.h>
#include "..\include\TouchScreen.h"
#include "..\Include\SaleTransaction.h"
#include "..\Include\ISo8583Map.h"
#include "..\Include\Common.h"
#include "..\Include\Bevertec.h"
#include "..\Include\BevertecClient.h"
#include "..\Include\ReciptPrint.h"
#include "..\Include\IPP_Key_injection.h"
#include "..\Include\Settlement.h"
#include "..\Include\Logon.h"
#include "..\include\BVTC_EMV.h"
#include <applidl.h>
#include <time.h>


#include <cardslot.h> // for terminate card slot 19-08-16
#include <vxemvap.h>
#include <iso8583.h>


extern short hClock;
extern short hMagReader;    // MagReader handle
extern char msg_id[MSG_ID_LEN+1];//5
extern char Proc_code[PROC_CODE_LEN+1];//7
BmapStruct BitmapStructOb ;

/****************************************************************************************************************
*	Function Name : readMagCard																																			*
*																																								*
*	Purpose				: To read card details from card swipe.											 														*																		*
																																				*
*****************************************************************************************************************/

short readMagCard(TransactionMsgStruc *transMsg)
{
	int ret=0; 
	char tempExpDate[EXPTIME_ARR_LEN+1]={0};//5
	short i= 0,j=0;
	char cardData[CARD_SIZE]={0}; //230
  char *ptr = NULL;
  const char delim[2] = "=";
  char track2data[TRACK2_DATA_LEN+1]={0};//stores the track2 data for extracting the PAN
  char exp [EXPTIME_ARR_LEN+1] ={0};
  
	if( hMagReader == -1)
	{
			hMagReader = open (DEV_CARD, 0);
      if(LOG_STATUS == LOG_ENABLE)
      {
			  LOG_PRINTF (("megnatic reader open = %d",hMagReader));	
      }
	}
	//Read card details from mag card reader
	ret = read (hMagReader, cardData, sizeof (cardData));
  if(LOG_STATUS == LOG_ENABLE)
  {
	  LOG_PRINTF (("card read return = %d",ret));
  }
	if(ret < 0) 
	{
		clrscr ();
		 
		write_at(Dmsg[CAN_NOT_READ_CARD_DATA].dispMsg, strlen(Dmsg[CAN_NOT_READ_CARD_DATA].dispMsg),Dmsg[CAN_NOT_READ_CARD_DATA].x_cor, Dmsg[CAN_NOT_READ_CARD_DATA].y_cor);
    
		error_tone();
		SVC_WAIT(150);
		error_tone();
		SVC_WAIT(3000);
		ClearKbdBuff();
		KBD_FLUSH();
		resetMsgDetails(transMsg);
		return _FAIL;
	}

	ret = card_parse (cardData, &transMsg->CardDetails, "1");//for track 1
  if(LOG_STATUS == LOG_ENABLE)
  {
	  LOG_PRINTF (("card parse return = %d",ret));	
  }
	if(ret != 1) 
	{
    if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF (("track 1 not found",ret));
    }
	}
  else
  {
	  sprintf(transMsg->CardHldrName,"%s",&transMsg->CardDetails.name);
  }

	//parse trake 2 data the card data
	ret = card_parse (cardData, &transMsg->CardDetails, "2");//for trake 2
	if(LOG_STATUS == LOG_ENABLE)
  {
    LOG_PRINTF (("card parse return = %d",ret));	
  }
	if(ret != 2) 
	{
		clrscr ();
    write_at(Dmsg[ERROR_IN_CARD_PARSING].dispMsg, strlen(Dmsg[ERROR_IN_CARD_PARSING].dispMsg),Dmsg[ERROR_IN_CARD_PARSING].x_cor, Dmsg[ERROR_IN_CARD_PARSING].y_cor);
		
		error_tone();
		SVC_WAIT(1000);
		ClearKbdBuff();
		KBD_FLUSH();
		resetMsgDetails(transMsg);
		return _FAIL;
	}

 // strncpy((char *)transMsg->Track2Data,TRACK2_DATA/*transMsg->CardDetails.track*/,strlen(TRACK2_DATA)/*strlen(transMsg->CardDetails.track)*/);        //35 Hardcoded Track2 data 
  strncpy((char *)transMsg->Track2Data,transMsg->CardDetails.track,strlen(transMsg->CardDetails.track));        //35 for dynamic mag swipe
 // strncpy(track2data,TRACK2_DATA,strlen(TRACK2_DATA));        //35 Sample Track 2 Data provided by Client 
  strncpy(track2data,transMsg->CardDetails.track,strlen(transMsg->CardDetails.track));        //35 Sample Track 2 Data provided by Client 
  if(LOG_STATUS == LOG_ENABLE)
  {
	  LOG_PRINTF (("trake 2 data = %s",transMsg->Track2Data));
    LOG_PRINTF (("trake 2 data = %s",transMsg->CardDetails.track));
  }
	ptr = strstr(track2data,delim);
  for(i =0 ;i<4;i++)//fetching expiry date from track 2 data
  {
    exp[i] = ptr [i+1];
  }
 
  strtok((char*)track2data, delim);//extracting account no from track2 data
  if(LOG_STATUS == LOG_ENABLE)
  {
    LOG_PRINTF (("track2data->PrimaryAccNum = %s",track2data));
  }
	
  strncpy(transMsg->PrimaryAccNum,track2data,strlen(track2data));  
  if(LOG_STATUS == LOG_ENABLE)
  {
    LOG_PRINTF (("PrimaryAccNum = %s",transMsg->PrimaryAccNum));
  }
	
	//formating expiery date data in card swipe 
	for(i=2,j=0;i<4;i++,j++)
	{
			tempExpDate[j]= exp[i];
	}
	for(i=0,j=2;i<2;i++,j++)
	{
			tempExpDate[j]= exp[i];
	}
	strcpy(transMsg->CardDetails.exp,tempExpDate);
	sprintf(transMsg->ExpiryDate,"%s",&transMsg->CardDetails.exp);
	if(LOG_STATUS == LOG_ENABLE)
  {
    LOG_PRINTF (("ExpiryDate = %s",transMsg->ExpiryDate));	
	  LOG_PRINTF (("CardHldrName = %s",transMsg->CardHldrName));
  }
	window(1,1,30,20);
	clrscr();
	
	write_at(Dmsg[PROCESSING].dispMsg, strlen(Dmsg[PROCESSING].dispMsg),Dmsg[PROCESSING].x_cor, Dmsg[PROCESSING].y_cor);

	return _SUCCESS;
}


/****************************************************************************************************************
*	Function Name : getCardNumber																																			*
*																																								*
*	Purpose				: Getting customer card number through manual entry											 														*																		*
																																				*
*****************************************************************************************************************/

short getCardNumber(TransactionMsgStruc *transMsg,char getKey)
{
	int ret = 0;
	char strCardNo [20] = {0};			// to get the card no
	char tempGetKey[2]={0};
	sprintf(tempGetKey,"%c",getKey);
	
	clrscr();
	display_at (1, 10, "Enter Card Number:", CLR_EOL);
	display_at (1, 11, tempGetKey, CLR_EOL);
	window(2,11,19,11);
	
	//Getting customer card number through manual entry
	ret = getkbd_entry (hClock, "",(signed char *)strCardNo, (unsigned int) 1200,
		(unsigned int) NUMERIC, (char*) szKeyMap,sizeof(szKeyMap),18, 16);
	if(LOG_STATUS == LOG_ENABLE)
  {
    LOG_PRINTF(("card number return = %d", ret));
  }
	if(ret == -3)
	{
		return KEY_CANCEL;
	}
	else if(ret == 0)
	{
		window(1,1,30,20);
		clrscr();
		error_tone();
		SVC_WAIT(190);
		error_tone();
		display_at(10, 10, TIME_OUT, CLR_LINE);
    if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF(("TIME OUT"));
    }
		SVC_WAIT(1000);
		ClearKbdBuff();
		KBD_FLUSH();
		resetMsgDetails(transMsg);
		return _FAIL;
	}

	sprintf(transMsg->PrimaryAccNum,"%c%s",getKey,strCardNo);
  if(LOG_STATUS == LOG_ENABLE)
  {
	  LOG_PRINTF (("PrimaryAccNum = %s",transMsg->PrimaryAccNum));	
  }
		
	return _SUCCESS;
}

/****************************************************************************************************************
*	Function Name : getExpiryDate																																			*
*																																								*
*	Purpose				: To get card Expeiry Date from mannual entry						 														*																		*
																																				*
*****************************************************************************************************************/


int getExpiryDate(TransactionMsgStruc *transMsg)
{
	int ret=0;
	char expDate [EXPTIME_ARR_LEN+1] = {0};		//5- to get Exp date
	int hClock = 0;
	
	clrscr();
	window(1,10,22,10);
	clrscr();
  
	write_at(Dmsg[CARD_EXPIRY_DATE].dispMsg, strlen(Dmsg[CARD_EXPIRY_DATE].dispMsg),Dmsg[CARD_EXPIRY_DATE].x_cor, Dmsg[CARD_EXPIRY_DATE].y_cor);

   
	
  write_at(Dmsg[MMYY].dispMsg, strlen(Dmsg[MMYY].dispMsg),Dmsg[MMYY].x_cor, Dmsg[MMYY].y_cor);
	window(Dmsg[MMYY].x_cor,Dmsg[MMYY].y_cor,Dmsg[MMYY].x_cor+4,Dmsg[MMYY].y_cor);
	
	//Getting Expiry Date through manual entry
	ret=getkbd_entry (hClock, "",(signed char *)expDate, (unsigned int) 12000,(unsigned int) NUMERIC, (char*) szKeyMap,sizeof(szKeyMap),4, 4);

	if(ret == BTN_CANCEL)
	{
				resetMsgDetails(transMsg);
				return KEY_CANCEL;
	}
	else if ( ret == 0)
	{
		// Time Out
		resetMsgDetails(transMsg);
		return _FAIL;
	}
	sprintf(transMsg->ExpiryDate,"%s",expDate);
  if(LOG_STATUS == LOG_ENABLE)
  {
	  LOG_PRINTF (("ExpiryDate = %s",transMsg->ExpiryDate));	
  }

	return _SUCCESS;
}


/****************************************************************************************************************
*	Function Name : swipeOrEMVCardEntry																													              *
*	Purpose		  : To get card details from swipe or mannual entry						 												              *
*	Input		  :  Address of main transaction message structure 												                              *
*	Output		  : Return value for the success or failure																						              *
*****************************************************************************************************************/

int swipeOrEMVCardEntry(TransactionMsgStruc *transMsg)
{
	int ret = 0;
	short flag = 1;
	char getKey = 0;
	int mask = 0;
	char track2data[TRACK2_DATA_LEN+1]={0};//stores the track2 data for extracting the PAN
	time_t initial_time = time(NULL);
	char timeLimit[TIME_ARR_LEN]={0}; //4
  	short time_limit;

	if( hMagReader == -1)
	{
		hMagReader = open (DEV_CARD, 0);
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF (("megnatic reader open = %d",hMagReader));	
		}
	}
	get_env("#TIME_OUT_LIMIT_IN_SECOND",timeLimit,sizeof(timeLimit));
	time_limit = atoi(timeLimit);
	window(1,1,30,20);
	clrscr ();
#ifdef SUP_CARD
	write_at("SUPERVISOR CARD",15,7,10);
	LOG_PRINTF(("SUPERVISOR CARD"));
#endif
	write_at(Dmsg[SWIPE_ENTER_CARD].dispMsg, strlen(Dmsg[SWIPE_ENTER_CARD].dispMsg),Dmsg[SWIPE_ENTER_CARD].x_cor, Dmsg[SWIPE_ENTER_CARD].y_cor);
  	do
	{
		ClearKbdBuff();
		KBD_FLUSH();
		mask = read_event();//
		mask = wait_event();//waiting for event
		if(mask & EVT_MAG)
		{
			flag = 1;
		}
		else if(mask & EVT_KBD) //checking for keyboard event
		{
				flag = 2;
				getKey=get_char();
    
				break;
		}
    else if(mask & EVT_ICC1_INS) //checking for EMV event
    {
 		  if(LOG_STATUS == LOG_ENABLE)
      {
        LOG_PRINTF (("EMV CARD..."));//22 POS Entry Mode
      }
      ret   = inRunEMVTransaction(transMsg->TrTypeFlag);
      if(ret == _SUCCESS)
      {
        	flag = 4;
          
          if(LOG_STATUS == LOG_ENABLE)
          {
            LOG_PRINTF (("inRunEMVTransaction Success!!"));//22 POS Entry Mode
          }
      }
      else if(ret != FALLBACK)
      {
        if(LOG_STATUS == LOG_ENABLE)
        {
		      LOG_PRINTF (("After emvFirstGenerationFlow fail ret =%d",ret));
        }
        CloseEMVSLot();
        return _FAIL;
      }
      else
      {
        
        if(LOG_STATUS == LOG_ENABLE)
        {
          LOG_PRINTF (("FALLBACK occurred..."));//22 POS Entry Mode
        }
        CloseEMVSLot();
        return ret;
      }
			break;
    }
		else
		{
			flag = 3;
			if(LOG_STATUS == LOG_ENABLE)
      {
			  LOG_PRINTF (("swipe/enter.."));
      }
		}

	}while (flag == 3 && (time(NULL) - initial_time) < time_limit);
  if(LOG_STATUS == LOG_ENABLE)
  {
	  LOG_PRINTF (("Stat Val = %d",flag));	
  }
  if(flag == 3)
	{
		clrscr();
		error_tone();
		SVC_WAIT(190);
		error_tone();
		write_at(TIME_OUT,strlen(TIME_OUT),(30-strlen(TIME_OUT))/2,10);
		SVC_WAIT(1000);
		ClearKbdBuff();
		KBD_FLUSH();
		resetMsgDetails(transMsg);
		return _FAIL;
	}
	else if(flag == 1)
	{
		//Read card details from mag card reader
    if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF (("in Megnatic Card"));
    }
    strcpy((char *)transMsg->POSEntryMode,"021");             //22 POS Entry Mode
    ret = readMagCard(transMsg);
		if(ret == _FAIL)
		{
			return _FAIL;
		}
    strcpy((char *)track2data,transMsg->Track2Data);             //22 POS Entry Mode
	  ret =  CheckServiceCode(track2data);
    if(ret != _SUCCESS)
		{
			return  FORCE_FOR_EMV;
		}
 
	}
	else if(flag == 2)
	{
		//Getting customer card number through manual entry
		if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF (("in KeyBord Entry getKey = %d",getKey));
    }
		if((getKey == KEY_CANCEL) ||(getKey == KEY_STR ) || (getKey >= 48 && getKey <=57 ) || (getKey == 8) || (getKey == 13))//if user press cancle key
		return _FAIL;
   
	}
  else if(flag == 4)
	{
		//Read card details from mag card reader
    if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF (("Going To call emvFirstGenerationFlow"));
    }
    
		ret = emvFirstGenerationFlow(transMsg);
    if(ret == _SUCCESS)
    {
      transMsg->EMV_Flag = 1 ;
      strcpy((char *)transMsg->POSEntryMode,"051");             //22 POS Entry Mode
    }
    if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF (("After emvFirstGenerationFlow ret =%d",ret));
    }
    CloseEMVSLot();
		return ret;
	}
	return _SUCCESS;
}


/****************************************************************************************************************
*	Function Name : GetInputFromUser									                                                            *										                                              *
*	Purpose		    : Taking input from user via keyboard 		 					                                            *
*	Input		      : Address of main transaction message structure 												                                                                      *
*	Output		    : returns success or failure 								                                                    *
*****************************************************************************************************************/

int GetInputFromUser(TransactionMsgStruc *transMsg,int amountType)
{
	char amount[AMOUNT_LEN+1]={0};//13  - buffer which stores AMOUNT keyboard entry from user
	int ret = -1 ; 
	char timeLimit[TIME_ARR_LEN]={0};

	short time_limit=0;
  EnblTouchScreen(DisableTouch);
	get_env("#TIME_OUT_LIMIT_IN_SECOND",timeLimit,sizeof(timeLimit));
	
	time_limit = atoi(timeLimit);
	time_limit = time_limit*100;
	do
	{
			memset(amount,0,sizeof(amount));
			
			window(1,1,30,20);
			clrscr();
			if(amountType == TRANSAMOUNT)
			{
				 write_at(Dmsg[ENTER_AMOUNT].dispMsg, strlen(Dmsg[ENTER_AMOUNT].dispMsg),Dmsg[ENTER_AMOUNT].x_cor, Dmsg[ENTER_AMOUNT].y_cor);
				window(19,Dmsg[ENTER_AMOUNT].y_cor,29,Dmsg[ENTER_AMOUNT].y_cor);
			}
			else if(amountType == CASHBACK_AMOUNT)
			{
				write_at(Dmsg[ENTER_CASHBACK_AMOUNT].dispMsg, strlen(Dmsg[ENTER_CASHBACK_AMOUNT].dispMsg),Dmsg[ENTER_CASHBACK_AMOUNT].x_cor, Dmsg[ENTER_CASHBACK_AMOUNT].y_cor);
       			window(18,Dmsg[ENTER_CASHBACK_AMOUNT].y_cor+1,28,Dmsg[ENTER_CASHBACK_AMOUNT].y_cor+1);
			}
			//function for taking keyboard entry into buffer
			ret = getkbd_entry (hClock, "",(signed char *) amount, (unsigned int)time_limit,(unsigned int)AMOUNT , (char*)szKeyMap,sizeof(szKeyMap),2,MAX_AMT_DIGIT,000);
			if(LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("GetInputFromUser return %d",ret));
			}
			if(ret == BTN_CANCEL)
			{
				return KEY_CANCEL;
			}
			else if(ret == 0)
			{
				// Time Out
				window(1,1,30,20);
				clrscr();
				error_tone();
				SVC_WAIT(190);
				error_tone();
				write_at(TIME_OUT,strlen(TIME_OUT),(30-strlen(TIME_OUT))/2,10);
				SVC_WAIT(1000);
				ClearKbdBuff();
				KBD_FLUSH();
				resetMsgDetails(transMsg);
				return _FAIL;
			}

			window(1,1,21,8);
			clrscr();
			if(LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF (("Amount taken = %s",amount));
			}
		}while(!strcmp(amount,"0.00"));
	

	if(amountType == TRANSAMOUNT)
	{
			strncpy(transMsg->Amount,amount,AMOUNT_LEN);
      if(LOG_STATUS == LOG_ENABLE)
      {
			  LOG_PRINTF (("Amount in & = %s",transMsg->Amount));
      }
	}
	

	return _SUCCESS;
}

/****************************************************************************************************************
*	Function Name : getTransNo									                                                                *
*	Purpose		    : Taking input from user via keyboard for transaction no		                                  *
*	Input		      : short										                                                                      *
*	Output		    : returns success or failure 								                                                    *
*****************************************************************************************************************/
short getTransNo(TransactionMsgStruc *transMsg)
{
	char TransRef[TRANSACTION_ID_LEN+1]={0};//BUFFER TO STORE TRANS REF ID
	short iretVal=0;
	char timeLimit[TIME_ARR_LEN]={0};//4
    short time_limit=0;
    
	EnblTouchScreen(DisableTouch);
	get_env("#TIME_OUT_LIMIT_IN_SECOND",timeLimit,sizeof(timeLimit));
	time_limit = atoi(timeLimit);
	time_limit = time_limit*100;
	window(1,1,30,20);
	clrscr();
	write_at(Dmsg[ENTER_TRANS_NO].dispMsg, strlen(Dmsg[ENTER_TRANS_NO].dispMsg),Dmsg[ENTER_TRANS_NO].x_cor, Dmsg[ENTER_TRANS_NO].y_cor);
    window(8,Dmsg[ENTER_TRANS_NO].y_cor+2,20,Dmsg[ENTER_TRANS_NO].y_cor+2);
	iretVal=getkbd_entry (hClock, "",(signed char *)TransRef, (unsigned int) time_limit,(unsigned int) NUMERIC, (char*) szKeyMap,sizeof(szKeyMap),12, 12);
	if(LOG_STATUS == LOG_ENABLE)
    {
        LOG_PRINTF(("getPaymentId RETURN =  %d",iretVal));
    }
	if(iretVal == BTN_CANCEL)
	{
		return KEY_CANCEL;
	}
		else if(iretVal == 0)
	{
				// Time Out
		window(1,1,30,20);
		clrscr();
		error_tone();
		SVC_WAIT(190);
		error_tone();
		write_at(TIME_OUT,strlen(TIME_OUT),(30-strlen(TIME_OUT))/2,10);
				//display_at(6, 4, TIME_OUT, CLR_LINE);
		SVC_WAIT(1000);
		ClearKbdBuff();
		KBD_FLUSH();
		resetMsgDetails(transMsg);
		return _FAIL;
	}
		strncpy(transMsg->RetrievalReferenceNo,TransRef,RETRIEVAL_REF_LEN);//COPYING trace audit number to RetrievalReferenceNo for sending purpus
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("TransRef  = %s",transMsg->RetrievalReferenceNo));
		}
	return _SUCCESS;
}
/****************************************************************************************************************
*	Function Name : getPaymentId									                                                                *
*	Purpose		    : Taking input from user via keyboard for payment id 					                                  *
*	Input		      : short										                                                                      *
*	Output		    : returns success or failure 								                                                    *
*****************************************************************************************************************/
short getPaymentId(TransactionMsgStruc *transMsg)
{
		char payment_Id[PAYMENT_ID_LEN+1]={0};//10 -BUFFER TO STORE PAYMNET ID FOR GOV APP
		short iretVal=0;
		
    char timeLimit[TIME_ARR_LEN]={0}; //4
    short time_limit=0;
    
    EnblTouchScreen(DisableTouch);
		
    get_env("#TIME_OUT_LIMIT_IN_SECOND",timeLimit,sizeof(timeLimit));
		window(1,1,30,20);
		clrscr();

    time_limit = atoi(timeLimit);
	  time_limit = time_limit*100;
   
    write_at(Dmsg[ENTER_PAYMENT_ID].dispMsg, strlen(Dmsg[ENTER_PAYMENT_ID].dispMsg),Dmsg[ENTER_PAYMENT_ID].x_cor, Dmsg[ENTER_PAYMENT_ID].y_cor);  
    
    window(10,Dmsg[ENTER_PAYMENT_ID].y_cor+2,19,Dmsg[ENTER_PAYMENT_ID].y_cor+2);
	
	 
		
    iretVal=getkbd_entry (hClock, "",(signed char *)payment_Id, (unsigned int) time_limit,(unsigned int) NUMERIC, (char*) szKeyMap,sizeof(szKeyMap),9, 9);
		if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("getPaymentId RETURN =  %d",iretVal));
    }
		if(iretVal == BTN_CANCEL)
		{
			return KEY_CANCEL;
		}
  
	
		strncpy(transMsg->PaymentId,payment_Id,PAYMENT_ID_LEN);
	//	strcpy(transMsg->FromAcID,transMsg->PaymentId);		//Agency Bank
    if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF(("pAYMENT ID = %s",transMsg->PaymentId));
	//	  LOG_PRINTF(("From Account = %s",transMsg->FromAcID));		//Agency Bank
    }
		return _SUCCESS;
}

/************************************Agency Bank get Account ***************************************************/
short getAccountNo(TransactionMsgStruc *transMsg)
{

		//char payment_Id[PAYMENT_ID_LEN+1]={0};//10 -BUFFER TO STORE PAYMNET ID FOR GOV APP
		char account_No[ACCOUNT_NO_LEN+1]={0};//12+1 -BUFFER TO STORE ACCOUNT NO FOR AGENCY BANK
		short iretVal=0;
		
    char timeLimit[TIME_ARR_LEN]={0}; //4
    short time_limit=0;
	LOG_PRINTF(("===============getAccountNo============="));    
	LOG_PRINTF(("===============AC_TYPE=%c =============",AC_TYPE));
    EnblTouchScreen(DisableTouch);
		
    get_env("#TIME_OUT_LIMIT_IN_SECOND",timeLimit,sizeof(timeLimit));
		window(1,1,30,20);
		clrscr();

    time_limit = atoi(timeLimit);
	  time_limit = time_limit*100;
   
	  if(AC_TYPE=='T')
	  {
		  write_at(Dmsg[ENTER_TO_ACCOUNT].dispMsg, strlen(Dmsg[ENTER_TO_ACCOUNT].dispMsg),Dmsg[ENTER_TO_ACCOUNT].x_cor, Dmsg[ENTER_TO_ACCOUNT].y_cor);  
    window(10,Dmsg[ENTER_TO_ACCOUNT].y_cor+2,21,Dmsg[ENTER_TO_ACCOUNT].y_cor+2);
	  }
	  else
	  {
		write_at(Dmsg[ENTER_FROM_ACCOUNT].dispMsg, strlen(Dmsg[ENTER_FROM_ACCOUNT].dispMsg),Dmsg[ENTER_FROM_ACCOUNT].x_cor, Dmsg[ENTER_FROM_ACCOUNT].y_cor);  
		window(10,Dmsg[ENTER_FROM_ACCOUNT].y_cor+2,21,Dmsg[ENTER_FROM_ACCOUNT].y_cor+2);
	
	  }
	 
	 iretVal=getkbd_entry (hClock, "",(signed char *)account_No, (unsigned int) time_limit,(unsigned int) NUMERIC, (char*) szKeyMap,sizeof(szKeyMap),12, 12);
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("getAccountNo RETURN =  %d",iretVal));
		}
		if(iretVal == BTN_CANCEL)
		{
			return KEY_CANCEL;
		}
		
  if(AC_TYPE=='T')
  {
	  strncpy(transMsg->ToAcNo,account_No,ACCOUNT_NO_LEN);
  }
  else
  {
	  strncpy(transMsg->FromAcNo,account_No,ACCOUNT_NO_LEN);
  }
	//	strncpy(transMsg->PaymentId,payment_Id,PAYMENT_ID_LEN);
		//strncpy(transMsg->FromAcNo,account_No,ACCOUNT_NO_LEN);
		//strcpy(transMsg->FromAcID,transMsg->PaymentId);		//Agency Bank
    if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF(("From Account No [%s] - To Account No [%s]",transMsg->FromAcNo,transMsg->ToAcNo));
	//	  LOG_PRINTF(("From Account = %s",transMsg->FromAcID));		//Agency Bank
    }
		return _SUCCESS;
}
/****************************************************************/

/****************************************************************************************************************
*	Function Name : packData									                                                                    *
*	Purpose		    : it compress the 2 byte data to 1 byte 					                                  *
*	Input		      : short										                                                                      *
*	Output		    : returns success or failure 			                                    *
*****************************************************************************************************************/
int packData(char *testch1,char *ch)
{
    unsigned int i=0;
    int ret ;
    char testch[2]={0};
    
    char *ptr1 = testch1 ;
   
    for(i = 0;  i < strlen(testch1) ; i+= 2 )
    {
       strncpy(testch,(char*)ptr1,1);
       ret = (int)strtol(testch, NULL, 16) ;
      
       ptr1++;
       ch[i/2]  = ret ;
      
       ch[i/2]  = ch[i/2] << 4 ;
       
       memset(testch,0,sizeof(testch));
       strncpy(testch,(char*)ptr1,1);
       ret = (int)strtol(testch, NULL, 16) ;
      
       ptr1++;
       ch[i/2] |= ret ;
       
    }   
    return 0;
}

/****************************************************************************************************************
*	Function Name : PaymentTransactionProcessing									                                                *										                                              *
*	Purpose		    : create request stream,sets bitmap ,communication and parse response	                          *
*	Input		      : TransactionMsgStruc structure pointer										                                      *
*	Output		    : returns success or failure 								                                                    *
*****************************************************************************************************************/

short PaymentTransactionProcessing(TransactionMsgStruc *transMsg)
{
	int i =0; 
//#define BITMAP_SIZE 128
//#define BITMAP_ARRAY_SIZE 16
	short BITMAP_LEN=8;
	char Bitmap[BITMAP_ARRAY_SIZE+1] ={0};//13 -Bitmap for Payment response  
	char UnpackBiitmap[UNPACK_BITMAP_ARRAY_SIZE+1]={0}; //17 - UnpackBiitmap response for payment
	char bits[BITMAP_SIZE+1] ={0}; // 65 
	char temp[BITMAP_ARRAY_SIZE+1] ={0}; //17
	short retVal = -1;  
	short ReqLen = 0; 
  //unsigned char reqBuff[PAYMENT_REQUEST_SIZE]={0};//96 -REQUEST BUFFER
	unsigned char reqBuff[PAYMENT_REQUEST_SIZE+ICC_RELATED_DATA_LEN+2]={0};//96 -REQUEST BUFFER
	unsigned char resBuff[PAYMENT_RESPONSE_SIZE]={0}; //100 RESPONSE BUFFER
	unsigned char testBuff[PAYMENT_UNAPACK_RES_SIZE]={0}; //200 PAYMENT UNPACK RESPONSE BUFFER
	short recvLength = 0;
  
  //Setting filed table for payment transaction
  field_struct iso8583_field_table[] = 
  { 
    /*Fld 8583 Convert Variable name and size no.	sz 	index *//*BCD_BCD,BCD_STR*/ //ASC_ASC->
    { 0, TPDU_LEN,BCD_STR,(void *) (BitmapStructOb.tpdu), sizeof(BitmapStructOb.tpdu) }, 
    { 0, MSG_ID_LEN, BCD_ASC, (void *) BitmapStructOb.message_id, sizeof( BitmapStructOb.message_id) },
    { 2+ SKIP, 19, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 3, 6, BCD_STR, (void *) BitmapStructOb.field_03, sizeof( BitmapStructOb.field_03) }, 
    { 4, 12, BCD_STR, (void *) BitmapStructOb.field_04, sizeof( BitmapStructOb.field_04) }, 
    { 5+ SKIP, 12, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 6+ SKIP, 12, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 7+ SKIP, 10, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 8+ SKIP, 8, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 9+ SKIP, 8, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 10+ SKIP, 8, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 11, 6, BCD_STR, (void *) BitmapStructOb.field_11, sizeof( BitmapStructOb.field_11) }, 
    { 12, 6, BCD_STR, (void *) BitmapStructOb.field_12, sizeof( BitmapStructOb.field_12) }, 
    { 13, 4, BCD_STR, (void *) BitmapStructOb.field_13, sizeof( BitmapStructOb.field_13) }, 
    { 14+ SKIP, 4, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 15+ SKIP, 4, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 16+ SKIP, 4, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 17+ SKIP, 4, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 18+ SKIP, 4, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 19+ SKIP, 3, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 20+ SKIP, 3, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 21+ SKIP, 3, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 22, 3, BCD_STR, (void *) BitmapStructOb.field_22, sizeof( BitmapStructOb.field_22) }, 
    { 23+ SKIP, 3, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 24, 3, BCD_STR, (void *) BitmapStructOb.field_24, sizeof( BitmapStructOb.field_24) }, 
    { 25, 2, BCD_STR, (void *) BitmapStructOb.field_25, sizeof( BitmapStructOb.field_25) }, 
    { 26+ SKIP, 2, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 27+ SKIP, 1, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 28+ SKIP, 8, XBC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 29+ SKIP, 8, XBC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 30+ SKIP, 8, XBC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 31+ SKIP, 8, XBC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 32+ SKIP, 11, BV2_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 33+ SKIP, 11, BV2_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 34+ SKIP, 28, AV2_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 35, 37, BV2_STR, (void *) BitmapStructOb.field_35, sizeof( BitmapStructOb.field_35) }, 
    { 36+ SKIP, 104, AV2_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 37, 12, BCD_STR, (void *) BitmapStructOb.field_37, sizeof( BitmapStructOb.field_37) }, 
    { 38, 6, BCD_STR, (void *) BitmapStructOb.field_38, sizeof( BitmapStructOb.field_38) }, 
    { 39, 2, BCD_STR, (void *) BitmapStructOb.field_39, sizeof( BitmapStructOb.field_39) }, 
    { 40+ SKIP, 3, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 41, 8, ASC_STR, (void *) BitmapStructOb.field_41, sizeof( BitmapStructOb.field_41) }, 
    { 42, 15, ASC_STR, (void *) BitmapStructOb.field_42, sizeof( BitmapStructOb.field_42) }, 
    { 43+ SKIP, 40, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 44+ SKIP, 25, AV2_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 45+SKIP, 76, AV2_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, //Skippping because using track 2 data(bit-35)
    { 46+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 47+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 48+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
	//{ 48, 9, AV3_STR, (void *) BitmapStructOb.field_48, sizeof( BitmapStructOb.field_48) }, 
	{ 49+ SKIP, 3, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 50+ SKIP, 3, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 51+ SKIP, 3, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 52, 64, BIT_BIT, (void *) BitmapStructOb.field_52, sizeof( BitmapStructOb.field_52) }, 
    { 53+ SKIP, 16, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 54+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.field_54, sizeof( BitmapStructOb.field_54) }, 
    { 55+ SKIP, 0, BIT_BIT, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 56+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 57+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 58+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 59+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 60+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 61+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 62+ SKIP, 999, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 63+ SKIP, 999, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
	{ 64+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 65+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 66+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 67+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 68+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 69+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 70+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 71+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 72+ SKIP, 999, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 73+ SKIP, 999, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
	{ 74+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 75+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 76+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 77+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 78+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 79+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
	{ 80+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 81+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 82+ SKIP, 999, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 83+ SKIP, 999, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
	{ 84+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 85+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 86+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 87+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 88+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 89+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
	{ 90+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 91+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 92+ SKIP, 999, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 93+ SKIP, 999, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
	{ 94+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 95+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 96+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 97+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 98+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 99+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
	{ 100+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 101+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
	{ 102, 12, AV2_STR, (void *) BitmapStructOb.field_102, sizeof( BitmapStructOb.field_102) }, //Agency Bank
	{ 103+ SKIP +STOP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
};

	strcpy(msg_id,SALEMSGTYPE);
	strcpy(Proc_code,transMsg->ProcessingCode);
	if(LOG_STATUS == LOG_ENABLE)
	{
		LOG_PRINTF (("->>>>>>>>>>processing code =%s",transMsg->ProcessingCode));
		LOG_PRINTF (("->>>>>>>>>>FROM Account =%s",transMsg->FromAcNo));
	}
	strcpy(transMsg->MessageTypeInd,SALEMSGTYPE);
	resetBitmapStructure(&BitmapStructOb);//memsetting all the elements of structure

	ProcessingISOBitmapEngine(iso8583_field_table, transMsg);
	ReqLen = retVal = assemble_packet(iso8583_field_table,reqBuff);
 
	if(retVal <= _SUCCESS)
	{   
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Error Occurred in assemble_packet retVal =  %d\n",retVal));
		}
		return retVal;
	}
	else
	{
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF((" assemble_packet retVal = %d ReqLen =%d\n",retVal,ReqLen));
		}
	}
	recvLength = DEFAULT_BUFLEN ;
	retVal = CommWithServer((char*)reqBuff,ReqLen,(char*)resBuff,&recvLength);
	//retVal = _FAIL ; //for checking assembled request packet
	//retVal = RECV_FAILED ;//for checking reversal
	if(retVal == _SUCCESS )//checking for response buffer size 
	{
		bcd2a((char *)testBuff,(char *)resBuff,recvLength);
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Testbuffin disassemblePacket retVal = %s \n",testBuff));
		}
		if(testBuff[18]>='8')
		BITMAP_LEN=16;
			
		strncpy(UnpackBiitmap,(char*)testBuff+UNPACK_SIZE_MSGLEN_OF_REQ_RES+TPDU_LEN+MSG_ID_LEN,BITMAP_LEN*2);
		LOG_PRINTF(("==============UnpackBiitmap = %s \n",UnpackBiitmap));
		packData(UnpackBiitmap,Bitmap);
		LOG_PRINTF(("==============UnpackBiitmap = %s \n",Bitmap));
		for( i=0 ;i < BITMAP_LEN ; i++)
		{
			CopyAllBits(Bitmap+i,temp);
			strcpy(bits+(i*8),temp);
		}
		resetBitmapStructure(&BitmapStructOb);//memsetting all the elements of structure
		ParseAndFillBitFields((char*)testBuff,bits);
		LOG_PRINTF(("==============After Parsing "));
		LOG_PRINTF(("Response Code : %s \n",BitmapStructOb.field_39));
		strcpy((char *)transMsg->ResponseCode,(char*)BitmapStructOb.field_39);             //copy the response code
		strcpy((char *)transMsg->AuthIdResponse,(char*)BitmapStructOb.field_38);           //copy AuthIdResponse
		strcpy((char *)transMsg->RetrievalReferenceNo,(char*)BitmapStructOb.field_37);     //copy RetrievalReferenceNo
		strcpy((char *)transMsg->TransLocalTime,(char*)BitmapStructOb.field_12);           //copy AuthIdResponse
		strcpy((char *)transMsg->TransLocalDate,(char*)BitmapStructOb.field_13);     //copy RetrievalReferenceNo
		window(1,1,30,20);
		clrscr();
		LOG_PRINTF(("transMsg->EMV_Flag %d \n",transMsg->EMV_Flag));
		if(transMsg->EMV_Flag == 1 )
		{
			display_TVR_TSI(transMsg);
		}
		if(!strcmp((char *)BitmapStructOb.field_39,"00"))
		{
			strcpy(transMsg->ResponseText,APPROVED_MSG);
			strcpy(transMsg->FromAcNo,(char *)BitmapStructOb.field_102);
			SaveTransDetails(transMsg);
			write_at(Dmsg[TRANSACTION_SUCCESSFULL].dispMsg, strlen(Dmsg[TRANSACTION_SUCCESSFULL].dispMsg),Dmsg[TRANSACTION_SUCCESSFULL].x_cor, Dmsg[TRANSACTION_SUCCESSFULL].y_cor);
			SVC_WAIT(2000);
			ClearKbdBuff();
			KBD_FLUSH();
			
			if(LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("Sale Transaction successfull\n"));    
			}
		}

		else if(!strcmp((char *)BitmapStructOb.field_39,"55"))
		{
			write_at(Dmsg[INVALID_PIN_INDX].dispMsg, strlen(Dmsg[INVALID_PIN_INDX].dispMsg),Dmsg[INVALID_PIN_INDX].x_cor, Dmsg[INVALID_PIN_INDX].y_cor);//invalid pin
			strcpy(transMsg->ResponseText,"INVALID PIN");
			SVC_WAIT(2000);
			ClearKbdBuff();
			KBD_FLUSH();
		}
		else if(!strcmp((char *)BitmapStructOb.field_39,"51"))
		{
			write_at(Dmsg[INSUFFICIENT_FUND].dispMsg, strlen(Dmsg[INSUFFICIENT_FUND].dispMsg),Dmsg[INSUFFICIENT_FUND].x_cor, Dmsg[INSUFFICIENT_FUND].y_cor);//INSUFFICIENT_FUNDS
			strcpy(transMsg->ResponseText,"INSUFFICIENT_FUNDS");
			SVC_WAIT(2000);
			ClearKbdBuff();
			KBD_FLUSH();
		}
		else
		{
			LOG_PRINTF(("==================Check Decline1")); 
			write_at(Dmsg[TRANSACTION_FAILED].dispMsg, strlen(Dmsg[TRANSACTION_FAILED].dispMsg),Dmsg[TRANSACTION_FAILED].x_cor, Dmsg[TRANSACTION_FAILED].y_cor);
			LOG_PRINTF(("==================Check Decline2"));
			SVC_WAIT(2000);
			LOG_PRINTF(("==================Check Decline3"));
			ClearKbdBuff();
			LOG_PRINTF(("==================Check Decline4"));
			KBD_FLUSH();
			strcpy(transMsg->ResponseText,"DECLINE");
			if(LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("Sale Transaction Fail reason : %s \n",BitmapStructOb.field_39));
			}
		}
		LOG_PRINTF(("==============Check 1 "));
		return _SUCCESS;
	}
	else
	{
		if(retVal == RECV_FAILED)//if did not receive a valid response before the transaction time-out period expired
		{
			if(SaveReversalDetails(transMsg) ==_SUCCESS)//saves reversal data
			{
				retVal = InitReversal(); //go for reversal once
				if(LOG_STATUS == LOG_ENABLE)
				{
					LOG_PRINTF(("reversal retVal =%d",retVal));
				}
			}
		}
		else
		{
			if(LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("Response buffer size is zero for sale\n"));  
			}
		}
	}
	return _FAIL; 
}
/******************************************************************************************************************
*	Function Name : InitSaleTransaction									                                                            *										                                             
*	Purpose		    : intializes transaction structure (get the trace audit no,tid,cardacceptorId,key injection etc .)*
*	Input		      : TransactionMsgStruc structure pointer	,short amount type and swipe card flag									  *                                                               
*	Output		    : returns success or failure 								                                                      *
*******************************************************************************************************************/

short InitSaleTransaction(TransactionMsgStruc *transMsg,short amountType,short swipeCardFlag)
{
		short iRetVal=0;
		char terminalId[TERMINAL_ID_LEN+1]={0};//9
		char cardAccepterId[CARD_ACCEPTOR_ID_LEN+1]={0};//16
    //char ch = 0;

		getTraceAuditNumber(transMsg);
		get_env("#TERMINAL_ID",(char*)terminalId,sizeof(terminalId));
		sprintf(transMsg->TerminalID,"%s",terminalId);
		get_env("#CARDACCEPTERID",(char*)cardAccepterId,sizeof(cardAccepterId));
		sprintf(transMsg->CardAcceptorID,"%s",cardAccepterId);
		if (transMsg->TrTypeFlag == WITHDRAWAL_MSG_TYPE_CASE)
			sprintf(transMsg->_transType,"%s","WITHDRAWAL");
		else
		sprintf(transMsg->_transType,"%s","SALE");
    LOG_PRINTF(("Amount  = %s ",transMsg->Amount));
    LOG_PRINTF(("POSEntryMode = %s",transMsg->POSEntryMode));

    EnblTouchScreen(DisableTouch);
		
		if(amountType == CASHBACK_AMOUNT)
		{
				iRetVal = GetInputFromUser(transMsg,CASHBACK_AMOUNT);//getting cashback amount from keyboard
				if(LOG_STATUS == LOG_ENABLE)
        {
         LOG_PRINTF(("ret from cashback amount =  %d ",iRetVal));
        }
		if((iRetVal == _FAIL ) || (iRetVal == KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL))
		{
          if(LOG_STATUS == LOG_ENABLE)
          {
					  LOG_PRINTF(("Error Occored while getting cash back amount =  %d ",iRetVal));
          }
					return GoToMainScreen;
		}
		}
   
		if(swipeCardFlag != 1)
		{
        
				iRetVal = swipeOrEMVCardEntry(transMsg);//Read card data 
				if((iRetVal == (KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL) || (iRetVal == _FAIL)))
				{
						return GoToMainScreen;
				}
        else if(iRetVal == FORCE_FOR_EMV)
        {
          LOG_PRINTF(("InitSaleTransaction:FORCE_FOR_EMV"));
         	iRetVal = EMVCardEntry(transMsg);//Read card data 
				  if((iRetVal == (KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL) || (iRetVal == _FAIL)))
				  {
						  return GoToMainScreen;
				  }        //22 POS Entry Mode
        }
         if(iRetVal == FALLBACK)
        {
          LOG_PRINTF(("InitSaleTransaction:FALLBACK"));
         
          iRetVal = swipeCardEntry(transMsg);//Read card data 
				  if((iRetVal == (KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL) || (iRetVal == _FAIL)))
				  {
						  return GoToMainScreen;
				  }
          strcpy((char *)transMsg->POSEntryMode,MSReICC_PIN_CAPABLE);             //22 POS Entry Mode
        }
        if(LOG_STATUS == LOG_ENABLE)
        {
					LOG_PRINTF(("TrMethordFlag After swipeOrEMVCardEntry %d getchar",transMsg->TrMethordFlag));
        }
		}
		else
    {
      window(1,1,30,20);
	    clrscr ();
      /// strcpy(transMsg->POSEntryMode,MSR_PIN_CAPABLE);
      strcpy(transMsg->POSConditionCode,NORMAL_PRESENTMENT);
      write_at(Dmsg[PROCESSING].dispMsg, strlen(Dmsg[PROCESSING].dispMsg),Dmsg[PROCESSING].x_cor, Dmsg[PROCESSING].y_cor);
			
    }
		if(transMsg->TrMethordFlag == edebit)//for debit transaction
		{
			LOG_PRINTF(("===========Debit Txn "));
        if((!strcmp((char *)transMsg->POSEntryMode,MSR_PIN_CAPABLE))||(!strcmp((char *)transMsg->POSEntryMode,MSReICC_PIN_CAPABLE)))//FOR MAGNETIC SWIPE
        {
				  iRetVal = ProcessingKeyInjection(transMsg->PrimaryAccNum);//getting user pin 
        }
        if(iRetVal == _SUCCESS)
				{
					iRetVal = PaymentTransactionProcessing(transMsg);
          if(LOG_STATUS == LOG_ENABLE)
          {
					  LOG_PRINTF(("ret val debit = %d ",iRetVal));
          }
				}
				else
				{
					return GoToMainScreen;
				}
		}
		else if(transMsg->TrMethordFlag == ecredit)//for credit transaction
		{
        if(LOG_STATUS == LOG_ENABLE)
        {
					LOG_PRINTF(("ret val CREDIT amount = %s getchar",transMsg->Amount));
        }
				strcpy(transMsg->ProcessingCode,CREDIT_SALEPROCODE);//-<<<<<<<<<<<----------   written for testing purpus 
				iRetVal = PaymentTransactionProcessing(transMsg);
		}
    else
    {
        LOG_PRINTF(("ret from cashback else  "));
    }

    LOG_PRINTF (("data TVR = %s",transMsg->TVR));
    
    if(iRetVal==_SUCCESS)
    {   
		    printRecipt(transMsg);
    }
    LOG_PRINTF ((" EMV data TSI = %s",transMsg->TSI));
		return GoToMainScreen;	
}
/******************************************************************************************************************
*	Function Name : resetBitmapStructure									                                                           *										                                             
*	Purpose		    : reset the bitmap strcuture to zero                                                               *
*	Input		      : bitmap structure pointer	                                                  									   *                                                               
*	Output		    : void                                			                                                       *
*******************************************************************************************************************/
void resetBitmapStructure(BmapStruct *ptrBitmapStructOb)
{
  memset(ptrBitmapStructOb, 0, sizeof(BmapStruct));  
}
/******************************************************************************************************************
*	Function Name : swipeCardEntry									                                                                 *										                                             
*	Purpose		    : Prompts for Swipe Card and read the required information form the card                           *
*	Input		      : TransactionMsgStruc structure pointer	                                                  				 *                                                               
*	Output		    : int                                			                                                         *
*******************************************************************************************************************/
int swipeCardEntry(TransactionMsgStruc *transMsg)
{
	int ret = 0;
	short flag = 1;
	char getKey = 0;
	int mask = 0;

  time_t initial_time = time(NULL);
	char timeLimit[TIME_ARR_LEN]={0}; //4
	char ch =0 ;
	short time_limit = 0;

	window(1,1,30,20);
	clrscr ();
	
  write_at(Dmsg[FALLBACK_OCCURRED].dispMsg, strlen(Dmsg[FALLBACK_OCCURRED].dispMsg),Dmsg[FALLBACK_OCCURRED].x_cor, Dmsg[FALLBACK_OCCURRED].y_cor);
  write_at(Dmsg[WANT_TO_SWIPE_CARD].dispMsg, strlen(Dmsg[WANT_TO_SWIPE_CARD].dispMsg),Dmsg[WANT_TO_SWIPE_CARD].x_cor, Dmsg[WANT_TO_SWIPE_CARD].y_cor);
  write_at(Dmsg[PRESS_ENTER_FOR_YES].dispMsg, strlen(Dmsg[PRESS_ENTER_FOR_YES].dispMsg),Dmsg[PRESS_ENTER_FOR_YES].x_cor, Dmsg[PRESS_ENTER_FOR_YES].y_cor);
  write_at(Dmsg[PRESS_CANCEL_FOR_NO].dispMsg, strlen(Dmsg[PRESS_CANCEL_FOR_NO].dispMsg),Dmsg[PRESS_CANCEL_FOR_NO].x_cor, Dmsg[PRESS_CANCEL_FOR_NO].y_cor);
  //SVC_WAIT(1000);
	ClearKbdBuff();
  KBD_FLUSH();
  do
	{
		ch =get_char();
	}while((ch != KEY_CR) && (ch != KEY_CANCEL) && (ch != KEY_STR));
  if((ch == KEY_CANCEL) || (ch == KEY_STR))
				return ch;
	if( hMagReader == -1)
	{
		hMagReader = open (DEV_CARD, 0);
    if(LOG_STATUS == LOG_ENABLE)
    {
			LOG_PRINTF (("megnatic reader open = %d",hMagReader));	
    }
	}
	get_env("#TIME_OUT_LIMIT_IN_SECOND",timeLimit,sizeof(timeLimit));
	
	time_limit = atoi(timeLimit);
	window(1,1,30,20);
	clrscr ();
	initial_time = time(NULL);
#ifdef SUP_CARD
	write_at("SUPERVISOR CARD", 15,7,10);
	LOG_PRINTF(("SUPERVISOR CARD"));
#endif
	write_at(Dmsg[SWIPE_YOUR_CARD].dispMsg, strlen(Dmsg[SWIPE_YOUR_CARD].dispMsg),Dmsg[SWIPE_YOUR_CARD].x_cor, Dmsg[SWIPE_YOUR_CARD].y_cor);
  
	do
	{
		ClearKbdBuff();
		KBD_FLUSH();
		mask = read_event();//
		mask = wait_event();//waiting for event
		if(mask & EVT_MAG)
		{
			flag = 1;
		}
		else if(mask & EVT_KBD) //checking for keyboard event
		{
			flag = 2;
			getKey=get_char();
			break;
		}
		else
		{
			flag = 3;
			if(LOG_STATUS == LOG_ENABLE)
      {
			  LOG_PRINTF (("swipe/enter.."));
      }
		}

	}while (flag == 3 && (time(NULL) - initial_time) < time_limit);
  if(LOG_STATUS == LOG_ENABLE)
  {
	  LOG_PRINTF (("Stat Val = %d",flag));	
  }
  if(flag == 3)
	{
		clrscr();
		error_tone();
		SVC_WAIT(190);
		error_tone();
		write_at(TIME_OUT,strlen(TIME_OUT),(30-strlen(TIME_OUT))/2,10);
		SVC_WAIT(1000);
		ClearKbdBuff();
		KBD_FLUSH();
		resetMsgDetails(transMsg);
		return _FAIL;
	}
	else if(flag == 1)
	{
		//Read card details from mag card reader
    if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF (("in Megnatic Card"));
    }
    //strcpy((char *)transMsg->POSEntryMode,"021");             //22 POS Entry Mode
		ret = readMagCard(transMsg);
		if(ret == _FAIL)
		{
			return _FAIL;
		}
	}
	else if(flag == 2)
	{
		if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF (("in KeyBord Entry getKey = %d",getKey));
    }
		if((getKey == KEY_CANCEL) ||(getKey == KEY_STR ) || (getKey >= 48 && getKey <=57 ) || (getKey == 8) || (getKey == 13))//if user press cancle key
		return _FAIL;
  }
	return _SUCCESS;
}
/******************************************************************************************************************
*	Function Name : display_TVR_TSI									                                                                 *										                                             
*	Purpose		    : display TVR and TSI information on the terminal for EMV transaction                              *
*	Input		      : TransactionMsgStruc structure pointer	                                                  				 *                                                               
*	Output		    : int                                			                                                         *
*******************************************************************************************************************/
void display_TVR_TSI(TransactionMsgStruc *transMsg)
{
  char tempBuff[20] = {0};
  sprintf (tempBuff, "%s%s","TVR : ",transMsg->TVR);
  
  write_at(tempBuff, strlen(tempBuff),8, 11);

  memset(tempBuff,0,sizeof(tempBuff));
  sprintf (tempBuff, "%s%s","TSI : ",transMsg->TSI);
  write_at(tempBuff, strlen(tempBuff),8, 12);
		
}
/******************************************************************************************************************
*	Function Name : CloseEMVSLot									                                                                   *										                                             
*	Purpose		    : Close EMV slot                                                                                   *
*	Input		      : void                                        				 *                                                               
*	Output		    : void                                			                                                       *
*******************************************************************************************************************/
void CloseEMVSLot()
{
  Terminate_CardSlot(1,SWITCH_OFF_CARD);//To close the card slot by  18-08-2016 

  inVXEMVAPCloseCardslot();//To close the card slot by  18-08-2016 

}
/******************************************************************************************************************
*	Function Name : CheckServiceCode									                                                               *										                                             
*	Purpose		    : checks the service code for card and check that the Card is EMV capable or not                   *
*	Input		      : char pointer to the track2 data buffer	                                                  				 *                                                               
*	Output		    : int                                			                                                         *
*******************************************************************************************************************/
int CheckServiceCode(char *track2data)
{
    char ServiceCode[SERVICE_CODE_LEN+1] ={0};
    int i = 0 , j =0; 
    char *ptr = NULL;
    const char delim[2] = "=";
 
  	ptr = strstr(track2data,delim);
   
    for(i =4,j=0 ;i<7;i++,j++)//fetching service code from track 2 data
    {
        ServiceCode[j] = ptr [i+1];
    }
    LOG_PRINTF (("ServiceCode = %s",ServiceCode));
    if((ServiceCode[0]=='2' )|| (ServiceCode[0]=='6'))
    {
      window(1,1,30,20);
	    clrscr();
      write_at("YOUR CARD IS EMV CAPABLE", strlen("YOUR CARD IS EMV CAPABLE"),4, 10);
      write_at("PLEASE INSERT CARD FOR EMV ", strlen("PLEASE INSERT CARD FOR EMV"),4, 11);
      SVC_WAIT(2000);
			ClearKbdBuff();
			KBD_FLUSH();
    
      return FORCE_FOR_EMV;
    }
    return _SUCCESS;
}
/******************************************************************************************************************
*	Function Name : EMVCardEntry									                                                                   *										                                             
*	Purpose		    : Takes the EMV card and read the required information from the chip                               *
*	Input		      : TransactionMsgStruc structure pointer	                                                  				 *                                                               
*	Output		    : int                                			                                                         *
*******************************************************************************************************************/
int EMVCardEntry(TransactionMsgStruc *transMsg)
{
	int ret = 0;
	short flag = 1;
	char getKey = 0;
	int mask = 0;
  
  time_t initial_time = time(NULL);
	char timeLimit[TIME_ARR_LEN]={0}; //4
	
	short time_limit;
	
	if( hMagReader == -1)
	{
			hMagReader = open (DEV_CARD, 0);
      if(LOG_STATUS == LOG_ENABLE)
      {
			  LOG_PRINTF (("megnatic reader open = %d",hMagReader));	
      }
	}
	get_env("#TIME_OUT_LIMIT_IN_SECOND",timeLimit,sizeof(timeLimit));
	

	time_limit = atoi(timeLimit);
	window(1,1,30,20);
	clrscr ();
	
	write_at(Dmsg[_INSERT_CARD].dispMsg, strlen(Dmsg[_INSERT_CARD].dispMsg),Dmsg[_INSERT_CARD].x_cor, Dmsg[_INSERT_CARD].y_cor);
  
	do
	{
		ClearKbdBuff();
		KBD_FLUSH();
		mask = read_event();//
		mask = wait_event();//waiting for event
		if(mask & EVT_KBD) //checking for keyboard event
		{
				flag = 2;
				getKey=get_char();
    
				break;
		}
    else if(mask & EVT_ICC1_INS) //checking for EMV event
    {
 		  if(LOG_STATUS == LOG_ENABLE)
      {
        LOG_PRINTF (("EMV CARD..."));//22 POS Entry Mode
      }
     
      ret   = inRunEMVTransaction(transMsg->TrTypeFlag);
      if(ret == _SUCCESS)
      {
        	flag = 4;
          
          if(LOG_STATUS == LOG_ENABLE)
          {
            LOG_PRINTF (("inRunEMVTransaction Success!!"));//22 POS Entry Mode
          }
      }
      else if(ret != FALLBACK)
      {
        if(LOG_STATUS == LOG_ENABLE)
        {
		      LOG_PRINTF (("After emvFirstGenerationFlow fail ret =%d",ret));
        }
        CloseEMVSLot();
        return _FAIL;
      }
      else
      {
        
        if(LOG_STATUS == LOG_ENABLE)
        {
          LOG_PRINTF (("FALLBACK occurred..."));//22 POS Entry Mode
        }
        CloseEMVSLot();
        return ret;
      }
			break;
    }
		else
		{
			flag = 3;
			if(LOG_STATUS == LOG_ENABLE)
      {
			  LOG_PRINTF (("swipe/enter.."));
      }
		}

	}while (flag == 3 && (time(NULL) - initial_time) < time_limit);
  if(LOG_STATUS == LOG_ENABLE)
  {
	  LOG_PRINTF (("Stat Val = %d",flag));	
  }
  if(flag == 3)
	{
		clrscr();
		error_tone();
		SVC_WAIT(190);
		error_tone();
		write_at(TIME_OUT,strlen(TIME_OUT),(30-strlen(TIME_OUT))/2,10);
		SVC_WAIT(1000);
		ClearKbdBuff();
		KBD_FLUSH();
		resetMsgDetails(transMsg);
		return _FAIL;
	}
	else if(flag == 2)
	{
		//Getting customer card number through manual entry
		if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF (("in KeyBord Entry getKey = %d",getKey));
    }
		if((getKey == KEY_CANCEL) ||(getKey == KEY_STR ) || (getKey >= 48 && getKey <=57 ) || (getKey == 8) || (getKey == 13))//if user press cancle key
		return _FAIL;
   
	}
  else if(flag == 4)
	{
		//Read card details from mag card reader
    if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF (("Going To call emvFirstGenerationFlow"));
    }
    
		ret = emvFirstGenerationFlow(transMsg);
    if(ret == _SUCCESS)
    {
      transMsg->EMV_Flag = 1 ;
      strcpy((char *)transMsg->POSEntryMode,"051");             //22 POS Entry Mode
    }
    if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF (("After emvFirstGenerationFlow ret =%d",ret));
    }
    CloseEMVSLot();
		return ret;
	}
	return _SUCCESS;
}
