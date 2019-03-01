/***************************************************************************
  Filename		     : Logon.c
  Project		       : Bevertec 
  Developer Neame  : Amar
  Module           : Logon and Reversal
  Description	     : This is the implementation of logon and reversal transaction
 ******************************************************************************/



#include <aclconio.h>

#include "..\Include\Bevertec.h"
#include "..\Include\BevertecClient.h"
#include "..\Include\ReciptPrint.h"
#include "..\Include\ISo8583Map.h"
#include "..\Include\Logon.h"
#include "..\Include\SaleTransaction.h"
#include "..\Include\Settlement.h"


extern char msg_id[MSG_ID_LEN+1];//5
extern char Proc_code[PROC_CODE_LEN+1];//7
extern BmapStruct BitmapStructOb ;
extern unsigned char EncSessionKey[UNPACK_ENC_PR_KEY_LEN+1];//33 
extern unsigned char EncSessionKeyKCV[UNPACK_ENC_PR_KCV_LEN+1]; 
extern unsigned char DataEncKey[UNPACK_ENC_PR_KEY_LEN+1]; 
extern unsigned char DataEncKeyKCV[UNPACK_ENC_PR_KCV_LEN+1]; 


/****************************************************************************************************************
*	Function Name : InitBalanceInquiry																											                    *
*	Purpose		    : intialize balance inquiry request for a transaction 		 			 																              *
*	Input					:	Address of main transaction message structure and transaction methord type 						                                              *
*	Output		    : returns success or failure 								                                  *
*****************************************************************************************************************/
short InitLogon(TransactionMsgStruc *transMsg)
{
		short iRetVal =_FAIL;
    char terminalId[TERMINAL_ID_LEN+1]={0};//9
		
		getTraceAuditNumber(transMsg);  //11 
		get_env("#TERMINAL_ID",(char*)terminalId,sizeof(terminalId)); //41
		sprintf(transMsg->TerminalID,"%s",terminalId); 
		sprintf(transMsg->_transType,"%s","Logon");
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("transMsg->TerminalID = =  %s ",transMsg->TerminalID));
    }

	iRetVal = LogonProcessing(transMsg);
    if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF(("ret val = =  %d ",iRetVal));
    }
      
	checkMasterKey();
    //if(iRetVal==_SUCCESS)
    //{
	//	key_injected = 0;
	//	return _SUCCESS;
    //}

    return _FAIL;
}

/****************************************************************************************************************
*	Function Name : LogonProcessing																											                          *
*	Purpose		    : This function send the request to server for each and every transaction 
									
*	Input					: Address of Structure which hold the data for settlement																				*
*	Output		    : returns success or failure 																																		*
*****************************************************************************************************************/

short LogonProcessing(TransactionMsgStruc *transMsg)
{
  int i =0;
  char Bitmap[BITMAP_ARRAY_SIZE] ={0};//8
  char UnpackBiitmap[UNPACK_BITMAP_ARRAY_SIZE]={0};//16
  char bits[BITMAP_SIZE] ={0};//64
  char temp[BITMAP_ARRAY_SIZE+1] ={0};//9

  short retVal = -1;  
	short ReqLen = 0; 
  unsigned char reqBuff[LOGON_REQUEST_SIZE]={0};//96
  unsigned char resBuff[LOGON_RESPONSE_SIZE]={0};//100
  unsigned char testBuff[LOGON_UNAPACK_RES_SIZE]={0};//200


  short recvLength = 0;
  
 // parsedLogonRes responseData ;
  ///////////////////////////////////////////
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
    { 37+ SKIP, 12, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 38+ SKIP, 6, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 39, 2, BCD_STR, (void *) BitmapStructOb.field_39, sizeof( BitmapStructOb.field_39) }, 
    { 40+ SKIP, 3, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 41, 8, ASC_STR, (void *) BitmapStructOb.field_41, sizeof( BitmapStructOb.field_41) }, 
    { 42+ SKIP, 15, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 43+ SKIP, 40, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 44+ SKIP, 25, AV2_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 45+SKIP, 76, AV2_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 46+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 47+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 48, 9, AV3_STR, (void *) BitmapStructOb.field_48, sizeof( BitmapStructOb.field_48) }, 
	    { 49+ SKIP, 3, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 50+ SKIP, 3, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 51+ SKIP, 3, ASC_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 52+ SKIP, 64, BIT_BIT, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 53+ SKIP, 16, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 54+ SKIP, 120, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 55+ SKIP, 999, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 56+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 57+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 58+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 59+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 60+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 61+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 62+ SKIP, 999, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 63+ SKIP, 999, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 64+ SKIP +STOP, 64, BIT_BIT, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
	//{ 102+ SKIP +STOP, 102, AV2_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, //Agency Bank
  }; 
  
  strcpy(msg_id,LOGON_MSG_TYPE);
  strcpy(Proc_code,LOGONPROCODE);
	
  strcpy(transMsg->MessageTypeInd,LOGON_MSG_TYPE);
  strcpy(transMsg->ProcessingCode,LOGONPROCODE);
  //

  transMsg->TrTypeFlag = LOGON_MSG_TYPE_CASE ;
  if(LOG_STATUS == LOG_ENABLE)
  {
	LOG_PRINTF (("->>>>>>>>>>msgtype =%d",transMsg->TrTypeFlag));
    LOG_PRINTF (("->>>>>>>>>>processing code =%s",transMsg->ProcessingCode));
  }
  
  resetBitmapStructure(&BitmapStructOb);//memsetting all the elements of structure
 // memset(&responseData, 0, sizeof(responseData));
  
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
  else
  {
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("assemble_packet retVal = %d \n",retVal));
    }
  }
 
  recvLength = DEFAULT_BUFLEN ;
  retVal = CommWithServer((char*)reqBuff,ReqLen,(char*)resBuff,&recvLength);
  
  if(retVal == _SUCCESS )//checking for response buffer size 
  {
    bcd2a((char *)testBuff,(char *)resBuff,recvLength);
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("Testbuffin LOGON disassemblePacket retVal = %s \n",testBuff));
    }
    //GOing To copy the Unpack bitmap from response  
    strncpy(UnpackBiitmap,(char*)testBuff+UNPACK_SIZE_MSGLEN_OF_REQ_RES+TPDU_LEN+MSG_ID_LEN,UNPACK_BITMAP_ARRAY_SIZE);  
    packData(UnpackBiitmap,Bitmap);
    for(i=0;i<8;i++)
    {
        CopyAllBits(Bitmap+i,temp);
        strcpy(bits+(i*8),temp);
    }
    resetBitmapStructure(&BitmapStructOb);//memsetting all the elements of structure    
    memset(EncSessionKey, 0, sizeof(EncSessionKey));
    ParseAndFillBitFields((char*)testBuff,bits);

    if(!strcmp((char *)BitmapStructOb.field_39,"00"))
    {
        window(1,1,30,20);
				clrscr();
        if(!strcmp((char *)EncSessionKey,"")) 
        {
		   // write_at("AGENT",5,12,9);   
			write_at(Dmsg[DID_NOT_GET_KEY].dispMsg, strlen(Dmsg[DID_NOT_GET_KEY].dispMsg),Dmsg[DID_NOT_GET_KEY].x_cor, Dmsg[DID_NOT_GET_KEY].y_cor);
           write_at(Dmsg[PLEASE_LOGON_AGAIN].dispMsg, strlen(Dmsg[PLEASE_LOGON_AGAIN].dispMsg),Dmsg[PLEASE_LOGON_AGAIN].x_cor, Dmsg[PLEASE_LOGON_AGAIN].y_cor);
				             
           SVC_WAIT(2000);
           ClearKbdBuff();
			     KBD_FLUSH();
           if(LOG_STATUS == LOG_ENABLE)
           {
            LOG_PRINTF(("Did not get key ,need to logon again\n"));     
           }
           return _FAIL;
        }
        else
        {
          strcpy(transMsg->ResponseText,APPROVED_MSG);
        
          write_at(Dmsg[LOGON_SUCCESSFUL].dispMsg, strlen(Dmsg[LOGON_SUCCESSFUL].dispMsg),Dmsg[LOGON_SUCCESSFUL].x_cor, Dmsg[LOGON_SUCCESSFUL].y_cor);//LOGON SUCCESSFUL
          SVC_WAIT(2000);
          ClearKbdBuff();
			    KBD_FLUSH();
          if(LOG_STATUS == LOG_ENABLE)
          {
            LOG_PRINTF(("Logon Transaction successful\n"));     
          }
        }
    }
    else
    {
        window(1,1,30,20);
				clrscr();
        write_at("LOGON FAILED",strlen("LOGON FAILED"),(30-strlen("LOGON FAILED"))/2,10);
				
        SVC_WAIT(2000);
        if(LOG_STATUS == LOG_ENABLE)
        {
          LOG_PRINTF(("LogonFail reason : %s \n",BitmapStructOb.field_39)); 
        }
    }
  }
  else
  {
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("Response buffer size is zero  for Logon\n"));  
    }
    return _FAIL; 
  }

	return _SUCCESS;
}
/****************************************************************************************************************
*	Function Name : ReversalProcessing																											                          *
*	Purpose		    : This function send the request to server for each and every transaction 
									
*	Input					: Address of Structure which hold the data for settlement																				*
*	Output		    : returns success or failure 																																		*
*****************************************************************************************************************/

short ReversalProcessing(TransactionMsgStruc *transMsg)
{
  int i =0;
  char Bitmap[BITMAP_ARRAY_SIZE+1] ={0};//8
  char UnpackBiitmap[UNPACK_BITMAP_ARRAY_SIZE+1]={0};//16
  char bits[BITMAP_SIZE+1] ={0};//64
  char temp[BITMAP_ARRAY_SIZE+1] ={0};//9
  short ReqLen = 0; 
  short retVal = -1;  
  short BITMAP_LEN=8;
	FILE *ifp=NULL;

  unsigned char reqBuff[REVERSAL_REQUEST_SIZE]={0};//108
  unsigned char resBuff[REVERSAL_RESPONSE_SIZE]={0};//110
  unsigned char testBuff[REVERSAL_UNAPACK_RES_SIZE]={0};//220
  short recvLength = 0;

  /////////////////////////////////////////
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
    { 55+ SKIP, 0, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 56+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 57+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 58+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 59+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 60, 12, BCD_STR, (void *) BitmapStructOb.AmountForVoid, sizeof( BitmapStructOb.AmountForVoid) }, 
    { 61+ SKIP, 999, AV3_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 62+ SKIP, 999, BCD_STR, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
    { 63, KSN_LENGTH, BCD_STR, (void *)BitmapStructOb.field_63, sizeof( BitmapStructOb.field_63 ) },
    { 64+ SKIP +STOP, 64, BIT_BIT, (void *) BitmapStructOb.discard, sizeof( BitmapStructOb.discard) }, 
	
  }; 
  ////////////////////
  strcpy(msg_id,REVERSAL_MSG_TYPE);
  strcpy(Proc_code,transMsg->ProcessingCode);
  if(LOG_STATUS == LOG_ENABLE)
  {
	  LOG_PRINTF (("->>>>>>>>>>processing code =%s",transMsg->ProcessingCode));
  }
  strcpy(transMsg->MessageTypeInd,REVERSAL_MSG_TYPE);
  
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
  // retVal = _FAIL ;
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
    packData(UnpackBiitmap,Bitmap);
    for(i=0;i<BITMAP_LEN;i++)
    {
        CopyAllBits(Bitmap+i,temp);
        strcpy(bits+(i*8),temp);
    }
  	
    resetBitmapStructure(&BitmapStructOb);//memsetting all the elements of structure

    ParseAndFillBitFields((char*)testBuff,bits);

    strcpy((char *)transMsg->ResponseCode,(char*)BitmapStructOb.field_39);             //copy the response code
    strcpy((char *)transMsg->AuthIdResponse,(char*)BitmapStructOb.field_38);           //copy AuthIdResponse
    strcpy((char *)transMsg->RetrievalReferenceNo,(char*)BitmapStructOb.field_37);     //copy RetrievalReferenceNo
	  strcpy((char *)transMsg->TransLocalTime,(char*)BitmapStructOb.field_12);           //copy AuthIdResponse
    strcpy((char *)transMsg->TransLocalDate,(char*)BitmapStructOb.field_13);     //copy RetrievalReferenceNo
	
    ifp = fopen(REVARSAL_DETAILS_FILE, "w");//in order to clear data from file need to open file in write mode
		if (ifp == NULL)
		{
				if(LOG_STATUS == LOG_ENABLE)
        {
          LOG_PRINTF (("Failed to open the file %s\n", REVARSAL_DETAILS_FILE));	
        }
				return _FAIL;
		}
		fclose(ifp);
    if(!strcmp((char *)BitmapStructOb.field_39,"00"))
    {
				strcpy(transMsg->ResponseText,APPROVED_MSG);
        
        window(1,1,30,20);
				clrscr();
			
				write_at(Dmsg[REVERSAL_SUCCESSFUL].dispMsg, strlen(Dmsg[REVERSAL_SUCCESSFUL].dispMsg),Dmsg[REVERSAL_SUCCESSFUL].x_cor, Dmsg[REVERSAL_SUCCESSFUL].y_cor);//REVERSAL SUCCESSFULL
        SVC_WAIT(2000);
				ClearKbdBuff();
				KBD_FLUSH();
        if(LOG_STATUS == LOG_ENABLE)
        {
          LOG_PRINTF(("Reversal successfull\n"));     
        }
    }
    else
    {
        strcpy(transMsg->ResponseText,"TRANSACTION DECLINED");
        window(1,1,30,20);
				clrscr();
        write_at("TRANSACTION DECLINED",strlen("TRANSACTION DECLINED"),(30-strlen("TRANSACTION DECLINED"))/2,10);
				
        SVC_WAIT(2000);
				ClearKbdBuff();
				KBD_FLUSH();
        if(LOG_STATUS == LOG_ENABLE)
        {
          LOG_PRINTF(("Reversal Fail reason : %s \n",BitmapStructOb.field_39));    
        }
      
    }
  }
  else
  {
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("Response buffer size is zero\n"));  
    }
    return _FAIL; 
  }

	return _SUCCESS;
}

/****************************************************************************************************************
*	Function Name : SaveReversalDetails																											                          *
*	Purpose		    : This function saves the reversal data into file
*	Input					: Address of Structure which hold the data for Reversal																				*
*	Output		    : returns success or failure 																																		*
*****************************************************************************************************************/
short SaveReversalDetails(TransactionMsgStruc *transMsg)
{
		FILE *ifp=NULL;
    int i =0;
   
    char RevStatus[2] = {0};
		RevDetails ReverDetailsObj= {0};
    get_env("#REV_STATUS",RevStatus,sizeof(RevStatus));
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("in the #REV_STATUS %s",RevStatus));
    }
  // revstat=1;
    if (!strcmp(RevStatus,"1"))
    {
      if(LOG_STATUS == LOG_ENABLE)
      {
        LOG_PRINTF(("already saved"));
      }
      
      return _SUCCESS;
    }
		memset(&ReverDetailsObj,0,sizeof(RevDetails));

		strncpy(ReverDetailsObj.cardNumber,transMsg->PrimaryAccNum,ACC_NUMBER_LEN); //BitField 3
		strncpy(ReverDetailsObj.expDate,transMsg->ExpiryDate,EXPTIME_ARR_LEN); //BitField 3
		
		
    strncpy(ReverDetailsObj.processingCode,transMsg->ProcessingCode,PROC_CODE_LEN); //BitField 3
		strncpy(ReverDetailsObj.amount,transMsg->Amount,AMOUNT_LEN);//BitField 4		
    strncpy(ReverDetailsObj.PosEntryMode,transMsg->POSEntryMode,POS_ENTRY_LEN); //BitField 22
    strncpy(ReverDetailsObj.NII,transMsg->NetworkInternationalId,NII_LEN); //BitField 24
    strncpy(ReverDetailsObj.PosCondCode,transMsg->POSConditionCode,POS_COND_LEN); //BitField 25	
    strncpy(ReverDetailsObj.Track2Data,transMsg->Track2Data,strlen(transMsg->Track2Data)); // Bit field 35
		strncpy(ReverDetailsObj.RetrievalRefNo,transMsg->RetrievalReferenceNo,RETRIEVAL_REF_LEN); //BitField 37
    strncpy(ReverDetailsObj.TerminalId,transMsg->TerminalID,TERMINAL_ID_LEN); // BitField 41
    strncpy(ReverDetailsObj.CardAcceptorId,transMsg->CardAcceptorID,CARD_ACCEPTOR_ID_LEN); //BitField 42
    strncpy(ReverDetailsObj.Payment_Id,transMsg->PaymentId,PAYMENT_ID_LEN); //BitField 48
		
    strncpy(ReverDetailsObj.trans_Type,transMsg->_transType,TRANS_TYPE_STRING);
		ReverDetailsObj.TransTypeFlg=transMsg->TrTypeFlag; 
		ReverDetailsObj.TransMethord_Flag=transMsg->TrMethordFlag;
		
    for(i=0; i<BIANRY_PIN_LEN ;i++)
    {
      ReverDetailsObj.PinData[i] = transMsg->PinData[i]   ;    //Bit Filed 52
    }

		ifp = fopen(REVARSAL_DETAILS_FILE, "w");
		if (ifp == NULL)
		{
      if(LOG_STATUS == LOG_ENABLE)
      {
				LOG_PRINTF (("Failed to open the file %s\n", REVARSAL_DETAILS_FILE));
      }
			return _FAIL;
		}
		else
		{
			if (fwrite(&ReverDetailsObj, sizeof(RevDetails), 1, ifp) != 1)
			{
        if(LOG_STATUS == LOG_ENABLE)
        {
				  LOG_PRINTF (( "Failed to write to %s\n", REVARSAL_DETAILS_FILE));	
        }
				fclose(ifp);
        
				return _FAIL;
			}
      put_env("#REV_STATUS","1",sizeof("1"));
			fclose(ifp);
		}
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF (( "Save reversal details "));	
    }
		
		return _SUCCESS;
}



/****************************************************************************************************************
*	Function Name : ReadTransDetails																											                    *
																																												*
*	Purpose		    : This function read  all details related to transaction made by user on behalf of payment id
  Input					: pointer to message structure																																	*
*	Output		    : returns success or failure 																																		*
*																																											*
*****************************************************************************************************************/


short ReadReversalDetails(TransactionMsgStruc *transMsg)
{
		FILE *ifp=NULL;
		RevDetails ReverDetailsObj= {0};
		int i =0;
    char RevStatus[2] = {0}; // "0" or "1" 
		memset(&ReverDetailsObj,0,sizeof(RevDetails));
    get_env("#REV_STATUS",RevStatus,sizeof(RevStatus));
    
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("#REV_STATUS %s",RevStatus));
    }
    if(!strcmp(RevStatus,"0"))
    {
      if(LOG_STATUS == LOG_ENABLE)
      {
        LOG_PRINTF (("RevStatusn is 0"));	
      }
      return _FAIL;
    }
    else
    {
      ifp = fopen(REVARSAL_DETAILS_FILE, "r");
    }

 		if (ifp == NULL)
		{
      if(LOG_STATUS == LOG_ENABLE)
      {
				LOG_PRINTF (("Failed to open the file %s\n", REVARSAL_DETAILS_FILE));	
      }
			return _FAIL;
		}

		fread(&ReverDetailsObj, sizeof(RevDetails), 1, ifp);
    
		memset(transMsg, '\0', sizeof(TransactionMsgStruc));
   
    if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF(("Payment_Id = %s ",ReverDetailsObj.Payment_Id));
		  LOG_PRINTF(("Amount= %s",ReverDetailsObj.amount));
		  LOG_PRINTF(("NII = %s",ReverDetailsObj.NII));
		  LOG_PRINTF(("TerminalIdentification  = %s",ReverDetailsObj.TerminalId));
		  LOG_PRINTF(("ProcessingCode = %s ",ReverDetailsObj.processingCode));
		  LOG_PRINTF(("POSEntryMode = %s ",ReverDetailsObj.PosEntryMode));
		  LOG_PRINTF(("POSConditionCode = %s ",ReverDetailsObj.PosCondCode));
		  LOG_PRINTF(("Track2Data = %s ",ReverDetailsObj.Track2Data));
		  LOG_PRINTF(("RetrievalReferenceNo = %s ",ReverDetailsObj.RetrievalRefNo));
		  LOG_PRINTF(("CardAcceptorId = %s ",ReverDetailsObj.CardAcceptorId));
    }
    strcpy(transMsg->ProcessingCode,ReverDetailsObj.processingCode);
		strcpy(transMsg->Amount,ReverDetailsObj.amount);
        
    strcpy(transMsg->POSEntryMode,ReverDetailsObj.PosEntryMode);
		strcpy(transMsg->NetworkInternationalId,ReverDetailsObj.NII);
    strcpy(transMsg->POSConditionCode,ReverDetailsObj.PosCondCode);
		strcpy(transMsg->Track2Data,ReverDetailsObj.Track2Data);
    strcpy(transMsg->RetrievalReferenceNo,ReverDetailsObj.RetrievalRefNo);
    strcpy(transMsg->TerminalID,ReverDetailsObj.TerminalId);
    strcpy(transMsg->CardAcceptorID,ReverDetailsObj.CardAcceptorId);
    strcpy(transMsg->PaymentId,ReverDetailsObj.Payment_Id);
		strcpy(transMsg->PrimaryAccNum,ReverDetailsObj.cardNumber);
		strcpy(transMsg->ExpiryDate,ReverDetailsObj.expDate);
        
    for(i=0;i<BIANRY_PIN_LEN ;i++)
    {
      transMsg->PinData[i] =  ReverDetailsObj.PinData[i];
			if(LOG_STATUS == LOG_ENABLE)
      {
        LOG_PRINTF(("pin [%d] = %c ",i,transMsg->PinData[i]));
      }
    }

    strcpy(transMsg->_transType,ReverDetailsObj.trans_Type);
    transMsg->TrTypeFlag = ReverDetailsObj.TransTypeFlg ;
    transMsg->TrMethordFlag = ReverDetailsObj.TransMethord_Flag ;
		if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("_transType = %s ",ReverDetailsObj.trans_Type));
		  LOG_PRINTF(("TrTypeFlag = %d ",ReverDetailsObj.TransTypeFlg));
		  LOG_PRINTF(("TrMethordFlag = %d ",ReverDetailsObj.TransMethord_Flag));
    }
		fclose(ifp);
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("#REV_STATUS %s",RevStatus));
    }
    
    return _SUCCESS;
			 
}

/****************************************************************************************************************
*	Function Name : InitReversal																											                    *
*	Purpose		    : intialize Reversal request for a transaction 		 			 																              *
*	Input					:	Address of main transaction message structure and transaction methord type 						                                              *
*	Output		    : returns success or failure 								                                  *
*****************************************************************************************************************/
short InitReversal()
{
		short iRetVal = _FAIL ;
    TransactionMsgStruc transMsg ={0};
    iRetVal  = ReadReversalDetails(&transMsg);
    if(iRetVal == _SUCCESS)
    {
      if(LOG_STATUS == LOG_ENABLE)
      {
        LOG_PRINTF(("Going for reversal first"));
      }
      window(1,1,30,20);
			clrscr();
			              
			write_at(Dmsg[GOING_FOR_REVERSAL].dispMsg, strlen(Dmsg[GOING_FOR_REVERSAL].dispMsg),Dmsg[GOING_FOR_REVERSAL].x_cor, Dmsg[GOING_FOR_REVERSAL].y_cor);
      SVC_WAIT(1000);
      ClearKbdBuff();
			KBD_FLUSH();
                     
    }
    else
    {
      if(LOG_STATUS == LOG_ENABLE)
      {
        LOG_PRINTF(("No data for reversal---"));
      }
      return _SUCCESS;//In case of no reversal pending
    }
		getTraceAuditNumber(&transMsg);  //11 
		iRetVal = ReversalProcessing(&transMsg); 
    if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF(("ret val get_char = =  %d ",iRetVal));
    }
   
    if(iRetVal==_SUCCESS)
    {
      put_env("#REV_STATUS","0",sizeof("0"));
      sprintf(transMsg._transType,"%s","REVERSAL");
      printRecipt(&transMsg);
			return _SUCCESS;
    }
    return _FAIL;
}

