/***********************************************************************************************
  Filename		     : Settlement.c
  Project		       : Bevertec 
  Developer Neame  : Amar
  Module           : Settlement,batch upload and settlement trailer 
  Description	     : This is the implementation of settlement ,batch upload ,settlement trailer 
                     and saving the transactions details in file
 ***********************************************************************************************/

#include "..\Include\Settlement.h"
#include "..\Include\BevertecClient.h"
#include "..\Include\ReciptPrint.h"
#include "..\Include\ISo8583Map.h"
#include "..\Include\SaleTransaction.h"
#include <acldev.h>


extern char msg_id[MSG_ID_LEN+1];//5
extern char Proc_code[PROC_CODE_LEN+1];//7
extern BmapStruct BitmapStructOb;
extern char loggedinOper[9];


//short BITMAP_LEN=8;
/********************************************************************************************
*	Function Name : processingSettlement						   																		    *
*	Purpose		    : This function send the request to server for each and every transaction   *
*	Input					: Address of Structure which hold the data for settlement										*
*	Output		    : returns success or failure 																								*
**********************************************************************************************/

short processingSettlement(TransactionMsgStruc *transMsg)
{ 
	int i =0;
  char Bitmap[BITMAP_ARRAY_SIZE+1] ={0};//8
  char UnpackBiitmap[UNPACK_BITMAP_ARRAY_SIZE+1]={0};//16
  char bits[BITMAP_SIZE+1] ={0};//64
  char temp[BITMAP_ARRAY_SIZE+1] ={0};//9
  short BITMAP_LEN=8;
  short retVal = -1;  
	short ReqLen = 0; 
  unsigned char reqBuff[SETLLEMENT_REQUEST_SIZE]={0};//122
  unsigned char resBuff[SETLLEMENT_RESPONSE_SIZE]={0};//CHANGES TO 120 FROM 200 WHILE #DEFINE
  
  unsigned char testBuff[SETLLEMENT_UNAPACK_RES_SIZE]={0};//240
  short recvLength = 0;
 

  //////////////////////////////////////
  field_struct iso8583_field_table[] = 
  { 
    /*Fld 8583 Convert Variable name and size no.	sz 	index *//*BCD_BCD,BCD_STR*/ //ASC_ASC->
    { 0, TPDU_LEN,BCD_STR,(void *) (BitmapStructOb.tpdu), sizeof(BitmapStructOb.tpdu) }, 
    { 0, MSG_ID_LEN, BCD_ASC, (void *) BitmapStructOb.message_id, sizeof( BitmapStructOb.message_id) },
    { 2+ SKIP, 19, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 3, 6, BCD_STR, (void *) BitmapStructOb.field_03, sizeof( BitmapStructOb.field_03) }, 
    { 4+ SKIP, 12, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
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
    { 22+ SKIP, 3, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 23+ SKIP, 3, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 24, 3, BCD_STR, (void *) BitmapStructOb.field_24, sizeof( BitmapStructOb.field_24) }, 
    { 25+ SKIP, 2, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 26+ SKIP, 2, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 27+ SKIP, 1, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 28+ SKIP, 8, XBC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 29+ SKIP, 8, XBC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 30+ SKIP, 8, XBC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 31+ SKIP, 8, XBC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 32+ SKIP, 11, BV2_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 33+ SKIP, 11, BV2_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 34+ SKIP, 28, AV2_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 35+ SKIP, 37, BV2_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
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
    { 48, 9, AV3_STR, (void *) BitmapStructOb.field_48, sizeof( BitmapStructOb.field_48) }, 
    { 49+ SKIP, 3, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 50+ SKIP, 3, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 51+ SKIP, 3, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 52, 64, BIT_BIT, (void *) BitmapStructOb.field_52, sizeof( BitmapStructOb.field_52) }, 
    { 53+ SKIP, 16, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 54+ SKIP, 120, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 55+ SKIP, 999, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 56+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 57+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 58+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 59+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 60, 6, ASC_STR, (void *) BitmapStructOb.field_60, sizeof( BitmapStructOb.field_60) }, 
    { 61+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 62+ SKIP, 999, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 63, 60, ASC_STR, (void *) BitmapStructOb.field_63, sizeof( BitmapStructOb.field_63)  },  
    { 64+ SKIP +STOP, 64, BIT_BIT, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
  }; 
 //BITMAP_LEN=8;
  strcpy(msg_id,SETTLEMENT_MSG_TYPE);
  strcpy(Proc_code,SETTLEMENTPROCODE);
  LOG_PRINTF(("After Memery Free in the prntSumRprt2 _stack_max() =%d",_stack_max()));
  if(LOG_STATUS == LOG_ENABLE)
  {
	  LOG_PRINTF (("->>>>>>>>>>processing code =%s",transMsg->ProcessingCode));
  }
  strcpy(transMsg->MessageTypeInd,SETTLEMENT_MSG_TYPE);
  strcpy(transMsg->ProcessingCode,SETTLEMENTPROCODE);

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
  
  recvLength = sizeof(resBuff) ;
  retVal = CommWithServer((char*)reqBuff,ReqLen,(char*)resBuff,&recvLength);
  
  if(retVal ==_SUCCESS )//checking for response buffer size 
  {
    bcd2a((char *)testBuff,(char *)resBuff,recvLength);
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("Testbuffin disassemblePacket retVal = %s \n",testBuff));
    }
      
    strncpy(UnpackBiitmap,(char*)testBuff+UNPACK_SIZE_MSGLEN_OF_REQ_RES+TPDU_LEN+MSG_ID_LEN,BITMAP_LEN*2);
    packData(UnpackBiitmap,Bitmap);
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
   
    if(!strcmp((char *)BitmapStructOb.field_39,"00"))
    {
			  //---------------------------------------------------------------
        window(1,1,30,20);
				clrscr();
        strcpy(transMsg->ResponseText,APPROVED_MSG);
				
				write_at(Dmsg[SETTLEMENT_SUCCESSFULL].dispMsg, strlen(Dmsg[SETTLEMENT_SUCCESSFULL].dispMsg),Dmsg[SETTLEMENT_SUCCESSFULL].x_cor, Dmsg[SETTLEMENT_SUCCESSFULL].y_cor);
        SVC_WAIT(2000);
        ClearKbdBuff();
				KBD_FLUSH();
        if(LOG_STATUS == LOG_ENABLE)
        {
          LOG_PRINTF(("Settlement successfull\n"));     
        }
    }
    else
    {
      window(1,1,30,20);
			clrscr();
     	write_at(Dmsg[SETTLEMENT_DECLINE].dispMsg, strlen(Dmsg[SETTLEMENT_DECLINE].dispMsg),Dmsg[SETTLEMENT_DECLINE].x_cor, Dmsg[SETTLEMENT_DECLINE].y_cor);	
      SVC_WAIT(1000);
      ClearKbdBuff();
			KBD_FLUSH();
      if(LOG_STATUS == LOG_ENABLE)
      {
        LOG_PRINTF(("Settlement Transaction Fail reason : %s \n",BitmapStructOb.field_39));    
      }

      return (2);//for settlement fail
    }
  }
  else
  {
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("Response buffer size is zero FOR SETTLEMNET\n"));  
    }
    return _FAIL; 
  }

	return _SUCCESS;
}

/****************************************************************************************************************
*	Function Name : InitSettlement																											                          *
*	Purpose		    : This function increase Trace audit number by one and save it in file and also save data which 
									is required for settlement in Settlement file
*	Input					: void																																													*
*	Output		    : returns success or failure 																																		*
*****************************************************************************************************************/

short InitSettlement(TransactionMsgStruc *transMsg)
{
	char terminalId[TERMINAL_ID_LEN+1]={0};//9
	char cardAccepterId[CARD_ACCEPTOR_ID_LEN+1]={0};//16
    char ReConcillationReq[RECONCILLATION_REQ_LEN+1]={0};
    int RetVl=0;

    LOG_PRINTF(("After Memery Free in the prntSumRprt2 _stack_max() =%d",_stack_max()));
	getTraceAuditNumber(transMsg);
	get_env("#TERMINAL_ID",(char*)terminalId,sizeof(terminalId));
	sprintf(transMsg->TerminalID,"%s",terminalId);
    if(LOG_STATUS == LOG_ENABLE)
    {
		LOG_PRINTF(("transMsg->Term         inalID = %s",transMsg->TerminalID));
    }
    get_env("#CARDACCEPTERID",(char*)cardAccepterId,sizeof(cardAccepterId));
	sprintf(transMsg->CardAcceptorID,"%s",cardAccepterId);
    sprintf(transMsg->_transType,"%s","SETTLEMENT");
    if(LOG_STATUS == LOG_ENABLE)
    {
		LOG_PRINTF(("transMsg->CardAcceptorID = %s",transMsg->CardAcceptorID));
    }
    window(1,1,30,20);
	clrscr();
	write_at(Dmsg[PROCESSING].dispMsg, strlen(Dmsg[PROCESSING].dispMsg),Dmsg[PROCESSING].x_cor, Dmsg[PROCESSING].y_cor);
		
    RetVl = RetriveDataForSettlement(transMsg);
    if(RetVl ==_SUCCESS)
    {
		strncpy(ReConcillationReq,transMsg->rqstConsilation,RECONCILLATION_REQ_LEN);
		updateBatchNumber(transMsg);//getting Batch number
		RetVl =  TransactionDetailsReciept();
    }
    else if(RetVl == -2)//if file is empty
    {
		window(1,1,30,20);
		clrscr();
     	write_at(Dmsg[NO_RECORD_FOUND].dispMsg, strlen(Dmsg[NO_RECORD_FOUND].dispMsg),Dmsg[NO_RECORD_FOUND].x_cor, Dmsg[NO_RECORD_FOUND].y_cor);
		SVC_WAIT(2000);
		ClearKbdBuff();
		KBD_FLUSH();
		return GoToMainScreen;
    }
    if(RetVl ==_SUCCESS)
    {
		RetVl = processingSettlement(transMsg);
    }
    
    if(RetVl == 2)
    {
		window(1,1,30,20);
		clrscr();
		write_at(Dmsg[UPLOADING_BATCH].dispMsg, strlen(Dmsg[UPLOADING_BATCH].dispMsg),Dmsg[UPLOADING_BATCH].x_cor, Dmsg[UPLOADING_BATCH].y_cor);	
		SVC_WAIT(2000);
		ClearKbdBuff();
		KBD_FLUSH();
		RetVl = processingBatchUpload();
		if(RetVl == _SUCCESS)
		{
			RetVl = processingSettlementTrailer(ReConcillationReq);
		}
		else
		{
			if(LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("BatchUpload failed"));               
			}
		}
    }
    else if (RetVl ==_FAIL)
    {
		return GoToMainScreen;
    }
    TotalReportReciept("CLOSE BATCH");
		//-------------------------clear data from file --------------------------------------
    CleanFileData();
	return _SUCCESS;
}
/****************************************************************************************************************
*	Function Name : SaveTransDetails																											                        *
																																												                        *
*	Purpose		    : This function save all details related to transaction made by user
*	Input					: pointer to message structure																																	*
*	Output		    : returns success or failure 																																		*
*																																											                          *
*****************************************************************************************************************/

short SaveTransDetails(TransactionMsgStruc *transMsg)
{
	FILE *ifp=NULL;
    TrDetails transDetails={0};

	memset(&transDetails,0,sizeof(TrDetails));
    strcpy(transDetails.processingCode,transMsg->ProcessingCode); //BitField 3
	strcpy(transDetails.amount,transMsg->Amount);//BitField 4
	strcpy(transDetails.cardNumber,transMsg->PrimaryAccNum);////BitField 2
	strcpy(transDetails.expDate,transMsg->ExpiryDate);//BitField 14
	strcpy(transDetails.trans_Type,transMsg->_transType);
	strcpy(transDetails.trResponse,transMsg->ResponseText);
	strcpy(transDetails.Payment_Id,transMsg->PaymentId);
	strcpy(transDetails.cardHolderName,transMsg->CardHldrName);//card holder name
	strcpy(transDetails.traceAuditNo,transMsg->TraceAuditNo);//BitField 11
	strcpy(transDetails.Payment_Id,transMsg->PaymentId); 
	strcpy(transDetails.orgTransTime,transMsg->TransLocalTime); //BitField 12
	strcpy(transDetails.orgTransDate,transMsg->TransLocalDate); //BitField 13
	strcpy(transDetails.AuthIdResponse,transMsg->AuthIdResponse); //BitField 38
	strcpy(transDetails.RetrievalRefNo,transMsg->RetrievalReferenceNo); //BitField 37
    strcpy(transDetails.CardAcceptorId,transMsg->CardAcceptorID); //BitField 42
	strcpy(transDetails.NII,transMsg->NetworkInternationalId); //BitField 24
    strcpy(transDetails.TerminalId,transMsg->TerminalID); //BitField 41		
    strcpy(transDetails.PosCondCode,transMsg->POSConditionCode); //BitField 25
	strcpy(transDetails.PosEntryMode,transMsg->POSEntryMode); //BitField 22
	strcpy(transDetails.FromAc_No,transMsg->FromAcNo); //BitField 102
	strcpy(transDetails.ToAc_No,transMsg->ToAcNo); //BitField 103
	strcpy(transDetails.TVR,transMsg->TVR); //TVR
	strcpy(transDetails.TSI,transMsg->TSI); //TSI
	strcpy(transDetails.CARD_TYPE,transMsg->CARD_TYPE); //CARD_TYPE
	transDetails.EMV_Flag=transMsg->EMV_Flag; 
	transDetails.TransTypeFlg=transMsg->TrTypeFlag; 
	transDetails.TransMethord_Flag=transMsg->TrMethordFlag;
	strcpy(transDetails.operatorID, loggedinOper);

		
	if(LOG_STATUS == LOG_ENABLE)
    {
		LOG_PRINTF (("=========Writing to the file %s\n", TRANS_DETAILS_FILE));	
    }

	ifp = fopen(TRANS_DETAILS_FILE, "a");
	if (ifp == NULL)
	{
        if(LOG_STATUS == LOG_ENABLE)
        {
			LOG_PRINTF (("Failed to open the file %s\n", TRANS_DETAILS_FILE));	
        }
		return _FAIL;
	}
	else
	{
		if (fwrite(&transDetails, sizeof(transDetails), 1, ifp) != 1)
		{
		    if(LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF (( "Failed to write to %s\n", TRANS_DETAILS_FILE));	
			}
			return _FAIL;
		}
		fclose(ifp);
	}
	return _SUCCESS;
}


/****************************************************************************************************************
*	Function Name : ReadTransDetails																											                        *
*	Purpose		    : This function read  all details related to transaction made by user on behalf of payment id   *
* Input					: pointer to message structure																																	*
*	Output		    : returns success or failure 																																		*
*****************************************************************************************************************/

short ReadTransDetails(TransactionMsgStruc *transMsg)
{
	FILE *ifp=NULL;
    TrDetails transDetails={0};
	short count =0;
	short detailsFoundFlag = 0;
	memset(&transDetails,0,sizeof(TrDetails));
		
	ifp = fopen(TRANS_DETAILS_FILE, "r");
	if (ifp == NULL)
	{
        if(LOG_STATUS == LOG_ENABLE)
        {
			LOG_PRINTF (("Failed to open the file %s\n", TRANS_DETAILS_FILE));	
        }
		return _FAIL;
	}
	while(fread(&transDetails, sizeof(transDetails), 1, ifp) == 1)
	{
        count++;
		if(!strcmp(transDetails.RetrievalRefNo,transMsg->RetrievalReferenceNo))//comparision with reference no
		{
			if(LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("RetrievalRefNo found "));
				LOG_PRINTF((" TraceAuditNo  = %s",transDetails.traceAuditNo));
				//LOG_PRINTF(("Payment_Id = %s ",transDetails.Payment_Id));
				LOG_PRINTF(("Amount= %s",transDetails.amount));
				LOG_PRINTF(("NII = %s",transDetails.NII));
				//LOG_PRINTF(("TerminalIdentification  = %s",transDetails.TerminalIdentification));
				//LOG_PRINTF(("CardAcceptorId = %s ",transDetails.CardAcceptorId));
				LOG_PRINTF((" orgTransTime = %s",transDetails.orgTransTime));
				LOG_PRINTF(("orgTransDate  = %s",transDetails.orgTransDate));
				LOG_PRINTF(("AuthIdResponse = %s ",transDetails.AuthIdResponse));
				LOG_PRINTF(("RetrievalRefNo= %s",transDetails.RetrievalRefNo));
				LOG_PRINTF(("TVR= %s",transDetails.TVR));
				LOG_PRINTF(("TSI= %s",transDetails.TSI));
				LOG_PRINTF(("CARD_TYPE= %s",transDetails.CARD_TYPE));
                LOG_PRINTF(("EMV flag= %d",transDetails.EMV_Flag));
				LOG_PRINTF(("Processing Code= %d",transDetails.processingCode));
			}
			strcpy(transMsg->PrimaryAccNum,transDetails.cardNumber);
			if(!strcmp(transMsg->_transType,"VOID"))
			{
				strcpy(transMsg->ProcessingCode,transDetails.processingCode);
			if(transMsg->ProcessingCode[0]!='2')
				transMsg->ProcessingCode[0]='0';
			
			transMsg->ProcessingCode[1]='2';
			}
			strcpy(transMsg->ExpiryDate,transDetails.expDate);
			strcpy(transMsg->_transType,transDetails.trans_Type);
			strcpy(transMsg->ResponseText,transDetails.trResponse);
			strcpy(transMsg->PaymentId,transDetails.Payment_Id);
			strcpy(transMsg->Amount,transDetails.amount);
			strcpy(transMsg->TransLocalTime,transDetails.orgTransTime);
			strcpy(transMsg->TransLocalDate,transDetails.orgTransDate);
			strcpy(transMsg->AuthIdResponse,transDetails.AuthIdResponse);
			strcpy(transMsg->RetrievalReferenceNo,transDetails.RetrievalRefNo);
			strcpy(transMsg->TVR,transDetails.TVR);
			strcpy(transMsg->TSI,transDetails.TSI);
			strcpy(transMsg->CARD_TYPE,transDetails.CARD_TYPE);
            transMsg->EMV_Flag = transDetails.EMV_Flag ;
			fclose(ifp);
			detailsFoundFlag = 1;
			break;
		}
	}
    if(count == 0)//if no data in file
    {
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("There is no transaction in file for Trans details"));
		}
		fclose(ifp);
		return (-2);
    }
	if(detailsFoundFlag != 1)
	{
		window(1,1,30,20);
		clrscr();
		error_tone();
		SVC_WAIT(190);
		error_tone();
		//resetMsgDetails(transMsg);
		//write_at("INVALID TRANS REF NO",strlen("INVALID TRANS REF NO"),(30-strlen("INVALID TRANS REF NO"))/2,10);
		write_at(Dmsg[INVALID_TRANS_REF_NO].dispMsg, strlen(Dmsg[INVALID_TRANS_REF_NO].dispMsg),Dmsg[INVALID_TRANS_REF_NO].x_cor, Dmsg[INVALID_TRANS_REF_NO].y_cor);
		//write_at("retry ?Yes[Enter] or No[Cancle]",strlen("retry ?Yes[Enter] or No[Cancle]"),(30-strlen("retry ?Yes[Enter] or No[Cancle]"))/2,10);
		//SVC_WAIT(4000);
		return _FAIL;
	}
	return _SUCCESS;
}


/****************************************************************************************************************
*	Function Name : ReadTransDetails																											                        *
*	Purpose		    : This function read  all details related to transaction made by user on behalf of payment id   *
* Input					: pointer to message structure																																	*
*	Output		    : returns success or failure 																																		*
*****************************************************************************************************************/

short ReadLastTransDetails(TransactionMsgStruc *transMsg)
{
	FILE *ifp=NULL;
    TrDetails transDetails={0};
	short count =0;
//	int fr;
	short detailsFoundFlag = 0;
	memset(&transDetails,0,sizeof(TrDetails));
		
	ifp = fopen(TRANS_DETAILS_FILE, "r");
	if (ifp == NULL)
	{
        if(LOG_STATUS == LOG_ENABLE)
        {
			LOG_PRINTF (("Failed to open the file %s\n", TRANS_DETAILS_FILE));	
        }
		return _FAIL;
	}
	while(fread(&transDetails, sizeof(transDetails), 1, ifp) == 1)
	{
		count++;
	}
    //    fr=fread(&transDetails, sizeof(transDetails), 1, ifp);
		
//		if(!strcmp(transDetails.RetrievalRefNo,transMsg->RetrievalReferenceNo))//comparision with reference no
	//	{
	if (count>0)
	{
			if(LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("RetrievalRefNo found "));
				LOG_PRINTF((" PAN  = %s",transDetails.cardNumber));
				LOG_PRINTF((" TraceAuditNo  = %s",transDetails.traceAuditNo));
				//LOG_PRINTF(("Payment_Id = %s ",transDetails.Payment_Id));
				LOG_PRINTF(("Amount= %s",transDetails.amount));
				LOG_PRINTF(("NII = %s",transDetails.NII));
				//LOG_PRINTF(("TerminalIdentification  = %s",transDetails.TerminalIdentification));
				//LOG_PRINTF(("CardAcceptorId = %s ",transDetails.CardAcceptorId));
				LOG_PRINTF((" orgTransTime = %s",transDetails.orgTransTime));
				LOG_PRINTF(("orgTransDate  = %s",transDetails.orgTransDate));
				LOG_PRINTF(("AuthIdResponse = %s ",transDetails.AuthIdResponse));
				LOG_PRINTF(("RetrievalRefNo= %s",transDetails.RetrievalRefNo));
				LOG_PRINTF(("TVR= %s",transDetails.TVR));
				LOG_PRINTF(("TSI= %s",transDetails.TSI));
				LOG_PRINTF(("CARD_TYPE= %s",transDetails.CARD_TYPE));
                LOG_PRINTF(("EMV flag= %d",transDetails.EMV_Flag));
				LOG_PRINTF(("Processing Code= %d",transDetails.processingCode));
			}
			strcpy(transMsg->PrimaryAccNum,transDetails.cardNumber);
			if(!strcmp(transMsg->_transType,"VOID"))
			{
				strcpy(transMsg->ProcessingCode,transDetails.processingCode);
				if(transMsg->ProcessingCode[0]!='2')
					transMsg->ProcessingCode[0]='0';
				transMsg->ProcessingCode[1]='2';
			}
			strcpy(transMsg->ExpiryDate,transDetails.expDate);
			strcpy(transMsg->_transType,transDetails.trans_Type);
			strcpy(transMsg->ResponseText,transDetails.trResponse);
			strcpy(transMsg->PaymentId,transDetails.Payment_Id);
			strcpy(transMsg->Amount,transDetails.amount);
			strcpy(transMsg->TransLocalTime,transDetails.orgTransTime);
			strcpy(transMsg->TransLocalDate,transDetails.orgTransDate);
			strcpy(transMsg->AuthIdResponse,transDetails.AuthIdResponse);
			strcpy(transMsg->RetrievalReferenceNo,transDetails.RetrievalRefNo);
			//strcpy(transMsg->Track2Data,transDetails.tra
			strcpy(transMsg->TVR,transDetails.TVR);
			strcpy(transMsg->TSI,transDetails.TSI);
			strcpy(transMsg->CARD_TYPE,transDetails.CARD_TYPE);
            transMsg->EMV_Flag = transDetails.EMV_Flag ;
			fclose(ifp);
			detailsFoundFlag = 1;
			//break;
		}
	//}while(fr==1);
    
	else //if(count == 0)//if no data in file
    {
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("There is no transaction in file for Trans details"));
		}
		fclose(ifp);
		return (-2);
    }
	if(detailsFoundFlag != 1)
	{
		window(1,1,30,20);
		clrscr();
		error_tone();
		SVC_WAIT(190);
		error_tone();
		//resetMsgDetails(transMsg);
		//write_at("INVALID TRANS REF NO",strlen("INVALID TRANS REF NO"),(30-strlen("INVALID TRANS REF NO"))/2,10);
		write_at(Dmsg[INVALID_TRANS_REF_NO].dispMsg, strlen(Dmsg[INVALID_TRANS_REF_NO].dispMsg),Dmsg[INVALID_TRANS_REF_NO].x_cor, Dmsg[INVALID_TRANS_REF_NO].y_cor);
		//write_at("retry ?Yes[Enter] or No[Cancle]",strlen("retry ?Yes[Enter] or No[Cancle]"),(30-strlen("retry ?Yes[Enter] or No[Cancle]"))/2,10);
		//SVC_WAIT(4000);
		return _FAIL;
	}
	return _SUCCESS;
}

/****************************************************************************************************************
*	Function Name : paddCount																											                                *
*	Purpose		    : This function pad count which is used in print the total report and transaction details       *
* Input					: short count and the ooutput buffer																														*
*	Output		    : returns success or failure 																																		*
*****************************************************************************************************************/

short paddCount(short data,char *Res)
{
		char temp[4]={0};
		
		sprintf(temp,"%d",data);
		memset(Res,'0',3);
		sprintf(Res+3-strlen(temp),temp,strlen(temp));
    if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF(("count = %s",Res));
    }
		return 0;
}

/****************************************************************************************************************
*	Function Name : deleteRecordFromFile																											                    *
*	Purpose		    : This function delete the record of txn after doing the void of that transaction               *
* Input					: pointer to the TransactionMsgStruc    																												*
*	Output		    : returns success or failure 																																		*
*****************************************************************************************************************/
short deleteRecordFromFile(TransactionMsgStruc *transMsg)
{
	FILE *fp=NULL;
	FILE *fp_tmp=NULL;
  TrDetails transDetails={0};
	int found=0;
	
	fp=fopen(TRANS_DETAILS_FILE, "r");
	if (fp == NULL)
	{
    if(LOG_STATUS == LOG_ENABLE)
    {
		LOG_PRINTF (("Failed to open the file %s\n", TRANS_DETAILS_FILE));	
    }
		return _FAIL;
	}
	fp_tmp=fopen(TMP_TRANS_DETAILS_FILE, "w");
	if (!fp_tmp) 
	{
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Unable to open file temp file."));
		}
		return -1;
	}
	
	while (fread(&transDetails,sizeof(transDetails),1,fp) == 1) 
	{
		if (strcmp (transMsg->RetrievalReferenceNo, transDetails.RetrievalRefNo) == 0)
		{
      if(LOG_STATUS == LOG_ENABLE)
      {
				LOG_PRINTF(("A record with requested RetrievalRefNo found and deleted.\n"));
      }
			found=1;
		}
		else 
		{
			fwrite(&transDetails, sizeof(transDetails), 1, fp_tmp);
      if(LOG_STATUS == LOG_ENABLE)
      {
				LOG_PRINTF(("written in new file\n"));
      }
		}
	}
	if (! found) 
  {
    if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF(("No record(s) found with the requested RetrievalReferenceNo: %s\n\n", transMsg->RetrievalReferenceNo));
    }
	}

	fclose(fp);
	fclose(fp_tmp);
	//--------------------------------------
	fp=fopen(TRANS_DETAILS_FILE, "w");
	if (fp == NULL)
	{
    if(LOG_STATUS == LOG_ENABLE)
    {
			LOG_PRINTF (("Failed to open the file %s\n", TRANS_DETAILS_FILE));	
    }
		return _FAIL;
	}
	fp_tmp=fopen(TMP_TRANS_DETAILS_FILE, "r");
	if (!fp_tmp) 
  {
    if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF(("Unable to open file temp file."));
    }
	  return -1;
	}
	while (fread(&transDetails,sizeof(transDetails),1,fp_tmp) == 1) 
	{
		fwrite(&transDetails, sizeof(transDetails), 1, fp);
    if(LOG_STATUS == LOG_ENABLE)
    {
			LOG_PRINTF(("written in new file\n"));
    }
	}
	fclose(fp);
	fclose(fp_tmp);
	//--------------------------------------
	return 0;

}
/****************************************************************************************************************
*	Function Name : RetriveDataForSettlement																											                *
*	Purpose		    : This function retrives the data for the settlement like reconcillation request                *
* Input					: pointer to the TransactionMsgStruc    																												*
*	Output		    : returns success or failure 																																		*
*****************************************************************************************************************/
short RetriveDataForSettlement(TransactionMsgStruc *transMsg)
{
	FILE *ifp = NULL;
    TrDetails transDetails ={0};
    StlDetails settlDetails = {0};
	char temp[AMOUNT_LEN+1]={0};//13
	double fcreditSalAmount = 0.00;
	double fcreditRefAmount = 0.00;
	
	double fSalAmount = 0.00;
	double fRefAmount = 0.00;
	double fWithAmount = 0.00;
	double fDepAmount = 0.00;
	double fTraAmount = 0.00;
	
	char countTrans[4]={0};
	char ReqAmount[AMOUNT_LEN+1]={0};//13
	char rqstBuff[RECONCILLATION_REQ_LEN+1]={0};//61
    short counter = 0; //counter for transaction

    //char tempLenDisp[5]={0};//testing display the length of the terminal

	settlDetails.saleCount = 0;
	settlDetails.refundCount = 0;
    
	memset(&transDetails,0,sizeof(transDetails));
	//////////////////debit
    memset(settlDetails.saleAmount,'0',sizeof(settlDetails.saleAmount));
	memset(settlDetails.refundAmount,'0',sizeof(settlDetails.refundAmount));
	settlDetails.saleAmount[AMOUNT_LEN]=0;//putting null char
	settlDetails.refundAmount[AMOUNT_LEN]=0;//putting null char

    //////////////////credit
    memset(settlDetails.creditSaleAmount,'0',sizeof(settlDetails.creditSaleAmount));
	memset(settlDetails.creditRefundAmount,'0',sizeof(settlDetails.creditRefundAmount));
	settlDetails.creditSaleAmount[AMOUNT_LEN]=0;//putting null char
	settlDetails.creditRefundAmount[AMOUNT_LEN]=0;//putting null char

    //DebugggingInRel("fopen",strlen("fopen"));
   
	ifp = fopen(TRANS_DETAILS_FILE, "r");
	if (ifp == NULL)
	{
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF (("Failed to open the file %s\n", TRANS_DETAILS_FILE));	
		}
		return _FAIL;
	}
	while (fread(&transDetails, sizeof(transDetails), 1, ifp) == 1)
	{
        counter++;
		LOG_PRINTF (("=============transDetails.TransMethord_Flag %d", transDetails.TransMethord_Flag));

		//Debit
		if((transDetails.TransTypeFlg== SALEMSGTYPE_CASE) && (transDetails.TransMethord_Flag==edebit))//if trans is debit sale type
		{
			fSalAmount = fSalAmount + atof(transDetails.amount);//need to put void also
			settlDetails.saleCount++;
		}
		if((transDetails.TransTypeFlg== WITHDRAWAL_MSG_TYPE_CASE) && (transDetails.TransMethord_Flag==edebit))//if trans is debit sale type
		{
			fWithAmount = fWithAmount + atof(transDetails.amount);//need to put void also
			settlDetails.withdrawCount++;
		}
		if((transDetails.TransTypeFlg== DEPOSIT_MSG_TYPE_CASE) && (transDetails.TransMethord_Flag==edebit))//if trans is debit sale type
		{
			fDepAmount = fDepAmount + atof(transDetails.amount);//need to put void also
			settlDetails.depositCount++;
		}
		if((transDetails.TransTypeFlg== TRANSFER_MSG_TYPE_CASE) && (transDetails.TransMethord_Flag==edebit))//if trans is debit sale type
		{
			fTraAmount = fTraAmount + atof(transDetails.amount);//need to put void also
			settlDetails.transferCount++;
		}
		else if((transDetails.TransTypeFlg== REFUNDMSGTYPE_CASE) && (transDetails.TransMethord_Flag==edebit))//if trans is debit refund type
		{
			fRefAmount = fRefAmount + atof(transDetails.amount);
			settlDetails.refundCount++ ;
		}
		else if((transDetails.TransTypeFlg== VOIDMSGTYPE_CASE) && (transDetails.TransMethord_Flag==edebit))//if trans is debit refund type
		{
			settlDetails.saleCount--;
			fSalAmount = fSalAmount -  atof(transDetails.amount);
		}
		//////Credit
		else if((transDetails.TransTypeFlg== SALEMSGTYPE_CASE) && (transDetails.TransMethord_Flag==ecredit))//if trans is credit sale type
		{
			fcreditSalAmount = fcreditSalAmount + atof(transDetails.amount);//need to put void also
			settlDetails.creditSalesCount++;
		}
		else if((transDetails.TransTypeFlg== REFUNDMSGTYPE_CASE) && (transDetails.TransMethord_Flag==ecredit))//if trans is credit refund type
		{
			fcreditRefAmount = fcreditRefAmount + atof(transDetails.amount);
			settlDetails.creditrefundCount++;
		}
		else if((transDetails.TransTypeFlg== VOIDMSGTYPE_CASE) && (transDetails.TransMethord_Flag==ecredit))//if trans is debit refund type
		{
			settlDetails.creditSalesCount--;
			fcreditSalAmount = fcreditSalAmount -  atof(transDetails.amount);
		}
        //memset(&transDetails,0,sizeof(transDetails));
	}
	if(counter == 0)//if no data in file
    {
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("There is no transaction in file for settlement"));
		}
		fclose(ifp);
		return (-2);
	}
    else
    {
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("In  else counter =%d",counter));
		}
    }
   
	//--------------- for credit ---------------------
	sprintf(temp,"%f",fcreditSalAmount);
	sprintf(settlDetails.creditSaleAmount,"%0.02f",fcreditSalAmount);
    if(LOG_STATUS == LOG_ENABLE)
    {
		LOG_PRINTF(("credit sale amount =%s",settlDetails.creditSaleAmount));
		LOG_PRINTF(("credit sale count =%d",settlDetails.creditSalesCount));
    }
	memset(temp,0,sizeof(temp));
	sprintf(temp,"%f",fcreditRefAmount);
	sprintf(settlDetails.creditRefundAmount,"%0.02f",fcreditRefAmount);
    if(LOG_STATUS == LOG_ENABLE)
    {
		LOG_PRINTF(("credit refund amount =%s",settlDetails.creditRefundAmount));
		LOG_PRINTF(("credit refund count =%d",settlDetails.creditrefundCount));
    }
	
	//----------------for debit  ---------------------
	sprintf(temp,"%f",fSalAmount);
	sprintf(settlDetails.saleAmount,"%0.02f",fSalAmount);
    if(LOG_STATUS == LOG_ENABLE)
    {
		LOG_PRINTF(("sale amount =%s",settlDetails.saleAmount));
		LOG_PRINTF(("sale count =%d",settlDetails.saleCount));
    }
  
	sprintf(temp,"%f",fWithAmount);
	sprintf(settlDetails.withdrawAmount,"%0.02f",fWithAmount);
    if(LOG_STATUS == LOG_ENABLE)
    {
		LOG_PRINTF(("withdraw amount =%s",settlDetails.withdrawAmount));
		LOG_PRINTF(("withdraw count =%d",settlDetails.withdrawCount));
    }
	sprintf(temp,"%f",fDepAmount);
	sprintf(settlDetails.depositAmount,"%0.02f",fDepAmount);
    if(LOG_STATUS == LOG_ENABLE)
    {
		LOG_PRINTF(("deposit amount =%s",settlDetails.depositAmount));
		LOG_PRINTF(("deposit count =%d",settlDetails.depositCount));
    }
	sprintf(temp,"%f",fTraAmount);
	sprintf(settlDetails.transferAmount,"%0.02f",fTraAmount);
    if(LOG_STATUS == LOG_ENABLE)
    {
		LOG_PRINTF(("transfer amount =%s",settlDetails.transferAmount));
		LOG_PRINTF(("transfer count =%d",settlDetails.transferCount));
    }

	memset(temp,0,sizeof(temp));
	sprintf(temp,"%f",fRefAmount);
	sprintf(settlDetails.refundAmount,"%0.02f",fRefAmount);
    if(LOG_STATUS == LOG_ENABLE)
    {
		LOG_PRINTF(("refund amount =%s",settlDetails.refundAmount));
		LOG_PRINTF(("refund count =%d",settlDetails.refundCount));
    }
	paddCount(settlDetails.creditSalesCount,countTrans);
	strcat(rqstBuff,countTrans);
   	paddAmount(settlDetails.creditSaleAmount,ReqAmount);
	strcat(rqstBuff,ReqAmount);
    memset(countTrans,0,4);
	paddCount(settlDetails.creditrefundCount,countTrans);		
    strcat(rqstBuff,countTrans);
	memset(ReqAmount,0,AMOUNT_LEN+1);
    paddAmount(settlDetails.creditRefundAmount,ReqAmount);
    strcat(rqstBuff,ReqAmount);
    memset(countTrans,0,4);
	paddCount(settlDetails.saleCount,countTrans);
	strcat(rqstBuff,countTrans);
	paddAmount(settlDetails.saleAmount,ReqAmount);
	strcat(rqstBuff,ReqAmount);
	memset(countTrans,0,4);
	paddCount(settlDetails.refundCount,countTrans);
	strcat(rqstBuff,countTrans);
	memset(ReqAmount,0,AMOUNT_LEN+1);
	paddAmount(settlDetails.refundAmount,ReqAmount);
	strcat(rqstBuff,ReqAmount);
    strncpy(transMsg->rqstConsilation,rqstBuff,RECONCILLATION_REQ_LEN);//putting rqst string in to main transMsg structure
		/////////////////////////////////////////////////////////////////////
    if(LOG_STATUS == LOG_ENABLE)
    {
		LOG_PRINTF(("rqstBuff = %s",transMsg->rqstConsilation));
    }
	//	DebugggingInRel("fclose",strlen("fclose"));
   	fclose(ifp);
	return _SUCCESS;
}
/****************************************************************************************************************
*	Function Name : printAnotherCopy																											                        *
*	Purpose		    : This function prints the another copy of a successfull txn before settlement                  *
* Input					: pointer to the TransactionMsgStruc    																												*
*	Output		    : returns success or failure 																																		*
*****************************************************************************************************************/
short printAnotherCopy(TransactionMsgStruc *transMsg)
{
      //char ch = 0;
		//  short iRetVal = 0;
#ifdef GOVT_APP //if application is government type
			short retPaperStatus = 0;
		  retPaperStatus = Paperstatus();
      if(retPaperStatus == KEY_CANCEL)
        return KEY_CANCEL;
      do
		  {
          iRetVal = getTransNo(transMsg);//getting Payment Id from user for Government
			    if(((iRetVal == _FAIL) || (iRetVal == KEY_STR) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL)))
			    {
					    return GoToMainScreen;
			    }
          iRetVal = ReadTransDetails(transMsg);//reading data from file
          if(iRetVal == _SUCCESS)
			    {
					    break;//for success
			    }
          else if(iRetVal == -2)
          {
              window(1,1,30,20);
			        clrscr();
     	        write_at(Dmsg[NO_TRANSACTION_TO_COPY].dispMsg, strlen(Dmsg[NO_TRANSACTION_TO_COPY].dispMsg),Dmsg[NO_TRANSACTION_TO_COPY].x_cor, Dmsg[NO_TRANSACTION_TO_COPY].y_cor);
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
			if(iRetVal == _FAIL)
					return GoToMainScreen;
			printRecipt(transMsg);
#endif
      return _SUCCESS;

}
/****************************************************************************************************************
*	Function Name : processingBatchUpload																											                    *
*	Purpose		    : This function sends the each transactions for batch upload one by one if the settlement fails *
* Input					: void																												*
*	Output		    : returns success or failure 																																		*
*****************************************************************************************************************/

short processingBatchUpload(void)
{ 
	int i =0;
	char Bitmap[BITMAP_ARRAY_SIZE+1] ={0};//8
	char UnpackBiitmap[UNPACK_BITMAP_ARRAY_SIZE+1]={0};//16
	char bits[BITMAP_SIZE+1] ={0};//64
	char temp[BITMAP_ARRAY_SIZE+1] ={0};//9
	FILE *ifp=NULL;
	TransactionMsgStruc *transMsg =NULL;
	short BITMAP_LEN =8;
	short ReqLen = 0; 
	short retVal = -1;  
	
	unsigned char reqBuff[BATCH_UPLOAD_REQUEST_SIZE]={0};//122
	unsigned char resBuff[BATCH_UPLOAD_RESPONSE_SIZE]={0};//200 to 120
	unsigned char testBuff[BATCH_UPLOAD_UNAPACK_RES_SIZE]={0};//240
	short recvLength = 0;
	TrDetails transDetails={0};

  
  //////////////////////////////////////
  field_struct iso8583_field_table[] = 
  { 
    /*Fld 8583 Convert Variable name and size no.	sz 	index *//*BCD_BCD,BCD_STR*/ //ASC_ASC->
    { 0, TPDU_LEN,BCD_STR,(void *) (BitmapStructOb.tpdu), sizeof(BitmapStructOb.tpdu) }, 
    { 0, MSG_ID_LEN, BCD_ASC, (void *) BitmapStructOb.message_id, sizeof( BitmapStructOb.message_id) },
    { 2, 19, BV2_STR, (void *) BitmapStructOb.field_02, sizeof( BitmapStructOb.field_02) }, 
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
    { 14, 4, BCD_STR, (void *) BitmapStructOb.field_14, sizeof( BitmapStructOb.field_14) }, 
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
    { 35+ SKIP, 37, BV2_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 36+ SKIP, 104, AV2_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 37, 12, ASC_STR, (void *) BitmapStructOb.field_37, sizeof( BitmapStructOb.field_37) }, 
    { 38+ SKIP, 6, BCD_STR, (void *) BitmapStructOb.field_38, sizeof( BitmapStructOb.field_38) }, 
    { 39+ SKIP, 2, BCD_STR, (void *) BitmapStructOb.field_39, sizeof( BitmapStructOb.field_39) }, 
    { 40+ SKIP, 3, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 41, 8, ASC_STR, (void *) BitmapStructOb.field_41, sizeof( BitmapStructOb.field_41) }, 
    { 42, 15, ASC_STR, (void *) BitmapStructOb.field_42, sizeof( BitmapStructOb.field_42) }, 
    { 43+ SKIP, 40, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 44+ SKIP, 25, AV2_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 45+SKIP, 76, AV2_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, //Skippping because using track 2 data(bit-35)
    { 46+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 47+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 48+ SKIP, 9, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 49+ SKIP, 3, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 50+ SKIP, 3, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 51+ SKIP, 3, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 52+ SKIP, 64, BIT_BIT, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 53+ SKIP, 16, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 54+ SKIP, 120, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 55+ SKIP, 0, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 56+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 57+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 58+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 59+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 60+ SKIP, 6, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 61+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 62+ SKIP, 999, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 63+ SKIP, 60, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard)  },  
    { 64+ SKIP +STOP, 64, BIT_BIT, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
  }; 
  //BITMAP_LEN=8;
	transMsg = (TransactionMsgStruc*)malloc(sizeof (TransactionMsgStruc));
	memset(transMsg,0,sizeof *transMsg);

  //memset(&BitmapStructOb, 0, sizeof(BmapStruct));
	resetBitmapStructure(&BitmapStructOb);//memsetting all the elements of structure
	ifp = fopen(TRANS_DETAILS_FILE, "r");
		if (ifp == NULL)
		{
			if(LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF (("Failed to open the file %s\n", TRANS_DETAILS_FILE));	
			}
			resetMsgDetails(transMsg);						//reset All Data which is present in Transaction Message Structure
			free(transMsg);
			return _FAIL;
		}
 		while (fread(&transDetails, sizeof(TrDetails), 1, ifp) != 0)
		{
			//memset(transMsg,0,sizeof(TransactionMsgStruc));
			LOG_PRINTF(("After Memery Free in the prntSumRprt2 _stack_max() =%d",_stack_max()));
   
    //resetMsgDetails(transMsg);						//reset All Data which is present in Transaction Message Structure

    memset(transMsg,0, sizeof(TransactionMsgStruc));
  
    strncpy(msg_id,BATCH_UPLOAD_MSG_TYPE,MSG_ID_LEN);
    strncpy(Proc_code,transDetails.processingCode,PROC_CODE_LEN);
    strncpy(transMsg->MessageTypeInd,BATCH_UPLOAD_MSG_TYPE,MSG_ID_LEN);//
    
    strncpy(transMsg->ProcessingCode,transDetails.processingCode,PROC_CODE_LEN); //BitField 3
    transMsg->TrTypeFlag = transDetails.TransTypeFlg; 
    
    strncpy(transMsg->PrimaryAccNum,transDetails.cardNumber,ACC_NUM_ARR_LEN);////BitField 2    
		strncpy(transMsg->Amount,transDetails.amount,AMOUNT_LEN);//BitField 4		
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF (("->>>>>>>>>Amountb in batch upload =%s",transMsg->Amount));
    }
		strncpy(transMsg->TraceAuditNo,transDetails.traceAuditNo,TRACE_AUDIT_LEN);//BitField 11
    strncpy(transMsg->TransLocalTime,transDetails.orgTransTime,TRANS_LOCALTIME_LEN); //BitField 12
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF (("->>>>>>>>>transMsg->TransLocalTime =%s",transMsg->TransLocalTime));
    }
		strncpy(transMsg->TransLocalDate,transDetails.orgTransDate,TRANS_LOCALDATE_LEN); //BitField 13
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF (("->>>>>>>>>transMsg->TransLocalDate =%s",transMsg->TransLocalDate));
    }
		strncpy(transMsg->ExpiryDate,transDetails.expDate,EXPTIME_ARR_LEN);//BitField 14
    strncpy(transMsg->POSEntryMode,transDetails.PosEntryMode,POS_ENTRY_LEN); //BitField 22
    strncpy(transMsg->NetworkInternationalId,transDetails.NII,NII_LEN); //BitField 24
    strncpy(transMsg->POSConditionCode,transDetails.PosCondCode,POS_COND_LEN); //BitField 25
    strncpy(transMsg->RetrievalReferenceNo,transDetails.RetrievalRefNo,RETRIEVAL_REF_LEN); //BitField 37
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF (("->>>>>>>>>transMsg->RetrievalReferenceNo =%s",transMsg->RetrievalReferenceNo));
    }
    strncpy(transMsg->TerminalID,transDetails.TerminalId,TERMINAL_ID_LEN); //BitField 41
  
    strncpy(transMsg->CardAcceptorID,transDetails.CardAcceptorId,CARD_ACCEPTOR_ID_LEN); //BitField 42
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("processingBatchUpload : TerminalID= %s CardAcceptorID =%s",transMsg->TerminalID,transMsg->CardAcceptorID));
    }
   
    ProcessingISOBitmapEngine(iso8583_field_table, transMsg);
    ReqLen = retVal = assemble_packet(iso8583_field_table,reqBuff);
 
    if(retVal <= _SUCCESS)
    { 
      if(LOG_STATUS == LOG_ENABLE)
      {
        LOG_PRINTF(("Error Occurred in assemble_packet retVal = %d \n",retVal));
      }
      ResetVariablesForBatchUpload(&transDetails,sizeof(TrDetails),&BitmapStructOb,testBuff,sizeof(testBuff),temp,Bitmap,UnpackBiitmap,reqBuff,sizeof(reqBuff),resBuff,sizeof(resBuff));//reset All Data which is present in Transaction Message Structure
	    continue;
    }
    
    recvLength = sizeof(resBuff) ;
    retVal = CommWithServer((char*)reqBuff,ReqLen,(char*)resBuff,&recvLength);
    
    if(retVal ==_SUCCESS )
    { 
      bcd2a((char *)testBuff,(char *)resBuff,recvLength);
      if(LOG_STATUS == LOG_ENABLE)
      {
        LOG_PRINTF(("Testbuffin disassemblePacket retVal = %s \n",testBuff));
      }
      
      strncpy(UnpackBiitmap,(char*)testBuff+UNPACK_SIZE_MSGLEN_OF_REQ_RES+TPDU_LEN+MSG_ID_LEN,BITMAP_LEN*2);
      packData(UnpackBiitmap,Bitmap);
      for(i=0;i<BITMAP_LEN;i++)
      {
          CopyAllBits(Bitmap+i,temp);
          strcpy(bits+(i*8),temp);
      }
      resetBitmapStructure(&BitmapStructOb);//memsetting all the elements of structure
		  ParseAndFillBitFields((char*)testBuff,bits);
    
			//---------------------------------------------------------------
      window(1,1,30,20);
			clrscr();
      	
			write_at(Dmsg[UPLOAD_NEXT_TRANSACTION].dispMsg, strlen(Dmsg[UPLOAD_NEXT_TRANSACTION].dispMsg),Dmsg[UPLOAD_NEXT_TRANSACTION].x_cor, Dmsg[UPLOAD_NEXT_TRANSACTION].y_cor);
      SVC_WAIT(500);
      ClearKbdBuff();
			KBD_FLUSH();
      if(LOG_STATUS == LOG_ENABLE)
      {
        LOG_PRINTF(("UPLOAD_NEXT_TRANSACTION \n"));     
      }
      
    }
    else
    {
      window(1,1,30,20);
			clrscr();
      	
			write_at(Dmsg[UPLOAD_NEXT_TRANSACTION].dispMsg, strlen(Dmsg[UPLOAD_NEXT_TRANSACTION].dispMsg),Dmsg[UPLOAD_NEXT_TRANSACTION].x_cor, Dmsg[UPLOAD_NEXT_TRANSACTION].y_cor);
      SVC_WAIT(500);
      ClearKbdBuff();
			KBD_FLUSH();
      if(LOG_STATUS == LOG_ENABLE)
      {
        LOG_PRINTF(("going to memset ...........\n"));  
      }
      ResetVariablesForBatchUpload(&transDetails,sizeof(TrDetails),&BitmapStructOb,testBuff,sizeof(testBuff),temp,Bitmap,UnpackBiitmap,reqBuff,sizeof(reqBuff),resBuff,sizeof(resBuff));

      continue ;
    }
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("going to memset ||||| ...........\n"));
    }
    ResetVariablesForBatchUpload(&transDetails,sizeof(TrDetails),&BitmapStructOb,testBuff,sizeof(testBuff),temp,Bitmap,UnpackBiitmap,reqBuff,sizeof(reqBuff),resBuff,sizeof(resBuff));
			
	}
 
  resetMsgDetails(transMsg);						//reset All Data which is present in Transaction Message Structure
	free(transMsg);
  fclose(ifp);
	return _SUCCESS;
}
/****************************************************************************************************************
*	Function Name : processingSettlementTrailer																											              *
*	Purpose		    : This function sends the Settlement Trailer after the batch upload                             *
* Input					: a pointer to the reconcillation request buffer																								*
*	Output		    : returns success or failure 																																		*
*****************************************************************************************************************/
short processingSettlementTrailer(char *ptrReqConcillation)
{ 
	int i =0;
  char Bitmap[BITMAP_ARRAY_SIZE+1] ={0};//8
  char UnpackBiitmap[UNPACK_BITMAP_ARRAY_SIZE+1]={0};//16
  char bits[BITMAP_SIZE+1] ={0};//64
  char temp[BITMAP_ARRAY_SIZE+1] ={0};//9

  char terminalId[TERMINAL_ID_LEN+1]={0};//9
	char cardAccepterId[CARD_ACCEPTOR_ID_LEN+1]={0};//16
	TransactionMsgStruc *transMsg =NULL;
  short BITMAP_LEN =8;
	short retVal = -1;  
	short ReqLen = 0;
  unsigned char reqBuff[SETLLEMENT_TRAILOR_REQUEST_SIZE]={0};//122
  unsigned char resBuff[SETLLEMENT_TRAILOR_RESPONSE_SIZE]={0};//200 to 120
  unsigned char testBuff[SETLLEMENT_TRAILOR_UNAPACK_RES_SIZE]={0};//240
  short recvLength = 0;
 
  //////////////////////////////////////
  field_struct iso8583_field_table[] = 
  { 
    /*Fld 8583 Convert Variable name and size no.	sz 	index *//*BCD_BCD,BCD_STR*/ //ASC_ASC->
    { 0, TPDU_LEN,BCD_STR,(void *) (BitmapStructOb.tpdu), sizeof(BitmapStructOb.tpdu) }, 
    { 0, MSG_ID_LEN, BCD_ASC, (void *) BitmapStructOb.message_id, sizeof( BitmapStructOb.message_id) },
    { 2+ SKIP, 19, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 3, 6, BCD_STR, (void *) BitmapStructOb.field_03, sizeof( BitmapStructOb.field_03) }, 
    { 4+ SKIP, 12, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
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
    { 22+ SKIP, 3, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 23+ SKIP, 3, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 24, 3, BCD_STR, (void *) BitmapStructOb.field_24, sizeof( BitmapStructOb.field_24) }, 
    { 25+ SKIP, 2, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 26+ SKIP, 2, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 27+ SKIP, 1, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 28+ SKIP, 8, XBC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 29+ SKIP, 8, XBC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 30+ SKIP, 8, XBC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 31+ SKIP, 8, XBC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 32+ SKIP, 11, BV2_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 33+ SKIP, 11, BV2_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 34+ SKIP, 28, AV2_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 35+ SKIP, 37, BV2_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
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
    { 48, 9, AV3_STR, (void *) BitmapStructOb.field_48, sizeof( BitmapStructOb.field_48) }, 
    { 49+ SKIP, 3, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 50+ SKIP, 3, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 51+ SKIP, 3, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 52, 64, BIT_BIT, (void *) BitmapStructOb.field_52, sizeof( BitmapStructOb.field_52) }, 
    { 53+ SKIP, 16, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 54+ SKIP, 120, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 55+ SKIP, 999, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 56+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 57+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 58+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 59+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 60, 6, ASC_STR, (void *) BitmapStructOb.field_60, sizeof( BitmapStructOb.field_60) }, 
    { 61+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 62+ SKIP, 999, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 63, 60, ASC_STR, (void *) BitmapStructOb.field_63, sizeof( BitmapStructOb.field_63)  },  
    { 64+ SKIP +STOP, 64, BIT_BIT, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
  }; 
 //BITMAP_LEN=8;
  transMsg = (TransactionMsgStruc*)malloc(sizeof (TransactionMsgStruc));
  memset(transMsg,0,sizeof *transMsg);

  strncpy(msg_id,SETTLEMENT_MSG_TYPE,MSG_ID_LEN);
  strncpy(Proc_code,SETTLEMENT_TRAILER_PROCODE,PROC_CODE_LEN);
  if(LOG_STATUS == LOG_ENABLE)
  {
	  LOG_PRINTF (("->>>>>>>>>>processing code =%s",transMsg->ProcessingCode));
  }
  strncpy(transMsg->MessageTypeInd,SETTLEMENT_MSG_TYPE,MSG_ID_LEN);
  strncpy(transMsg->ProcessingCode,SETTLEMENT_TRAILER_PROCODE,PROC_CODE_LEN);
  strncpy(transMsg->rqstConsilation,ptrReqConcillation,RECONCILLATION_REQ_LEN);
  transMsg->TrTypeFlag = SETTLEMENT_MSG_TYPE_CASE;

  getTraceAuditNumber(transMsg);
  updateBatchNumber(transMsg);//getting Batch number
	get_env("#TERMINAL_ID",(char*)terminalId,sizeof(terminalId));
	sprintf(transMsg->TerminalID,"%s",terminalId);
	if(LOG_STATUS == LOG_ENABLE)
  {
    LOG_PRINTF(("transMsg->TerminalID = %s",transMsg->TerminalID));
  }

  get_env("#CARDACCEPTERID",(char*)cardAccepterId,sizeof(cardAccepterId));
	sprintf(transMsg->CardAcceptorID,"%s",cardAccepterId);
  sprintf(transMsg->_transType,"%s","SETTLEMENT");
  if(LOG_STATUS == LOG_ENABLE)
  {
    LOG_PRINTF(("transMsg->CardAcceptorID = %s",transMsg->CardAcceptorID));
  }

  resetBitmapStructure(&BitmapStructOb);//memsetting all the elements of structure
   
  ProcessingISOBitmapEngine(iso8583_field_table, transMsg);
  ReqLen = retVal = assemble_packet(iso8583_field_table,reqBuff);
 
  if(retVal <= _SUCCESS)
  {    
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("Error Occurred in assemble_packet retVal = %d \n",retVal));
    }
    free(transMsg);
    return retVal;
  }
  
  recvLength = DEFAULT_BUFLEN ;
  retVal = CommWithServer((char*)reqBuff,ReqLen,(char*)resBuff,&recvLength);
  
  if(retVal == _SUCCESS )//checking for response buffer size 
  {  
    bcd2a((char *)testBuff,(char *)resBuff,recvLength);
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("SettlementTrailer Testbuffin disassemblePacket retVal = %s \n",testBuff));
    }      
    strncpy(UnpackBiitmap,(char*)testBuff+UNPACK_SIZE_MSGLEN_OF_REQ_RES+TPDU_LEN+MSG_ID_LEN,BITMAP_LEN*2);
    packData(UnpackBiitmap,Bitmap);
    for(i=0;i<BITMAP_LEN;i++)
    {
        CopyAllBits(Bitmap+i,temp);
        strcpy(bits+(i*8),temp);
    }
    resetBitmapStructure(&BitmapStructOb);//memsetting all the elements of structure
		ParseAndFillBitFields((char*)testBuff,bits);
    if(!strcmp((char *)BitmapStructOb.field_39,"00"))
    {
			  //---------------------------------------------------------------
        window(1,1,30,20);
				clrscr();
        strcpy(transMsg->ResponseText,APPROVED_MSG);
				
				write_at(Dmsg[SETTLEMENT_TRAILER_SUCCESS].dispMsg, strlen(Dmsg[SETTLEMENT_TRAILER_SUCCESS].dispMsg),Dmsg[SETTLEMENT_TRAILER_SUCCESS].x_cor, Dmsg[SETTLEMENT_TRAILER_SUCCESS].y_cor);
        SVC_WAIT(2000);
        ClearKbdBuff();
				KBD_FLUSH();
        if(LOG_STATUS == LOG_ENABLE)
        {
          LOG_PRINTF(("SETTLEMENT_TRAILER_SUCCESS\n"));     
        }
    }
    else
    {
      window(1,1,30,20);
			clrscr();
     	write_at(Dmsg[SETTLEMENT_TRAILER_DECLINE].dispMsg, strlen(Dmsg[SETTLEMENT_TRAILER_DECLINE].dispMsg),Dmsg[SETTLEMENT_TRAILER_DECLINE].x_cor, Dmsg[SETTLEMENT_TRAILER_DECLINE].y_cor);	
      SVC_WAIT(1000);
      ClearKbdBuff();
			KBD_FLUSH();
      if(LOG_STATUS == LOG_ENABLE)
      {
        LOG_PRINTF(("SETTLEMENT_TRAILER_DECLINE Fail reason : %s \n",BitmapStructOb.field_39));    
      }

    }
     
  }
  else
  {
	  if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("SettlementTrailer:Response buffer size is zero\n"));  
    }
    free(transMsg);
    return _FAIL; 
  }
  free(transMsg);
	return _SUCCESS;
}
/****************************************************************************************************************
*	Function Name : ResetVariablesForBatchUpload																											            *
*	Purpose		    : This function restes the variables after the each transaction during the batch upload         *
* Input					: a pointer to the trans details ,size of trans details ,a pointer to the bitmap structure
                  a pointer to the request and response buffer                                                                                              *
*	Output		    : returns success or failure 																																		*
*****************************************************************************************************************/
void ResetVariablesForBatchUpload(TrDetails *ptrTransDetails,int size_TransDetails,BmapStruct *ptrbmpstruct,unsigned char *testbuff,int size_testBuff,char *tempbuff,char *BitmapBuff,char *UnpackBitmapBuff,unsigned char *reqBuff,int size_reqBuff,unsigned char *resBuff,int size_resBuff)
{
   memset(ptrTransDetails,0,size_TransDetails);	  
   resetBitmapStructure(ptrbmpstruct);//memsetting all the elements of structure
   memset(testbuff, 0, size_testBuff);
   memset(tempbuff, 0, BITMAP_ARRAY_SIZE+1);
   memset(BitmapBuff, 0, BITMAP_ARRAY_SIZE);
   memset(UnpackBitmapBuff, 0, UNPACK_BITMAP_ARRAY_SIZE);
   memset(reqBuff, 0, size_reqBuff);
   memset(resBuff, 0,size_resBuff);
}
/****************************************************************************************************************
*	Function Name : CleanFileData																											                            *
*	Purpose		    : This function cleans the file data                                                            *
* Input					: a pointer to the reconcillation request buffer																								*
*	Output		    : returns success or failure 																																		*
*****************************************************************************************************************/
short CleanFileData()
{
    FILE *ifp = NULL;
    ifp = fopen(TRANS_DETAILS_FILE, "w");//in order to clear data from file need to open file in write mode
		if (ifp == NULL)
		{
      if(LOG_STATUS == LOG_ENABLE)
      {
				LOG_PRINTF (("Failed to open the file %s\n", TRANS_DETAILS_FILE));	
      }
			return _FAIL;
		}
		fclose(ifp);
    return _SUCCESS;
}
