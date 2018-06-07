/***********************************************************************************************
  Filename		     : Transactions.c
  Project		       : Bevertec 
  Developer Neame  : Amar
  Module           : Processing void,refund and balance inquiry transaction
  Description	     : This is the implementation of transactions like void ,refund and balance inquiry
 ***********************************************************************************************/

#include <aclconio.h>

#include "..\include\Transactions.h"
#include "..\include\Common.h"
#include "..\Include\Bevertec.h"
#include "..\Include\SaleTransaction.h"
#include "..\Include\TouchScreen.h"
#include "..\Include\IPP_Key_injection.h"

#include "..\Include\BevertecClient.h"
#include "..\Include\ReciptPrint.h"
#include "..\Include\ISo8583Map.h"
#include "..\Include\Settlement.h"
#include "..\Include\Logon.h"
#include "..\Include\Supervisor.h"
#include "..\include\BVTC_EMV.h"

extern short hClock;
extern char msg_id[MSG_ID_LEN+1];//5
extern char Proc_code[PROC_CODE_LEN+1];//7
extern BmapStruct BitmapStructOb ;
short BITMAP_LEN=8;
extern unsigned char KeySessionKeyEncripted[GISKE_KEY_LEN + 1] = {0};//Giske Block specs for session key

/****************************************************************************************************************************
*	Function Name : getTraceAuditNumber									                                                                      *
*	Purpose		    : get	trace audit no from dld file		                                                                      * 
*	Input					:	Address of main transaction message structure                                                             *						                                 
*	Output		    : returns success or failure 								                                                                *
										                                                                                                        *
*****************************************************************************************************************************/

void getTraceAuditNumber(TransactionMsgStruc *transMsg)
{
		char tempStr[TRACE_AUDIT_LEN+1]={0};//7
		int iTraceAudit=0/*,i=0*/;
		char result[TRACE_AUDIT_LEN+1]={0};//7
		get_env("#TRACE_AUDIT_NO",transMsg->TraceAuditNo,sizeof(transMsg->TraceAuditNo));
		if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("current TraceAuditNo = %s",transMsg->TraceAuditNo));
    }
		iTraceAudit = atoi(transMsg->TraceAuditNo);
    if(iTraceAudit == MAX_TRACE_AUDIT_NO)
    {
      iTraceAudit = 1;
    }
    else
    {
		  iTraceAudit += 1;
    }
		
    int2str(tempStr,iTraceAudit);
    pad(result,tempStr,'0',6,RIGHT);
	
		put_env("#TRACE_AUDIT_NO",result,sizeof(result));
    if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF((" updated TraceAuditNo = %s",result));
    }

}
/****************************************************************************************************************************
*	Function Name : updateBatchNumber									                                                                         *
*	Purpose		    : get	Batch	no from dld file		                                                                            * 
*	Input					:	Address of main transaction message structure and transaction methord type like debit ,credit or loyality *						               
*	Output		    : returns success or failure 								                                                                *
										                                                                                                         *
*****************************************************************************************************************************/
void updateBatchNumber(TransactionMsgStruc *transMsg)
{
		char tempStr[BATCH_ID_LEN+1]={0};//7
		int iBatchNo=0/*,i=0*/;
		char result[BATCH_ID_LEN+1]={0};//7
		get_env("#TRACE_BATCH_NO",transMsg->BatchNo,sizeof(transMsg->BatchNo));
    if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF(("current Batch No = %s",transMsg->BatchNo));
    }
	iBatchNo = atoi(transMsg->BatchNo);

    if(iBatchNo == MAX_BATCH_NO)
    {
      iBatchNo = 1;
    }
    else
    {
		  iBatchNo += 1;
    }

    int2str(tempStr,iBatchNo);
    pad(result,tempStr,'0',6,RIGHT);
	 
		put_env("#TRACE_BATCH_NO",result,sizeof(result));
    if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF((" updated Batch No = %s",result));
    }

}

/****************************************************************************************************************
*	Function Name : voidTransaction									                                          *
*	Purpose		    : void a transaction					                              *
*	Input					:	Address of main transaction message structure and transaction methord type like debit ,credit or loyality 						                                              *
*	Output		    : returns success or failure 								                                  *
*											                                          *
*****************************************************************************************************************/

short InitVoid(TransactionMsgStruc *transMsg)
{
    short ret = 0;
	short Selectionstatus;
    char ch = 0;
    char terminalId[TERMINAL_ID_LEN+1]={0};//9
	char cardAccepterId[CARD_ACCEPTOR_ID_LEN+1]={0};//16
    char PAN[20];
	clrscr();
	

	/*
	if(validateMerchantPassword() != _SUCCESS)
		return _FAIL;
   	*/
#ifdef SUP_CARD
	if (Sup_Login == 0)
	{
		LOG_PRINTF(("Supervisor Card validation"));
		transMsg->TrTypeFlag = PIN_VALIDATE_MSG_TYPE_CASE;
		ret = InitPINValidate(transMsg);
		if (ret)
			return GoToMainScreen;
		transMsg->TrTypeFlag = VOIDMSGTYPE_CASE;
		LOG_PRINTF(("Supervisor Card validation- Successful"));
	}
#endif		
	getTraceAuditNumber(transMsg);
	get_env("#TERMINAL_ID",(char*)terminalId,sizeof(terminalId));
	sprintf(transMsg->TerminalID,"%s",terminalId);
	get_env("#CARDACCEPTERID",(char*)cardAccepterId,sizeof(cardAccepterId));
	sprintf(transMsg->CardAcceptorID,"%s",cardAccepterId);
    sprintf(transMsg->_transType,"%s","VOID");
	memset(PAN,0,sizeof(PAN));
#ifndef GOVT_APP //if application is government type
		do
		{
		//	ret = getTransNo(transMsg);//getting Payment Id from user for Government
			if(((ret == _FAIL) || (ret == KEY_STR) || (ret == KEY_CANCEL) || (ret == BTN_CANCEL)))
			{
				return GoToMainScreen;
			}
			window(1,1,30,20);
			clrscr();
			write_at(Dmsg[PROCESSING].dispMsg, strlen(Dmsg[PROCESSING].dispMsg),Dmsg[PROCESSING].x_cor, Dmsg[PROCESSING].y_cor);
			ret = ReadLastTransDetails(transMsg);//reading data from file	
			strcpy(PAN,transMsg->PrimaryAccNum);
			memset(transMsg->_transType,0,sizeof(transMsg->_transType));
			sprintf(transMsg->_transType,"%s","VOID");
			if(ret == _SUCCESS)
			{
				break;//for success
			}
			else if(ret == -2)
			{
				window(1,1,30,20);
			    clrscr();
     			write_at(Dmsg[NO_TRANSACTION_TO_VOID].dispMsg, strlen(Dmsg[NO_TRANSACTION_TO_VOID].dispMsg),Dmsg[NO_TRANSACTION_TO_VOID].x_cor, Dmsg[NO_TRANSACTION_TO_VOID].y_cor);
				SVC_WAIT(2000);
				ClearKbdBuff();
			    KBD_FLUSH();
				return GoToMainScreen;
			}
			else //if return fail
			{
				do
				{
					write_at("DO YOU WANT TO RETRY ?",strlen("DO YOU WANT TO RETRY ?"),(30-strlen("DO YOU WANT TO RETRY ?"))/2,11);
					write_at("PRESS ENTER FOR YES",strlen("PRESS ENTER FOR YES"),(30-strlen("PRESS ENTER FOR YES"))/2,12);
					write_at("PRESS CANCEL FOR NO",strlen("PRESS CANCEL FOR NO"),(30-strlen("PRESS CANCEL FOR NO"))/2,13);
					ch =get_char();
				}while((ch != KEY_CR) && (ch != KEY_CANCEL) && (ch != KEY_STR));
				if((ch == KEY_CANCEL) || (ch == KEY_STR))
					break;
			}
		}while(ch == KEY_CR);
		if(((ret == _FAIL) || (ret == KEY_STR) || (ret == KEY_CANCEL) || (ret == BTN_CANCEL)))
				return GoToMainScreen;

#endif
		/*
    if(transMsg->TrMethordFlag == edebit )//for debit refund
	{
		SetImage(ACC_TYPE_BMP);//setting image for Account type 
		EnblTouchScreen(AccountType);
		Selectionstatus = get_char();
		if((Selectionstatus == eChequeAccount) || (Selectionstatus == eSavingAccount) || (Selectionstatus == eCreditAccount))				 
			ret =  debitAccountType(transMsg,Selectionstatus);//call function to perform according to account type like cheque,saving or credit
		else if((Selectionstatus == KEY_CANCEL ) || (Selectionstatus == KEY_STR) || Selectionstatus == eAccTypeBack)
			return GoToMainScreen;															
		EnblTouchScreen(DisableTouch);
	}
    else
    {
		EnblTouchScreen(DisableTouch);
    }*/
	ret = swipeOrEMVCardEntry(transMsg);//Read card data 
	if((ret == _FAIL ) || (ret == KEY_STR ) || (ret == KEY_CANCEL) || (ret == BTN_CANCEL))
	{
		return ret;
	}
	else if(ret == FORCE_FOR_EMV)
	{
		LOG_PRINTF(("InitVoidTransaction:FORCE_FOR_EMV"));
		ret = EMVCardEntry(transMsg);//Read card data 
		if((ret == (KEY_STR ) || (ret == KEY_CANCEL) || (ret == BTN_CANCEL) || (ret == _FAIL)))
		{
			return GoToMainScreen;
		}        //22 POS Entry Mode
	}
	if(ret == FALLBACK)
	{
		LOG_PRINTF(("InitVoid:FALLBACK"));
		ret = swipeCardEntry(transMsg);//Read card data 
		if((ret == (KEY_STR ) || (ret == KEY_CANCEL) || (ret == BTN_CANCEL) || (ret == _FAIL)))
		{
			return GoToMainScreen;
		}
		strcpy((char *)transMsg->POSEntryMode,MSReICC_PIN_CAPABLE);             //22 POS Entry Mode
	}
     // LOG_PRINTF(("swipeCardFlag != 1"));
	
/*	if(transMsg->TrMethordFlag == edebit)
	{			
		if((!strcmp((char *)transMsg->POSEntryMode,MSR_PIN_CAPABLE))||(!strcmp((char *)transMsg->POSEntryMode,MSReICC_PIN_CAPABLE)))//FOR MAGNETIC SWIPE
		{
			ret = ProcessingKeyInjection(transMsg->PrimaryAccNum);//getting user pin 
		}
	}
	else if(transMsg->TrMethordFlag == ecredit)
	{
		strcpy(transMsg->ProcessingCode,CREDIT_VOIDPROCODE);//-<<<<<<<<<<<----------   written for testing purpus 
		ret = _SUCCESS;
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("for credit card"));	
		}
	}
	*/
	if(ret==_SUCCESS)
	{		  
		LOG_PRINTF(("PAN in file = %s ",PAN));
		LOG_PRINTF(("PAN in Card Swiped = %s ",transMsg->PrimaryAccNum));
		if(strcmp(PAN,transMsg->PrimaryAccNum))
		{
			LOG_PRINTF(("Card number didn't match ",ret));
			window(1,1,30,20);
			clrscr();
     		write_at("WRONG CARD",10,5,5);
			SVC_WAIT(3000);
			ClearKbdBuff();
			KBD_FLUSH();
			return GoToMainScreen;
		}

		ret = ProcessingVoidTransaction(transMsg);
		if(LOG_STATUS == LOG_ENABLE)
		{
		    LOG_PRINTF(("ret val = =  %d ",ret));
		}
    }
    else
    {
		return GoToMainScreen;
    }
    if(ret==_SUCCESS)
    {		
		printRecipt(transMsg);
    }
    return _SUCCESS;
}

/****************************************************************************************************************
*	Function Name : ProcessingVoidTransaction									                                          *
*	Purpose		    : void a transaction					                              *
*	Input					:	Address of main transaction message structure and transaction methord type like debit ,credit or loyality 						                                              *
*	Output		    : returns success or failure 								                                  *
*											                                          *
*****************************************************************************************************************/
short ProcessingVoidTransaction(TransactionMsgStruc *transMsg)
{
	short retVal = -1;  
	int i =0;
	char Bitmap[BITMAP_ARRAY_SIZE+1] ={0}; //8
	char UnpackBiitmap[UNPACK_BITMAP_ARRAY_SIZE+1]={0}; //16
	char bits[BITMAP_SIZE+1] ={0};//64
	char temp[BITMAP_ARRAY_SIZE+1] ={0};//9
  
#ifndef EMV_ENABLE
	unsigned char reqBuff[VOID_REQUEST_SIZE]={0};//108
#else
	unsigned char reqBuff[VOID_REQUEST_SIZE+ICC_RELATED_DATA_LEN+2]={0};//108+151+2
#endif 
	unsigned char resBuff[VOID_RESPONSE_SIZE]={0};//110 to 200 for End 2 end encryption
	unsigned char testBuff[VOID_UNAPACK_RES_SIZE]={0};//220 to 400 for End 2 end encryption
	short ReqLen = 0; 
	short recvLength = 0;

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
    { 37, 12, ASC_STR, (void *) BitmapStructOb.field_37, sizeof( BitmapStructOb.field_37) }, 
    { 38, 6, BCD_STR, (void *) BitmapStructOb.field_38, sizeof( BitmapStructOb.field_38) }, 
    { 39, 2, BCD_STR, (void *) BitmapStructOb.field_39, sizeof( BitmapStructOb.field_39) }, 
    { 40+ SKIP, 3, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 41, 8, ASC_STR, (void *) BitmapStructOb.field_41, sizeof( BitmapStructOb.field_41) }, 
    { 42, 15, ASC_STR, (void *) BitmapStructOb.field_42, sizeof( BitmapStructOb.field_42) }, 
    { 43+ SKIP, 40, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 44+ SKIP, 25, AV2_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 45+SKIP, 76, AV2_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 46+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 47+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 48+ SKIP, 9, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 49+ SKIP, 3, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 50+ SKIP, 3, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 51+ SKIP, 3, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 52, 64, BIT_BIT, (void *) BitmapStructOb.field_52, sizeof( BitmapStructOb.field_52) }, 
    { 53+ SKIP, 16, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 54+ SKIP, 120, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 55+ SKIP, 0, BIT_BIT, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 56+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 57+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 58+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 59+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 60, 12, BCD_STR, (void *) BitmapStructOb.AmountForVoid, sizeof( BitmapStructOb.AmountForVoid) }, 
    { 61+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 62+ SKIP, 999, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 63+ SKIP, 999, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 64+ SKIP +STOP, 64, BIT_BIT, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
  }; 
	if(LOG_STATUS == LOG_ENABLE)
	{
		LOG_PRINTF(("test ProcessingISOBitmap = %s \n",transMsg->ProcessingCode));
	}
	strcpy(msg_id,VOIDMSGTYPE);
	strcpy(Proc_code,transMsg->ProcessingCode);
	strcpy(transMsg->MessageTypeInd,VOIDMSGTYPE);
	resetBitmapStructure(&BitmapStructOb);//memsetting all the elements of structure
	ProcessingISOBitmapEngine(iso8583_field_table, transMsg);
	ReqLen = retVal = assemble_packet(iso8583_field_table,reqBuff);
	if(retVal <= _SUCCESS)
	{  
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Error Occurred in assemble_packet retVal = %d \n",retVal));

		}
		memset(&BitmapStructOb, 0, sizeof(BmapStruct));
		return retVal;
	}
	recvLength = DEFAULT_BUFLEN ;
	retVal = CommWithServer((char*)reqBuff,ReqLen,(char*)resBuff,&recvLength);
	//retVal = _FAIL ;
	if( retVal ==_SUCCESS )//checking for response buffer size 
	{
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("resBuff = [%s]\n recvLength= %d ",resBuff,recvLength));
		}
		bcd2a((char *)testBuff,(char *)resBuff,recvLength);
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Testbuffin disassemblePacket retVal = [%s] \n",testBuff));
		}

		if(testBuff[18]>='8')
		BITMAP_LEN=16;
			
		strncpy(UnpackBiitmap,(char*)testBuff+UNPACK_SIZE_MSGLEN_OF_REQ_RES+TPDU_LEN+MSG_ID_LEN,BITMAP_LEN*2);

   		//strncpy(UnpackBiitmap,(char*)testBuff+UNPACK_SIZE_MSGLEN_OF_REQ_RES+TPDU_LEN+MSG_ID_LEN,UNPACK_BITMAP_ARRAY_SIZE);  
		packData(UnpackBiitmap,Bitmap);
		for(i=0;i<BITMAP_LEN;i++)
		{
			CopyAllBits(Bitmap+i,temp);
			strcpy(bits+(i*8),temp);
		}
		//memset(&BitmapStructOb,0,sizeof(BitmapStructOb));
		resetBitmapStructure(&BitmapStructOb);//memsetting all the elements of structure
		ParseAndFillBitFields((char*)testBuff,bits);
		//------------------------------------------------------------
		strcpy((char *)transMsg->ResponseCode,(char*)BitmapStructOb.field_39);             //copy the response code
		strcpy((char *)transMsg->AuthIdResponse,(char*)BitmapStructOb.field_38);           //copy AuthIdResponse
		strcpy((char *)transMsg->RetrievalReferenceNo,(char*)BitmapStructOb.field_37);     //copy RetrievalReferenceNo
		window(1,1,30,20);
		clrscr();
		if(transMsg->EMV_Flag == 1 )
		{
			display_TVR_TSI(transMsg);
		}
		if(!strcmp((char *)BitmapStructOb.field_39,"00"))
		{
			/*  window(1,1,30,20);
				clrscr();*/
			strcpy(transMsg->ResponseText,APPROVED_MSG);
			deleteRecordFromFile(transMsg);
			write_at(Dmsg[TRANSACTION_SUCCESSFULL].dispMsg, strlen(Dmsg[TRANSACTION_SUCCESSFULL].dispMsg),Dmsg[TRANSACTION_SUCCESSFULL].x_cor, Dmsg[TRANSACTION_SUCCESSFULL].y_cor);
			SVC_WAIT(500);
			ClearKbdBuff();
			KBD_FLUSH();
			if(LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("Void successfull !!\n"));     
			}
		}
		else if(!strcmp((char *)BitmapStructOb.field_39,"55"))
		{
				/*window(1,1,30,20);
				clrscr();*/
			write_at(Dmsg[INVALID_PIN_INDX].dispMsg, strlen(Dmsg[INVALID_PIN_INDX].dispMsg),Dmsg[INVALID_PIN_INDX].x_cor, Dmsg[INVALID_PIN_INDX].y_cor);//invalid pin
			strcpy(transMsg->ResponseText,"INVALID PIN");
			SVC_WAIT(2000);
			ClearKbdBuff();
			KBD_FLUSH();
		}
		else if(!strcmp((char *)BitmapStructOb.field_39,"51"))
		{/*
				window(1,1,30,20);
				clrscr();
			*/	
			write_at(Dmsg[INVALID_PIN_INDX].dispMsg, strlen(Dmsg[INSUFFICIENT_FUND].dispMsg),Dmsg[INSUFFICIENT_FUND].x_cor, Dmsg[INSUFFICIENT_FUND].y_cor);//INSUFFICIENT_FUNDS
			strcpy(transMsg->ResponseText,"INSUFFICIENT_FUNDS");
			SVC_WAIT(2000);
			ClearKbdBuff();
			KBD_FLUSH();
		}
		else
		{
			/* window(1,1,30,20);
			clrscr();*/
			write_at(Dmsg[TRANSACTION_FAILED].dispMsg, strlen(Dmsg[TRANSACTION_FAILED].dispMsg),Dmsg[TRANSACTION_FAILED].x_cor, Dmsg[TRANSACTION_FAILED].y_cor);
			SVC_WAIT(1000);
			ClearKbdBuff();
			KBD_FLUSH();
            strcpy(transMsg->ResponseText,"DECLINE");
			if(LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("Void Transaction Fail reason : %s \n",BitmapStructOb.field_39));    
			}
		}
        return _SUCCESS;
	}
	else
	{
		if(retVal == RECV_FAILED)//if did not receive a valid response before the transaction time-out period expired
		{
			if(SaveReversalDetails(transMsg) ==_SUCCESS)//saves reversal data
			{       
				retVal = InitReversal();// go for reversal
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
				LOG_PRINTF(("Response buffer size is zero for void\n"));  
			}
         }
	}
	return _FAIL; 
}


/****************************************************************************************************************
*	Function Name : debitAccountType									                                          *
*	Purpose		    : perform according to debit account type	 					                              *
*	Input					:	event key of selected account type
*	Output		    : returns success or failure 								                                  *
*										                                          *
*****************************************************************************************************************/

short debitAccountType(TransactionMsgStruc *transMsg,int selectedAccountKey)
{
		if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF(("selectedAccountKey = %d ",selectedAccountKey));
    }
		switch(selectedAccountKey)
		{
				case 	eChequeAccount :
										
						if(transMsg->TrTypeFlag == BALINQMSGTYPE_CASE)
								strcpy(transMsg->ProcessingCode,CHECK_BALINQPROCODE);
						else if(transMsg->TrTypeFlag == REFUNDMSGTYPE_CASE)
								strcpy(transMsg->ProcessingCode,CHECK_REFUNDPROCODE);
						else if(transMsg->TrTypeFlag == SALEMSGTYPE_CASE)
								strcpy(transMsg->ProcessingCode,CHECK_SALEPROCODE);
						else if(transMsg->TrTypeFlag == VOIDMSGTYPE_CASE)
								strcpy(transMsg->ProcessingCode,CHECK_VOIDPROCODE);
						else if(transMsg->TrTypeFlag == WITHDRAWAL_MSG_TYPE_CASE)
								strcpy(transMsg->ProcessingCode,CHECK_WITHDRAWPROCODE);
						else if(transMsg->TrTypeFlag == DEPOSIT_MSG_TYPE_CASE)
								strcpy(transMsg->ProcessingCode,CHECK_DEPOSITPROCODE);
						
								break;
				case	eSavingAccount :
										//write_at("eSavingAccount refund", strlen("eSavingAccount refund"),1, 10);
						if(transMsg->TrTypeFlag == BALINQMSGTYPE_CASE)
								strcpy(transMsg->ProcessingCode,SAVE_BALINQPROCODE);
						else if(transMsg->TrTypeFlag == REFUNDMSGTYPE_CASE)
								strcpy(transMsg->ProcessingCode,SAVE_REFUNDPROCODE);
						else if(transMsg->TrTypeFlag == SALEMSGTYPE_CASE)
								strcpy(transMsg->ProcessingCode,SAVE_SALEPROCODE);
						else if(transMsg->TrTypeFlag == VOIDMSGTYPE_CASE)
								strcpy(transMsg->ProcessingCode,SAVE_VOIDPROCODE);		
						else if(transMsg->TrTypeFlag == WITHDRAWAL_MSG_TYPE_CASE)
								strcpy(transMsg->ProcessingCode,SAVE_WITHDRAWPROCODE);
						else if(transMsg->TrTypeFlag == DEPOSIT_MSG_TYPE_CASE)
								strcpy(transMsg->ProcessingCode,SAVE_DEPOSITPROCODE);
						
								break;
				case  eCreditAccount :
										//write_at("eCreditAccount refund", strlen("eCreditAccount refund"),1, 10);
						if(transMsg->TrTypeFlag == BALINQMSGTYPE_CASE)
								strcpy(transMsg->ProcessingCode,CREDIT_BALINQPROCODE);
						else if(transMsg->TrTypeFlag == REFUNDMSGTYPE_CASE)
								strcpy(transMsg->ProcessingCode,CREDIT_REFUNDPROCODE);
						else if(transMsg->TrTypeFlag == SALEMSGTYPE_CASE)
								strcpy(transMsg->ProcessingCode,CREDIT_SALEPROCODE);
						else if(transMsg->TrTypeFlag == VOIDMSGTYPE_CASE)
								strcpy(transMsg->ProcessingCode,CREDIT_VOIDPROCODE);
								break;
				case  eOtherAccount :
										//write_at("eCreditAccount refund", strlen("eCreditAccount refund"),1, 10);
						if(transMsg->TrTypeFlag == BALINQMSGTYPE_CASE)
								strcpy(transMsg->ProcessingCode,OTHER_BALINQPROCODE);
						else if(transMsg->TrTypeFlag == REFUNDMSGTYPE_CASE)
								strcpy(transMsg->ProcessingCode,OTHER_REFUNDPROCODE);
						else if(transMsg->TrTypeFlag == SALEMSGTYPE_CASE)
								strcpy(transMsg->ProcessingCode,OTHER_SALEPROCODE);
						else if(transMsg->TrTypeFlag == VOIDMSGTYPE_CASE)
								strcpy(transMsg->ProcessingCode,OTHER_VOIDPROCODE);
						else if(transMsg->TrTypeFlag == WITHDRAWAL_MSG_TYPE_CASE)
								strcpy(transMsg->ProcessingCode,OTHER_CASHPROCODE);
						else if(transMsg->TrTypeFlag == DEPOSIT_MSG_TYPE_CASE)
								strcpy(transMsg->ProcessingCode,OTHER_DEPOSITPROCODE);
						
								break;
				

				case eAccTypeBack:
						return _FAIL;
				default:
								return _FAIL;
		}
		LOG_PRINTF(("transMsg->ProcessingCode = %s ",transMsg->ProcessingCode));
		return _SUCCESS;
}

/****************************************************************************************************************
*	Function Name : InitRefund																											                    *
*	Purpose		    : intialize refund request for a transaction 		 																              *
*	Input					:	Address of main transaction message structure and transaction methord type 						                                              *
*	Output		    : returns success or failure 								                                  *
*										                                          *
*****************************************************************************************************************/

short InitRefund(TransactionMsgStruc *transMsg)
{
	short iRetVal = 0;
	short Selectionstatus;
    char terminalId[TERMINAL_ID_LEN+1]={0};//9
	char cardAccepterId[CARD_ACCEPTOR_ID_LEN+1]={0};//16

	//if(validateMerchantPassword() != _SUCCESS)
	//		return _FAIL;
	
    getTraceAuditNumber(transMsg);  //11 
	get_env("#TERMINAL_ID",(char*)terminalId,sizeof(terminalId)); //41
	sprintf(transMsg->TerminalID,"%s",terminalId); 
	get_env("#CARDACCEPTERID",(char*)cardAccepterId,sizeof(cardAccepterId));
	sprintf(transMsg->CardAcceptorID,"%s",cardAccepterId);  //42
    sprintf(transMsg->_transType,"%s","REFUND");
    
		
	iRetVal = GetInputFromUser(transMsg,TRANSAMOUNT);//getting amount from user input
	if((iRetVal == KEY_STR) || (iRetVal == KEY_CANCEL) || (iRetVal == _FAIL) || (iRetVal == BTN_CANCEL))
		return iRetVal;

    if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF(("TrMethordFlag = %d",transMsg->TrMethordFlag));
    }

#ifdef GOVT_APP //if application is government type
		
    iRetVal = getPaymentId(transMsg);//getting Payment Id from user for Government
		if(( iRetVal ==_FAIL) || (iRetVal == KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL))
		{ 
				return GoToMainScreen;	
		}
	
#endif
	if(transMsg->TrMethordFlag == edebit )//for debit refund
	{
		SetImage(ACC_TYPE_BMP);//setting image for Account type 
		EnblTouchScreen(AccountType);
		Selectionstatus = get_char();
			
		if((Selectionstatus == eChequeAccount) || (Selectionstatus == eSavingAccount) || (Selectionstatus == eCreditAccount))				 
				iRetVal =  debitAccountType(transMsg,Selectionstatus);//call function to perform according to account type like cheque,saving or credit
		else if((Selectionstatus == KEY_CANCEL ) || (Selectionstatus == KEY_STR) || Selectionstatus == eAccTypeBack)
				return GoToMainScreen;		
		EnblTouchScreen(DisableTouch);
	}
	else
	{
		EnblTouchScreen(DisableTouch);
    }
		//if(swipeCardFlag != 1)
		//{
	iRetVal = swipeOrEMVCardEntry(transMsg);//Read card data 
	if((iRetVal == _FAIL ) || (iRetVal == KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL))
	{
		return iRetVal;
	}
    else if(iRetVal == FORCE_FOR_EMV)
    {
      LOG_PRINTF(("InitRefubdTransaction:FORCE_FOR_EMV"));
      iRetVal = EMVCardEntry(transMsg);//Read card data 
			if((iRetVal == (KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL) || (iRetVal == _FAIL)))
			{
					return GoToMainScreen;
			}        //22 POS Entry Mode
    }
    if(iRetVal == FALLBACK)
    {
      LOG_PRINTF(("InitBalanceInquiry:FALLBACK"));
      iRetVal = swipeCardEntry(transMsg);//Read card data 
			if((iRetVal == _FAIL ) || (iRetVal == KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL))
			{
					return iRetVal;
			}
      strcpy((char *)transMsg->POSEntryMode,MSReICC_PIN_CAPABLE);             //22 POS Entry Mode
    }
       // LOG_PRINTF(("swipeCardFlag != 1"));
		//}
	if(transMsg->TrMethordFlag == edebit )//for debit refund
    {
		if((!strcmp((char *)transMsg->POSEntryMode,MSR_PIN_CAPABLE))||(!strcmp((char *)transMsg->POSEntryMode,MSReICC_PIN_CAPABLE)))//FOR MAGNETIC SWIPE
        {
			if (key_injected == 0)
			{
				iRetVal = KeyInjection(transMsg->PrimaryAccNum);
			}
			else
			{
				iRetVal = PinPrompt(transMsg->PrimaryAccNum);//getting user pin
			}
        }
    }
	else if(transMsg->TrMethordFlag == ecredit)
	{
		strcpy(transMsg->ProcessingCode,CREDIT_REFUNDPROCODE);//-<<<<<<<<<<<----------   written for testing purpose 
		iRetVal = _SUCCESS;//for credit card implementation
	}

    if(iRetVal==_SUCCESS)
    {
      iRetVal = RefundTransactionProcessing(transMsg);
      if(LOG_STATUS == LOG_ENABLE)
      {
			  LOG_PRINTF(("ret val = =  %d ",iRetVal));
      }
    }
    else
    {
      return GoToMainScreen;
    }

    if(iRetVal==_SUCCESS)
    {
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("ret val = =  %d ",iRetVal));
		}
		printRecipt(transMsg);
    }
    
	return _SUCCESS;
}

/****************************************************************************************************************
*	Function Name : InitTransfer																											                    *
*	Purpose		    : intialize refund request for a transaction 		 																              *
*	Input					:	Address of main transaction message structure and transaction methord type 						                                              *
*	Output		    : returns success or failure 								                                  *
*										                                          *
*****************************************************************************************************************/

short InitTransfer(TransactionMsgStruc *transMsg)
{
	short iRetVal = 0;
	short Selectionstatus;
    char terminalId[TERMINAL_ID_LEN+1]={0};//9
	char cardAccepterId[CARD_ACCEPTOR_ID_LEN+1]={0};//16
/*		
		if(validateMerchantPassword() != _SUCCESS)		//Disabled for Agency bank
				return _FAIL;
*/
	
	FROM_OTHER_AC=0;
    getTraceAuditNumber(transMsg);  //11 
	get_env("#TERMINAL_ID",(char*)terminalId,sizeof(terminalId)); //41
	sprintf(transMsg->TerminalID,"%s",terminalId); 
	get_env("#CARDACCEPTERID",(char*)cardAccepterId,sizeof(cardAccepterId));
	sprintf(transMsg->CardAcceptorID,"%s",cardAccepterId);  //42
    sprintf(transMsg->_transType,"%s","TRANSFER");
    
	LOG_PRINTF(("TRANSFER: Selectionstatus %d", Selectionstatus));
		
		iRetVal = GetInputFromUser(transMsg,TRANSAMOUNT);//getting amount from user input
		if((iRetVal == KEY_STR) || (iRetVal == KEY_CANCEL) || (iRetVal == _FAIL) || (iRetVal == BTN_CANCEL))
      return iRetVal;
    if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF(("TrMethordFlag = %d",transMsg->TrMethordFlag));
    }

		if(transMsg->TrMethordFlag == edebit )//for debit refund
		{
			SetImage(ACC_TYPE_BMP);//setting image for Account type 
			EnblTouchScreen(Agency_AccountType);
			Selectionstatus = get_char();

			switch (Selectionstatus)
			{
			case eChequeAccount:
				SetImage(TO_ACC_TYPE_BMP_SO);//setting image for Account type 
				EnblTouchScreen(Agency_Sav_Oth_AccountType);
				break;
			case eSavingAccount:
				SetImage(TO_ACC_TYPE_BMP_CO);//setting image for Account type 
				EnblTouchScreen(Agency_Che_Oth_AccountType);
				break;
			case eOtherAccount:
				FROM_OTHER_AC =1;
				iRetVal = getAccountNo(transMsg);//getting Account number
				if((iRetVal == (KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL) || (iRetVal == _FAIL)))
				{
					return iRetVal;
				}
				SetImage(ACC_TYPE_BMP);//setting image for Account type 
				EnblTouchScreen(Agency_ToAccountType);
				break;

			default:
				EnblTouchScreen(DisableTouch);
				return GoToMainScreen;
				//break;
			}
			
					
			if((Selectionstatus == KEY_CANCEL ) || (Selectionstatus == KEY_STR) || Selectionstatus == eAccTypeBack)
			{
				EnblTouchScreen(DisableTouch);
				return GoToMainScreen;	
			}
      
		
		Selectionstatus = get_char();
	//Agency Bank get account
		switch(Selectionstatus)
		{
			case eSavToCheAccount:
				strcpy(transMsg->ProcessingCode,TRANSFER_SAV_TO_CHEPROCODE);
				break;
			case eSavToOthAccount:
				strcpy(transMsg->ProcessingCode,TRANSFER_SAV_TO_OTHPROCODE);
				break;
			case eCheToSavAccount:
				strcpy(transMsg->ProcessingCode,TRANSFER_CHE_TO_SAVPROCODE);
				break;
			case eCheToOthAccount:
				strcpy(transMsg->ProcessingCode,TRANSFER_CHE_TO_OTHPROCODE);
				break;
			case eOthToCheAccount:
				strcpy(transMsg->ProcessingCode,TRANSFER_OTH_TO_CHEPROCODE);
				break;
			case eOthToSavAccount:
				strcpy(transMsg->ProcessingCode,TRANSFER_OTH_TO_SAVPROCODE);
				break;
			case eOthToOthAccount:
				strcpy(transMsg->ProcessingCode,TRANSFER_OTH_TO_OTHPROCODE);
				break;
			default:
				EnblTouchScreen(DisableTouch);
				return GoToMainScreen;
				//break;
		}
		LOG_PRINTF(("TRANSFER: Selectionstatus %d Processing Code %s", Selectionstatus,transMsg->ProcessingCode));
		if(Selectionstatus== eSavToOthAccount || Selectionstatus== eCheToOthAccount || Selectionstatus== eOthToOthAccount)
		{
			OTHER_AC=1;
			AC_TYPE='T';		//Set Ac_type to 'T' To account
			iRetVal = getAccountNo(transMsg);//getting Account number
			AC_TYPE='F';	//Set Ac_type back to 'F' From account
			if((iRetVal == (KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL) || (iRetVal == _FAIL)))
			{
				return iRetVal;
			}
			Selectionstatus = eCashBackNo;
		}

	}
    else
    {
      EnblTouchScreen(DisableTouch);
    }
		//if(swipeCardFlag != 1)
		//{
		iRetVal = swipeOrEMVCardEntry(transMsg);//Read card data 
		if((iRetVal == _FAIL ) || (iRetVal == KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL))
		{
				return iRetVal;
		}
    else if(iRetVal == FORCE_FOR_EMV)
    {
      LOG_PRINTF(("InitTransferTransaction:FORCE_FOR_EMV"));
      iRetVal = EMVCardEntry(transMsg);//Read card data 
			if((iRetVal == (KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL) || (iRetVal == _FAIL)))
			{
					return GoToMainScreen;
			}        //22 POS Entry Mode
    }
    if(iRetVal == FALLBACK)
    {
      LOG_PRINTF(("Init:FALLBACK"));
      iRetVal = swipeCardEntry(transMsg);//Read card data 
			if((iRetVal == _FAIL ) || (iRetVal == KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL))
			{
					return iRetVal;
			}
      strcpy((char *)transMsg->POSEntryMode,MSReICC_PIN_CAPABLE);             //22 POS Entry Mode
    }
       // LOG_PRINTF(("swipeCardFlag != 1"));
		//}
		if(transMsg->TrMethordFlag == edebit )//for debit refund
    {
       if((!strcmp((char *)transMsg->POSEntryMode,MSR_PIN_CAPABLE))||(!strcmp((char *)transMsg->POSEntryMode,MSReICC_PIN_CAPABLE)))//FOR MAGNETIC SWIPE
       {
				  iRetVal = PinPrompt(transMsg->PrimaryAccNum);//getting user pin 
       }
    }
		else if(transMsg->TrMethordFlag == ecredit)
		{
				strcpy(transMsg->ProcessingCode,CREDIT_REFUNDPROCODE);//-<<<<<<<<<<<----------   written for testing purpus 
				iRetVal = _SUCCESS;//for credit card implementation
		}
    if(iRetVal==_SUCCESS)
    {
      iRetVal = TransferTransactionProcessing(transMsg);
      if(LOG_STATUS == LOG_ENABLE)
      {
			  LOG_PRINTF(("ret val = =  %d ",iRetVal));
      }
    }
    else
    {
      return GoToMainScreen;
    }

    if(iRetVal==_SUCCESS)
    {
      if(LOG_STATUS == LOG_ENABLE)
      {
				LOG_PRINTF(("ret val = =  %d ",iRetVal));
      }
		  printRecipt(transMsg);
    }
    
		return _SUCCESS;
}

/****************************************************************************************************************
*	Function Name : InitCardActivation																			*
*	Purpose		    : intialize a card activation request		 		 										*
*	Input			: Address of main transaction message structure and transaction method type 				*
*	Output		    : returns success or failure 								                                *
*																												*
*****************************************************************************************************************/
short InitCardActivation(TransactionMsgStruc *transMsg)
{

	short iRetVal = 0;
	char terminalId[TERMINAL_ID_LEN + 1] = { 0 };//9
	char cardAccepterId[CARD_ACCEPTOR_ID_LEN + 1] = { 0 };//16

	LOG_PRINTF(("InitCardActivation variables initializad"))

	getTraceAuditNumber(transMsg);  //11 
	get_env("#TERMINAL_ID", (char*)terminalId, sizeof(terminalId)); //41
	sprintf(transMsg->TerminalID, "%s", terminalId);
	get_env("#CARDACCEPTERID", (char*)cardAccepterId, sizeof(cardAccepterId));
	sprintf(transMsg->CardAcceptorID, "%s", cardAccepterId);  //42
	sprintf(transMsg->_transType, "%s", "CARD ACTIVATION");

	LOG_PRINTF(("PROMPT FOR CARD"));
	iRetVal = swipeOrEMVCardEntry(transMsg);//Read card data 
	if ((iRetVal == _FAIL) || (iRetVal == KEY_STR) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL))
	{
		return iRetVal;
	}
	else if (iRetVal == FORCE_FOR_EMV)
	{
		LOG_PRINTF(("InitRefubdTransaction:FORCE_FOR_EMV"));
		iRetVal = EMVCardEntry(transMsg);//Read card data 
		if ((iRetVal == (KEY_STR) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL) || (iRetVal == _FAIL)))
		{
			return GoToMainScreen;
		}        //22 POS Entry Mode
	}
	if (iRetVal == FALLBACK)
	{
		LOG_PRINTF(("Init:FALLBACK"));
		iRetVal = swipeCardEntry(transMsg);//Read card data 
		if ((iRetVal == _FAIL) || (iRetVal == KEY_STR) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL))
		{
			return iRetVal;
		}
		strcpy((char *)transMsg->POSEntryMode, MSReICC_PIN_CAPABLE);             //22 POS Entry Mode
	}

	LOG_PRINTF(("transMsg->POSEntryMode=%s ", transMsg->POSEntryMode));

	if (!strcmp(transMsg->POSEntryMode, "051"))
	{
		if (key_injected == 0)
		{
			LOG_PRINTF(("Init:KeyInjection"));
			iRetVal = KeyInjection(transMsg->PrimaryAccNum);
		}
		else
		{
			LOG_PRINTF(("Init:PinPrompt"));
			iRetVal = PinPrompt(transMsg->PrimaryAccNum);//getting user pin 
		}
	}


	LOG_PRINTF(("pinblock=%s ", transMsg->PinData));

	LOG_PRINTF(("Card read successful, passing to function handle bitmap and host transmission"));
	if (iRetVal == _SUCCESS)
	{
		iRetVal = CardActivationTransactionProcessing(transMsg);
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("ret val = =  %d ", iRetVal));
		}
	}
	else
	{
		return GoToMainScreen;
	}

	if (iRetVal == _SUCCESS)
	{
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("ret val = =  %d ", iRetVal));
		}
		printRecipt(transMsg);
	}

	return _SUCCESS;
}


/****************************************************************************************************************
*	Function Name : InitPinChange																				*
*	Purpose		    : intialize a pin change request					 										*
*	Input			: Address of main transaction message structure and transaction method type 				*
*	Output		    : returns success or failure 								                                *
*																												*
*****************************************************************************************************************/
short InitPinChange(TransactionMsgStruc *transMsg)
{
	short iRetVal = 0;
	char terminalId[TERMINAL_ID_LEN + 1] = { 0 };//9
	char cardAccepterId[CARD_ACCEPTOR_ID_LEN + 1] = { 0 };//16

	LOG_PRINTF(("InitPinChange variables initializad"))

	getTraceAuditNumber(transMsg);  //11 
	get_env("#TERMINAL_ID", (char*)terminalId, sizeof(terminalId)); //41
	sprintf(transMsg->TerminalID, "%s", terminalId);
	get_env("#CARDACCEPTERID", (char*)cardAccepterId, sizeof(cardAccepterId));
	sprintf(transMsg->CardAcceptorID, "%s", cardAccepterId);  //42
	sprintf(transMsg->_transType, "%s", "PIN CHANGE");

	LOG_PRINTF(("PROMPT FOR CARD"));
	iRetVal = swipeOrEMVCardEntry(transMsg);//Read card data 
	if ((iRetVal == _FAIL) || (iRetVal == KEY_STR) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL))
	{
		return iRetVal;
	}
	else if (iRetVal == FORCE_FOR_EMV)
	{
		LOG_PRINTF(("InitRefubdTransaction:FORCE_FOR_EMV"));
		iRetVal = EMVCardEntry(transMsg);//Read card data 
		if ((iRetVal == (KEY_STR) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL) || (iRetVal == _FAIL)))
		{
			return GoToMainScreen;
		}        //22 POS Entry Mode
	}
	if (iRetVal == FALLBACK)
	{
		LOG_PRINTF(("Init:FALLBACK"));
		iRetVal = swipeCardEntry(transMsg);//Read card data 
		if ((iRetVal == _FAIL) || (iRetVal == KEY_STR) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL))
		{
			return iRetVal;
		}
		strcpy((char *)transMsg->POSEntryMode, MSReICC_PIN_CAPABLE);             //22 POS Entry Mode
	}
	if (!strcmp(transMsg->POSEntryMode, "051"))
	{
		if (key_injected == 0)
		{
			iRetVal = KeyInjection(transMsg->PrimaryAccNum);
			LOG_PRINTF(("iRetval old pin=%d ", iRetVal));
			iRetVal = PinPrompt(transMsg->PrimaryAccNum);//getting new user pin
			LOG_PRINTF(("iRetval new pin=%d ", iRetVal));
		}
		else
		{
			iRetVal = PinPrompt(transMsg->PrimaryAccNum);//getting old user pin
			LOG_PRINTF(("iRetval old pin=%d ", iRetVal));
			iRetVal = PinPrompt(transMsg->PrimaryAccNum);//getting new user pin
			LOG_PRINTF(("iRetval new pin=%d ", iRetVal));
		}
	}
	else
	{
		iRetVal = PinPrompt(transMsg->PrimaryAccNum);//getting new user pin
		LOG_PRINTF(("iRetval new pin=%d ", iRetVal));
	}


	LOG_PRINTF(("pinblock=%s ", transMsg->PinData));

	LOG_PRINTF(("Card read successful, passing to function handle bitmap and host transmission"));
	if (iRetVal == _SUCCESS)
	{
		iRetVal = PinChangeTransactionProcessing(transMsg);
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("ret val = =  %d ", iRetVal));
		}
	}
	else
	{
		return GoToMainScreen;
	}

	if (iRetVal == _SUCCESS)
	{
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("ret val = =  %d ", iRetVal));
		}
		printRecipt(transMsg);
	}

	return _SUCCESS;
}


/****************************************************************************************************************
*	Function Name : InitDeposit																											                    *
*	Purpose		    : intialize refund request for a transaction 		 																              *
*	Input					:	Address of main transaction message structure and transaction methord type 						                                              *
*	Output		    : returns success or failure 								                                  *
*										                                          *
*****************************************************************************************************************/


short InitDeposit(TransactionMsgStruc *transMsg)
{
		short iRetVal = 0;
		short Selectionstatus;
		char terminalId[TERMINAL_ID_LEN+1]={0};//9
		char cardAccepterId[CARD_ACCEPTOR_ID_LEN+1]={0};//16
/*		
		if(validateMerchantPassword() != _SUCCESS)		//Disabled for Agency bank
				return _FAIL;
*/
	//AC_TYPE='T';
    getTraceAuditNumber(transMsg);  //11 
		get_env("#TERMINAL_ID",(char*)terminalId,sizeof(terminalId)); //41
		sprintf(transMsg->TerminalID,"%s",terminalId); 
		get_env("#CARDACCEPTERID",(char*)cardAccepterId,sizeof(cardAccepterId));
		sprintf(transMsg->CardAcceptorID,"%s",cardAccepterId);  //42
    if(transMsg->TrTypeFlag == WITHDRAWAL_MSG_TYPE_CASE)
		sprintf(transMsg->_transType,"%s","WITHDRAW");
	else
	{
		sprintf(transMsg->_transType,"%s","DEPOSIT");
		AC_TYPE='T';		//Set Ac_type to 'T' To account
	}
    
		
		iRetVal = GetInputFromUser(transMsg,TRANSAMOUNT);//getting amount from user input
		if((iRetVal == KEY_STR) || (iRetVal == KEY_CANCEL) || (iRetVal == _FAIL) || (iRetVal == BTN_CANCEL))
      return iRetVal;
    if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF(("TrMethordFlag = %d",transMsg->TrMethordFlag));
    }

		if(transMsg->TrMethordFlag == edebit )//for debit refund
		{
			SetImage(ACC_TYPE_BMP);//setting image for Account type 
			EnblTouchScreen(Agency_AccountType);
			Selectionstatus = get_char();
				
			if((Selectionstatus == eChequeAccount) || (Selectionstatus == eSavingAccount) || (Selectionstatus == eOtherAccount))				 
					iRetVal =  debitAccountType(transMsg,Selectionstatus);//call function to perform according to account type like cheque,saving or credit
			else if((Selectionstatus == KEY_CANCEL ) || (Selectionstatus == KEY_STR) || Selectionstatus == eAccTypeBack)
					return GoToMainScreen;		
      EnblTouchScreen(DisableTouch);
		}
    else
    {
      EnblTouchScreen(DisableTouch);
    }
	//Agency Bank get account
	if(Selectionstatus== eOtherAccount)
	{
		OTHER_AC=1;
	
		//AC_TYPE='T';		//Set Ac_type to 'T' To account
		iRetVal = getAccountNo(transMsg);//getting Account number
		AC_TYPE='F';	//Set Ac_type back to 'F' From account
		if((iRetVal == (KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL) || (iRetVal == _FAIL)))
		{
			return iRetVal;
		}
		Selectionstatus = eCashBackNo;
	}

		//if(swipeCardFlag != 1)
		//{
		iRetVal = swipeOrEMVCardEntry(transMsg);//Read card data 
		if((iRetVal == _FAIL ) || (iRetVal == KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL))
		{
				return iRetVal;
		}
		else if(iRetVal == FORCE_FOR_EMV)
		{
			LOG_PRINTF(("InitRefubdTransaction:FORCE_FOR_EMV"));
			iRetVal = EMVCardEntry(transMsg);//Read card data 
			if((iRetVal == (KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL) || (iRetVal == _FAIL)))
				{
					return GoToMainScreen;
				}        //22 POS Entry Mode
		}
		if(iRetVal == FALLBACK)
		{
			LOG_PRINTF(("Init:FALLBACK"));
			iRetVal = swipeCardEntry(transMsg);//Read card data 
			if((iRetVal == _FAIL ) || (iRetVal == KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL))
			{
				return iRetVal;
			}
			strcpy((char *)transMsg->POSEntryMode,MSReICC_PIN_CAPABLE);             //22 POS Entry Mode
		}
       // LOG_PRINTF(("swipeCardFlag != 1"));
		//}
		if(transMsg->TrMethordFlag == edebit )//for debit refund
    {
       if((!strcmp((char *)transMsg->POSEntryMode,MSR_PIN_CAPABLE))||(!strcmp((char *)transMsg->POSEntryMode,MSReICC_PIN_CAPABLE)))//FOR MAGNETIC SWIPE
       {
				  iRetVal = PinPrompt(transMsg->PrimaryAccNum);//getting user pin 
       }
    }
	/*	else if(transMsg->TrMethordFlag == ecredit)
		{
				strcpy(transMsg->ProcessingCode,CREDIT_REFUNDPROCODE);//-<<<<<<<<<<<----------   written for testing purpus 
				iRetVal = _SUCCESS;//for credit card implementation
		}*/
    if(iRetVal==_SUCCESS)
    {
      iRetVal = RefundTransactionProcessing(transMsg);
      if(LOG_STATUS == LOG_ENABLE)
      {
			  LOG_PRINTF(("ret val = =  %d ",iRetVal));
      }
    }
    else
    {
      return GoToMainScreen;
    }

    if(iRetVal==_SUCCESS)
    {
      if(LOG_STATUS == LOG_ENABLE)
      {
				LOG_PRINTF(("ret val = =  %d ",iRetVal));
      }
		  printRecipt(transMsg);
    }
    
		return _SUCCESS;
}

/****************************************************************************************************************
*	Function Name	: CardActivationTransactionProcessing										                *
*	Purpose		    : Intialize field table ,assemble-disassemble data 	for card activation transaction         *
*	Input			: Address of main transaction message structure and transaction method type					*
*	Output		    : Returns success or failure 								                                *
*****************************************************************************************************************/
short CardActivationTransactionProcessing(TransactionMsgStruc *transMsg)
{
	short retVal = -1;
	int i = 0;
	char Bitmap[BITMAP_ARRAY_SIZE + 1] = { 0 }; //8
	char UnpackBiitmap[UNPACK_BITMAP_ARRAY_SIZE + 1] = { 0 };//16
	char bits[BITMAP_SIZE + 1] = { 0 };//64
	char temp[BITMAP_ARRAY_SIZE + 1] = { 0 };//9

#ifndef EMV_ENABLE
	unsigned char reqBuff[REFUND_REQUEST_SIZE] = { 0 };//96
#else
	unsigned char reqBuff[REFUND_REQUEST_SIZE + ICC_RELATED_DATA_LEN + 2] = { 0 };//96+151+2
#endif 
																				  
	unsigned char resBuff[REFUND_RESPONSE_SIZE] = { 0 };//100 to 200 after e2e encryprion
	unsigned char testBuff[REFUND_UNAPACK_RES_SIZE] = { 0 };//200 to 400 after e2e encryprion
	short ReqLen = 0;
	short recvLength = 0;

	field_struct iso8583_field_table[] =
	{
		/*Fld 8583 Convert Variable name and size no.	sz 	index *//*BCD_BCD,BCD_STR*/ //ASC_ASC->
		{ 0, TPDU_LEN,BCD_STR,(void *)(BitmapStructOb.tpdu), sizeof(BitmapStructOb.tpdu) },
		{ 0, MSG_ID_LEN, BCD_ASC, (void *)BitmapStructOb.message_id, sizeof(BitmapStructOb.message_id) },
		{ 2 + SKIP, 19, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 3, 6, BCD_STR, (void *)BitmapStructOb.field_03, sizeof(BitmapStructOb.field_03) },
		{ 4 + SKIP, 12, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 5 + SKIP, 12, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 6 + SKIP, 12, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 7 + SKIP, 10, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 8 + SKIP, 8, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 9 + SKIP, 8, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 10 + SKIP, 8, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 11, 6, BCD_STR, (void *)BitmapStructOb.field_11, sizeof(BitmapStructOb.field_11) },
		{ 12 + SKIP, 8, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 13 + SKIP, 8, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		//{ 12, 6, BCD_STR, (void *)BitmapStructOb.field_12, sizeof(BitmapStructOb.field_12) },
		//{ 13, 4, BCD_STR, (void *)BitmapStructOb.field_13, sizeof(BitmapStructOb.field_13) },
		{ 14 + SKIP, 4, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 15 + SKIP, 4, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 16 + SKIP, 4, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 17 + SKIP, 4, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 18 + SKIP, 4, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 19 + SKIP, 3, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 20 + SKIP, 3, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 21 + SKIP, 3, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 22, 3, BCD_STR, (void *)BitmapStructOb.field_22, sizeof(BitmapStructOb.field_22) },
		{ 23 + SKIP, 3, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 24, 3, BCD_STR, (void *)BitmapStructOb.field_24, sizeof(BitmapStructOb.field_24) },
		{ 25, 2, BCD_STR, (void *)BitmapStructOb.field_25, sizeof(BitmapStructOb.field_25) },
		{ 26 + SKIP, 2, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 27 + SKIP, 1, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 28 + SKIP, 8, XBC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 29 + SKIP, 8, XBC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 30 + SKIP, 8, XBC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 31 + SKIP, 8, XBC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 32 + SKIP, 11, BV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 33 + SKIP, 11, BV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 34 + SKIP, 28, AV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 35, 37, BV2_STR, (void *)BitmapStructOb.field_35, sizeof(BitmapStructOb.field_35) },
		{ 36 + SKIP, 104, AV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 37, 12, BCD_STR, (void *)BitmapStructOb.field_37, sizeof(BitmapStructOb.field_37) },
		{ 38, 6, BCD_STR, (void *)BitmapStructOb.field_38, sizeof(BitmapStructOb.field_38) },
		{ 39, 2, BCD_STR, (void *)BitmapStructOb.field_39, sizeof(BitmapStructOb.field_39) },
		{ 40 + SKIP, 3, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 41, 8, ASC_STR, (void *)BitmapStructOb.field_41, sizeof(BitmapStructOb.field_41) },
		{ 42, 15, ASC_STR, (void *)BitmapStructOb.field_42, sizeof(BitmapStructOb.field_42) },
		{ 43 + SKIP, 40, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 44 + SKIP, 25, AV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 45 + SKIP, 76, AV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 46 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 47 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		//{ 48, 9, AV3_STR, (void *) BitmapStructOb.field_48, sizeof( BitmapStructOb.field_48) }, 
		{ 48 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 49 + SKIP, 3, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 50 + SKIP, 3, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 51 + SKIP, 3, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 52, 64, BIT_BIT, (void *)BitmapStructOb.field_52, sizeof(BitmapStructOb.field_52) },
		{ 53 + SKIP, 16, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 54 + SKIP, 120, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		//{ 54, 999, AV3_STR, (void *)BitmapStructOb.field_54, sizeof(BitmapStructOb.field_54) },
		{ 55 + SKIP,  0, BIT_BIT, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 56 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 57 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 58 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 59 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 60 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 61 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 62 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 63 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 64 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 65 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 66 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 67 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 68 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) }, 
		{ 69 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 70 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 71 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 72 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 73 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 74 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 75 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 76 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 77 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 78 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 79 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 80 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 81 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 82 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 83 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 84 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 85 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 86 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 87 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 88 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 89 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 90 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 91 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 92 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 93 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 94 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 95 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 96 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 97 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 98 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 99 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 100 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 101 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		//{ 102+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) },
		//{ 102, 12, AV2_STR, (void *)BitmapStructOb.field_102, sizeof(BitmapStructOb.field_102) },
		//{ 103, 12, AV2_STR, (void *)BitmapStructOb.field_103, sizeof(BitmapStructOb.field_103) },
		{ 102 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 103 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 104 + SKIP + STOP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	};
	strcpy(msg_id, ACTIVATIONMSGTYPE);
	strcpy(Proc_code, CHECK_ACTIVATIONPROCODE);

	LOG_PRINTF(("pinblock=%s ", transMsg->PinData));

	strcpy(transMsg->ProcessingCode, Proc_code);
	strcpy(transMsg->MessageTypeInd, ACTIVATIONMSGTYPE);
	BITMAP_LEN = 8;
	//memset(&BitmapStructOb, 0, sizeof(BmapStruct));
	resetBitmapStructure(&BitmapStructOb);//memsetting all the elements of structure

	ProcessingISOBitmapEngine(iso8583_field_table, transMsg);
	ReqLen = retVal = assemble_packet(iso8583_field_table, reqBuff);

	if (retVal <= _SUCCESS)
	{
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Error Occurred in assemble_packet retVal = %d \n", retVal));
		}
		return retVal;
	}

	recvLength = DEFAULT_BUFLEN;
	retVal = CommWithServer((char*)reqBuff, ReqLen, (char*)resBuff, &recvLength);
	//retVal = _FAIL ;
	if (retVal == _SUCCESS)//checking for response buffer size 
	{
		bcd2a((char *)testBuff, (char *)resBuff, recvLength);
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Testbuffin disassemblePacket retVal = %s \n", testBuff));
			LOG_PRINTF(("Testbuffin Bitmap 1 = %c \n", testBuff[18]));
		}
		if (testBuff[18] >= '8')
			BITMAP_LEN = 16;
		//strncpy(UnpackBiitmap,(char*)testBuff+UNPACK_SIZE_MSGLEN_OF_REQ_RES+TPDU_LEN+MSG_ID_LEN,UNPACK_BITMAP_ARRAY_SIZE);
		strncpy(UnpackBiitmap, (char*)testBuff + UNPACK_SIZE_MSGLEN_OF_REQ_RES + TPDU_LEN + MSG_ID_LEN, BITMAP_LEN * 2);

		LOG_PRINTF(("UnpackBiitmap = %s", UnpackBiitmap));

		packData(UnpackBiitmap, Bitmap);
		//for(i=0;i<BITMAP_ARRAY_SIZE;i++)
		for (i = 0;i<BITMAP_LEN;i++)
		{
			CopyAllBits(Bitmap + i, temp);
			strcpy(bits + (i * 8), temp);
		}
		//memset(&BitmapStructOb,0,sizeof(BitmapStructOb));
		resetBitmapStructure(&BitmapStructOb);//memsetting all the elements of structure
		ParseAndFillBitFields((char*)testBuff, bits);

		//------------------------------------------------------------
		strcpy((char *)transMsg->TransLocalTime, (char*)BitmapStructOb.field_12);
		strcpy((char *)transMsg->TransLocalDate, (char*)BitmapStructOb.field_13);
		strcpy((char *)transMsg->ResponseCode, (char*)BitmapStructOb.field_39);             //copy the response code
		strcpy((char *)transMsg->AuthIdResponse, (char*)BitmapStructOb.field_38);           //copy AuthIdResponse
		strcpy((char *)transMsg->RetrievalReferenceNo, (char*)BitmapStructOb.field_37);     //copy RetrievalReferenceNo

		window(1, 1, 30, 20);
		clrscr();
		if (transMsg->EMV_Flag == 1)
		{
			display_TVR_TSI(transMsg);
		}
		if (!strcmp((char *)BitmapStructOb.field_39, "00"))
		{
			/*  window(1,1,30,20);
			clrscr();*/
			strcpy(transMsg->ResponseText, APPROVED_MSG);
			strcpy(transMsg->FromAcNo, (char *)BitmapStructOb.field_102);
			strcpy(transMsg->ToAcNo, (char *)BitmapStructOb.field_103);
			LOG_PRINTF(("From Account = %s", transMsg->FromAcNo));
			LOG_PRINTF(("TO Account = %s", transMsg->ToAcNo));

			SaveTransDetails(transMsg);

			write_at(Dmsg[TRANSACTION_SUCCESSFULL].dispMsg, strlen(Dmsg[TRANSACTION_SUCCESSFULL].dispMsg), Dmsg[TRANSACTION_SUCCESSFULL].x_cor, Dmsg[TRANSACTION_SUCCESSFULL].y_cor);
			SVC_WAIT(2000);
			ClearKbdBuff();
			KBD_FLUSH();
			if (LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("Refund/Deposit successfull !!\n"));
			}
		}
		else if (!strcmp((char *)BitmapStructOb.field_39, "55"))
		{
			/*	window(1,1,30,20);
			clrscr();*/
			write_at(Dmsg[INVALID_PIN_INDX].dispMsg, strlen(Dmsg[INVALID_PIN_INDX].dispMsg), Dmsg[INVALID_PIN_INDX].x_cor, Dmsg[INVALID_PIN_INDX].y_cor);//invalid pin
			strcpy(transMsg->ResponseText, "INVALID PIN");
			SVC_WAIT(2000);
			ClearKbdBuff();
			KBD_FLUSH();
		}
		else if (!strcmp((char *)BitmapStructOb.field_39, "51"))
		{
			/*	window(1,1,30,20);
			clrscr();*/
			write_at(Dmsg[INVALID_PIN_INDX].dispMsg, strlen(Dmsg[INSUFFICIENT_FUND].dispMsg), Dmsg[INSUFFICIENT_FUND].x_cor, Dmsg[INSUFFICIENT_FUND].y_cor);//INSUFFICIENT_FUNDS
			strcpy(transMsg->ResponseText, "INSUFFICIENT_FUNDS");
			SVC_WAIT(2000);
			ClearKbdBuff();
			KBD_FLUSH();

		}
		else
		{
			/*window(1,1,30,20);
			clrscr();
			*/
			write_at(Dmsg[TRANSACTION_FAILED].dispMsg, strlen(Dmsg[TRANSACTION_FAILED].dispMsg), Dmsg[TRANSACTION_FAILED].x_cor, Dmsg[TRANSACTION_FAILED].y_cor);

			SVC_WAIT(1000);
			ClearKbdBuff();
			KBD_FLUSH();

			strcpy(transMsg->ResponseText, "DECLINE");
			if (LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("Refund Transaction Fail reason : %s \n", BitmapStructOb.field_39));
			}
		}
		/*if(transMsg->EMV_Flag == 1 )
		{
		display_TVR_TSI(transMsg);
		}*/
		return _SUCCESS;
	}
	/*else
	{
		if (retVal == RECV_FAILED)//if did not receive a valid response before the transaction time-out period expired
		{
			if (SaveReversalDetails(transMsg) == _SUCCESS)//saves reversal data
			{
				retVal = InitReversal();//go for reversal
				if (LOG_STATUS == LOG_ENABLE)
				{
					LOG_PRINTF(("reversal retVal =%d", retVal));
				}
			}
		}
		else
		{
			if (LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("Response buffer size is zero for Refund\n"));
			}
		}
	}*/
	return _FAIL;
}



/****************************************************************************************************************
*	Function Name	: PinChangeTransactionProcessing     										                *
*	Purpose		    : Intialize field table ,assemble-disassemble data 	for pin change transaction              *
*	Input			: Address of main transaction message structure and transaction method type					*
*	Output		    : Returns success or failure 								                                *
*****************************************************************************************************************/
short PinChangeTransactionProcessing(TransactionMsgStruc *transMsg)
{
	short retVal = -1;
	int i = 0;
	char Bitmap[BITMAP_ARRAY_SIZE + 1] = { 0 }; //8
	char UnpackBiitmap[UNPACK_BITMAP_ARRAY_SIZE + 1] = { 0 };//16
	char bits[BITMAP_SIZE + 1] = { 0 };//64
	char temp[BITMAP_ARRAY_SIZE + 1] = { 0 };//9

#ifndef EMV_ENABLE
	unsigned char reqBuff[REFUND_REQUEST_SIZE] = { 0 };//96
#else
	unsigned char reqBuff[REFUND_REQUEST_SIZE + ICC_RELATED_DATA_LEN + 2] = { 0 };//96+151+2
#endif 

	unsigned char resBuff[REFUND_RESPONSE_SIZE] = { 0 };//100 to 200 after e2e encryprion
	unsigned char testBuff[REFUND_UNAPACK_RES_SIZE] = { 0 };//200 to 400 after e2e encryprion
	short ReqLen = 0;
	short recvLength = 0;

	field_struct iso8583_field_table[] =
	{
		/*Fld 8583 Convert Variable name and size no.	sz 	index *//*BCD_BCD,BCD_STR*/ //ASC_ASC->
		{ 0, TPDU_LEN,BCD_STR,(void *)(BitmapStructOb.tpdu), sizeof(BitmapStructOb.tpdu) },
		{ 0, MSG_ID_LEN, BCD_ASC, (void *)BitmapStructOb.message_id, sizeof(BitmapStructOb.message_id) },
		{ 2 + SKIP, 19, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 3, 6, BCD_STR, (void *)BitmapStructOb.field_03, sizeof(BitmapStructOb.field_03) },
		{ 4 + SKIP, 12, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 5 + SKIP, 12, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 6 + SKIP, 12, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 7 + SKIP, 10, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 8 + SKIP, 8, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 9 + SKIP, 8, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 10 + SKIP, 8, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 11, 6, BCD_STR, (void *)BitmapStructOb.field_11, sizeof(BitmapStructOb.field_11) },
		{ 12 + SKIP, 8, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 13 + SKIP, 8, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 14 + SKIP, 4, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 15 + SKIP, 4, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 16 + SKIP, 4, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 17 + SKIP, 4, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 18 + SKIP, 4, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 19 + SKIP, 3, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 20 + SKIP, 3, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 21 + SKIP, 3, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 22, 3, BCD_STR, (void *)BitmapStructOb.field_22, sizeof(BitmapStructOb.field_22) },
		{ 23 + SKIP, 3, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 24, 3, BCD_STR, (void *)BitmapStructOb.field_24, sizeof(BitmapStructOb.field_24) },
		{ 25, 2, BCD_STR, (void *)BitmapStructOb.field_25, sizeof(BitmapStructOb.field_25) },
		{ 26 + SKIP, 2, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 27 + SKIP, 1, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 28 + SKIP, 8, XBC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 29 + SKIP, 8, XBC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 30 + SKIP, 8, XBC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 31 + SKIP, 8, XBC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 32 + SKIP, 11, BV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 33 + SKIP, 11, BV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 34 + SKIP, 28, AV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 35, 37, BV2_STR, (void *)BitmapStructOb.field_35, sizeof(BitmapStructOb.field_35) },
		{ 36 + SKIP, 104, AV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 37 + SKIP, 104, AV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 38 + SKIP, 104, AV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 39 + SKIP, 104, AV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		//{ 37, 12, BCD_STR, (void *)BitmapStructOb.field_37, sizeof(BitmapStructOb.field_37) },
		//{ 38, 6, BCD_STR, (void *)BitmapStructOb.field_38, sizeof(BitmapStructOb.field_38) },
		//{ 39, 2, BCD_STR, (void *)BitmapStructOb.field_39, sizeof(BitmapStructOb.field_39) },
		{ 40 + SKIP, 3, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 41, 8, ASC_STR, (void *)BitmapStructOb.field_41, sizeof(BitmapStructOb.field_41) },
		{ 42, 15, ASC_STR, (void *)BitmapStructOb.field_42, sizeof(BitmapStructOb.field_42) },
		{ 43 + SKIP, 40, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 44 + SKIP, 25, AV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 45 + SKIP, 76, AV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 46 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 47 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 48 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 49 + SKIP, 3, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 50 + SKIP, 3, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 51 + SKIP, 3, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 52, 64, BIT_BIT, (void *)BitmapStructOb.field_52, sizeof(BitmapStructOb.field_52) },
		{ 53 + SKIP, 16, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 54 + SKIP, 120, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		//{ 54, 999, AV3_STR, (void *)BitmapStructOb.field_54, sizeof(BitmapStructOb.field_54) },
		{ 55 + SKIP,  0, BIT_BIT, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 56 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 57 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 58 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 59 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 60 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 61 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 62 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 63, 96, BIT_BIT, (void *)BitmapStructOb.field_63, sizeof(BitmapStructOb.field_63) },
		//{ 63 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 64 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 65 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 66 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 67 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 68 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 69 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 70 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 71 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 72 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 73 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 74 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 75 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 76 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 77 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 78 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 79 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 80 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 81 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 82 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 83 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 84 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 85 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 86 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 87 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 88 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 89 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 90 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 91 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 92 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 93 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 94 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 95 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 96 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 97 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 98 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 99 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 100 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 101 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 102 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 103 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 104 + SKIP + STOP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	};
	strcpy(msg_id, PINCHANGEMSGTYPE);
	strcpy(Proc_code, CHECK_PINCHANGEPROCODE);

	strcpy(transMsg->ProcessingCode, Proc_code);
	strcpy(transMsg->MessageTypeInd, ACTIVATIONMSGTYPE);
	BITMAP_LEN = 8;
	//memset(&BitmapStructOb, 0, sizeof(BmapStruct));
	resetBitmapStructure(&BitmapStructOb);//memsetting all the elements of structure

	ProcessingISOBitmapEngine(iso8583_field_table, transMsg);
	ReqLen = retVal = assemble_packet(iso8583_field_table, reqBuff);



	if (retVal <= _SUCCESS)
	{
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Error Occurred in assemble_packet retVal = %d \n", retVal));
		}
		return retVal;
	}

	recvLength = DEFAULT_BUFLEN;
	retVal = CommWithServer((char*)reqBuff, ReqLen, (char*)resBuff, &recvLength);
	//retVal = _FAIL ;
	if (retVal == _SUCCESS)//checking for response buffer size 
	{
		bcd2a((char *)testBuff, (char *)resBuff, recvLength);
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Testbuffin disassemblePacket retVal = %s \n", testBuff));
			LOG_PRINTF(("Testbuffin Bitmap 1 = %c \n", testBuff[18]));
		}
		if (testBuff[18] >= '8')
			BITMAP_LEN = 16;
		//strncpy(UnpackBiitmap,(char*)testBuff+UNPACK_SIZE_MSGLEN_OF_REQ_RES+TPDU_LEN+MSG_ID_LEN,UNPACK_BITMAP_ARRAY_SIZE);
		strncpy(UnpackBiitmap, (char*)testBuff + UNPACK_SIZE_MSGLEN_OF_REQ_RES + TPDU_LEN + MSG_ID_LEN, BITMAP_LEN * 2);

		LOG_PRINTF(("UnpackBiitmap = %s", UnpackBiitmap));

		packData(UnpackBiitmap, Bitmap);
		//for(i=0;i<BITMAP_ARRAY_SIZE;i++)
		for (i = 0;i<BITMAP_LEN;i++)
		{
			CopyAllBits(Bitmap + i, temp);
			strcpy(bits + (i * 8), temp);
		}
		//memset(&BitmapStructOb,0,sizeof(BitmapStructOb));
		resetBitmapStructure(&BitmapStructOb);//memsetting all the elements of structure
		ParseAndFillBitFields((char*)testBuff, bits);

		//------------------------------------------------------------
		strcpy((char *)transMsg->TransLocalTime, (char*)BitmapStructOb.field_12);
		strcpy((char *)transMsg->TransLocalDate, (char*)BitmapStructOb.field_13);
		strcpy((char *)transMsg->ResponseCode, (char*)BitmapStructOb.field_39);             //copy the response code
		strcpy((char *)transMsg->AuthIdResponse, (char*)BitmapStructOb.field_38);           //copy AuthIdResponse
		strcpy((char *)transMsg->RetrievalReferenceNo, (char*)BitmapStructOb.field_37);     //copy RetrievalReferenceNo

		window(1, 1, 30, 20);
		clrscr();
		if (transMsg->EMV_Flag == 1)
		{
			display_TVR_TSI(transMsg);
		}
		if (!strcmp((char *)BitmapStructOb.field_39, "00"))
		{
			/*  window(1,1,30,20);
			clrscr();*/
			strcpy(transMsg->ResponseText, APPROVED_MSG);
			strcpy(transMsg->FromAcNo, (char *)BitmapStructOb.field_102);
			strcpy(transMsg->ToAcNo, (char *)BitmapStructOb.field_103);
			LOG_PRINTF(("From Account = %s", transMsg->FromAcNo));
			LOG_PRINTF(("TO Account = %s", transMsg->ToAcNo));

			SaveTransDetails(transMsg);

			write_at(Dmsg[TRANSACTION_SUCCESSFULL].dispMsg, strlen(Dmsg[TRANSACTION_SUCCESSFULL].dispMsg), Dmsg[TRANSACTION_SUCCESSFULL].x_cor, Dmsg[TRANSACTION_SUCCESSFULL].y_cor);
			SVC_WAIT(2000);
			ClearKbdBuff();
			KBD_FLUSH();
			if (LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("Refund/Deposit successfull !!\n"));
			}
		}
		else if (!strcmp((char *)BitmapStructOb.field_39, "55"))
		{
			/*	window(1,1,30,20);
			clrscr();*/
			write_at(Dmsg[INVALID_PIN_INDX].dispMsg, strlen(Dmsg[INVALID_PIN_INDX].dispMsg), Dmsg[INVALID_PIN_INDX].x_cor, Dmsg[INVALID_PIN_INDX].y_cor);//invalid pin
			strcpy(transMsg->ResponseText, "INVALID PIN");
			SVC_WAIT(2000);
			ClearKbdBuff();
			KBD_FLUSH();
		}
		else if (!strcmp((char *)BitmapStructOb.field_39, "51"))
		{
			/*	window(1,1,30,20);
			clrscr();*/
			write_at(Dmsg[INVALID_PIN_INDX].dispMsg, strlen(Dmsg[INSUFFICIENT_FUND].dispMsg), Dmsg[INSUFFICIENT_FUND].x_cor, Dmsg[INSUFFICIENT_FUND].y_cor);//INSUFFICIENT_FUNDS
			strcpy(transMsg->ResponseText, "INSUFFICIENT_FUNDS");
			SVC_WAIT(2000);
			ClearKbdBuff();
			KBD_FLUSH();

		}
		else
		{
			/*window(1,1,30,20);
			clrscr();
			*/
			write_at(Dmsg[TRANSACTION_FAILED].dispMsg, strlen(Dmsg[TRANSACTION_FAILED].dispMsg), Dmsg[TRANSACTION_FAILED].x_cor, Dmsg[TRANSACTION_FAILED].y_cor);

			SVC_WAIT(1000);
			ClearKbdBuff();
			KBD_FLUSH();

			strcpy(transMsg->ResponseText, "DECLINE");
			if (LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("Refund Transaction Fail reason : %s \n", BitmapStructOb.field_39));
			}
		}
		/*if(transMsg->EMV_Flag == 1 )
		{
		display_TVR_TSI(transMsg);
		}*/
		return _SUCCESS;
	}
	/*else
	{
		if (retVal == RECV_FAILED)//if did not receive a valid response before the transaction time-out period expired
		{
			if (SaveReversalDetails(transMsg) == _SUCCESS)//saves reversal data
			{
				retVal = InitReversal();//go for reversal
				if (LOG_STATUS == LOG_ENABLE)
				{
					LOG_PRINTF(("reversal retVal =%d", retVal));
				}
			}
		}
		else
		{
			if (LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("Response buffer size is zero for Refund\n"));
			}
		}
	}*/
	return _FAIL;
}


/****************************************************************************************************************
*	Function Name : RefundTransactionProcessing																											                    *
*	Purpose		    : intialize field table ,assemble-disassemble data 	for refund transaction	 																              *
*	Input					:	Address of main transaction message structure and transaction methord type 						                                              *
*	Output		    : returns success or failure 								                                  *
*****************************************************************************************************************/
short RefundTransactionProcessing(TransactionMsgStruc *transMsg)
{
  short retVal = -1;  
	int i = 0;
  char Bitmap[BITMAP_ARRAY_SIZE+1] ={0}; //8
  char UnpackBiitmap[UNPACK_BITMAP_ARRAY_SIZE+1]={0};//16
  char bits[BITMAP_SIZE+1] ={0};//64
  char temp[BITMAP_ARRAY_SIZE+1] ={0};//9
  
#ifndef EMV_ENABLE
  unsigned char reqBuff[REFUND_REQUEST_SIZE]={0};//96
#else
  unsigned char reqBuff[REFUND_REQUEST_SIZE+ICC_RELATED_DATA_LEN+2]={0};//96+151+2
#endif 
  //unsigned char reqBuff[REFUND_REQUEST_SIZE]={0};//96
  unsigned char resBuff[REFUND_RESPONSE_SIZE]={0};//100 to 200 after e2e encryprion
  unsigned char testBuff[REFUND_UNAPACK_RES_SIZE]={0};//200 to 400 after e2e encryprion
  short ReqLen = 0; 
  short recvLength = 0;

  field_struct iso8583_field_table[] =
  {
	  /*Fld 8583 Convert Variable name and size no.	sz 	index *//*BCD_BCD,BCD_STR*/ //ASC_ASC->
	  { 0, TPDU_LEN,BCD_STR,(void *)(BitmapStructOb.tpdu), sizeof(BitmapStructOb.tpdu) },
	  { 0, MSG_ID_LEN, BCD_ASC, (void *)BitmapStructOb.message_id, sizeof(BitmapStructOb.message_id) },
	  { 2 + SKIP, 19, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 3, 6, BCD_STR, (void *)BitmapStructOb.field_03, sizeof(BitmapStructOb.field_03) },
	  { 4, 12, BCD_STR, (void *)BitmapStructOb.field_04, sizeof(BitmapStructOb.field_04) },
	  { 5 + SKIP, 12, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 6 + SKIP, 12, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 7 + SKIP, 10, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 8 + SKIP, 8, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 9 + SKIP, 8, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 10 + SKIP, 8, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 11, 6, BCD_STR, (void *)BitmapStructOb.field_11, sizeof(BitmapStructOb.field_11) },
	  { 12, 6, BCD_STR, (void *)BitmapStructOb.field_12, sizeof(BitmapStructOb.field_12) },
	  { 13, 4, BCD_STR, (void *)BitmapStructOb.field_13, sizeof(BitmapStructOb.field_13) },
	  { 14 + SKIP, 4, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 15 + SKIP, 4, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 16 + SKIP, 4, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 17 + SKIP, 4, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 18 + SKIP, 4, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 19 + SKIP, 3, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 20 + SKIP, 3, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 21 + SKIP, 3, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 22, 3, BCD_STR, (void *)BitmapStructOb.field_22, sizeof(BitmapStructOb.field_22) },
	  { 23 + SKIP, 3, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 24, 3, BCD_STR, (void *)BitmapStructOb.field_24, sizeof(BitmapStructOb.field_24) },
	  { 25, 2, BCD_STR, (void *)BitmapStructOb.field_25, sizeof(BitmapStructOb.field_25) },
	  { 26 + SKIP, 2, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 27 + SKIP, 1, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 28 + SKIP, 8, XBC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 29 + SKIP, 8, XBC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 30 + SKIP, 8, XBC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 31 + SKIP, 8, XBC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 32 + SKIP, 11, BV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 33 + SKIP, 11, BV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 34 + SKIP, 28, AV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 35, 37, BV2_STR, (void *)BitmapStructOb.field_35, sizeof(BitmapStructOb.field_35) },
	  { 36 + SKIP, 104, AV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 37 + SKIP, 104, AV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  //{ 37, 12, BCD_STR, (void *)BitmapStructOb.field_37, sizeof(BitmapStructOb.field_37) },
	  { 38, 6, BCD_STR, (void *)BitmapStructOb.field_38, sizeof(BitmapStructOb.field_38) },
	  { 39, 2, BCD_STR, (void *)BitmapStructOb.field_39, sizeof(BitmapStructOb.field_39) },
	  { 40 + SKIP, 3, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 41, 8, ASC_STR, (void *)BitmapStructOb.field_41, sizeof(BitmapStructOb.field_41) },
	  { 42, 15, ASC_STR, (void *)BitmapStructOb.field_42, sizeof(BitmapStructOb.field_42) },
	  { 43 + SKIP, 40, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 44 + SKIP, 25, AV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 45 + SKIP, 76, AV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 46 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 47 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  //{ 48, 9, AV3_STR, (void *) BitmapStructOb.field_48, sizeof( BitmapStructOb.field_48) }, 
	  { 48 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 49 + SKIP, 3, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 50 + SKIP, 3, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 51 + SKIP, 3, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 52, 64, BIT_BIT, (void *)BitmapStructOb.field_52, sizeof(BitmapStructOb.field_52) },
	  { 53 + SKIP, 16, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 54 + SKIP, 120, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 54, 999, AV3_STR, (void *)BitmapStructOb.field_54, sizeof(BitmapStructOb.field_54) },
	  { 55 + SKIP,  0, BIT_BIT, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 56 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 57 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 58 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 59 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 60 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 61 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 62 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 63 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 64 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 65 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 66 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 67 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 68 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 69 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 70 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 71 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 72 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 73 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 74 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 75 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 76 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 77 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 78 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 79 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 80 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 81 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 82 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 83 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 84 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 85 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 86 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 87 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 88 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 89 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 90 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 91 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 92 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 93 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 94 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 95 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 96 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 97 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 98 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 99 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 100 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  { 101 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  //{ 102+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) },
	  { 102, 12, AV2_STR, (void *)BitmapStructOb.field_102, sizeof(BitmapStructOb.field_102) },
	  { 103, 12, AV2_STR, (void *)BitmapStructOb.field_103, sizeof(BitmapStructOb.field_103) },
	  { 104 + SKIP + STOP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
  };
  strcpy(msg_id,MSGTYPE);
  strcpy(Proc_code,transMsg->ProcessingCode);

  strcpy(transMsg->MessageTypeInd,MSGTYPE);
  BITMAP_LEN=8;
  //memset(&BitmapStructOb, 0, sizeof(BmapStruct));
  resetBitmapStructure(&BitmapStructOb);//memsetting all the elements of structure

  ProcessingISOBitmapEngine(iso8583_field_table, transMsg);
  ReqLen = retVal = assemble_packet(iso8583_field_table,reqBuff);
 
  if(retVal <= _SUCCESS)
  {    
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("Error Occurred in assemble_packet retVal = %d \n",retVal));
    }
    return retVal;
  }

  recvLength = DEFAULT_BUFLEN ;
  retVal = CommWithServer((char*)reqBuff,ReqLen,(char*)resBuff,&recvLength);
  //retVal = _FAIL ;
  if( retVal == _SUCCESS )//checking for response buffer size 
  {
    bcd2a((char *)testBuff,(char *)resBuff,recvLength);
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("Testbuffin disassemblePacket retVal = %s \n",testBuff));
	  LOG_PRINTF(("Testbuffin Bitmap 1 = %c \n",testBuff[18]));
    }
	if(testBuff[18]>='8')
		BITMAP_LEN=16;
	//strncpy(UnpackBiitmap,(char*)testBuff+UNPACK_SIZE_MSGLEN_OF_REQ_RES+TPDU_LEN+MSG_ID_LEN,UNPACK_BITMAP_ARRAY_SIZE);
	strncpy(UnpackBiitmap,(char*)testBuff+UNPACK_SIZE_MSGLEN_OF_REQ_RES+TPDU_LEN+MSG_ID_LEN,BITMAP_LEN*2);
	
	LOG_PRINTF(("UnpackBiitmap = %s",UnpackBiitmap));
    
	packData(UnpackBiitmap,Bitmap);
    //for(i=0;i<BITMAP_ARRAY_SIZE;i++)
	for(i=0;i<BITMAP_LEN;i++)
    {
        CopyAllBits(Bitmap+i,temp);
        strcpy(bits+(i*8),temp);
    }
		//memset(&BitmapStructOb,0,sizeof(BitmapStructOb));
    resetBitmapStructure(&BitmapStructOb);//memsetting all the elements of structure
    ParseAndFillBitFields((char*)testBuff,bits);

		//------------------------------------------------------------
    strcpy((char *)transMsg->TransLocalTime,(char*)BitmapStructOb.field_12);           
	strcpy((char *)transMsg->TransLocalDate,(char*)BitmapStructOb.field_13);     
	strcpy((char *)transMsg->ResponseCode,(char*)BitmapStructOb.field_39);             //copy the response code
    strcpy((char *)transMsg->AuthIdResponse,(char*)BitmapStructOb.field_38);           //copy AuthIdResponse
    strcpy((char *)transMsg->RetrievalReferenceNo,(char*)BitmapStructOb.field_37);     //copy RetrievalReferenceNo
	  
    window(1,1,30,20);
		clrscr();
    if(transMsg->EMV_Flag == 1 )
    {
      display_TVR_TSI(transMsg);
    }
    if(!strcmp((char *)BitmapStructOb.field_39,"00"))
    {
      /*  window(1,1,30,20);
				clrscr();*/
				strcpy(transMsg->ResponseText,APPROVED_MSG);
				strcpy(transMsg->FromAcNo,(char *)BitmapStructOb.field_102);
				strcpy(transMsg->ToAcNo,(char *)BitmapStructOb.field_103);
				LOG_PRINTF(("From Account = %s",transMsg->FromAcNo));
				LOG_PRINTF(("TO Account = %s",transMsg->ToAcNo));

				SaveTransDetails(transMsg);
        
				write_at(Dmsg[TRANSACTION_SUCCESSFULL].dispMsg, strlen(Dmsg[TRANSACTION_SUCCESSFULL].dispMsg),Dmsg[TRANSACTION_SUCCESSFULL].x_cor, Dmsg[TRANSACTION_SUCCESSFULL].y_cor);
        SVC_WAIT(2000);
        ClearKbdBuff();
			  KBD_FLUSH();
        if(LOG_STATUS == LOG_ENABLE)
        {
          LOG_PRINTF(("Refund/Deposit successfull !!\n"));     
        }
    }
		else if(!strcmp((char *)BitmapStructOb.field_39,"55"))
		{
			/*	window(1,1,30,20);
				clrscr();*/
				write_at(Dmsg[INVALID_PIN_INDX].dispMsg, strlen(Dmsg[INVALID_PIN_INDX].dispMsg),Dmsg[INVALID_PIN_INDX].x_cor, Dmsg[INVALID_PIN_INDX].y_cor);//invalid pin
				strcpy(transMsg->ResponseText,"INVALID PIN");
				SVC_WAIT(2000);
				ClearKbdBuff();
				KBD_FLUSH();
		}
		else if(!strcmp((char *)BitmapStructOb.field_39,"51"))
		{
			/*	window(1,1,30,20);
				clrscr();*/
				write_at(Dmsg[INVALID_PIN_INDX].dispMsg, strlen(Dmsg[INSUFFICIENT_FUND].dispMsg),Dmsg[INSUFFICIENT_FUND].x_cor, Dmsg[INSUFFICIENT_FUND].y_cor);//INSUFFICIENT_FUNDS
				strcpy(transMsg->ResponseText,"INSUFFICIENT_FUNDS");
				SVC_WAIT(2000);
				ClearKbdBuff();
				KBD_FLUSH();
       
		}
    else
    {
        /*window(1,1,30,20);
				clrscr();
        */
				write_at(Dmsg[TRANSACTION_FAILED].dispMsg, strlen(Dmsg[TRANSACTION_FAILED].dispMsg),Dmsg[TRANSACTION_FAILED].x_cor, Dmsg[TRANSACTION_FAILED].y_cor);
				
        SVC_WAIT(1000);
        ClearKbdBuff();
			  KBD_FLUSH();
              
        strcpy(transMsg->ResponseText,"DECLINE");
        if(LOG_STATUS == LOG_ENABLE)
        {
          LOG_PRINTF(("Refund Transaction Fail reason : %s \n",BitmapStructOb.field_39));    
        }
    }
    /*if(transMsg->EMV_Flag == 1 )
    {
      display_TVR_TSI(transMsg);
    }*/
    return _SUCCESS;
  }
  else
  {
    if(retVal ==RECV_FAILED)//if did not receive a valid response before the transaction time-out period expired
    {
      if(SaveReversalDetails(transMsg) == _SUCCESS)//saves reversal data
      {
        retVal  = InitReversal();//go for reversal
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
        LOG_PRINTF(("Response buffer size is zero for Refund\n"));  
      }
    }
  }
  return _FAIL; 
}

/****************************************************************************************************************
*	Function Name : TransferTransactionProcessing																											                    *
*	Purpose		    : intialize field table ,assemble-disassemble data 	for refund transaction	 																              *
*	Input					:	Address of main transaction message structure and transaction methord type 						                                              *
*	Output		    : returns success or failure 								                                  *
*****************************************************************************************************************/
short TransferTransactionProcessing(TransactionMsgStruc *transMsg)
{
  short retVal = -1;  
	int i = 0;
  char Bitmap[BITMAP_ARRAY_SIZE+1] ={0}; //8
  char UnpackBiitmap[UNPACK_BITMAP_ARRAY_SIZE+1]={0};//16
  char bits[BITMAP_SIZE+1] ={0};//64
  char temp[BITMAP_ARRAY_SIZE+1] ={0};//9
  
#ifndef EMV_ENABLE
  unsigned char reqBuff[REFUND_REQUEST_SIZE]={0};//96
#else
  unsigned char reqBuff[REFUND_REQUEST_SIZE+ICC_RELATED_DATA_LEN+2]={0};//96+151+2
#endif 
  //unsigned char reqBuff[REFUND_REQUEST_SIZE]={0};//96
  unsigned char resBuff[REFUND_RESPONSE_SIZE]={0};//100 to 200 after e2e encryprion
  unsigned char testBuff[REFUND_UNAPACK_RES_SIZE]={0};//200 to 400 after e2e encryprion
  short ReqLen = 0; 
  short recvLength = 0;
  
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
    { 45+SKIP, 76, AV2_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 46+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 47+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    //{ 48, 9, AV3_STR, (void *) BitmapStructOb.field_48, sizeof( BitmapStructOb.field_48) }, 
	{ 48+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 49+ SKIP, 3, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 50+ SKIP, 3, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 51+ SKIP, 3, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 52, 64, BIT_BIT, (void *) BitmapStructOb.field_52, sizeof( BitmapStructOb.field_52) }, 
    { 53+ SKIP, 16, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 54+ SKIP, 120, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 55+ SKIP,  0, BIT_BIT, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
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
	{ 102, 12, AV2_STR, (void *) BitmapStructOb.field_102, sizeof( BitmapStructOb.field_102) },   
	{ 103, 12, AV2_STR, (void *) BitmapStructOb.field_103, sizeof( BitmapStructOb.field_103) },  
	{ 104+ SKIP +STOP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
  }; 
  strcpy(msg_id,MSGTYPE);
  strcpy(Proc_code,transMsg->ProcessingCode);

  strcpy(transMsg->MessageTypeInd,MSGTYPE);
 BITMAP_LEN=8;
  //memset(&BitmapStructOb, 0, sizeof(BmapStruct));
  resetBitmapStructure(&BitmapStructOb);//memsetting all the elements of structure

  ProcessingISOBitmapEngine(iso8583_field_table, transMsg);
  ReqLen = retVal = assemble_packet(iso8583_field_table,reqBuff);
 
  if(retVal <= _SUCCESS)
  {    
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("Error Occurred in assemble_packet retVal = %d \n",retVal));
    }
    return retVal;
  }

  recvLength = DEFAULT_BUFLEN ;
  retVal = CommWithServer((char*)reqBuff,ReqLen,(char*)resBuff,&recvLength);
  //retVal = _FAIL ;
  if( retVal == _SUCCESS )//checking for response buffer size 
  {
    bcd2a((char *)testBuff,(char *)resBuff,recvLength);
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("Testbuffin disassemblePacket retVal = %s \n",testBuff));
	  LOG_PRINTF(("Testbuffin Bitmap 1 = %c \n",testBuff[18]));
    }
	if(testBuff[18]>='8')
		BITMAP_LEN=16;
	//strncpy(UnpackBiitmap,(char*)testBuff+UNPACK_SIZE_MSGLEN_OF_REQ_RES+TPDU_LEN+MSG_ID_LEN,UNPACK_BITMAP_ARRAY_SIZE);
	strncpy(UnpackBiitmap,(char*)testBuff+UNPACK_SIZE_MSGLEN_OF_REQ_RES+TPDU_LEN+MSG_ID_LEN,BITMAP_LEN*2);
	
	LOG_PRINTF(("UnpackBiitmap = %s",UnpackBiitmap));
    
	packData(UnpackBiitmap,Bitmap);
    //for(i=0;i<BITMAP_ARRAY_SIZE;i++)
	for(i=0;i<BITMAP_LEN;i++)
    {
        CopyAllBits(Bitmap+i,temp);
        strcpy(bits+(i*8),temp);
    }
		//memset(&BitmapStructOb,0,sizeof(BitmapStructOb));
    resetBitmapStructure(&BitmapStructOb);//memsetting all the elements of structure
    ParseAndFillBitFields((char*)testBuff,bits);

		//------------------------------------------------------------
	strcpy((char *)transMsg->TransLocalTime,(char*)BitmapStructOb.field_12);           
	strcpy((char *)transMsg->TransLocalDate,(char*)BitmapStructOb.field_13);
    strcpy((char *)transMsg->ResponseCode,(char*)BitmapStructOb.field_39);             //copy the response code
    strcpy((char *)transMsg->AuthIdResponse,(char*)BitmapStructOb.field_38);           //copy AuthIdResponse
    strcpy((char *)transMsg->RetrievalReferenceNo,(char*)BitmapStructOb.field_37);     //copy RetrievalReferenceNo
	  
    window(1,1,30,20);
		clrscr();
    if(transMsg->EMV_Flag == 1 )
    {
      display_TVR_TSI(transMsg);
    }
    if(!strcmp((char *)BitmapStructOb.field_39,"00"))
    {
      /*  window(1,1,30,20);
				clrscr();*/
				strcpy(transMsg->ResponseText,APPROVED_MSG);
				strcpy(transMsg->FromAcNo,(char *)BitmapStructOb.field_102);
				strcpy(transMsg->ToAcNo,(char *)BitmapStructOb.field_103);
				SaveTransDetails(transMsg);
        
				write_at(Dmsg[TRANSACTION_SUCCESSFULL].dispMsg, strlen(Dmsg[TRANSACTION_SUCCESSFULL].dispMsg),Dmsg[TRANSACTION_SUCCESSFULL].x_cor, Dmsg[TRANSACTION_SUCCESSFULL].y_cor);
        SVC_WAIT(2000);
        ClearKbdBuff();
			  KBD_FLUSH();
        if(LOG_STATUS == LOG_ENABLE)
        {
          LOG_PRINTF(("TRANSFER successfull !!\n"));     
        }
    }
		else if(!strcmp((char *)BitmapStructOb.field_39,"55"))
		{
			/*	window(1,1,30,20);
				clrscr();*/
				write_at(Dmsg[INVALID_PIN_INDX].dispMsg, strlen(Dmsg[INVALID_PIN_INDX].dispMsg),Dmsg[INVALID_PIN_INDX].x_cor, Dmsg[INVALID_PIN_INDX].y_cor);//invalid pin
				strcpy(transMsg->ResponseText,"INVALID PIN");
				SVC_WAIT(2000);
				ClearKbdBuff();
				KBD_FLUSH();
		}
		else if(!strcmp((char *)BitmapStructOb.field_39,"51"))
		{
			/*	window(1,1,30,20);
				clrscr();*/
				write_at(Dmsg[INVALID_PIN_INDX].dispMsg, strlen(Dmsg[INSUFFICIENT_FUND].dispMsg),Dmsg[INSUFFICIENT_FUND].x_cor, Dmsg[INSUFFICIENT_FUND].y_cor);//INSUFFICIENT_FUNDS
				strcpy(transMsg->ResponseText,"INSUFFICIENT_FUNDS");
				SVC_WAIT(2000);
				ClearKbdBuff();
				KBD_FLUSH();
       
		}
    else
    {
        /*window(1,1,30,20);
				clrscr();
        */
				write_at(Dmsg[TRANSACTION_FAILED].dispMsg, strlen(Dmsg[TRANSACTION_FAILED].dispMsg),Dmsg[TRANSACTION_FAILED].x_cor, Dmsg[TRANSACTION_FAILED].y_cor);
				
        SVC_WAIT(1000);
        ClearKbdBuff();
			  KBD_FLUSH();
              
        strcpy(transMsg->ResponseText,"DECLINE");
        if(LOG_STATUS == LOG_ENABLE)
        {
          LOG_PRINTF(("Refund Transaction Fail reason : %s \n",BitmapStructOb.field_39));    
        }
    }
    /*if(transMsg->EMV_Flag == 1 )
    {
      display_TVR_TSI(transMsg);
    }*/
    return _SUCCESS;
  }
  else
  {
    if(retVal ==RECV_FAILED)//if did not receive a valid response before the transaction time-out period expired
    {
      if(SaveReversalDetails(transMsg) == _SUCCESS)//saves reversal data
      {
        retVal  = InitReversal();//go for reversal
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
        LOG_PRINTF(("Response buffer size is zero for Refund\n"));  
      }
    }
  }
  return _FAIL; 
}




/****************************************************************************************************************
*	Function Name : InitBalanceInquiry																											                      *
*	Purpose		    : intialize balance inquiry request for a transaction 		 			 																*
*	Input					:	Address of main transaction message structure and transaction methord type 						        *
*	Output		    : returns success or failure 								                                                    *
*****************************************************************************************************************/


short InitBalanceInquiry(TransactionMsgStruc *transMsg)
{
	short iRetVal = -1;
	short Selectionstatus = -1;
    char terminalId[TERMINAL_ID_LEN+1]={0};//9 =8 + 1
	char cardAccepterId[CARD_ACCEPTOR_ID_LEN+1]={0};//16 = 15+1

	getTraceAuditNumber(transMsg);  //11 bit 
	get_env("#TERMINAL_ID",(char*)terminalId,sizeof(terminalId)); //41 bit 
	sprintf(transMsg->TerminalID,"%s",terminalId); 
	get_env("#CARDACCEPTERID",(char*)cardAccepterId,sizeof(cardAccepterId));
	sprintf(transMsg->CardAcceptorID,"%s",cardAccepterId);  //42 bit
    sprintf(transMsg->_transType,"%s","BALANCE INQUIRY");

	if(transMsg->TrMethordFlag == edebit )//for debit refund
	{
		SetImage(ACC_TYPE_BMP);//setting image for Account type 
#ifdef AGENCY_BANK
		EnblTouchScreen(Agency_AccountType);		
#else
			EnblTouchScreen(AccountType);
#endif				
		//Selectionstatus = SelectEvent;
		Selectionstatus = get_char();
		LOG_PRINTF(("Selectionstatus : %d \n",Selectionstatus));    	
		if((Selectionstatus == eChequeAccount) || (Selectionstatus == eSavingAccount) || (Selectionstatus == eOtherAccount))
				iRetVal =debitAccountType(transMsg,Selectionstatus);//call function to perform according to account type 
		else if((Selectionstatus == KEY_CANCEL ) || (Selectionstatus == KEY_STR) || Selectionstatus == eAccTypeBack)
					return GoToMainScreen;
		
		
		//like cheque,saving or credit
        EnblTouchScreen(DisableTouch);
	}
    else
    {
		EnblTouchScreen(DisableTouch);
    }
    if((iRetVal == _FAIL ) || (iRetVal == KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL))
	{
		return iRetVal;
	}
	//Balance Enq- get account number
	if(Selectionstatus== eOtherAccount)
	{
		OTHER_AC=1;
		iRetVal = getAccountNo(transMsg);//getting account number
		if((iRetVal == (KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL) || (iRetVal == _FAIL)))
		{
			return iRetVal;
		}
		Selectionstatus = eCashBackNo;
	}
		iRetVal = swipeOrEMVCardEntry(transMsg);//Read card data 
		if((iRetVal == _FAIL ) || (iRetVal == KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL))
		{
				return iRetVal;
		}
    else if(iRetVal == FORCE_FOR_EMV)
    {
      LOG_PRINTF(("InitBalanceInquiryTransaction:FORCE_FOR_EMV"));
      iRetVal = EMVCardEntry(transMsg);//Read card data 
			if((iRetVal == (KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL) || (iRetVal == _FAIL)))
			{
					return GoToMainScreen;
			}        //22 POS Entry Mode
    }
    if(iRetVal == FALLBACK)
    {
      LOG_PRINTF(("InitBalanceInquiry:FALLBACK"));
      iRetVal = swipeCardEntry(transMsg);//Read card data 
			if((iRetVal == _FAIL ) || (iRetVal == KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL))
			{
					return iRetVal;
			}
      strcpy((char *)transMsg->POSEntryMode,MSReICC_PIN_CAPABLE);             //22 POS Entry Mode
    }
		if(transMsg->TrMethordFlag == edebit )//for debit refund
    {
      if((!strcmp((char *)transMsg->POSEntryMode,MSR_PIN_CAPABLE))||(!strcmp((char *)transMsg->POSEntryMode,MSReICC_PIN_CAPABLE)))//FOR MAGNETIC SWIPE
      {
				iRetVal = PinPrompt(transMsg->PrimaryAccNum);//getting user pin 
      }
    }
    else if( transMsg->TrMethordFlag == ecredit)
		{	
				strcpy(transMsg->ProcessingCode,CREDIT_BALINQPROCODE);//-<<<<<<<<<<<----------   written for testing purpus 
				iRetVal=_SUCCESS;//no need to write for credit card by default it will be success due to swipe card.
		}
		if(iRetVal==_SUCCESS)
    {
		    iRetVal = BalanceEnquiryProcessing(transMsg);
        if(LOG_STATUS == LOG_ENABLE)
        {
			    LOG_PRINTF(("ret val = =  %d ",iRetVal));
        }
    }
    else
    {
        return GoToMainScreen;
    }
    if(iRetVal == _SUCCESS)
    {
        window(1,1,30,20);
				clrscr();
        printRecipt(transMsg);	
				
    }
		return _SUCCESS;
}


/****************************************************************************************************************
*	Function Name : BalanceEnquiryProcessing																											                 *
*	Purpose		    : intialize field table ,assemble-disassemble data 	for BalanceEnquiry transaction	 						 *
*	Input					:	Address of main transaction message structure and transaction methord type 						         *
*	Output		    : returns success or failure 								                                                     *
*****************************************************************************************************************/
short BalanceEnquiryProcessing(TransactionMsgStruc *transMsg)
{
  int i =0;
  char Bitmap[BITMAP_ARRAY_SIZE+1] ={0};//17
  char UnpackBiitmap[UNPACK_BITMAP_ARRAY_SIZE+1]={0};//33
  char bits[BITMAP_SIZE+1] ={0};//65
  char temp[BITMAP_ARRAY_SIZE+1] ={0};//17
	double tempAmount = 0;
  
  short retVal = -1;  
  field_struct iso8583_field_table;
	
  //unsigned char reqBuff[BALANCE_ENQ_REQUEST_SIZE]={0};//87
#ifndef EMV_ENABLE
  unsigned char reqBuff[BALANCE_ENQ_REQUEST_SIZE]={0};//87
#else
  unsigned char reqBuff[BALANCE_ENQ_REQUEST_SIZE+ICC_RELATED_DATA_LEN+2]={0};//87+151+2 //after EMV integration
#endif 
  //unsigned char reqBuff[BALANCE_ENQ_REQUEST_SIZE+ICC_RELATED_DATA_LEN+2]={0};//87+151+2 //after EMV integration
  unsigned char resBuff[BALANCE_ENQ_RESPONSE_SIZE]={0};//200
  unsigned char testBuff[BALANCE_ENQ_UNAPACK_RES_SIZE]={0};//400
  short ReqLen = 0; 
  short recvLength = 0;

  if (pin_enc)
  {
	  field_struct iso8583_field_table1[] =
	  {
		  /*Fld 8583 Convert Variable name and size no.	sz 	index *//*BCD_BCD,BCD_STR*/ //ASC_ASC->
		  { 0, TPDU_LEN,BCD_STR,(void *)(BitmapStructOb.tpdu), sizeof(BitmapStructOb.tpdu) },
		  { 0, MSG_ID_LEN, BCD_ASC, (void *)BitmapStructOb.message_id, sizeof(BitmapStructOb.message_id) },
		  { 2 + SKIP, 19, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 3, 6, BCD_STR, (void *)BitmapStructOb.field_03, sizeof(BitmapStructOb.field_03) },
		  { 4, 12, BCD_STR, (void *)BitmapStructOb.field_04, sizeof(BitmapStructOb.field_04) },
		  { 5 + SKIP, 12, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 6 + SKIP, 12, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 7 + SKIP, 10, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 8 + SKIP, 8, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 9 + SKIP, 8, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 10 + SKIP, 8, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 11, 6, BCD_STR, (void *)BitmapStructOb.field_11, sizeof(BitmapStructOb.field_11) },
		  { 12, 6, BCD_STR, (void *)BitmapStructOb.field_12, sizeof(BitmapStructOb.field_12) },
		  { 13, 4, BCD_STR, (void *)BitmapStructOb.field_13, sizeof(BitmapStructOb.field_13) },
		  { 14 + SKIP, 4, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 15 + SKIP, 4, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 16 + SKIP, 4, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 17 + SKIP, 4, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 18 + SKIP, 4, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 19 + SKIP, 3, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 20 + SKIP, 3, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 21 + SKIP, 3, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 22, 3, BCD_STR, (void *)BitmapStructOb.field_22, sizeof(BitmapStructOb.field_22) },
		  { 23 + SKIP, 3, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 24, 3, BCD_STR, (void *)BitmapStructOb.field_24, sizeof(BitmapStructOb.field_24) },
		  { 25, 2, BCD_STR, (void *)BitmapStructOb.field_25, sizeof(BitmapStructOb.field_25) },
		  { 26 + SKIP, 2, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 27 + SKIP, 1, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 28 + SKIP, 8, XBC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 29 + SKIP, 8, XBC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 30 + SKIP, 8, XBC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 31 + SKIP, 8, XBC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 32 + SKIP, 11, BV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 33 + SKIP, 11, BV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 34 + SKIP, 28, AV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 35, 37, BV2_STR, (void *)BitmapStructOb.field_35, sizeof(BitmapStructOb.field_35) },
		  { 36 + SKIP, 104, AV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 37, 12, BCD_STR, (void *)BitmapStructOb.field_37, sizeof(BitmapStructOb.field_37) },
		  { 38, 6, BCD_STR, (void *)BitmapStructOb.field_38, sizeof(BitmapStructOb.field_38) },
		  { 39, 2, BCD_STR, (void *)BitmapStructOb.field_39, sizeof(BitmapStructOb.field_39) },
		  { 40 + SKIP, 3, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 41, 8, ASC_STR, (void *)BitmapStructOb.field_41, sizeof(BitmapStructOb.field_41) },
		  { 42, 15, ASC_STR, (void *)BitmapStructOb.field_42, sizeof(BitmapStructOb.field_42) },
		  { 43 + SKIP, 40, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 44 + SKIP, 25, AV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 45 + SKIP, 76, AV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 46 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 47 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 48 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  //{ 48, 9, AV3_STR, (void *) BitmapStructOb.field_48, sizeof( BitmapStructOb.field_48) }, 
		  { 49 + SKIP, 3, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 50 + SKIP, 3, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 51 + SKIP, 3, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 52, 64, BIT_BIT, (void *)BitmapStructOb.field_52, sizeof(BitmapStructOb.field_52) },
		  { 53 + SKIP, 16, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 54, 999, AV3_STR, (void *)BitmapStructOb.field_54, sizeof(BitmapStructOb.field_54) },
		  { 55 + SKIP, 0, BIT_BIT, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 56 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 57 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 58 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 59 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 60 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 61 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 62 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 63 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) }, 
		  { 64 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 65 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 66 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 67 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 68 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 69 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 70 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 71 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 72 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 73 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 74 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 75 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 76 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 77 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 78 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 79 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 80 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 81 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 82 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 83 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 84 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 85 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 86 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 87 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 88 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 89 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 90 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 91 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 92 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 93 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 94 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 95 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 96 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 97 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 98 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 99 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 100 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 101 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 102, 12, AV2_STR, (void *)BitmapStructOb.field_102, sizeof(BitmapStructOb.field_102) }, //Agency Bank
		  { 103 + SKIP + STOP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  };
	  memcpy(&iso8583_field_table, &iso8583_field_table1, sizeof(iso8583_field_table1));
  }
  else
  {
	  field_struct iso8583_field_table1[] =
	  {
		  /*Fld 8583 Convert Variable name and size no.	sz 	index *//*BCD_BCD,BCD_STR*/ //ASC_ASC->
		  { 0, TPDU_LEN,BCD_STR,(void *)(BitmapStructOb.tpdu), sizeof(BitmapStructOb.tpdu) },
		  { 0, MSG_ID_LEN, BCD_ASC, (void *)BitmapStructOb.message_id, sizeof(BitmapStructOb.message_id) },
		  { 2 + SKIP, 19, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 3, 6, BCD_STR, (void *)BitmapStructOb.field_03, sizeof(BitmapStructOb.field_03) },
		  { 4, 12, BCD_STR, (void *)BitmapStructOb.field_04, sizeof(BitmapStructOb.field_04) },
		  { 5 + SKIP, 12, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 6 + SKIP, 12, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 7 + SKIP, 10, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 8 + SKIP, 8, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 9 + SKIP, 8, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 10 + SKIP, 8, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 11, 6, BCD_STR, (void *)BitmapStructOb.field_11, sizeof(BitmapStructOb.field_11) },
		  { 12, 6, BCD_STR, (void *)BitmapStructOb.field_12, sizeof(BitmapStructOb.field_12) },
		  { 13, 4, BCD_STR, (void *)BitmapStructOb.field_13, sizeof(BitmapStructOb.field_13) },
		  { 14 + SKIP, 4, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 15 + SKIP, 4, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 16 + SKIP, 4, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 17 + SKIP, 4, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 18 + SKIP, 4, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 19 + SKIP, 3, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 20 + SKIP, 3, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 21 + SKIP, 3, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 22, 3, BCD_STR, (void *)BitmapStructOb.field_22, sizeof(BitmapStructOb.field_22) },
		  { 23 + SKIP, 3, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 24, 3, BCD_STR, (void *)BitmapStructOb.field_24, sizeof(BitmapStructOb.field_24) },
		  { 25, 2, BCD_STR, (void *)BitmapStructOb.field_25, sizeof(BitmapStructOb.field_25) },
		  { 26 + SKIP, 2, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 27 + SKIP, 1, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 28 + SKIP, 8, XBC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 29 + SKIP, 8, XBC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 30 + SKIP, 8, XBC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 31 + SKIP, 8, XBC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 32 + SKIP, 11, BV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 33 + SKIP, 11, BV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 34 + SKIP, 28, AV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 35, 37, BV2_STR, (void *)BitmapStructOb.field_35, sizeof(BitmapStructOb.field_35) },
		  { 36 + SKIP, 104, AV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 37, 12, BCD_STR, (void *)BitmapStructOb.field_37, sizeof(BitmapStructOb.field_37) },
		  { 38, 6, BCD_STR, (void *)BitmapStructOb.field_38, sizeof(BitmapStructOb.field_38) },
		  { 39, 2, BCD_STR, (void *)BitmapStructOb.field_39, sizeof(BitmapStructOb.field_39) },
		  { 40 + SKIP, 3, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 41, 8, ASC_STR, (void *)BitmapStructOb.field_41, sizeof(BitmapStructOb.field_41) },
		  { 42, 15, ASC_STR, (void *)BitmapStructOb.field_42, sizeof(BitmapStructOb.field_42) },
		  { 43 + SKIP, 40, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 44 + SKIP, 25, AV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 45 + SKIP, 76, AV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 46 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 47 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 48 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  //{ 48, 9, AV3_STR, (void *) BitmapStructOb.field_48, sizeof( BitmapStructOb.field_48) }, 
		  { 49 + SKIP, 3, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 50 + SKIP, 3, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 51 + SKIP, 3, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 52, 64, BIT_BIT, (void *)BitmapStructOb.field_52, sizeof(BitmapStructOb.field_52) },
		  { 53 + SKIP, 16, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 54, 999, AV3_STR, (void *)BitmapStructOb.field_54, sizeof(BitmapStructOb.field_54) },
		  { 55 + SKIP, 0, BIT_BIT, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 56 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 57 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 58 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 59 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 60 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 61 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 62 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 63, KSN_PP_LENGTH, BCD_STR, (void *)BitmapStructOb.field_63, sizeof(BitmapStructOb.field_63) },
		  //{ 63+ SKIP, 999, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
		  { 64 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 65 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 66 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 67 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 68 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 69 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 70 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 71 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 72 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 73 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 74 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 75 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 76 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 77 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 78 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 79 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 80 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 81 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 82 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 83 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 84 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 85 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 86 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 87 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 88 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 89 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 90 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 91 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 92 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 93 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 94 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 95 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 96 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 97 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 98 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 99 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 100 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 101 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		  { 102, 12, AV2_STR, (void *)BitmapStructOb.field_102, sizeof(BitmapStructOb.field_102) }, //Agency Bank
		  { 103 + SKIP + STOP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	  };
	  memcpy(&iso8583_field_table, &iso8583_field_table1, sizeof(iso8583_field_table1));
  }
  
  strcpy(msg_id,BALINQMSGTYPE);
  strcpy(Proc_code,transMsg->ProcessingCode);
  strcpy(transMsg->MessageTypeInd,BALINQMSGTYPE);
  BITMAP_LEN=8;
  resetBitmapStructure(&BitmapStructOb);//memsetting all the elements of structure

  ProcessingISOBitmapEngine(&iso8583_field_table, transMsg);
  ReqLen = retVal = assemble_packet(&iso8583_field_table, reqBuff);
 
  if(retVal <= _SUCCESS)
  {   
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("Error Occurred in assemble_packet retVal = %d \n",retVal));
    }
    return retVal;
  }

  recvLength = DEFAULT_BUFLEN ;
  retVal = CommWithServer((char*)reqBuff,ReqLen,(char*)resBuff,&recvLength);
  //retVal = _FAIL ;
  
  //LOG_PRINTF(("Bal inq testing-- memcpy testBuff to resBuff"));
 //memcpy(resBuff,tempBuff,799);
  if(retVal == _SUCCESS )//checking for response buffer size 
  {
    bcd2a((char *)testBuff,(char *)resBuff,recvLength);
	//bcd2a((char *)testBuff,(char *)tempBuff,recvLength);
	//  memcpy(testBuff,temp1Buffer,sizeof(testBuff));
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("Testbuffin disassemblePacket retVal = %s \n",testBuff));
    }
      
	if(testBuff[18]>='8')
		BITMAP_LEN=16;
	strncpy(UnpackBiitmap,(char*)testBuff+UNPACK_SIZE_MSGLEN_OF_REQ_RES+TPDU_LEN+MSG_ID_LEN,BITMAP_LEN*2);
	//    strncpy(UnpackBiitmap,(char*)testBuff+UNPACK_SIZE_MSGLEN_OF_REQ_RES+TPDU_LEN+MSG_ID_LEN,UNPACK_BITMAP_ARRAY_SIZE);
    LOG_PRINTF(("==============UnpackBiitmap = %s \n",UnpackBiitmap));
		packData(UnpackBiitmap,Bitmap);
		LOG_PRINTF(("==============UnpackBiitmap = %s \n",Bitmap));

		for(i=0;i<BITMAP_LEN;i++)
    {
        CopyAllBits(Bitmap+i,temp);
        strcpy(bits+(i*8),temp);
    }
    resetBitmapStructure(&BitmapStructOb);//memsetting all the elements of structure
    ParseAndFillBitFields((char*)testBuff,bits);
    //parse
    strcpy((char *)transMsg->ResponseCode,(char*)BitmapStructOb.field_39);             //copy the response code
    strcpy((char *)transMsg->AuthIdResponse,(char*)BitmapStructOb.field_38);           //copy AuthIdResponse
    strcpy((char *)transMsg->RetrievalReferenceNo,(char*)BitmapStructOb.field_37);     //copy RetrievalReferenceNo
    //-----------------convert server amount to decimal string amount----------------------------
	
		tempAmount = atof((char*)BitmapStructOb.field_04);
		tempAmount = tempAmount/100;
		sprintf(transMsg->Amount,"%0.02f",tempAmount);
    if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF(("Balance = %s",transMsg->Amount));
    }
    window(1,1,30,20);
		clrscr();
    if(transMsg->EMV_Flag == 1 )
    {
      display_TVR_TSI(transMsg);
    }
		if(!strcmp((char *)BitmapStructOb.field_39,"00"))
    {
       
        strcpy(transMsg->ResponseText,APPROVED_MSG);
		strcpy((char *)transMsg->FromAcNo,(char *)BitmapStructOb.field_102);
				write_at(Dmsg[TRANSACTION_SUCCESSFULL].dispMsg, strlen(Dmsg[TRANSACTION_SUCCESSFULL].dispMsg),Dmsg[TRANSACTION_SUCCESSFULL].x_cor, Dmsg[TRANSACTION_SUCCESSFULL].y_cor);//TRANSACTION SUCCESSFULL
				
        SVC_WAIT(2000);
        ClearKbdBuff();
			  KBD_FLUSH();
        if(LOG_STATUS == LOG_ENABLE)
        {
          LOG_PRINTF(("Balance inquiry transaction successful\n"));     
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
    else
    {
        
				write_at(Dmsg[TRANSACTION_FAILED].dispMsg, strlen(Dmsg[TRANSACTION_FAILED].dispMsg),Dmsg[TRANSACTION_FAILED].x_cor, Dmsg[TRANSACTION_FAILED].y_cor);
        
        SVC_WAIT(2000);
        ClearKbdBuff();
			  KBD_FLUSH();
             
        strcpy(transMsg->ResponseText,"TRANSACTION DECLINED");
        if(LOG_STATUS == LOG_ENABLE)
        {
          LOG_PRINTF(("BALANCE INQUIRY Transaction Fail reason : %s \n",BitmapStructOb.field_39)); 
        }
    }
    
  }
  else
  {
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("Response buffer size is zero for balance inquiry\n"));  
    }
    return _FAIL; 
  }

	return _SUCCESS;
}



/****************************************************************************************************************
*	Function Name : InitPINValidate																											                      *
*	Purpose		    : intialize balance inquiry request for a transaction 		 			 																*
*	Input					:	Address of main transaction message structure and transaction methord type 						        *
*	Output		    : returns success or failure 								                                                    *
*****************************************************************************************************************/
short InitPINValidate(TransactionMsgStruc *transMsg)
{
	short iRetVal = -1;
	short Selectionstatus = -1;
    char terminalId[TERMINAL_ID_LEN+1]={0};//9 =8 + 1
	char cardAccepterId[CARD_ACCEPTOR_ID_LEN+1]={0};//16 = 15+1
	transMsg->TrTypeFlag=PIN_VALIDATE_MSG_TYPE_CASE;
	getTraceAuditNumber(transMsg);  //11 bit 
	get_env("#TERMINAL_ID",(char*)terminalId,sizeof(terminalId)); //41 bit 
	sprintf(transMsg->TerminalID,"%s",terminalId); 
	get_env("#CARDACCEPTERID",(char*)cardAccepterId,sizeof(cardAccepterId));
	sprintf(transMsg->CardAcceptorID,"%s",cardAccepterId);  //42 bit
    sprintf(transMsg->_transType,"%s","PIN VALIDATE");
	sprintf(transMsg->ProcessingCode,"%s",PIN_VALIDATE_PRCODE);
	LOG_PRINTF(("Selectionstatus : %d \n",Selectionstatus)); 
		
	iRetVal = swipeOrEMVCardEntry(transMsg);//Read card data 
	if((iRetVal == _FAIL ) || (iRetVal == KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL))
	{
		return iRetVal;
	}
    else if(iRetVal == FORCE_FOR_EMV)
    {
		LOG_PRINTF(("InitPINValidate_Transaction:FORCE_FOR_EMV"));
		iRetVal = EMVCardEntry(transMsg);//Read card data 
		if((iRetVal == (KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL) || (iRetVal == _FAIL)))
		{
			return GoToMainScreen;
		}        //22 POS Entry Mode
    }
    if(iRetVal == FALLBACK)
    {
		LOG_PRINTF(("PIN VALIDATE:FALLBACK"));
		iRetVal = swipeCardEntry(transMsg);//Read card data 
		if((iRetVal == _FAIL ) || (iRetVal == KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL))
		{
			return iRetVal;
		}
		strcpy((char *)transMsg->POSEntryMode,MSReICC_PIN_CAPABLE);             //22 POS Entry Mode
    }
	if((!strcmp((char *)transMsg->POSEntryMode,MSR_PIN_CAPABLE))||(!strcmp((char *)transMsg->POSEntryMode,MSReICC_PIN_CAPABLE)))//FOR MAGNETIC SWIPE
    {	
		iRetVal = KeyInjection(transMsg->PrimaryAccNum);//Key injection and getting user pin 
    }
    
	if(iRetVal==_SUCCESS)
    {
	    iRetVal = PINValidateProcessing(transMsg);
        if(LOG_STATUS == LOG_ENABLE)
        {
			    LOG_PRINTF(("ret val = =  %d ",iRetVal));
        }
    }
    else
    {
        return GoToMainScreen;
    }
    if(iRetVal == _SUCCESS)
    {
        window(1,1,30,20);
				clrscr();
        //printRecipt(transMsg);	
				
    }
	else
		return _FAIL;
	return _SUCCESS;
}


/****************************************************************************************************************
*	Function Name : PINValidateProcessing																											                 *
*	Purpose		    : intialize field table ,assemble-disassemble data 	for BalanceEnquiry transaction	 						 *
*	Input					:	Address of main transaction message structure and transaction methord type 						         *
*	Output		    : returns success or failure 								                                                     *
*****************************************************************************************************************/

short PINValidateProcessing(TransactionMsgStruc *transMsg)
{
	int i =0; 
//#define BITMAP_SIZE 128
//#define BITMAP_ARRAY_SIZE 16
  short BITMAP_LEN=8;
	char Bitmap[BITMAP_ARRAY_SIZE+1] ={0};//17 -Bitmap for Payment response  
	char UnpackBiitmap[UNPACK_BITMAP_ARRAY_SIZE+1]={0}; //33 - UnpackBiitmap response for payment
	char bits[BITMAP_SIZE+1] ={0}; // 65 
	char temp[BITMAP_ARRAY_SIZE+1] ={0}; //17
   
	short retVal = -1;  
	short ReqLen = 0; 
  //unsigned char reqBuff[PAYMENT_REQUEST_SIZE]={0};//96 -REQUEST BUFFER
	unsigned char reqBuff[PAYMENT_REQUEST_SIZE+ICC_RELATED_DATA_LEN+2]={0};//96 -REQUEST BUFFER
	unsigned char resBuff[PAYMENT_RESPONSE_SIZE]={0}; //100 RESPONSE BUFFER
	unsigned char testBuff[PAYMENT_UNAPACK_RES_SIZE]={0}; //200 PAYMENT UNPACK RESPONSE BUFFER
	short recvLength = 0;

	field_struct iso8583_field_table[] =
	{
		//Fld 8583 Convert Variable name and size no.	sz 	index BCD_BCD,BCD_STR //ASC_ASC->
		{ 0, TPDU_LEN,BCD_STR,(void *)(BitmapStructOb.tpdu), sizeof(BitmapStructOb.tpdu) },
		{ 0, MSG_ID_LEN, BCD_ASC, (void *)BitmapStructOb.message_id, sizeof(BitmapStructOb.message_id) },
		{ 2 + SKIP, 19, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 3, 6, BCD_STR, (void *)BitmapStructOb.field_03, sizeof(BitmapStructOb.field_03) },
		//{ 4, 12, BCD_STR, (void *) BitmapStructOb.field_04, sizeof( BitmapStructOb.field_04) }, 
		{ 4 + SKIP, 12, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 5 + SKIP, 12, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 6 + SKIP, 12, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 7 + SKIP, 10, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 8 + SKIP, 8, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 9 + SKIP, 8, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 10 + SKIP, 8, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 11, 6, BCD_STR, (void *)BitmapStructOb.field_11, sizeof(BitmapStructOb.field_11) },
		{ 12, 6, BCD_STR, (void *)BitmapStructOb.field_12, sizeof(BitmapStructOb.field_12) },
		{ 13, 4, BCD_STR, (void *)BitmapStructOb.field_13, sizeof(BitmapStructOb.field_13) },
		{ 14 + SKIP, 4, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 15 + SKIP, 4, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 16 + SKIP, 4, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 17 + SKIP, 4, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 18 + SKIP, 4, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 19 + SKIP, 3, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 20 + SKIP, 3, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 21 + SKIP, 3, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 22, 3, BCD_STR, (void *)BitmapStructOb.field_22, sizeof(BitmapStructOb.field_22) },
		{ 23 + SKIP, 3, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 24, 3, BCD_STR, (void *)BitmapStructOb.field_24, sizeof(BitmapStructOb.field_24) },
		{ 25, 2, BCD_STR, (void *)BitmapStructOb.field_25, sizeof(BitmapStructOb.field_25) },
		{ 26 + SKIP, 2, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 27 + SKIP, 1, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 28 + SKIP, 8, XBC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 29 + SKIP, 8, XBC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 30 + SKIP, 8, XBC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 31 + SKIP, 8, XBC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 32 + SKIP, 11, BV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 33 + SKIP, 11, BV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 34 + SKIP, 28, AV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 35, 37, BV2_STR, (void *)BitmapStructOb.field_35, sizeof(BitmapStructOb.field_35) },
		{ 36 + SKIP, 104, AV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 37, 12, BCD_STR, (void *)BitmapStructOb.field_37, sizeof(BitmapStructOb.field_37) },
		{ 38, 6, BCD_STR, (void *)BitmapStructOb.field_38, sizeof(BitmapStructOb.field_38) },
		{ 39, 2, BCD_STR, (void *)BitmapStructOb.field_39, sizeof(BitmapStructOb.field_39) },
		{ 40 + SKIP, 3, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 41, 8, ASC_STR, (void *)BitmapStructOb.field_41, sizeof(BitmapStructOb.field_41) },
		{ 42, 15, ASC_STR, (void *)BitmapStructOb.field_42, sizeof(BitmapStructOb.field_42) },
		{ 43 + SKIP, 40, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 44 + SKIP, 25, AV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 45 + SKIP, 76, AV2_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) }, //Skippping because using track 2 data(bit-35)
		{ 46 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 47 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 48 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		//{ 48, 9, AV3_STR, (void *) BitmapStructOb.field_48, sizeof( BitmapStructOb.field_48) }, 
		{ 49 + SKIP, 3, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 50 + SKIP, 3, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 51 + SKIP, 3, ASC_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 52, 64, BIT_BIT, (void *)BitmapStructOb.field_52, sizeof(BitmapStructOb.field_52) },
		{ 53 + SKIP, 16, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 54 + SKIP, 120, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 55 + SKIP, 0, BIT_BIT, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 56 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 57 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 58 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 59 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 60 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 61 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 62 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 63 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 64 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 65 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 66 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 67 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 68 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 69 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 70 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 71 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 72 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 73 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 74 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 75 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 76 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 77 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 78 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 79 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 80 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 81 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 82 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 83 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 84 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 85 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 86 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 87 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 88 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 89 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 90 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 91 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 92 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 93 + SKIP, 999, BCD_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 94 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 95 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 96 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 97 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 98 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 99 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 100 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 101 + SKIP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
		{ 102 + SKIP, 12, AV2_STR, (void *)BitmapStructOb.field_102, sizeof(BitmapStructOb.field_102) }, //Agency Bank
		{ 103 + SKIP + STOP, 999, AV3_STR, (void *)BitmapStructOb.discard, sizeof(BitmapStructOb.discard) },
	};

	LOG_PRINTF(("field 35:%s", BitmapStructOb.field_35));

	strcpy(msg_id,MSGTYPE);
	strcpy(Proc_code,transMsg->ProcessingCode);
	if(LOG_STATUS == LOG_ENABLE)
	{
		LOG_PRINTF (("->>>>>>>>>>processing code =%s",transMsg->ProcessingCode));
	}
	strcpy(transMsg->MessageTypeInd,MSGTYPE);
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
		write_at("SUPERVISOR",10,10,8);
		if(!strcmp((char *)BitmapStructOb.field_39,"00"))
		{
			//strcpy(transMsg->ResponseText,APPROVED_MSG);
			strcpy(transMsg->ResponseText,APPROVED_MSG);
		//	Sup_Login=1;
			
			write_at(Dmsg[TRANSACTION_SUCCESSFULL].dispMsg, strlen(Dmsg[TRANSACTION_SUCCESSFULL].dispMsg),Dmsg[TRANSACTION_SUCCESSFULL].x_cor, Dmsg[TRANSACTION_SUCCESSFULL].y_cor);
			SVC_WAIT(2000);
			ClearKbdBuff();
			KBD_FLUSH();
			
			if(LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("PIN Validation Transaction successful\n"));    
			}
			return _SUCCESS;
		}

		else if(!strcmp((char *)BitmapStructOb.field_39,"55"))
		{
			
			write_at(Dmsg[INVALID_PIN_INDX].dispMsg, strlen(Dmsg[INVALID_PIN_INDX].dispMsg),Dmsg[INVALID_PIN_INDX].x_cor, Dmsg[INVALID_PIN_INDX].y_cor);//invalid pin
			strcpy(transMsg->ResponseText,"INVALID PIN");
			SVC_WAIT(2000);
			ClearKbdBuff();
			KBD_FLUSH();
		}
		else
		{
			
			write_at(Dmsg[TRANSACTION_FAILED].dispMsg, strlen(Dmsg[TRANSACTION_FAILED].dispMsg),Dmsg[TRANSACTION_FAILED].x_cor, Dmsg[TRANSACTION_FAILED].y_cor);
			SVC_WAIT(2000);
			ClearKbdBuff();
			KBD_FLUSH();
			strcpy(transMsg->ResponseText,"DECLINE");
			if(LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("PIN Validation Transaction Fail reason : %s \n",BitmapStructOb.field_39));
			}
		}
	//	return _SUCCESS;
	}
	
	return _FAIL; 
}

