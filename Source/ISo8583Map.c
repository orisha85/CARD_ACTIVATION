/***************************************************************************
  Filename		     : ISo8583Map.c
  Project		       : Bevertec 
  Developer Neame  : Amar
  Module           : ISO 8583 request and response
  Description	     : This is the implementation of creating request in the 
                     ISO 8583 format and parse the response 
*****************************************************************************/


#include <stdlib.h>
#include <string.h>
#include "..\Include\Common.h"
#include "..\Include\ISo8583Map.h"
#include "..\Include\Settlement.h"
#include "..\Include\SaleTransaction.h"
#include "..\include\BVTC_EMV.h"
#include <prot8583.h>
#include <iso8583.h>
#include "prot8583.h"
#include "portdef.h"

char msg_id[MSG_ID_LEN+1];//5
char Proc_code[PROC_CODE_LEN+1];//7
extern BmapStruct BitmapStructOb ;
extern unsigned char EncSessionKey[UNPACK_ENC_PR_KEY_LEN+1];//33 
extern unsigned char EncPinBlock[BITMAP_ARRAY_SIZE];//8
extern unsigned char EncOldPinBlock[BITMAP_ARRAY_SIZE];
extern unsigned char KSN_pp[KSN_LENGTH];
extern sEMV_DATA emvdata;		//global emv structure
extern unsigned char EncSessionKeyKCV[UNPACK_ENC_PR_KCV_LEN+1]; 
extern unsigned char DataEncKey[UNPACK_ENC_PR_KEY_LEN+1]; 
extern unsigned char DataEncKeyKCV[UNPACK_ENC_PR_KCV_LEN+1]; 
char temp_pin2[24];

/******************************************************************
 If variant fields are used, this function must return a value 
 will be used to match against the values in the first column of  
 variant field tables. 
******************************************************************/ 

unsigned int return_variant1()
{
unsigned char x [3];
MEMCLR (x, 3);
/* Most significant 2 digits of proc_code. */
SVC_UNPK4 ((char *)x, Proc_code, 1);
return (str2int (((char *)x)));
}

/****************************************************************
 If variant fields are used, this function must return a value
 which will be used to match against the values in the second
 column of variant field tables.  
****************************************************************/ 

unsigned int return_variant2()
{
  unsigned char x [5];
  MEMCLR (x, 5);
  SVC_UNPK4 ((char *)x, msg_id, 2);
  return (str2int (((char *)x)));
}

/********************************************************************************************************************
*	Function Name : paddAmount																													                      *																																						                *
*	Purpose		    : pad amount with zeros on left for concat it in request packet		 												                              *
*	Input		      : a pointer to the amount variable and the output buffer				        		              										  *
*	Output		    : Return value for the success or failure																			              			  *
*********************************************************************************************************************/
short paddAmount(char *Amount,char *finalAmount)
{
		char mainAmt[8]={0};
		char decAmt[3]={0};
		
		short i=0;
		char *lptr = NULL;
		char *rptr = NULL;
		lptr = Amount;
	
		rptr=strstr(Amount,".");
		rptr= rptr-1;
		for(i=0;lptr<=rptr;i++)
		{
				mainAmt[i]=*lptr;
				lptr++;
		}
		rptr =rptr+2;
 
		strncpy(decAmt,rptr,2);
 
		strcat(mainAmt,decAmt);
		memset(finalAmount,'0',AMOUNT_LEN);
		strncpy(finalAmount+AMOUNT_LEN-strlen(mainAmt),mainAmt,strlen(mainAmt));
   
		return _SUCCESS;
}
/********************************************************************************************************************
*	Function Name : CreateRequestStream																													                      *																																						                *
*	Purpose		    : Create Request to set values in variable		 												                              *
*	Input		      : a pointer to the bitmap structture object					        		              										  *
*	Output		    : Return value for the success or failure																			              			  *
*********************************************************************************************************************/
void CreateRequestStream(TransactionMsgStruc *ptrTransactionMsgStrucOb)
{
  short i = 0 ;
  short j = 0;
  int x = 0;
  char ReqAmount[AMOUNT_LEN+1]={0};//13
  int field_63_header[] = { 48, 34, 51, 51 };
  unsigned char field_63_new_pin[] =  { '0', '0', '1', '0', '4', 'E', '5', '0'};
  char temp[10];
  unsigned char *ptr = ptrTransactionMsgStrucOb->NewPinData;
  int ret = 0;
  char testch[2] = { 0 }; 
  char temp_pin[11];

  if(LOG_STATUS == LOG_ENABLE)
  {
	  LOG_PRINTF (("**************Track2Data get_char=%s",ptrTransactionMsgStrucOb->Track2Data));
    LOG_PRINTF (("**************POSEntryMode get_char=%s",ptrTransactionMsgStrucOb->POSEntryMode));
  }
  strncpy((char *)BitmapStructOb.tpdu,TPDU,TPDU_LEN);					// tpdu  copied from the sample txns provided by client
  /*Only Id field of the TPDU is checked for valid values. The Originator Address and Destination Address are swapped on the response.*/

  //commneted after emv entgration strncpy((char *)ptrTransactionMsgStrucOb->POSEntryMode,POS_ENTRY_MODE,POS_ENTRY_LEN);    
  strncpy((char *)ptrTransactionMsgStrucOb->NetworkInternationalId,NETWORK_INTER_ID,NII_LEN);                //24 NII
	strncpy((char *)ptrTransactionMsgStrucOb->POSConditionCode,POS_COND_CODE,POS_COND_LEN);                //25 POS Condition Code
  //strncpy((char *)ptrTransactionMsgStrucOb->Track2Data,TRACK2_DATA,strlen(TRACK2_DATA));        //35 Sample Track 2 Data provided by Client 
	//strncpy((char *)ptrTransactionMsgStrucOb->Track2Data,ptrTransactionMsgStrucOb->CardDetails.track,strlen(ptrTransactionMsgStrucOb->CardDetails.track)); // Comment this if you are not using this line
  
  if(LOG_STATUS == LOG_ENABLE)
  {
	  LOG_PRINTF ((" DE55get_char = %s",emvdata.DE55));
  }

  //if(!strcmp((char *)ptrTransactionMsgStrucOb->POSEntryMode,ICC_PIN_CAPABLE)) 
  //{
  //  if(strcmp((char *)ptrTransactionMsgStrucOb->MessageTypeInd,REVERSAL_MSG_TYPE)!=0)
  //  {
  if(ptrTransactionMsgStrucOb->EMV_Flag == 1)//for an EMV transaction
  {
    if(strcmp((char *)emvdata.DE55,"")!=0)
    {
      //strncpy((char *)ptrTransactionMsgStrucOb->EMV_Tag_Data,emvdata.DE55,strlen(emvdata.DE55));           //  For sending EMV data in plain txt
      packData(emvdata.DE55,ptrTransactionMsgStrucOb->EMV_Tag_Data);
      //packData("5F2A02084082021800950580800080009A031601069C01009F02060000000024009F1001219F1A0203889F260800010203040506079F2701809F330360F8C09F34035E03009F360200019F3704CD28696F",ptrTransactionMsgStrucOb->EMV_Tag_Data);// For sending EMV data hard coded
      strncpy((char *)ptrTransactionMsgStrucOb->TVR,emvdata.TVR,strlen(emvdata.TVR));                //tvR FOR PRINTING
      strncpy((char *)ptrTransactionMsgStrucOb->TSI,emvdata.TSI,strlen(emvdata.TSI));                //tsi FOR PRINTING
      strncpy((char *)ptrTransactionMsgStrucOb->CARD_TYPE,emvdata.CardType,strlen(emvdata.CardType));                //tsi FOR PRINTING
      strncpy((char *)ptrTransactionMsgStrucOb->Verify_Method,emvdata.VerificationMethod,strlen(emvdata.VerificationMethod));   //VERIFICATION METHOD FOR PRINTING
      if(LOG_STATUS == LOG_ENABLE)
      {
        LOG_PRINTF (("packing EMV data TVR = %s",ptrTransactionMsgStrucOb->TVR));
        LOG_PRINTF (("packing EMV data TSI = %s",ptrTransactionMsgStrucOb->TSI));
      }
    }
  }
 
  if(ptrTransactionMsgStrucOb->TrMethordFlag == edebit)//Copy the Pin Block 
  {

	  if (strlen(EncOldPinBlock) > 0)
	  {
		  for (i = 0; i < BIANRY_PIN_LEN; i++)
		  {
			  ptrTransactionMsgStrucOb->PinData[i] = EncOldPinBlock[i];
		  }

		  for (x = 0; x < 8; x++)
		  {
			  sprintf(temp, "%c", field_63_new_pin[x]);
			  LOG_PRINTF(("%d", field_63_new_pin[x]));
			  LOG_PRINTF((" temp: %s", temp));
			  strcat(ptrTransactionMsgStrucOb->NewPinData, temp);
			  LOG_PRINTF(("for ptrTransactionMsgStrucOb->NewPinData: %s", ptrTransactionMsgStrucOb->NewPinData));
		  }
		  strncat(ptrTransactionMsgStrucOb->NewPinData, EncPinBlock, sizeof(EncPinBlock));
		  LOG_PRINTF((" ptrTransactionMsgStrucOb->NewPinData: %s", ptrTransactionMsgStrucOb->NewPinData));

		  packData(ptrTransactionMsgStrucOb->NewPinData, temp_pin);
	  }
	  else
	  {
		  for (i = 0; i < BIANRY_PIN_LEN; i++)
		  {
			  ptrTransactionMsgStrucOb->PinData[i] = EncPinBlock[i];
		  }
	  }

	  //for (i = 0; i < BIANRY_PIN_LEN; i++)
	  //{
	//	  ptrTransactionMsgStrucOb->NewPinData[i] = EncPinBlock[i];
	  //}


	//for (x = 0; x < 4; x++)
	//{
	//	sprintf(temp, "%c", field_63_header[x]);	
	//	LOG_PRINTF(("%d", field_63_header[x]));
	//	LOG_PRINTF(("%s", temp));
	//	strcat(ptrTransactionMsgStrucOb->KSN, temp);
	//}
	//strcpy(ptrTransactionMsgStrucOb->KSN, temp);
	//strncat(ptrTransactionMsgStrucOb->KSN, KSN_pp, sizeof(KSN_pp));

	LOG_PRINTF(("Old pin : %s", ptrTransactionMsgStrucOb->PinData));
	LOG_PRINTF(("New pin : %s", ptrTransactionMsgStrucOb->NewPinData));

	strcpy(temp_pin2,ptrTransactionMsgStrucOb->NewPinData);

	memset(EncOldPinBlock, 0, BIANRY_PIN_LEN);
    memset(EncPinBlock,0,BIANRY_PIN_LEN);//resetting the PinBlock
  }

	switch(ptrTransactionMsgStrucOb->TrTypeFlag)
	{
	case SALEMSGTYPE_CASE:
		strncpy((char *)BitmapStructOb.message_id, ptrTransactionMsgStrucOb->MessageTypeInd, MSG_ID_LEN);     // message id Authorization request 
																											  //strcpy((char *)BitmapStructOb.field_02,ptrTransactionMsgStrucOb->PrimaryAccNum);           // Primary Account Number 
		strncpy((char *)BitmapStructOb.field_03, ptrTransactionMsgStrucOb->ProcessingCode, PROC_CODE_LEN);       // Processing Code 
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("amount =%s get char", ptrTransactionMsgStrucOb->Amount));
		}
		//strcpy((char *)BitmapStructOb.field_04,ptrTransactionMsgStrucOb->Amount);            // Transaction Amount 
		paddAmount(ptrTransactionMsgStrucOb->Amount, ReqAmount);
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Amount"));
		}
		strncpy((char *)BitmapStructOb.field_04, ReqAmount, AMOUNT_LEN);
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Requset Amount in sale get_char();= %s", ReqAmount));
		}

		strncpy((char *)BitmapStructOb.field_11, ptrTransactionMsgStrucOb->TraceAuditNo, TRACE_AUDIT_LEN);           // System trace audit number 
		strncpy((char *)BitmapStructOb.field_22, ptrTransactionMsgStrucOb->POSEntryMode, POS_ENTRY_LEN);		// POS Entry Mode
		strncpy((char *)BitmapStructOb.field_24, ptrTransactionMsgStrucOb->NetworkInternationalId, NII_LEN);        // NII
		strncpy((char *)BitmapStructOb.field_25, ptrTransactionMsgStrucOb->POSConditionCode, POS_COND_LEN);        // POS Condition Code
		strncpy((char *)BitmapStructOb.field_35, ptrTransactionMsgStrucOb->Track2Data, strlen(ptrTransactionMsgStrucOb->Track2Data));        // Track 2 data
		strncpy((char *)BitmapStructOb.field_41, ptrTransactionMsgStrucOb->TerminalID, TERMINAL_ID_LEN);        // Terminal ID
		strncpy((char *)BitmapStructOb.field_42, ptrTransactionMsgStrucOb->CardAcceptorID, CARD_ACCEPTOR_ID_LEN);        // Terminal ID
		strncpy((char *)BitmapStructOb.field_48, ptrTransactionMsgStrucOb->PaymentId, PAYMENT_ID_LEN);        // Payment ID
																											  //strncpy((char *)BitmapStructOb.field_102,ptrTransactionMsgStrucOb->FromAcNo,ACCOUNT_NO_LEN);        //From Account Agency Bank

		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Field 03=%s ", BitmapStructOb.field_03));
			LOG_PRINTF(("Field 04=%s ", BitmapStructOb.field_04));
			LOG_PRINTF(("Field 11=%s ", BitmapStructOb.field_11));
			LOG_PRINTF(("Field 22=%s ", BitmapStructOb.field_22));
			LOG_PRINTF(("Field 24=%s ", BitmapStructOb.field_24));
			LOG_PRINTF(("Field 25=%s ", BitmapStructOb.field_25));
			LOG_PRINTF(("Field 35=%s ", BitmapStructOb.field_35));
			LOG_PRINTF(("Field 41=%s ", BitmapStructOb.field_41));
			LOG_PRINTF(("Field 42=%s ", BitmapStructOb.field_42));
			LOG_PRINTF(("Field 48=%s ", BitmapStructOb.field_48));
			LOG_PRINTF(("Field 102=%s ", BitmapStructOb.field_102));		//Agency Bank

			LOG_PRINTF(("field copied till_48=%s ", BitmapStructOb.field_48));
			LOG_PRINTF(("field copied till_102=%s ", BitmapStructOb.field_102));		//Agency Bank

		}

		if (ptrTransactionMsgStrucOb->TrMethordFlag == edebit)//copy pin for debit card only
		{
			for (i = 0;i<BIANRY_PIN_LEN;i++)
			{
				BitmapStructOb.field_52[i] = ptrTransactionMsgStrucOb->PinData[i];
			}
			strncpy(BitmapStructOb.field_63, ptrTransactionMsgStrucOb->KSN, sizeof(ptrTransactionMsgStrucOb->KSN)); //63 KSN Value

			LOG_PRINTF(("Field 52=%s ", BitmapStructOb.field_52));
			LOG_PRINTF(("Field 63=%s ", BitmapStructOb.field_63));
		}

		if (!strcmp((char *)ptrTransactionMsgStrucOb->MessageTypeInd, BATCH_UPLOAD_MSG_TYPE))//FOR BATCH UPLOAD 
		{
			strncpy((char *)BitmapStructOb.field_02, ptrTransactionMsgStrucOb->PrimaryAccNum, strlen(ptrTransactionMsgStrucOb->PrimaryAccNum));           // Primary Account Number 
			strncpy((char *)BitmapStructOb.field_12, ptrTransactionMsgStrucOb->TransLocalTime, TRANS_LOCALTIME_LEN);
			strncpy((char *)BitmapStructOb.field_13, ptrTransactionMsgStrucOb->TransLocalDate, TRANS_LOCALDATE_LEN);
			strncpy((char *)BitmapStructOb.field_14, ptrTransactionMsgStrucOb->ExpiryDate, EXPTIME_ARR_LEN);	        // Expiration date  
			strncpy((char *)BitmapStructOb.field_37, ptrTransactionMsgStrucOb->RetrievalReferenceNo, RETRIEVAL_REF_LEN);	        // Expiration date  
		}
		LOG_PRINTF(("Field 02=%s ", BitmapStructOb.field_02));
		LOG_PRINTF(("Field 12=%s ", BitmapStructOb.field_12));
		LOG_PRINTF(("Field 13=%s ", BitmapStructOb.field_13));
		LOG_PRINTF(("Field 14=%s ", BitmapStructOb.field_14));
		LOG_PRINTF(("Field 37=%s ", BitmapStructOb.field_37));

		if (!strcmp((char *)ptrTransactionMsgStrucOb->POSEntryMode, ICC_PIN_CAPABLE))//FOR BATCH UPLOAD 
		{
			LOG_PRINTF(("ICC+_EMV"));
			for (i = 0;i<(strlen(emvdata.DE55) / 2);i++) //
														 //for(i=0;i<81;i++)// For sending EMV data hard coded
			{
				BitmapStructOb.field_55[i] = ptrTransactionMsgStrucOb->EMV_Tag_Data[i];
			}
			strncpy((char *)BitmapStructOb.field_55, ptrTransactionMsgStrucOb->EMV_Tag_Data, strlen(ptrTransactionMsgStrucOb->EMV_Tag_Data));           //  For sending EMV data in plain txt
		}
		break;
			
	case WITHDRAWAL_MSG_TYPE_CASE:
		strncpy((char *)BitmapStructOb.message_id,ptrTransactionMsgStrucOb->MessageTypeInd,MSG_ID_LEN);     // message id Authorization request 
		//strcpy((char *)BitmapStructOb.field_02,ptrTransactionMsgStrucOb->PrimaryAccNum);           // Primary Account Number 
		strncpy((char *)BitmapStructOb.field_03,ptrTransactionMsgStrucOb->ProcessingCode,PROC_CODE_LEN);       // Processing Code 
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF (("amount =%s get char",ptrTransactionMsgStrucOb->Amount));
		}
		//strcpy((char *)BitmapStructOb.field_04,ptrTransactionMsgStrucOb->Amount);            // Transaction Amount 
		paddAmount(ptrTransactionMsgStrucOb->Amount,ReqAmount);
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF (("Amount"));
		}
		strncpy((char *)BitmapStructOb.field_04,ReqAmount,AMOUNT_LEN);
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF (("Requset Amount in sale get_char();= %s",ReqAmount));
		}
    
		strncpy((char *)BitmapStructOb.field_11,ptrTransactionMsgStrucOb->TraceAuditNo,TRACE_AUDIT_LEN);           // System trace audit number 
		strncpy((char *)BitmapStructOb.field_22,ptrTransactionMsgStrucOb->POSEntryMode,POS_ENTRY_LEN);		// POS Entry Mode
		strncpy((char *)BitmapStructOb.field_24,ptrTransactionMsgStrucOb->NetworkInternationalId,NII_LEN);        // NII
		strncpy((char *)BitmapStructOb.field_25,ptrTransactionMsgStrucOb->POSConditionCode,POS_COND_LEN);        // POS Condition Code
		strncpy((char *)BitmapStructOb.field_35,ptrTransactionMsgStrucOb->Track2Data,strlen(ptrTransactionMsgStrucOb->Track2Data));        // Track 2 data
		strncpy((char *)BitmapStructOb.field_41,ptrTransactionMsgStrucOb->TerminalID,TERMINAL_ID_LEN);        // Terminal ID
		strncpy((char *)BitmapStructOb.field_42,ptrTransactionMsgStrucOb->CardAcceptorID,CARD_ACCEPTOR_ID_LEN);        // Terminal ID
		strncpy((char *)BitmapStructOb.field_48,ptrTransactionMsgStrucOb->PaymentId,PAYMENT_ID_LEN);        // Payment ID
		//strncpy((char *)BitmapStructOb.field_102,ptrTransactionMsgStrucOb->FromAcNo,ACCOUNT_NO_LEN);        //From Account Agency Bank
		
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Field 03=%s ",BitmapStructOb.field_03));
			LOG_PRINTF(("Field 04=%s ",BitmapStructOb.field_04));
			LOG_PRINTF(("Field 11=%s ",BitmapStructOb.field_11));
			LOG_PRINTF(("Field 22=%s ",BitmapStructOb.field_22));
			LOG_PRINTF(("Field 24=%s ",BitmapStructOb.field_24));
			LOG_PRINTF(("Field 25=%s ",BitmapStructOb.field_25));
			LOG_PRINTF(("Field 35=%s ",BitmapStructOb.field_35));
			LOG_PRINTF(("Field 41=%s ",BitmapStructOb.field_41));
			LOG_PRINTF(("Field 42=%s ",BitmapStructOb.field_42));
			LOG_PRINTF(("Field 48=%s ",BitmapStructOb.field_48));
			LOG_PRINTF(("Field 102=%s ",BitmapStructOb.field_102));		//Agency Bank
		
			LOG_PRINTF(("field copied till_48=%s ",BitmapStructOb.field_48));
			LOG_PRINTF(("field copied till_102=%s ",BitmapStructOb.field_102));		//Agency Bank
			
		}
   
		if(ptrTransactionMsgStrucOb->TrMethordFlag == edebit)//copy pin for debit card only
		{
			for(i=0;i<BIANRY_PIN_LEN;i++)
			{
				BitmapStructOb.field_52[i] = ptrTransactionMsgStrucOb->PinData[i];
			}
			strncpy(BitmapStructOb.field_63, ptrTransactionMsgStrucOb->KSN, sizeof(ptrTransactionMsgStrucOb->KSN)); //63 KSN Value

			LOG_PRINTF(("Field 52=%s ", BitmapStructOb.field_52));
			LOG_PRINTF(("Field 63=%s ", BitmapStructOb.field_63));
 		}

		if(!strcmp((char *)ptrTransactionMsgStrucOb->MessageTypeInd,BATCH_UPLOAD_MSG_TYPE))//FOR BATCH UPLOAD 
		{
			strncpy((char *)BitmapStructOb.field_02,ptrTransactionMsgStrucOb->PrimaryAccNum,strlen(ptrTransactionMsgStrucOb->PrimaryAccNum));           // Primary Account Number 
			strncpy((char *)BitmapStructOb.field_12,ptrTransactionMsgStrucOb->TransLocalTime,TRANS_LOCALTIME_LEN);     
			strncpy((char *)BitmapStructOb.field_13,ptrTransactionMsgStrucOb->TransLocalDate,TRANS_LOCALDATE_LEN);     
			strncpy((char *)BitmapStructOb.field_14,ptrTransactionMsgStrucOb->ExpiryDate,EXPTIME_ARR_LEN);	        // Expiration date  
			strncpy((char *)BitmapStructOb.field_37,ptrTransactionMsgStrucOb->RetrievalReferenceNo,RETRIEVAL_REF_LEN);	        // Expiration date  
		}
			LOG_PRINTF(("Field 02=%s ",BitmapStructOb.field_02));
			LOG_PRINTF(("Field 12=%s ",BitmapStructOb.field_12));
			LOG_PRINTF(("Field 13=%s ",BitmapStructOb.field_13));
			LOG_PRINTF(("Field 14=%s ",BitmapStructOb.field_14));
			LOG_PRINTF(("Field 37=%s ",BitmapStructOb.field_37));
	
		if(!strcmp((char *)ptrTransactionMsgStrucOb->POSEntryMode,ICC_PIN_CAPABLE))//FOR BATCH UPLOAD 
		{
			LOG_PRINTF (("ICC+_EMV"));
			for(i=0;i<(strlen(emvdata.DE55)/2);i++) //
			//for(i=0;i<81;i++)// For sending EMV data hard coded
			{
				BitmapStructOb.field_55[i] = ptrTransactionMsgStrucOb->EMV_Tag_Data[i];
			}
			strncpy((char *)BitmapStructOb.field_55,ptrTransactionMsgStrucOb->EMV_Tag_Data,strlen(ptrTransactionMsgStrucOb->EMV_Tag_Data));           //  For sending EMV data in plain txt
		}
	    break;

	case BALINQMSGTYPE_CASE: 
		strncpy((char *)BitmapStructOb.message_id,ptrTransactionMsgStrucOb->MessageTypeInd,MSG_ID_LEN);     // message id Authorization request 
		//strcpy((char *)BitmapStructOb.field_02,ptrTransactionMsgStrucOb->PrimaryAccNum);           // Primary Account Number for manual entry
		strncpy((char *)BitmapStructOb.field_03,ptrTransactionMsgStrucOb->ProcessingCode,PROC_CODE_LEN);       // Processing Code 
		strncpy((char *)BitmapStructOb.field_11,ptrTransactionMsgStrucOb->TraceAuditNo,TRACE_AUDIT_LEN);           // System trace audit number 
		//strcpy((char *)BitmapStructOb.field_14,ptrTransactionMsgStrucOb->ExpiryDate);	        //14 Expiration date for manual entry only  
		strncpy((char *)BitmapStructOb.field_22,ptrTransactionMsgStrucOb->POSEntryMode,POS_ENTRY_LEN);		// POS Entry Mode
		strncpy((char *)BitmapStructOb.field_24,ptrTransactionMsgStrucOb->NetworkInternationalId,NII_LEN);        // NII
		strncpy((char *)BitmapStructOb.field_25,ptrTransactionMsgStrucOb->POSConditionCode,POS_COND_LEN);        // POS Condition Code
		strncpy((char *)BitmapStructOb.field_35,ptrTransactionMsgStrucOb->Track2Data,strlen(ptrTransactionMsgStrucOb->Track2Data));        // Track 2 data
		strncpy((char *)BitmapStructOb.field_41,ptrTransactionMsgStrucOb->TerminalID,TERMINAL_ID_LEN);        // Terminal ID
		strncpy((char *)BitmapStructOb.field_42,ptrTransactionMsgStrucOb->CardAcceptorID,CARD_ACCEPTOR_ID_LEN);        // Terminal ID
		strncpy((char *)BitmapStructOb.field_102,ptrTransactionMsgStrucOb->FromAcNo,ACCOUNT_NO_LEN);        //From Account Agency Bank
  		if(ptrTransactionMsgStrucOb->TrMethordFlag == edebit)//copy pin for debit card only
		{
			for(i=0;i<BIANRY_PIN_LEN;i++)                                                                        //52 PIN Data
			{
				BitmapStructOb.field_52[i] = ptrTransactionMsgStrucOb->PinData[i];
		   	}

			strncpy(BitmapStructOb.field_63, ptrTransactionMsgStrucOb->KSN, sizeof(ptrTransactionMsgStrucOb->KSN));
		}	
		if(!strcmp((char *)ptrTransactionMsgStrucOb->POSEntryMode,ICC_PIN_CAPABLE))//FOR BATCH UPLOAD 
		{
			for(i=0;i<(strlen(emvdata.DE55)/2);i++)
			{
				BitmapStructOb.field_55[i] = ptrTransactionMsgStrucOb->EMV_Tag_Data[i];
			}
		}
		break;
	
	case VOIDMSGTYPE_CASE:  
		strncpy((char *)BitmapStructOb.message_id,ptrTransactionMsgStrucOb->MessageTypeInd,MSG_ID_LEN);     // message id Authorization request 
		//strcpy((char *)BitmapStructOb.field_02,ptrTransactionMsgStrucOb->PrimaryAccNum);           // Primary Account Number 
		strncpy((char *)BitmapStructOb.field_03,ptrTransactionMsgStrucOb->ProcessingCode,PROC_CODE_LEN);       // Processing Code 
		paddAmount(ptrTransactionMsgStrucOb->Amount,ReqAmount);
		strncpy((char *)BitmapStructOb.field_04,ReqAmount,AMOUNT_LEN);
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF (("Requset Amount in Void = %s",ReqAmount));
		}
		strncpy((char *)BitmapStructOb.field_11,ptrTransactionMsgStrucOb->TraceAuditNo,TRACE_AUDIT_LEN);           // System trace audit number 
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF (("Void trace Audit no = %s",(char*)BitmapStructOb.field_11));
		}
		strncpy((char *)BitmapStructOb.field_12,ptrTransactionMsgStrucOb->TransLocalTime,TRANS_LOCALTIME_LEN);     
		strncpy((char *)BitmapStructOb.field_13,ptrTransactionMsgStrucOb->TransLocalDate,TRANS_LOCALDATE_LEN);     
		//strcpy((char *)BitmapStructOb.field_14,ptrTransactionMsgStrucOb->ExpiryDate);	        // Expiration date  
		strncpy((char *)BitmapStructOb.field_22,ptrTransactionMsgStrucOb->POSEntryMode,POS_ENTRY_LEN);		// POS Entry Mode
		strncpy((char *)BitmapStructOb.field_24,ptrTransactionMsgStrucOb->NetworkInternationalId,NII_LEN);        // NII
		strncpy((char *)BitmapStructOb.field_25,ptrTransactionMsgStrucOb->POSConditionCode,POS_COND_LEN);        // POS Condition Code
		strncpy((char *)BitmapStructOb.field_35,ptrTransactionMsgStrucOb->Track2Data,strlen(ptrTransactionMsgStrucOb->Track2Data));        // Track 2 data
		strncpy((char *)BitmapStructOb.field_37,ptrTransactionMsgStrucOb->RetrievalReferenceNo,RETRIEVAL_REF_LEN);        //37 Retrival reference no 
		strncpy((char *)BitmapStructOb.field_41,ptrTransactionMsgStrucOb->TerminalID,TERMINAL_ID_LEN);        //41 Terminal ID
		strncpy((char *)BitmapStructOb.field_42,ptrTransactionMsgStrucOb->CardAcceptorID,CARD_ACCEPTOR_ID_LEN);        //42 Terminal ID
		strncpy((char *)BitmapStructOb.AmountForVoid,ReqAmount,AMOUNT_LEN);        //USED TO COPY THE AMOUNT TO BIT 60 BECAUSE 
    
		if(ptrTransactionMsgStrucOb->TrMethordFlag == edebit)//copy pin for debit card only
		{
			for(i=0;i<BIANRY_PIN_LEN;i++)
			{
				BitmapStructOb.field_52[i] = ptrTransactionMsgStrucOb->PinData[i];
			}
			strncpy(BitmapStructOb.field_63, ptrTransactionMsgStrucOb->KSN, sizeof(ptrTransactionMsgStrucOb->KSN));
		}

    if(!strcmp((char *)ptrTransactionMsgStrucOb->MessageTypeInd,BATCH_UPLOAD_MSG_TYPE))
    {
      strncpy((char *)BitmapStructOb.field_02,ptrTransactionMsgStrucOb->PrimaryAccNum,strlen(ptrTransactionMsgStrucOb->PrimaryAccNum));           // Primary Account Number 
      strncpy((char *)BitmapStructOb.field_12,ptrTransactionMsgStrucOb->TransLocalTime,TRANS_LOCALTIME_LEN);     
      strncpy((char *)BitmapStructOb.field_13,ptrTransactionMsgStrucOb->TransLocalDate,TRANS_LOCALDATE_LEN);     
      strncpy((char *)BitmapStructOb.field_14,ptrTransactionMsgStrucOb->ExpiryDate,EXPTIME_ARR_LEN);	        // Expiration date  
      strncpy((char *)BitmapStructOb.field_37,ptrTransactionMsgStrucOb->RetrievalReferenceNo,RETRIEVAL_REF_LEN);	        // Expiration date  
      
    }

    if(!strcmp((char *)ptrTransactionMsgStrucOb->POSEntryMode,ICC_PIN_CAPABLE))//FOR BATCH UPLOAD 
    {
      for(i=0;i<(strlen(emvdata.DE55)/2);i++)
			{
        BitmapStructOb.field_55[i] = ptrTransactionMsgStrucOb->EMV_Tag_Data[i];
      }
    }
	break;
	case DEPOSIT_MSG_TYPE_CASE:
	case REFUNDMSGTYPE_CASE: 

    	strncpy((char *)BitmapStructOb.message_id,ptrTransactionMsgStrucOb->MessageTypeInd,MSG_ID_LEN);     // message id Authorization request 
		  //strcpy((char *)BitmapStructOb.field_02,ptrTransactionMsgStrucOb->PrimaryAccNum);           // Primary Account Number 
		  strncpy((char *)BitmapStructOb.field_03,ptrTransactionMsgStrucOb->ProcessingCode,PROC_CODE_LEN);       // Processing Code 
		  // strcpy((char *)BitmapStructOb.field_04,ptrTransactionMsgStrucOb->Amount);            //4 Transaction Amount 
			paddAmount(ptrTransactionMsgStrucOb->Amount,ReqAmount);
		  strncpy((char *)BitmapStructOb.field_04,ReqAmount,AMOUNT_LEN);
      if(LOG_STATUS == LOG_ENABLE)
      {
			  LOG_PRINTF (("Requset Amount in refund/deposit = %s",ReqAmount));
      }
		  strncpy((char *)BitmapStructOb.field_11,ptrTransactionMsgStrucOb->TraceAuditNo,TRACE_AUDIT_LEN);           // System trace audit number 
	    // strcpy((char *)BitmapStructOb.field_14,ptrTransactionMsgStrucOb->ExpiryDate);	        // Expiration date  
	    strncpy((char *)BitmapStructOb.field_22,ptrTransactionMsgStrucOb->POSEntryMode,POS_ENTRY_LEN);		// POS Entry Mode
      strncpy((char *)BitmapStructOb.field_24,ptrTransactionMsgStrucOb->NetworkInternationalId,NII_LEN);        // NII
		  strncpy((char *)BitmapStructOb.field_25,ptrTransactionMsgStrucOb->POSConditionCode,POS_COND_LEN);        // POS Condition Code
		  strncpy((char *)BitmapStructOb.field_35,ptrTransactionMsgStrucOb->Track2Data,strlen(ptrTransactionMsgStrucOb->Track2Data));        // Track 2 data
		  strncpy((char *)BitmapStructOb.field_41,ptrTransactionMsgStrucOb->TerminalID,TERMINAL_ID_LEN);        // Terminal ID
      strncpy((char *)BitmapStructOb.field_42,ptrTransactionMsgStrucOb->CardAcceptorID,CARD_ACCEPTOR_ID_LEN);        // Terminal ID
      strncpy((char *)BitmapStructOb.field_48,ptrTransactionMsgStrucOb->PaymentId,PAYMENT_ID_LEN);        // 
	  strncpy((char *)BitmapStructOb.field_103,ptrTransactionMsgStrucOb->ToAcNo,ACCOUNT_NO_LEN);        // Agencey Bank
	  if(LOG_STATUS == LOG_ENABLE)
    {
		LOG_PRINTF(("Field 03=%s ",BitmapStructOb.field_03));
		LOG_PRINTF(("Field 04=%s ",BitmapStructOb.field_04));
		LOG_PRINTF(("Field 11=%s ",BitmapStructOb.field_11));
		LOG_PRINTF(("Field 22=%s ",BitmapStructOb.field_22));
		LOG_PRINTF(("Field 24=%s ",BitmapStructOb.field_24));
		LOG_PRINTF(("Field 25=%s ",BitmapStructOb.field_25));
		LOG_PRINTF(("Field 35=%s ",BitmapStructOb.field_35));
		LOG_PRINTF(("Field 41=%s ",BitmapStructOb.field_41));
		LOG_PRINTF(("Field 42=%s ",BitmapStructOb.field_42));
		LOG_PRINTF(("Field 48=%s ",BitmapStructOb.field_48));
		LOG_PRINTF(("Field 103=%s ",BitmapStructOb.field_102));		//Agency Bank
		
		LOG_PRINTF(("field copied till_48=%s get_char()",BitmapStructOb.field_48));
	  LOG_PRINTF(("field copied till_103=%s ",BitmapStructOb.field_103));		//Agency Bank

    }

		  if(ptrTransactionMsgStrucOb->TrMethordFlag == edebit)//copy pin for debit card only
		  {
				for(i=0;i<BIANRY_PIN_LEN;i++)                                                                            //52 PIN Data
				{
					BitmapStructOb.field_52[i] = ptrTransactionMsgStrucOb->PinData[i];
				}
				strncpy(BitmapStructOb.field_63, ptrTransactionMsgStrucOb->KSN, sizeof(ptrTransactionMsgStrucOb->KSN));
		  }
		LOG_PRINTF(("Field 52=%s ",BitmapStructOb.field_52));
      if(!strcmp((char *)ptrTransactionMsgStrucOb->MessageTypeInd,BATCH_UPLOAD_MSG_TYPE))
      {
        strncpy((char *)BitmapStructOb.field_02,ptrTransactionMsgStrucOb->PrimaryAccNum,strlen(ptrTransactionMsgStrucOb->PrimaryAccNum));           // Primary Account Number 
        strncpy((char *)BitmapStructOb.field_12,ptrTransactionMsgStrucOb->TransLocalTime,TRANS_LOCALTIME_LEN);     
        strncpy((char *)BitmapStructOb.field_13,ptrTransactionMsgStrucOb->TransLocalDate,TRANS_LOCALDATE_LEN);     
        strncpy((char *)BitmapStructOb.field_14,ptrTransactionMsgStrucOb->ExpiryDate,EXPTIME_ARR_LEN);	        // Expiration date  
        strncpy((char *)BitmapStructOb.field_37,ptrTransactionMsgStrucOb->RetrievalReferenceNo,RETRIEVAL_REF_LEN);	        // Expiration date  
      
      }
      if(!strcmp((char *)ptrTransactionMsgStrucOb->POSEntryMode,ICC_PIN_CAPABLE))//FOR BATCH UPLOAD 
      {
         for(i=0;i<(strlen(emvdata.DE55)/2);i++)
			  {
          BitmapStructOb.field_55[i] = ptrTransactionMsgStrucOb->EMV_Tag_Data[i];
        }
      }
	break;
	case TRANSFER_MSG_TYPE_CASE:
			
    	strncpy((char *)BitmapStructOb.message_id,ptrTransactionMsgStrucOb->MessageTypeInd,MSG_ID_LEN);     // message id Authorization request 
		  //strcpy((char *)BitmapStructOb.field_02,ptrTransactionMsgStrucOb->PrimaryAccNum);           // Primary Account Number 
		  strncpy((char *)BitmapStructOb.field_03,ptrTransactionMsgStrucOb->ProcessingCode,PROC_CODE_LEN);       // Processing Code 
		  // strcpy((char *)BitmapStructOb.field_04,ptrTransactionMsgStrucOb->Amount);            //4 Transaction Amount 
			paddAmount(ptrTransactionMsgStrucOb->Amount,ReqAmount);
		  strncpy((char *)BitmapStructOb.field_04,ReqAmount,AMOUNT_LEN);
      if(LOG_STATUS == LOG_ENABLE)
      {
			  LOG_PRINTF (("Requset Amount in Transfer = %s",ReqAmount));
      }
		  strncpy((char *)BitmapStructOb.field_11,ptrTransactionMsgStrucOb->TraceAuditNo,TRACE_AUDIT_LEN);           // System trace audit number 
	    // strcpy((char *)BitmapStructOb.field_14,ptrTransactionMsgStrucOb->ExpiryDate);	        // Expiration date  
	    strncpy((char *)BitmapStructOb.field_22,ptrTransactionMsgStrucOb->POSEntryMode,POS_ENTRY_LEN);		// POS Entry Mode
      strncpy((char *)BitmapStructOb.field_24,ptrTransactionMsgStrucOb->NetworkInternationalId,NII_LEN);        // NII
		  strncpy((char *)BitmapStructOb.field_25,ptrTransactionMsgStrucOb->POSConditionCode,POS_COND_LEN);        // POS Condition Code
		  strncpy((char *)BitmapStructOb.field_35,ptrTransactionMsgStrucOb->Track2Data,strlen(ptrTransactionMsgStrucOb->Track2Data));        // Track 2 data
		  strncpy((char *)BitmapStructOb.field_41,ptrTransactionMsgStrucOb->TerminalID,TERMINAL_ID_LEN);        // Terminal ID
      strncpy((char *)BitmapStructOb.field_42,ptrTransactionMsgStrucOb->CardAcceptorID,CARD_ACCEPTOR_ID_LEN);        // Terminal ID
      strncpy((char *)BitmapStructOb.field_48,ptrTransactionMsgStrucOb->PaymentId,PAYMENT_ID_LEN);        // 
	  strncpy((char *)BitmapStructOb.field_102,ptrTransactionMsgStrucOb->FromAcNo,ACCOUNT_NO_LEN);        // Agencey Bank
	  strncpy((char *)BitmapStructOb.field_103,ptrTransactionMsgStrucOb->ToAcNo,ACCOUNT_NO_LEN);        // Agencey Bank
	  if(LOG_STATUS == LOG_ENABLE)
    {
		LOG_PRINTF(("Field 03=%s ",BitmapStructOb.field_03));
		LOG_PRINTF(("Field 04=%s ",BitmapStructOb.field_04));
		LOG_PRINTF(("Field 11=%s ",BitmapStructOb.field_11));
		LOG_PRINTF(("Field 22=%s ",BitmapStructOb.field_22));
		LOG_PRINTF(("Field 24=%s ",BitmapStructOb.field_24));
		LOG_PRINTF(("Field 25=%s ",BitmapStructOb.field_25));
		LOG_PRINTF(("Field 35=%s ",BitmapStructOb.field_35));
		LOG_PRINTF(("Field 41=%s ",BitmapStructOb.field_41));
		LOG_PRINTF(("Field 42=%s ",BitmapStructOb.field_42));
		LOG_PRINTF(("Field 48=%s ",BitmapStructOb.field_48));
		LOG_PRINTF(("Field 103=%s ",BitmapStructOb.field_102));		//Agency Bank
		
		LOG_PRINTF(("field copied till_48=%s get_char()",BitmapStructOb.field_48));
	  LOG_PRINTF(("field copied till_102=%s ",BitmapStructOb.field_102));		//Agency Bank
		LOG_PRINTF(("field copied till_103=%s ",BitmapStructOb.field_103));		//Agency Bank

    }

		  if(ptrTransactionMsgStrucOb->TrMethordFlag == edebit)//copy pin for debit card only
		  {
				for(i=0;i<BIANRY_PIN_LEN;i++)                                                                            //52 PIN Data
				{
					BitmapStructOb.field_52[i] = ptrTransactionMsgStrucOb->PinData[i];
				}
				strncpy(BitmapStructOb.field_63, ptrTransactionMsgStrucOb->KSN, sizeof(ptrTransactionMsgStrucOb->KSN));
		  }
		LOG_PRINTF(("Field 52=%s ",BitmapStructOb.field_52));
      if(!strcmp((char *)ptrTransactionMsgStrucOb->MessageTypeInd,BATCH_UPLOAD_MSG_TYPE))
      {
        strncpy((char *)BitmapStructOb.field_02,ptrTransactionMsgStrucOb->PrimaryAccNum,strlen(ptrTransactionMsgStrucOb->PrimaryAccNum));           // Primary Account Number 
        strncpy((char *)BitmapStructOb.field_12,ptrTransactionMsgStrucOb->TransLocalTime,TRANS_LOCALTIME_LEN);     
        strncpy((char *)BitmapStructOb.field_13,ptrTransactionMsgStrucOb->TransLocalDate,TRANS_LOCALDATE_LEN);     
        strncpy((char *)BitmapStructOb.field_14,ptrTransactionMsgStrucOb->ExpiryDate,EXPTIME_ARR_LEN);	        // Expiration date  
        strncpy((char *)BitmapStructOb.field_37,ptrTransactionMsgStrucOb->RetrievalReferenceNo,RETRIEVAL_REF_LEN);	        // Expiration date  
      
      }
      if(!strcmp((char *)ptrTransactionMsgStrucOb->POSEntryMode,ICC_PIN_CAPABLE))//FOR BATCH UPLOAD 
      {
         for(i=0;i<(strlen(emvdata.DE55)/2);i++)
			  {
          BitmapStructOb.field_55[i] = ptrTransactionMsgStrucOb->EMV_Tag_Data[i];
        }
      }
	  break;
	case PIN_VALIDATE_MSG_TYPE_CASE: 
		LOG_PRINTF(("case: PIN VALIDATE"));
		strncpy((char *)BitmapStructOb.message_id,ptrTransactionMsgStrucOb->MessageTypeInd,MSG_ID_LEN);     // message id Authorization request 
		//strcpy((char *)BitmapStructOb.field_02,ptrTransactionMsgStrucOb->PrimaryAccNum);           // Primary Account Number for manual entry
		strncpy((char *)BitmapStructOb.field_03,ptrTransactionMsgStrucOb->ProcessingCode,PROC_CODE_LEN);       // Processing Code 
		strncpy((char *)BitmapStructOb.field_11,ptrTransactionMsgStrucOb->TraceAuditNo,TRACE_AUDIT_LEN);           // System trace audit number 
		//strcpy((char *)BitmapStructOb.field_14,ptrTransactionMsgStrucOb->ExpiryDate);	        //14 Expiration date for manual entry only  
		strncpy((char *)BitmapStructOb.field_22,ptrTransactionMsgStrucOb->POSEntryMode,POS_ENTRY_LEN);		// POS Entry Mode
		strncpy((char *)BitmapStructOb.field_24,ptrTransactionMsgStrucOb->NetworkInternationalId,NII_LEN);        // NII
		strncpy((char *)BitmapStructOb.field_25,ptrTransactionMsgStrucOb->POSConditionCode,POS_COND_LEN);        // POS Condition Code
		strncpy((char *)BitmapStructOb.field_35,ptrTransactionMsgStrucOb->Track2Data,strlen(ptrTransactionMsgStrucOb->Track2Data));        // Track 2 data
		strncpy((char *)BitmapStructOb.field_41,ptrTransactionMsgStrucOb->TerminalID,TERMINAL_ID_LEN);        // Terminal ID
		strncpy((char *)BitmapStructOb.field_42,ptrTransactionMsgStrucOb->CardAcceptorID,CARD_ACCEPTOR_ID_LEN);        // Terminal ID
		//strncpy((char *)BitmapStructOb.field_102,ptrTransactionMsgStrucOb->FromAcNo,ACCOUNT_NO_LEN);        //From Account Agency Bank

		LOG_PRINTF(("Field 03=%s ", BitmapStructOb.field_03));
		LOG_PRINTF(("Field 11=%s ", BitmapStructOb.field_11));
		LOG_PRINTF(("Field 22=%s ", BitmapStructOb.field_22));
		LOG_PRINTF(("Field 24=%s ", BitmapStructOb.field_24));
		LOG_PRINTF(("Field 25=%s ", BitmapStructOb.field_25));
		LOG_PRINTF(("Field 35=%s ", BitmapStructOb.field_35));
		LOG_PRINTF(("Field 41=%s ", BitmapStructOb.field_41));
		LOG_PRINTF(("Field 42=%s ", BitmapStructOb.field_42));
	
		if(ptrTransactionMsgStrucOb->TrMethordFlag == edebit)//copy pin for debit card only
		{
			for(i=0;i<BIANRY_PIN_LEN;i++)                                                                        //52 PIN Data
			{
				BitmapStructOb.field_52[i] = ptrTransactionMsgStrucOb->PinData[i];
		   	}
			strncpy(BitmapStructOb.field_63, ptrTransactionMsgStrucOb->KSN, sizeof(ptrTransactionMsgStrucOb->KSN)); //63 KSN Value
		}	

		LOG_PRINTF(("Field 52=%s ", BitmapStructOb.field_52));
		LOG_PRINTF(("Field 63=%s ", BitmapStructOb.field_63));

		if(!strcmp((char *)ptrTransactionMsgStrucOb->POSEntryMode,ICC_PIN_CAPABLE))//FOR BATCH UPLOAD 
		{
			for(i=0;i<(strlen(emvdata.DE55)/2);i++)
			{
				BitmapStructOb.field_55[i] = ptrTransactionMsgStrucOb->EMV_Tag_Data[i];
			}
		}
		break;

	case PINCHANGEMSGTYPE_CASE: 
		strncpy((char *)BitmapStructOb.message_id, ptrTransactionMsgStrucOb->MessageTypeInd, MSG_ID_LEN);     // message id Authorization request 
		strncpy((char *)BitmapStructOb.field_03, ptrTransactionMsgStrucOb->ProcessingCode, PROC_CODE_LEN);       // Processing Code 
		strncpy((char *)BitmapStructOb.field_11, ptrTransactionMsgStrucOb->TraceAuditNo, TRACE_AUDIT_LEN);           // System trace audit number 
		strncpy((char *)BitmapStructOb.field_22, ptrTransactionMsgStrucOb->POSEntryMode, POS_ENTRY_LEN);		// POS Entry Mode
		strncpy((char *)BitmapStructOb.field_24, ptrTransactionMsgStrucOb->NetworkInternationalId, NII_LEN);        // NII
		strncpy((char *)BitmapStructOb.field_25, ptrTransactionMsgStrucOb->POSConditionCode, POS_COND_LEN);        // POS Condition Code
		strncpy((char *)BitmapStructOb.field_35, ptrTransactionMsgStrucOb->Track2Data, strlen(ptrTransactionMsgStrucOb->Track2Data));        // Track 2 data
		strncpy((char *)BitmapStructOb.field_41, ptrTransactionMsgStrucOb->TerminalID, TERMINAL_ID_LEN);        // Terminal ID
		strncpy((char *)BitmapStructOb.field_42, ptrTransactionMsgStrucOb->CardAcceptorID, CARD_ACCEPTOR_ID_LEN);        // Terminal ID

		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Field 03=%s ", BitmapStructOb.field_03));
			LOG_PRINTF(("Field 11=%s ", BitmapStructOb.field_11));
			LOG_PRINTF(("Field 22=%s ", BitmapStructOb.field_22));
			LOG_PRINTF(("Field 24=%s ", BitmapStructOb.field_24));
			LOG_PRINTF(("Field 25=%s ", BitmapStructOb.field_25));
			LOG_PRINTF(("Field 35=%s ", BitmapStructOb.field_35));
			LOG_PRINTF(("Field 41=%s ", BitmapStructOb.field_41));
			LOG_PRINTF(("Field 42=%s ", BitmapStructOb.field_42));
		}

		for (i = 0;i<BIANRY_PIN_LEN;i++)                                                                        //52 PIN Data
		{
			BitmapStructOb.field_52[i] = ptrTransactionMsgStrucOb->PinData[i];
		}
		LOG_PRINTF(("Field 52=%s ", BitmapStructOb.field_52));

		for (i = 0;i<12;i++)                                                                        //63 PIN Data
		{
			BitmapStructOb.field_63[i] = temp_pin[i];
		}
		LOG_PRINTF(("Field 63=%s ", BitmapStructOb.field_63));

		

		if (!strcmp((char *)ptrTransactionMsgStrucOb->POSEntryMode, ICC_PIN_CAPABLE))//FOR BATCH UPLOAD 
		{
			LOG_PRINTF(("ICC+_EMV"))
			for (i = 0;i<(strlen(emvdata.DE55) / 2);i++) //for(i=0;i<81;i++)// For sending EMV data hard coded
			{
				BitmapStructOb.field_55[i] = ptrTransactionMsgStrucOb->EMV_Tag_Data[i];
			}
			strncpy((char *)BitmapStructOb.field_55, ptrTransactionMsgStrucOb->EMV_Tag_Data, strlen(ptrTransactionMsgStrucOb->EMV_Tag_Data));           //  For sending EMV data in plain txt
			
		}
		break;
	case ACTIVATIONMSGTYPE_CASE: 
		strncpy((char *)BitmapStructOb.message_id, ptrTransactionMsgStrucOb->MessageTypeInd, MSG_ID_LEN);     // message id Authorization request 
		//strcpy((char *)BitmapStructOb.field_02,ptrTransactionMsgStrucOb->PrimaryAccNum);           // Primary Account Number 
		strncpy((char *)BitmapStructOb.field_03, ptrTransactionMsgStrucOb->ProcessingCode, PROC_CODE_LEN);       // Processing Code 
		strncpy((char *)BitmapStructOb.field_11, ptrTransactionMsgStrucOb->TraceAuditNo, TRACE_AUDIT_LEN);           // System trace audit number 
		strncpy((char *)BitmapStructOb.field_22, ptrTransactionMsgStrucOb->POSEntryMode, POS_ENTRY_LEN);		// POS Entry Mode
		strncpy((char *)BitmapStructOb.field_24, ptrTransactionMsgStrucOb->NetworkInternationalId, NII_LEN);        // NII
		strncpy((char *)BitmapStructOb.field_25, ptrTransactionMsgStrucOb->POSConditionCode, POS_COND_LEN);        // POS Condition Code
		strncpy((char *)BitmapStructOb.field_35, ptrTransactionMsgStrucOb->Track2Data, strlen(ptrTransactionMsgStrucOb->Track2Data));        // Track 2 data
		strncpy((char *)BitmapStructOb.field_41, ptrTransactionMsgStrucOb->TerminalID, TERMINAL_ID_LEN);        // Terminal ID
		strncpy((char *)BitmapStructOb.field_42, ptrTransactionMsgStrucOb->CardAcceptorID, CARD_ACCEPTOR_ID_LEN);        // Terminal ID
		//strncpy((char *)BitmapStructOb.field_48, ptrTransactionMsgStrucOb->PaymentId, PAYMENT_ID_LEN);        // Payment ID
																											  //strncpy((char *)BitmapStructOb.field_102,ptrTransactionMsgStrucOb->FromAcNo,ACCOUNT_NO_LEN);        //From Account Agency Bank

		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Field 03=%s ", BitmapStructOb.field_03));
			LOG_PRINTF(("Field 11=%s ", BitmapStructOb.field_11));
			LOG_PRINTF(("Field 22=%s ", BitmapStructOb.field_22));
			LOG_PRINTF(("Field 24=%s ", BitmapStructOb.field_24));
			LOG_PRINTF(("Field 25=%s ", BitmapStructOb.field_25));
			LOG_PRINTF(("Field 35=%s ", BitmapStructOb.field_35));
			LOG_PRINTF(("Field 41=%s ", BitmapStructOb.field_41));
			LOG_PRINTF(("Field 42=%s ", BitmapStructOb.field_42));
		}

		for (i = 0;i<BIANRY_PIN_LEN;i++)                                                                        //52 PIN Data
		{
			BitmapStructOb.field_52[i] = ptrTransactionMsgStrucOb->PinData[i];
		}

		LOG_PRINTF(("pinblock=%s ", ptrTransactionMsgStrucOb->PinData));
		LOG_PRINTF(("Field 52=%s ", BitmapStructOb.field_52));
		
		if (sizeof(ptrTransactionMsgStrucOb->KSN) > 0)
		{
			strncpy(BitmapStructOb.field_63, ptrTransactionMsgStrucOb->KSN, sizeof(ptrTransactionMsgStrucOb->KSN)); //63 KSN Value
			LOG_PRINTF(("Field 63=%s ", BitmapStructOb.field_63));
		}		
		
		if (!strcmp((char *)ptrTransactionMsgStrucOb->POSEntryMode, ICC_PIN_CAPABLE))//FOR BATCH UPLOAD 
		{
			LOG_PRINTF(("ICC+_EMV"));
			for (i = 0;i<(strlen(emvdata.DE55) / 2);i++) //
														 //for(i=0;i<81;i++)// For sending EMV data hard coded
			{
				BitmapStructOb.field_55[i] = ptrTransactionMsgStrucOb->EMV_Tag_Data[i];
			}
			strncpy((char *)BitmapStructOb.field_55, ptrTransactionMsgStrucOb->EMV_Tag_Data, strlen(ptrTransactionMsgStrucOb->EMV_Tag_Data));           //  For sending EMV data in plain txt
		}
		break;
	case MOBILETOPUPMSGTYPE_CASE:  
	break;
	case SETTLEMENT_MSG_TYPE_CASE:  
				strncpy((char *)BitmapStructOb.message_id,ptrTransactionMsgStrucOb->MessageTypeInd,MSG_ID_LEN);     // message id Authorization request 
		    strncpy((char *)BitmapStructOb.field_03,ptrTransactionMsgStrucOb->ProcessingCode,PROC_CODE_LEN);       // Processing Code 
		    strncpy((char *)BitmapStructOb.field_11,ptrTransactionMsgStrucOb->TraceAuditNo,TRACE_AUDIT_LEN);           // System trace audit number 
	      strncpy((char *)BitmapStructOb.field_24,ptrTransactionMsgStrucOb->NetworkInternationalId,NII_LEN);        // NII
		    strncpy((char *)BitmapStructOb.field_41,ptrTransactionMsgStrucOb->TerminalID,TERMINAL_ID_LEN);        // Terminal ID
        strncpy((char *)BitmapStructOb.field_42,ptrTransactionMsgStrucOb->CardAcceptorID,CARD_ACCEPTOR_ID_LEN);        // Terminal ID
      	strncpy((char *)BitmapStructOb.field_60,ptrTransactionMsgStrucOb->BatchNo,BATCH_ID_LEN); 
    		strncpy((char *)BitmapStructOb.field_63,ptrTransactionMsgStrucOb->rqstConsilation,RECONCILLATION_REQ_LEN); 
	break;
	case LOGON_MSG_TYPE_CASE: 
        strncpy((char *)BitmapStructOb.message_id,ptrTransactionMsgStrucOb->MessageTypeInd,MSG_ID_LEN);     // message id Authorization request 
		    strncpy((char *)BitmapStructOb.field_03,ptrTransactionMsgStrucOb->ProcessingCode,PROC_CODE_LEN);       // Processing Code 
		    strncpy((char *)BitmapStructOb.field_11,ptrTransactionMsgStrucOb->TraceAuditNo,TRACE_AUDIT_LEN);           // System trace audit number 
	      strncpy((char *)BitmapStructOb.field_24,ptrTransactionMsgStrucOb->NetworkInternationalId,NII_LEN);        // NII
		    strncpy((char *)BitmapStructOb.field_41,ptrTransactionMsgStrucOb->TerminalID,TERMINAL_ID_LEN);        // Terminal ID
           
	break;
  default:
  break ;
	
	}
}

/********************************************************************************************************************
*	Function Name : SetingBitMap																													                            *
																																						                *
*	Purpose		    : Setting particular bit in bitmap 			 												                                    *
*	Input		      : a pointer to the bitmap structture object					        		              										  *
*	Output		    : Return value for the success or failure																			              			  *
																																					              *
*********************************************************************************************************************/
void SetingBitMap(TransactionMsgStruc *ptrTransactionMsgStrucOb)
{
	//char RevStatus[2] = {0}; // "0" or "1" 
	if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF (("SetingBitMap =%d",ptrTransactionMsgStrucOb->TrTypeFlag));
		  LOG_PRINTF (("======OTHER_AC %d =================",OTHER_AC));
    }
	map_clear((unsigned char *) BitmapStructOb.t_map,BITMAP_SIZE);                        // to 1st bitmap

	switch(ptrTransactionMsgStrucOb->TrTypeFlag)
	{
		
		case SALEMSGTYPE_CASE:  // 0120/0121 Authorization advice (repeat)
		case WITHDRAWAL_MSG_TYPE_CASE:
				if(!strcmp((char *)ptrTransactionMsgStrucOb->MessageTypeInd,BATCH_UPLOAD_MSG_TYPE))
				{
					if(LOG_STATUS == LOG_ENABLE)
					{
						LOG_PRINTF (("SetingBitMap in batch upload"));
					}
					map_man((char *) BitmapStructOb.t_map,2,3,4,11,12,13,14,22,24,25,37,41,42+ STOP);//for payment  
				}
				else if(!strcmp((char *)ptrTransactionMsgStrucOb->POSEntryMode,ICC_PIN_CAPABLE))//for EMV event
				{
					LOG_PRINTF (("SetingBitMap in ICC_PIN_CAPABLE"));
					if(ptrTransactionMsgStrucOb->TrMethordFlag == edebit)
					{
						LOG_PRINTF (("SetingBitMap in edebit ICC_PIN_CAPABLE"));
						if(OTHER_AC==1){
							if (pin_enc)
								map_man((char *)BitmapStructOb.t_map, 1, 3, 4, 11, 22, 24, 25, 35, 41, 42, 48, 52, 55, 102 + STOP);//for payment  
							else
								map_man((char *) BitmapStructOb.t_map,1,3,4,11,22,24,25,35,41,42,48,52,55,63,102+ STOP);//for payment  
						}
						else{
							if (pin_enc)
								map_man((char *) BitmapStructOb.t_map,3,4,11,22,24,25,35,41,42,48,52,55+ STOP);//for payment  
							else
								map_man((char *) BitmapStructOb.t_map,3,4,11,22,24,25,35,41,42,48,52,55,63+ STOP);//for payment  
						}
					}
					else if(ptrTransactionMsgStrucOb->TrMethordFlag == ecredit)//for credit card
					{
						LOG_PRINTF (("SetingBitMap in ecredit ICC_PIN_CAPABLE"));
						map_man((char *) BitmapStructOb.t_map,3,4,11,22,24,25,35,41,42,48,55+STOP);//for payment  
					}
				}
				else
				{
					if(ptrTransactionMsgStrucOb->TrMethordFlag == edebit)
					{
						LOG_PRINTF (("======================Check================="));
						if(OTHER_AC==1){
							LOG_PRINTF (("======================Payment/WD Other AC -Bitmap ================="));
							if (pin_enc)
								map_man((char *)BitmapStructOb.t_map, 1, 3, 4, 11, 22, 24, 25, 35, 41, 42, 52, 102 + STOP);//for payment  
							else
								map_man((char *) BitmapStructOb.t_map,1,3,4,11,22,24,25,35,41,42,52,63,102+ STOP);//for payment  
						}
						else
						{
							if (pin_enc)
								map_man((char *)BitmapStructOb.t_map, 3, 4, 11, 22, 24, 25, 35, 41, 42, 52 + STOP);
							else
								map_man((char *)BitmapStructOb.t_map, 3, 4, 11, 22, 24, 25, 35, 41, 42, 52, 63 + STOP);//for payment  
							  //map_man((char *) BitmapStructOb.t_map,3,4,11,22,24,25,35,41,42,48,52,102+ STOP);//for payment  Agency Bank
						}
					}
					else if(ptrTransactionMsgStrucOb->TrMethordFlag == ecredit)//for credit card
						map_man((char *) BitmapStructOb.t_map,3,4,11,22,24,25,35,41,42,48+STOP);//for payment  
				}
				
				break;
		case BALINQMSGTYPE_CASE:  
			    if(!strcmp((char *)ptrTransactionMsgStrucOb->POSEntryMode,ICC_PIN_CAPABLE))//for EMV event
				{
					if(ptrTransactionMsgStrucOb->TrMethordFlag == edebit)//for debit card
					{
						if(OTHER_AC==1){
							if (pin_enc)
								map_man((char *) BitmapStructOb.t_map,1,3,11,22,24,25,35,41,42,52,55,102+ STOP);   // to set fiels in 1st bitmap
							else
								map_man((char *) BitmapStructOb.t_map,1,3,11,22,24,25,35,41,42,52,55,63,102+ STOP);   // to set fiels in 1st bitmap
						}
						else{
							if (pin_enc)
								map_man((char *) BitmapStructOb.t_map,3,11,22,24,25,35,41,42,52,55+ STOP);   // to set fiels in 1st bitmap
							else
								map_man((char *) BitmapStructOb.t_map,3,11,22,24,25,35,41,42,52,55,63+ STOP);   // to set fiels in 1st bitmap
						}

					}
					else if(ptrTransactionMsgStrucOb->TrMethordFlag == ecredit)//for credit card
						map_man((char *) BitmapStructOb.t_map,3,11,22,24,25,35,41,42,55,63+STOP);   // to set fiedls in 1st bitmap
				}
				else
				{
					if(ptrTransactionMsgStrucOb->TrMethordFlag == edebit)//for debit card
					{
						if(OTHER_AC==1){
							if (pin_enc)
								map_man((char *) BitmapStructOb.t_map,1,3,11,22,24,25,35,41,42,52,102+ STOP);//for payment  
							else
								map_man((char *) BitmapStructOb.t_map,1,3,11,22,24,25,35,41,42,52,63,102+ STOP);//for payment  
						}
						else
						{
							if (pin_enc)
								map_man((char *)BitmapStructOb.t_map, 3, 11, 22, 24, 25, 35, 41, 42, 52 + STOP);   // to set fiels in 1st bitmap
							else
								map_man((char *)BitmapStructOb.t_map, 3, 11, 22, 24, 25, 35, 41, 42, 52, 63 + STOP);   // to set fiels in 1st bitmap
						}
					}
					else if(ptrTransactionMsgStrucOb->TrMethordFlag == ecredit)//for credit card
					  map_man((char *) BitmapStructOb.t_map,3,11,22,24,25,35,41,42+STOP);   // to set fiels in 1st bitmap
				}
				break;
		case VOIDMSGTYPE_CASE:  
				if(!strcmp((char *)ptrTransactionMsgStrucOb->MessageTypeInd,BATCH_UPLOAD_MSG_TYPE))
				{
					if(LOG_STATUS == LOG_ENABLE)
					{
						LOG_PRINTF (("SetingBitMap in batch upload"));
					}
					map_man((char *) BitmapStructOb.t_map,2,3,4,11,12,13,14,22,24,25,37,41,42+ STOP);//for payment  
				}
			/*	else if(!strcmp((char *)ptrTransactionMsgStrucOb->POSEntryMode,ICC_PIN_CAPABLE))//for EMV event
				{
					if(ptrTransactionMsgStrucOb->TrMethordFlag == edebit)//for debit card
						map_man((char *) BitmapStructOb.t_map,3,4,11,12,13,22,24,25,35,37,41,42,52,55,60+ STOP);   // to set fiels in 1st bitmap
					else if(ptrTransactionMsgStrucOb->TrMethordFlag == ecredit)//for credit card
						map_man((char *) BitmapStructOb.t_map,3,4,11,12,13,22,24,25,35,37,41,42,55,60+ STOP);   // to set fiels in 1st bitmap
				}*/
				else
				{
					if(ptrTransactionMsgStrucOb->TrMethordFlag == edebit)//for debit card
						map_man((char *) BitmapStructOb.t_map,3,4,11,12,13,22,24,25,35,37,41,42,60+ STOP);   // to set fiels in 1st bitmap
					else if(ptrTransactionMsgStrucOb->TrMethordFlag == ecredit)//for credit card
						map_man((char *) BitmapStructOb.t_map,3,4,11,12,13,22,24,25,35,37,41,42,60+ STOP);   // to set fiels in 1st bitmap
				}
				break;
		case DEPOSIT_MSG_TYPE_CASE:
	    case REFUNDMSGTYPE_CASE:  
				LOG_PRINTF (("======================Refund/Deposit -Bitmap ================="));
				if(!strcmp((char *)ptrTransactionMsgStrucOb->MessageTypeInd,BATCH_UPLOAD_MSG_TYPE))
				{
					if(LOG_STATUS == LOG_ENABLE)
					{
						LOG_PRINTF (("SetingBitMap in batch upload"));
					}
					map_man((char *) BitmapStructOb.t_map,2,3,4,11,12,13,14,22,24,25,37,41,42+ STOP);//for payment  
				}
				else if(!strcmp((char *)ptrTransactionMsgStrucOb->POSEntryMode,ICC_PIN_CAPABLE))//for EMV event
				{
					if(ptrTransactionMsgStrucOb->TrMethordFlag == edebit)//for debit card
					{
						if(OTHER_AC==1){
							LOG_PRINTF (("======================EMV Refund/Deposit Other AC -Bitmap ================="));
							if (pin_enc)
								map_man((char *) BitmapStructOb.t_map,1,3,4,11,22,24,25,35,41,42,48,52,55,103+ STOP);//for payment  
							else
								map_man((char *) BitmapStructOb.t_map,1,3,4,11,22,24,25,35,41,42,48,52,55,63,103+ STOP);//for payment  
						}
						else
						{
							if (pin_enc)
								map_man((char *)BitmapStructOb.t_map, 3, 4, 11, 22, 24, 25, 35, 41, 42, 52, 55 + STOP);   // to set fiels in 1st bitmap
							else
								map_man((char *)BitmapStructOb.t_map, 3, 4, 11, 22, 24, 25, 35, 41, 42, 52, 55, 63 + STOP);   // to set fiels in 1st bitmap
						}
					}
					else if(ptrTransactionMsgStrucOb->TrMethordFlag == ecredit)//for credit card
						map_man((char *) BitmapStructOb.t_map,3,4,11,22,24,25,35,41,42,55,63+ STOP);   // to set fiels in 1st bitmap
				}
				else
				{
					if(ptrTransactionMsgStrucOb->TrMethordFlag == edebit)//for debit card
					{
						if(OTHER_AC==1){
							LOG_PRINTF (("======================MAG Refund/Deposit Other AC -Bitmap ================="));
							if (pin_enc)
								map_man((char *) BitmapStructOb.t_map,1,3,4,11,22,24,25,35,41,42,48,52,103+ STOP);//for payment  
							else
								map_man((char *) BitmapStructOb.t_map,1,3,4,11,22,24,25,35,41,42,48,52,63,103+ STOP);//for payment  
						}
						else
						{
							if (pin_enc)
								map_man((char *)BitmapStructOb.t_map, 3, 4, 11, 22, 24, 25, 35, 41, 42, 52+ STOP);   // to set fiels in 1st bitmap
							else
								map_man((char *)BitmapStructOb.t_map, 3, 4, 11, 22, 24, 25, 35, 41, 42, 52, 63 + STOP);   // to set fiels in 1st bitmap
						}
					}
					else if(ptrTransactionMsgStrucOb->TrMethordFlag == ecredit)//for credit card
						map_man((char *) BitmapStructOb.t_map,3,4,11,22,24,25,35,41,42+ STOP);   // to set fiels in 1st bitmap
				}
				break;
		case TRANSFER_MSG_TYPE_CASE:
			LOG_PRINTF (("======================Transfer -Bitmap ================="));
				if(!strcmp((char *)ptrTransactionMsgStrucOb->MessageTypeInd,BATCH_UPLOAD_MSG_TYPE))
				{
					if(LOG_STATUS == LOG_ENABLE)
					{
						LOG_PRINTF (("SetingBitMap in batch upload"));
					}
					map_man((char *) BitmapStructOb.t_map,2,3,4,11,12,13,14,22,24,25,37,41,42+ STOP);//for payment  
				}
				else if(!strcmp((char *)ptrTransactionMsgStrucOb->POSEntryMode,ICC_PIN_CAPABLE))//for EMV event
				{
					if(ptrTransactionMsgStrucOb->TrMethordFlag == edebit)//for debit card
					{
						/*					
						if(FROM_OTHER_AC==1 && OTHER_AC==0){
							LOG_PRINTF (("======================Transfer From Other AC -Bitmap ================="));
							map_man((char *) BitmapStructOb.t_map,1,3,4,11,22,24,25,35,41,42,48,52,102+ STOP);//for payment  
						}
						else if(FROM_OTHER_AC==1 && OTHER_AC==1 ){
								LOG_PRINTF (("======================Transfer to Other AC -Bitmap ================="));
								map_man((char *) BitmapStructOb.t_map,1,3,4,11,22,24,25,35,41,42,48,52,102,103+ STOP);//for payment  
							}
						else if(FROM_OTHER_AC==0 && OTHER_AC==1 ){
								LOG_PRINTF (("======================Transfer to Other AC -Bitmap ================="));
								map_man((char *) BitmapStructOb.t_map,1,3,4,11,22,24,25,35,41,42,48,52,103+ STOP);//for payment  
							}
						
						else	*/
							map_man((char *) BitmapStructOb.t_map,1,3,4,11,22,24,25,35,41,42,52,55,63,102,103+ STOP);   // to set fiels in 1st bitmap
					}
					else if(ptrTransactionMsgStrucOb->TrMethordFlag == ecredit)//for credit card
						map_man((char *) BitmapStructOb.t_map,3,4,11,22,24,25,35,41,42,55,63+ STOP);   // to set fiels in 1st bitmap

				}
				else
				{
					if(ptrTransactionMsgStrucOb->TrMethordFlag == edebit)//for debit card
					{
						/*					
						if(FROM_OTHER_AC==1 && OTHER_AC==0){
							LOG_PRINTF (("======================Transfer From Other AC -Bitmap ================="));
							map_man((char *) BitmapStructOb.t_map,1,3,4,11,22,24,25,35,41,42,48,52,102+ STOP);//for payment  
						}
						else if(FROM_OTHER_AC==1 && OTHER_AC==1 ){
								LOG_PRINTF (("======================Transfer to Other AC -Bitmap ================="));
								map_man((char *) BitmapStructOb.t_map,1,3,4,11,22,24,25,35,41,42,48,52,102,103+ STOP);//for payment  
							}
						else if(FROM_OTHER_AC==0 && OTHER_AC==1 ){
								LOG_PRINTF (("======================Transfer to Other AC -Bitmap ================="));
								map_man((char *) BitmapStructOb.t_map,1,3,4,11,22,24,25,35,41,42,48,52,103+ STOP);//for payment  
							}
						
						else	*/
							map_man((char *) BitmapStructOb.t_map,1,3,4,11,22,24,25,35,41,42,52,63,102,103+ STOP);   // to set fiels in 1st bitmap
					}
					else if(ptrTransactionMsgStrucOb->TrMethordFlag == ecredit)//for credit card
						map_man((char *) BitmapStructOb.t_map,3,4,11,22,24,25,35,41,42+ STOP);   // to set fiels in 1st bitmap
				}
				break;
			case PIN_VALIDATE_MSG_TYPE_CASE:  
			    if(!strcmp((char *)ptrTransactionMsgStrucOb->POSEntryMode,ICC_PIN_CAPABLE))//for EMV event
				{
					if (pin_enc)
						map_man((char *) BitmapStructOb.t_map,3,11,22,24,25,35,41,42,52,55+ STOP);   // to set fiels in 1st bitmap
					else
						map_man((char *) BitmapStructOb.t_map,3,11,22,24,25,35,41,42,52,55,63+ STOP);   // to set fiels in 1st bitmap
				}
				else
				{
					if (pin_enc)
						map_man((char *)BitmapStructOb.t_map, 3, 11, 22, 24, 25, 35, 41, 42, 52+ STOP);   // to set fiels in 1st bitmap
					else
						map_man((char *)BitmapStructOb.t_map, 3, 11, 22, 24, 25, 35, 41, 42, 52, 63 + STOP);   // to set fiels in 1st bitmap
				}
				break;
	    case PINCHANGEMSGTYPE_CASE:  
				if (!strcmp((char *)ptrTransactionMsgStrucOb->POSEntryMode, ICC_PIN_CAPABLE))//for EMV event
				{
					map_man((char *)BitmapStructOb.t_map, 3, 11, 22, 24, 25, 35, 41, 42, 52, 55, 63 + STOP);
				}
				else
				{
					map_man((char *)BitmapStructOb.t_map, 3, 11, 22, 24, 25, 35, 41, 42, 52, 63 + STOP);
					//map_man((char *)BitmapStructOb.t_map, 3, 11, 22, 24, 25, 35, 41, 42, 52 + STOP);
				}
				break;
	    case ACTIVATIONMSGTYPE_CASE: 
				if (!strcmp((char *)ptrTransactionMsgStrucOb->POSEntryMode, ICC_PIN_CAPABLE))//for EMV event
				{
					map_man((char *)BitmapStructOb.t_map, 3, 11, 22, 24, 25, 35, 41, 42, 52, 55 + STOP);
				}
				else
				{
					map_man((char *)BitmapStructOb.t_map, 3, 11, 22, 24, 25, 35, 41, 42, 52 + STOP);
				}
				break;
	    case MOBILETOPUPMSGTYPE_CASE:  
				break;
	    case SETTLEMENT_MSG_TYPE_CASE: 
				map_man((char *) BitmapStructOb.t_map,3,11,24,41,42,60,63+ STOP);   // to set fiels in 1st bitmap
				break;
		case LOGON_MSG_TYPE_CASE:
				map_man((char *) BitmapStructOb.t_map,3,11,24,41+ STOP);   // to set fiels in 1st bitmap
				break;
		default:
				break ;
		
	}
	
}

/********************************************************************************************************************
*	Function Name : bcd2a																													                          *
																																						                *
*	Purpose		    : to change bcd in string 			 												                                                *
*	Input		      : a pointer to the dest buffer and a pointer to the 
                  src buffer and length ofm the src buffer			              									s*
*	Output		    : Return value for the success or failure																			              			  *
																																					              *
*********************************************************************************************************************/
void bcd2a(char *dest ,char *src ,short bcdlen)
{
	int i;
	for (i=0; i<bcdlen; i++)
	{	
		sprintf(dest+(i*2), "%02x ", (unsigned char)(src[i]));
	}
	dest[bcdlen*2] = 0x00;
}
/********************************************************************************************************************
*	Function Name : assemble_packet																													                          *
																																						                                        *
*	Purpose		    : to assemble the packet into iso 8583 format                                                       *
*	Input		      : a pointer to the field struct  and a pointer to the request buffer                                *
*	Output		    : Return value no of packets assembled or failure																			              			  *
																																					                                          *
*********************************************************************************************************************/
int assemble_packet(field_struct *iso8583_field_table,unsigned char *reqBuff) 
{ 
	char tempBuffer_[800]; //
	char tempBuffer2[800];
	int size = 0;
	int retVal = -1,i=0,j=0;
	char var[2] = {0};
	char var2[5] = {0};//AFTER EMV INTEGRATION
	char Finalvar2[5] = {0};//AFTER EMV INTEGRATION
	unsigned char *ptr = reqBuff ;//for putting whole message length
	unsigned char *ptr1 = NULL ;//for emv data length in 2 bytes
	unsigned char *ptr2 = NULL;
	int reqbuf = sizeof(reqBuff);
	char temp_hold[16];

	memset(BitmapStructOb.buffer, 0, sizeof(BitmapStructOb.buffer));
	if(LOG_STATUS == LOG_ENABLE)
	{  
		LOG_PRINTF(("Going to call process_8583"));
	}	
	retVal = BitmapStructOb.packet_sz = process_8583 (0, iso8583_field_table, BitmapStructOb.t_map, BitmapStructOb.buffer,sizeof( BitmapStructOb.buffer));
	
	if(LOG_STATUS == LOG_ENABLE)
	{ 
		LOG_PRINTF (("assemble packet size %d ", BitmapStructOb.packet_sz));
	}
	if(retVal > 0)
	{
   
		memset(tempBuffer_,0,sizeof(tempBuffer_));//memsetting the temp buffer
		bcd2a((char *)tempBuffer_, (char *)BitmapStructOb.buffer, BitmapStructOb.packet_sz);
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF((" after process_8583:[%s]",tempBuffer_));      
		}
    
		memset(tempBuffer_,0,800);
		if(!strcmp((char *)BitmapStructOb.message_id,"0500"))//if settlemenmt message type
		{
		  if(LOG_STATUS == LOG_ENABLE)
		  {
			LOG_PRINTF(("process_8583: Settlement"));
		  }
		  for(i = 2,j=0;i<48;j++,i++)
			{
				 reqBuff[i] = BitmapStructOb.buffer[j];
			}
		  reqBuff[48] = 0x00 ;
		  reqBuff[49] = 0x06 ;
		  for(i = 50;i<56;j++,i++)
			{
				 reqBuff[i] = BitmapStructOb.buffer[j];
			}
		  reqBuff[56] = 0x00 ;
		  reqBuff[57] = 0x60 ;
		  for(i = 58;i<retVal+6;j++,i++)
			{
				 reqBuff[i] = BitmapStructOb.buffer[j];
			}
		  retVal = retVal+4;//2 byte length of trace audit no + 2 bytes length of reconcillation request   
		  sprintf(var,"%x",retVal);
		  reqBuff[0]=0x00;
		  ptr++;
		  packData(var,(char*)ptr);
		  retVal = retVal + 2;//2 byte length of whole message
		  bcd2a((char *)tempBuffer_, (char *)reqBuff, retVal);
		  if(LOG_STATUS == LOG_ENABLE)
		  {
			  LOG_PRINTF(("process_8583 after adding length:[%s]",tempBuffer_));
		  }  
		}
	else //except settlement
	{
      if(LOG_STATUS == LOG_ENABLE)
      {
        LOG_PRINTF(("process_8583: except Settle"));
      }
      for(i = 2,j=0;i<BitmapStructOb.packet_sz+2;j++,i++)
      {
        reqBuff[i] = BitmapStructOb.buffer[j];
      }
	  
      LOG_PRINTF(("BitmapStructOb.field_22 =%s  message_id =%s",BitmapStructOb.field_22,BitmapStructOb.message_id));
      if(strcmp((char *)BitmapStructOb.message_id,REVERSAL_MSG_TYPE)!=0)
      {
        if(!strcmp((char *)BitmapStructOb.field_22,ICC_PIN_CAPABLE))//FOR BATCH UPLOAD 
        {
          LOG_PRINTF(("ICC_PIN_CAPABLE: strlen(emvdata.DE55)/2 %d",(strlen(emvdata.DE55)/2)));
          int2str(var2,(strlen(emvdata.DE55)/2)); //commented for now 
          
          LOG_PRINTF(("var2[%s]",var2));
          pad(Finalvar2,var2,'0',4,RIGHT);
          LOG_PRINTF(("After pad var2[%s]",Finalvar2));
           
          if((!strcmp((char *)BitmapStructOb.field_03,SAVE_VOIDPROCODE))||(!strcmp((char *)BitmapStructOb.field_03,CHECK_VOIDPROCODE))||(!strcmp((char *)BitmapStructOb.field_03,CREDIT_VOIDPROCODE)))
          {
            LOG_PRINTF(("process_8583: EMV Void"));
            ptr1 = reqBuff+BitmapStructOb.packet_sz+2 - 6;
            packData(Finalvar2,(char*)ptr1);
            for(i = (BitmapStructOb.packet_sz+2+2 )-6,j=0;i<BitmapStructOb.packet_sz+2+2+(strlen(emvdata.DE55)/2)-6;j++,i++)
            {
              reqBuff[i] = BitmapStructOb.field_55[j];
            }
            for(i = BitmapStructOb.packet_sz+2+2+(strlen(emvdata.DE55)/2)-6,j=BitmapStructOb.packet_sz-6;i<BitmapStructOb.packet_sz+2+2+(strlen(emvdata.DE55)/2);j++,i++)
            {
              reqBuff[i] = BitmapStructOb.buffer[j];
            }
          }
          else
          {
            LOG_PRINTF(("process_8583:EMV except Void"));
			if (!strcmp(BitmapStructOb.field_03, "910000")) //We need to place bit 63 and the end of the request. 
			{
				LOG_PRINTF(("BitmapStructOb.field_55: %s", BitmapStructOb.field_55));
				ptr1 = reqBuff + BitmapStructOb.packet_sz + 2 - 12;
				packData(Finalvar2, (char*)ptr1);
				for (i = (BitmapStructOb.packet_sz + 2 + 2) - 12, j = 0;i < BitmapStructOb.packet_sz + 2 + 2 + (strlen(emvdata.DE55) / 2);j++, i++)
				{
					reqBuff[i] = BitmapStructOb.field_55[j];
					//LOG_PRINTF(("%c", reqBuff[i]));
				}
				for (i = BitmapStructOb.packet_sz + 2 + 2 + (strlen(emvdata.DE55) / 2) - 12, j = BitmapStructOb.packet_sz - 12;i < BitmapStructOb.packet_sz + 2 + 2 + (strlen(emvdata.DE55) / 2);j++, i++)
				{
					reqBuff[i] = BitmapStructOb.buffer[j];
				}
			}
			else
			{
				LOG_PRINTF(("process_8583:EMV except Void"));
				ptr1 = reqBuff + BitmapStructOb.packet_sz + 2;
				packData(Finalvar2, (char*)ptr1);
				for (i = BitmapStructOb.packet_sz + 2 + 2, j = 0;i<BitmapStructOb.packet_sz + 2 + 2 + (strlen(emvdata.DE55) / 2);j++, i++)
				{
					reqBuff[i] = BitmapStructOb.field_55[j];
				}
			}
          }
          retVal = retVal +2+(strlen(emvdata.DE55)/2);//2 bytes length of emv data + length of emv data
          memset(var2,0,sizeof(var2));
          memset(Finalvar2,0,sizeof(Finalvar2));
        }
      }

	  LOG_PRINTF(("retVal[%d]", retVal));
      sprintf(var2,"%x",retVal);
      LOG_PRINTF(("var2[%s]",var2));
      pad(Finalvar2,var2,'0',4,RIGHT);
      LOG_PRINTF(("After pad var2[%s]",Finalvar2));
      packData(Finalvar2,(char*)ptr);
      retVal=retVal+2 ;//2 byte length of whole message 

      bcd2a((char *)tempBuffer_, (char *)reqBuff, retVal);
	  if(LOG_STATUS == LOG_ENABLE)
      {
        LOG_PRINTF(("process_8583 get_char();:[%s]",tempBuffer_));     
      }
    }    
    reqBuff[6]=0xe8;
    
  }
	return retVal;
} 

/********************************************************************************************************************
*	Function Name : ProcessingISOBitmapEngine															              *
*	Purpose		    : Create  Request stream ,set tpdu length and Set bitmap                 					 									*
*	Input		      : a pointer to the field struct table ,a pointer to the transatcion details and a pointer to the 
                  Bitmap structture object					              										                              *
*	Output		    : Return value for the success or failure																			              			  *
*********************************************************************************************************************/
short ProcessingISOBitmapEngine(field_struct *iso8583_field_table,TransactionMsgStruc *ptrTransactionMsgStrucOb)
{  
  /****************************************************************
   Convert Table This table must contain all the pairs which are 
   used by the application to transform the data. The embedded
   defines establish the convert table indexes which are used 
   in iso8583_field_table The name of this table, CONVERT_TABLE, 
   should not be changed.Entries may be added to the end of this
   table, but existing entries should not be modified. 
  *****************************************************************/ 

  converters convert_table[] = 
  { 	/* Defined in ISO8583.H */ 
	  { asc_to_asc, asc_to_asc }, // ASC_ASC 0 
	  { av3_to_av3, av3_to_av3 }, // AV3_AV3 1
	  { bit_to_bit, bit_to_bit }, // BIT_BIT 2 
	  { bcd_to_bcd, bcd_to_bcd }, // BCD_BCD 3  
	  { asc_to_bcd, bcd_to_asc }, /* BCD_ASC 4 */ 
	  { str_to_asc, asc_to_str }, /* ASC_STR 5 */ 
	  { str_to_bcd, bcd_to_str }, /* BCD_STR 6 */ 
	  { str_to_bcd, bcd_to_snz }, /* BCD_SNZ 7 */ 
	  { str_to_av2, av2_to_str }, /* AV2_STR 8 */ 
	  { str_to_bv2, bv2_to_str }, /* BV2_STR 9 */ 
	  { str_to_av3, av3_to_str }, /* AV3_STR 10 */ 
	  { str_to_xbc, xbc_to_str }, /* XBC_STR 11 */ 
	  { hst_to_bin, bin_to_hst }, /* BIN_HST 12 */ 
	  { hst_to_bi2, bi2_to_hst }, /* BI2_HST 13 */ 
	  { hst_to_bi3, bi3_to_hst } /* BI3_HST 14 */ 

  }; 

  int checkTpdu = -1;
  int i;
	CreateRequestStream(ptrTransactionMsgStrucOb);    
	SetingBitMap(ptrTransactionMsgStrucOb);
	//Agency Bank Check
	for(i=0;i<16;i++)
		LOG_PRINTF (("BitmapStructOb.t_map = %u ",BitmapStructOb.t_map[i]));
	
	pip_main(BitmapStructOb.t_map,(char *)BitmapStructOb.message_id, (char *)BitmapStructOb.field_03, NULL);
	if(LOG_STATUS == LOG_ENABLE)
  {
    LOG_PRINTF (("pip_main msgid = %s  and prcode = %s ",BitmapStructOb.message_id,BitmapStructOb.field_03));
  }
	
	LOG_PRINTF (("From Ac bit 102 = %s ",BitmapStructOb.field_102));
	LOG_PRINTF (("To Ac bit 103 = %s ",BitmapStructOb.field_103));
  //// This API has to be called before using any other ISO8583 API
	iso8583_main(convert_table, return_variant1,return_variant2,BitmapStructOb.dest,BitmapStructOb.source);  
  
	prot8583_main(NULL);

	checkTpdu = set_tpdu_length(5);
	if(LOG_STATUS == LOG_ENABLE)
	{
		LOG_PRINTF (("checkTpdu get_char=%d ",checkTpdu));
	}
	return checkTpdu; 
}

/********************************************************************************************************************
*	Function Name : ParseAndFillBitFields																												                    *
*	Purpose		    : to parse response data and put the bit fields information into appropriate variable               *
*	Input		      : a pointer to the response buffer and a pointer to the response bits                               *
*	Output		    : Return value for the success or failure																			              			  *
*********************************************************************************************************************/
int ParseAndFillBitFields(char *resp ,char *bits)
{
  int offset =0;
  //int n=0;
  short aclen=0;
  char ac_len[3]={0};
  char b54_len[5]={0};
  parsedRes parsedResObj  ;
  parsedAmount parsedAmountOb ;

  memset(&parsedResObj, 0, sizeof(parsedRes));
  memset(&parsedAmountOb, 0, sizeof(parsedAmount));
  strncpy((char*)BitmapStructOb.message_id,resp+14,MSG_ID_LEN);
  offset = offset+34 ;
  
  LOG_PRINTF(("======resp [%s]",resp));
  LOG_PRINTF(("======Bitmap [%s]",bits));
  if(bits[0]=='1')
  {
	  offset=offset+16;
  }
  LOG_PRINTF(("======offset [%d]",offset));
  if(bits[2] =='1' )
  {
    strncpy((char*)BitmapStructOb.field_03,resp+offset,PROC_CODE_LEN);  
    offset =offset + PROC_CODE_LEN ;
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("field_03 = %s",BitmapStructOb.field_03));
    }
  }
  if(bits[3] =='1' )
  {
    strncpy((char*)BitmapStructOb.field_04,resp+offset,AMOUNT_LEN);  
    offset =offset + AMOUNT_LEN ;
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("field_04 = %s",BitmapStructOb.field_04));
    }
  }
  if(bits[10] =='1' )
  {
    strncpy((char*)BitmapStructOb.field_11,resp+offset,TRACE_AUDIT_LEN);  
    offset =offset + TRACE_AUDIT_LEN ;
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("field_11 = %s",BitmapStructOb.field_11));
    }
  }
  if(bits[11] =='1' )
  {
    strncpy((char*)BitmapStructOb.field_12,resp+offset,TRANS_LOCALTIME_LEN);  
    offset =offset + TRANS_LOCALTIME_LEN ;
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("field_12 = %s",BitmapStructOb.field_12));
    }
  }
  if(bits[12] =='1' )
  {
    strncpy((char*)BitmapStructOb.field_13,resp+offset,TRANS_LOCALDATE_LEN);  
    offset =offset + TRANS_LOCALDATE_LEN ;
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("field_13 = %s",BitmapStructOb.field_13));
    }
  }
  if(bits[23] =='1' )
  {
    strncpy((char*)BitmapStructOb.field_24,resp+offset,NII_LEN);  
    offset =offset + NII_LEN+1 ;
  }
  if(bits[36] =='1' )
  {
    strncpy((char*)parsedResObj.retrivalReferenceNo,resp+offset,(RETRIEVAL_REF_LEN*2));  
		
    packData(parsedResObj.retrivalReferenceNo,(char*)BitmapStructOb.field_37);
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("ptrFinalRespOb->retrivalReferenceNo = %s",BitmapStructOb.field_37)); 
    }
    offset =offset + (RETRIEVAL_REF_LEN*2) ;
  }
  if(bits[37] =='1' )
  {
    strncpy((char*)parsedResObj.AuthIdResponse,resp+offset,(AUTH_ID_LEN*2));  
    
    packData(parsedResObj.AuthIdResponse,(char*)BitmapStructOb.field_38);
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("dec AuthIdResponse = %s",BitmapStructOb.field_38));
    }
   
    offset =offset + (AUTH_ID_LEN*2) ;
  }
  if(bits[38] =='1' )
  {
    strncpy((char*)parsedResObj.responseCode,resp+offset,(RESPONSE_CODE_LEN*2));  
    
    packData(parsedResObj.responseCode,(char*)BitmapStructOb.field_39);
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF((" dec responseCode = %s",BitmapStructOb.field_39));
    }
    offset =offset + (RESPONSE_CODE_LEN*2) ;
  }
  if(bits[40] =='1' )//Checking terminal id
  {
    strncpy((char*)parsedResObj.TerId,resp+offset,(TERMINAL_ID_LEN*2));  
    
    packData(parsedResObj.TerId,(char*)BitmapStructOb.field_41);
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF((" Terminal ID = %s",BitmapStructOb.field_41));
    }
    offset =offset + (TERMINAL_ID_LEN*2) ;
  }
  if(bits[47] =='1' && (strncmp((char*)BitmapStructOb.message_id,"0810",4)==0))//Checking pRIVATE KEY FOR LOGON
  {
    //Copy Data Encryption Key
    strncpy((char*)DataEncKey,resp+offset+UNPACK_SIZE_MSGLEN_OF_REQ_RES,UNPACK_ENC_PR_KEY_LEN);  
	  offset =offset + UNPACK_SIZE_MSGLEN_OF_REQ_RES + UNPACK_ENC_PR_KEY_LEN ;
	  //Copy Data Encryption Key KCV
	  strncpy((char*)DataEncKeyKCV,resp+offset,UNPACK_ENC_PR_KCV_LEN);  
	  offset =offset + UNPACK_ENC_PR_KCV_LEN  ;
	
	  //Copy Data Encryption Key 
	  strncpy((char*)EncSessionKey,resp+offset,UNPACK_ENC_PR_KEY_LEN);  
	  offset =offset + UNPACK_ENC_PR_KEY_LEN  ;
	  //Copy Data Encryption Key KCV
	  strncpy((char*)EncSessionKeyKCV,resp+offset,UNPACK_ENC_PR_KCV_LEN);  
	  offset =offset + UNPACK_ENC_PR_KCV_LEN  ;
	
    LOG_PRINTF((" DataEncKey dec responseCode = %s",DataEncKey));
	  LOG_PRINTF((" DataEncKeyKCV dec responseCode = %s",DataEncKeyKCV));
	  LOG_PRINTF((" EncSessionKeyKCV dec responseCode = %s",EncSessionKeyKCV));
	  LOG_PRINTF((" EncSessionKey dec responseCode = %s",EncSessionKey));
    
    //offset =offset + UNPACK_SIZE_MSGLEN_OF_REQ_RES + UNPACK_ENC_PR_KEY_LEN ;

    /*
	   strncpy((char*)EncSessionKey,resp+offset+UNPACK_SIZE_MSGLEN_OF_REQ_RES,UNPACK_ENC_PR_KEY_LEN);  
   
      LOG_PRINTF((" dec responseCode = %s",EncSessionKey));
    
      offset =offset + UNPACK_SIZE_MSGLEN_OF_REQ_RES + UNPACK_ENC_PR_KEY_LEN ;
	  */
  }
  if(bits[52] =='1' )
  {
    strncpy((char*)BitmapStructOb.field_53,resp+offset,SECURITY_CONTROL_LEN);
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF((" dec field_53 = %s",BitmapStructOb.field_53));
    }
    offset =offset +SECURITY_CONTROL_LEN ;
  }
  if(bits[53] =='1' )
  {
		strncpy(b54_len,resp+offset,4);
			aclen=atoi(b54_len);
		LOG_PRINTF((" Bit 54 len = %s - %d",b54_len,aclen))
	  strncpy((char*)parsedResObj.AddAmount,resp+offset+4,aclen*2);

	//  strncpy((char*)parsedResObj.AddAmount,resp+offset+UNPACK_SIZE_MSGLEN_OF_REQ_RES,BALANCE_AMOUNT_RES_LEN -UNPACK_SIZE_MSGLEN_OF_REQ_RES);
    
    packData(parsedResObj.AddAmount,(char*)BitmapStructOb.field_54);
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF((" dec field_54 = %s",BitmapStructOb.field_54));
    }
    parseAmountResponse(&parsedAmountOb);
    //offset = offset+BALANCE_AMOUNT_RES_LEN;
	offset = offset+4+(aclen*2);
  }  
	if(bits[101] =='1' )			//Field 102 - From Account
	{
		strncpy(ac_len,resp+offset,2);
		aclen=atoi(ac_len);
		LOG_PRINTF((" From Account len = %s - %d",ac_len,aclen))
		strncpy((char*)parsedResObj.FromAc,resp+offset+2,(aclen*2));  
    	packData(parsedResObj.FromAc,(char*)BitmapStructOb.field_102);
		if(LOG_STATUS == LOG_ENABLE)
		{
		LOG_PRINTF((" From Account = %s",BitmapStructOb.field_102));
		}
		offset =offset + 2+ (aclen*2) ;
	}
	
	if(bits[102] =='1' )		//Field 103 - To Account
	{
		strncpy(ac_len,resp+offset,2);
		aclen=atoi(ac_len);
		LOG_PRINTF((" To Account len = %s - %d",ac_len,aclen))
		strncpy((char*)parsedResObj.FromAc,resp+offset+2,(aclen*2));  
    	packData(parsedResObj.FromAc,(char*)BitmapStructOb.field_103);
    		
		//strncpy((char*)BitmapStructOb.field_103,resp+offset+2,aclen*2);
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF((" dec field_103 = %s",BitmapStructOb.field_103));
    }
    offset =offset+2+(aclen*2) ;
  }
   return 0;
}

/********************************************************************************************************************
*	Function Name : CopyAllBits																													                              *
*	Purpose		    : to copy the bits into the buffer                                                                  *
*	Input		      : a pointer to the input buffer and the output buffer                                               *
*	Output		    : void                                          															              			  *
*********************************************************************************************************************/
void CopyAllBits(char *byte,char *temp)
{
  int i =0,m=0;
  for(i = 7; 0 <= i; i --)
  { 
    if((*byte >> i) & 0x01)
    {
      *(temp+m) = '1' ;
    }
    else
    {
      *(temp+m) = '0' ;
    }
    m++;
  } 
}

/********************************************************************************************************************
*	Function Name : parseAmountResponse																									                              *
*	Purpose		    : parse amount response for balnce inwquiry transaction                                                                  *
*	Input		      : an integer                                                                                        *
*	Output		    : void                                          															              			  *
*********************************************************************************************************************/
short parseAmountResponse(parsedAmount *ptrParsedAmountob)
{
  short offset =0;
  offset = ACCOUNT_TYPE1_LEN+AMOUNT_TYPE1_LEN+ACCOUNT_CURRENCY1_LEN+1;
  strncpy(ptrParsedAmountob->LedgerAmount,(char*)BitmapStructOb.field_54+offset,AMOUNT_LEN);//8
  if(LOG_STATUS == LOG_ENABLE)
  {
    LOG_PRINTF(("ptrParsedAmountob->LedgerAmount = %s",ptrParsedAmountob->LedgerAmount));
  }

  offset = offset+ AMOUNT_LEN +ACCOUNT_TYPE2_LEN+AMOUNT_TYPE2_LEN+ACCOUNT_CURRENCY2_LEN+1;
  strncpy(ptrParsedAmountob->AvailAmount,(char*)BitmapStructOb.field_54+offset,AMOUNT_LEN);  //28
  if(LOG_STATUS == LOG_ENABLE)
  {
    LOG_PRINTF(("ptrParsedAmountob->AvailAmount = %s",ptrParsedAmountob->AvailAmount));
  }
  strcpy((char*)BitmapStructOb.field_04,(char*)ptrParsedAmountob->AvailAmount);  
  if(LOG_STATUS == LOG_ENABLE)
  {
    LOG_PRINTF(("Available Amount = %s",BitmapStructOb.field_04));
  }
  return 0;
}

