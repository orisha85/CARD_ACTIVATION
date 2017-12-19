/******************************************************************
  Filename		     : BevertecClient.c
  Project		       : Bevertec 
  Developer Neame  : Vikas
  Modification     : Purvin added two function for Encryption and decryption
  Module           : Communication
  Description	: This is the implementation of TCP/IP Socket communicatin.
******************************************************************/
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <svc.h>
#include <svctxo.h>
#include <applidl.h>
#include <acldev.h>
#include <aclstr.h>
#include <message.h>
#include <printer.h>
#include <errno.h>
#include <time.h>

#include "logsys.h"
#include "varrec.h"
#include "eeslapi.h" 

//For Network and SSL


#include <svc_net.h>
#include <svc_ipd.h>
#include "logsys.h"
#include <errno.h>
#include <errno.h>
//#include <err.h>
#include <ssl.h> 
#include <des.h>

#include "..\Include\BevertecClient.h"
#include "..\include\Common.h"
#include "..\Include\Settlement.h"
#include "..\include\Appevent.h"
#include "..\include\ISo8583Map.h"
#include "..\Include\IPP_Key_injection.h"

extern unsigned char DataEncKey[UNPACK_ENC_PR_KEY_LEN+1]; 
/*****************************************************************************************************************
*Function Name : GetDecryptedData																				                                          *
*	Purpose		   : This function decrypts the data after recieving form the server							                        *
*	Input		     : response message to encrypt, length of the request,a pointer to the response buffer and  
                 length of response buffer 														                                            *
*	Output		   : Return value for the success or failure			                                                    *
******************************************************************************************************************/
static int GetDecryptedData(char *szReq,short lenRqst,char *szResp,short *rcvSize)
{
	//Purvin: FOR E2EE using DES
	unsigned char iVecArr[INIT_VECTOR_LEN+1]; //intial vector for Encryptiion (first 8 btyes of GISKe header)
	unsigned char enckey[PACK_KEY_SIZE+1];
	unsigned char DecDataEnckey[PACK_KEY_SIZE+1];
	unsigned char packMasterKey[PACK_KEY_SIZE+1];
	unsigned char tempBuffer[800];//400 to 800
	//( to remove warning)unsigned char tempdispBuffer[400];
	
	char cardAccepterId[CARD_ACCEPTOR_ID_LEN+1]={0};//16
	char terminalId[TERMINAL_ID_LEN+1]={0};//9
	char msgType[MSG_ID_LEN+1];
	char encType[MSG_ID_LEN+1];
	int iHeaderLen = szReq[1];
	char tempLen[5];
	int indexReq=0;
	int indexNewReq=0;
	int inputLenCrypto=0;
	int otputLenCrypto=0;
	short Bit_len=8;

	memset(cardAccepterId,0x00,sizeof(cardAccepterId));
	memset(terminalId,0x00,sizeof(terminalId));

	memset(msgType,0x00,sizeof(msgType));
	memset(iVecArr,0,sizeof(iVecArr));
	memset(enckey,0,sizeof(enckey));
	memset(iVecArr,0,sizeof(iVecArr));
	memset(enckey,0,sizeof(enckey));
	

	indexReq = 2; // Skip First 40 bytes of response and get encrypted data
	

	indexNewReq=2;
	//                  2  +  5
	//COPY TPDU TCP HEADER + TPDU 
	memcpy((char*)&tempBuffer[indexReq],&szReq[indexNewReq],TPDU_LEN/2);	// COPY TPDU
	indexReq = indexNewReq += TPDU_LEN/2;

	memcpy(encType,&szReq[indexNewReq],MSG_ID_LEN/2);   // COPY encType TYPE
	indexNewReq += 2 + 23;

	memcpy(msgType,&szReq[indexNewReq],MSG_ID_LEN/2);   // COPY Message Type TYPE
/*	memcpy((char*)&tempBuffer[indexReq],&szReq[indexNewReq],2+8);   // COPY Message TYPE + BITMAP
	indexReq+=10;
	indexNewReq += 10;
	inputLenCrypto =  (iHeaderLen + 2) - indexNewReq;
*/
		if ((msgType[0]==0x08) && (msgType[1]==0x10))
	{
		memcpy((char*)&tempBuffer[indexReq],&szReq[indexNewReq],2+8);   // COPY Message TYPE + BITMAP
	indexReq+=10;
	indexNewReq += 10;
	inputLenCrypto =  (iHeaderLen + 2) - indexNewReq;
		memcpy(&tempBuffer[indexReq],&szReq[indexNewReq],inputLenCrypto);
		indexReq+=inputLenCrypto;
	}
	else
	{
	LOG_PRINTF(("===============indexNewReq - %d szReq[indexNewReq] -  %04x ",indexNewReq+2,szReq[indexNewReq+2]));
	if(szReq[indexNewReq+2]>=0x80)
		Bit_len=16;
	LOG_PRINTF(("===============Bit_len [%d] ",Bit_len));
	memcpy((char*)&tempBuffer[indexReq],&szReq[indexNewReq],2+Bit_len);   // COPY Message TYPE + BITMAP
	indexReq+=Bit_len+2;
	indexNewReq += Bit_len+2;
	inputLenCrypto =  (iHeaderLen + 2) - indexNewReq;
	packData((char*)DataEncKey,(char*)enckey);//packing Data Encryption Key

	packData((char*)MASTER_KEY,(char*)packMasterKey);//packing master key

	TDES_Decrypt_Logon_key(enckey,DecDataEnckey,packMasterKey); // Decrypt Data Encryption key with Master Key

	
	TDES_CBC16((unsigned char*)&szReq[indexNewReq],inputLenCrypto,(unsigned char*)&tempBuffer[indexReq],&otputLenCrypto,iVecArr,DecDataEnckey,DES_DECRYPT);


	indexReq += otputLenCrypto;
	}
	
	memcpy(szResp,tempBuffer,indexReq);
		
	//indexNewReq+=otputLenCrypto;
		//Convert LEN in to 2BYTe Header
	indexReq-=2;
	memset(tempLen,0x00,sizeof(tempLen));
	sprintf(tempLen,"%04x",indexReq);
	packData(tempLen,(char*)&szResp[0]);
	*rcvSize = indexReq+2;
	
	return 0;
}

/*****************************************************************************************************************
*Function Name : GetEncryptedData																				                                          *
*	Purpose		   : This function encrypts the data before sending to the server							                        *
*	Input		     : request message to encrypt, length of the request,a pointer to the response buffer and  
                 length of response buffer 														                                            *
*	Output		   : Return value for the success or failure			                                                    *
******************************************************************************************************************/
static int GetEncryptedData(char *szReq,short lenRqst,char *szResp,short *rcvSize)
{
	//Purvin: FOR E2EE using DES
	unsigned char iVecArr[INIT_VECTOR_LEN+1]; //intial v	ector for Encryption (first 8 btyes of GISKe header)
	unsigned char enckey[PACK_KEY_SIZE+1];
	unsigned char DecDataEnckey[PACK_KEY_SIZE+1];
	unsigned char packMasterKey[PACK_KEY_SIZE+1];
	unsigned char tempBuffer[800];//400 to 800
	char cardAccepterId[CARD_ACCEPTOR_ID_LEN+1]={0};//16
	char terminalId[TERMINAL_ID_LEN+1]={0};//9
	char msgType[MSG_ID_LEN+1];
	int iHeaderLen = szReq[1];
	char tempLen[4];
	//( to remove warning)char temphexLen[2];
	int indexReq=0;
	int indexNewReq=0;
	int inputLenCrypto=0;
	int otputLenCrypto=0;
	char Bitmap[50];
	short Bitmap_len=8;
	
	memset(tempBuffer,0x00,sizeof(tempBuffer));
	memset(cardAccepterId,0x00,sizeof(cardAccepterId));
	memset(terminalId,0x00,sizeof(terminalId));
	memset(msgType,0x00,sizeof(msgType));
	memset(iVecArr,0,sizeof(iVecArr));
		
		
	indexReq=2+(TPDU_LEN/2);
	
	memcpy(msgType,&szReq[indexReq],MSG_ID_LEN/2);   // COPY MSG TYPE

	get_env("#TERMINAL_ID",(char*)terminalId,sizeof(terminalId)); //41

	get_env("#CARDACCEPTERID",(char*)cardAccepterId,sizeof(cardAccepterId)); // 42

  indexNewReq=2;
	//COPY TPDU

	memcpy((char*)&tempBuffer[indexNewReq],&szReq[indexNewReq],TPDU_LEN/2);	 // COPY TPDU DATA				// tpdu  copied from the sample txns provided by client
	indexNewReq+=(TPDU_LEN /2);

	/*//COPY ENCRYPTION FLAG
	if ((msgType[0]==0x08) && (msgType[1]==0x00))
	{
		memcpy((char*)&tempBuffer[indexNewReq],"  ",2);
		//return 0;
	}
	else*/

	memcpy((char*)&tempBuffer[indexNewReq],"EE",2);
			

	indexNewReq+=2;
	//COPY MERCHANT ID
	memcpy((char*)&tempBuffer[indexNewReq],cardAccepterId,CARD_ACCEPTOR_ID_LEN);
	indexNewReq+=CARD_ACCEPTOR_ID_LEN;
	//COPY Terminal ID
	memcpy((char*)&tempBuffer[indexNewReq],terminalId,TERMINAL_ID_LEN);
	indexNewReq+=TERMINAL_ID_LEN;
	//COPY Message Type
	memcpy((char*)&tempBuffer[indexNewReq],msgType,MSG_ID_LEN/2);
	indexNewReq+=(MSG_ID_LEN/2);	
	
	indexReq += 2; // Add MESSAGE ID LEN

	LOG_PRINTF(("========= indexReq [%d] indexNewReq [%d]",indexReq,indexNewReq));

	//memcpy((char*)&tempBuffer[indexNewReq],&szReq[indexReq],8); // COPY BITMAP which is Fix 8 byte in length
	//indexNewReq+=8;

	//COPY DATA Encrypted
	//indexReq += 8; // Add BIT MAP  LEN
	LOG_PRINTF(("========= indexReq [%d] indexNewReq [%d] OTHER_AC [%d]",indexReq,indexNewReq,OTHER_AC));
	if ((msgType[0]==0x08) && (msgType[1]==0x00))			//Logon Message
	{
		memcpy((char*)&tempBuffer[indexNewReq],&szReq[indexReq],8); // COPY BITMAP which is Fix 8 byte in length
		indexNewReq+=8;
		indexReq += 8;
		memcpy((char*)&tempBuffer[indexNewReq],&szReq[indexReq],16); // COPY rest of the data
		indexNewReq+=16;
	}
	else
	{
		if(szReq[indexReq]>=0x80){
			LOG_PRINTF(("True===========Bitmap [%d] d: %d - c:%c - x:%x ============",indexReq,szReq[indexReq],szReq[indexReq],szReq[indexReq]));
			Bitmap_len=16;
		}
		else
		{
			LOG_PRINTF(("False===========Bitmap [%d] d: %d - c:%c - x:%x ============",indexReq,szReq[indexReq],szReq[indexReq],szReq[indexReq]));
		}
	
		/*
		if(OTHER_AC==1 ||FROM_OTHER_AC==1 )
			Bitmap_len=16;		//Agency Bank, If Other account selected
	*/
		memcpy((char*)&tempBuffer[indexNewReq],&szReq[indexReq],Bitmap_len); // COPY BITMAP which is Fix 8 byte in length	//Agency Bank
		indexNewReq+=Bitmap_len;		//Agency bank
		indexReq += Bitmap_len;			//Agency Bank
		inputLenCrypto = (iHeaderLen + 2) - indexReq;

		packData((char*)DataEncKey,(char*)enckey);//packing Data Encryption Key
		packData((char*)MASTER_KEY,(char*)packMasterKey);//packing master key
		TDES_Decrypt_Logon_key(enckey,DecDataEnckey,packMasterKey); // Decrypt Data Encryption key with Master Key
		TDES_CBC16((unsigned char*)&szReq[indexReq],inputLenCrypto,&tempBuffer[indexNewReq],&otputLenCrypto,iVecArr,DecDataEnckey,DES_ENCRYPT);
		
		indexNewReq+=otputLenCrypto;
	}
	OTHER_AC=0;
  //Convert LEN in to 2BYTe Header
	memset(tempLen,0x00,sizeof(tempLen));
	sprintf(tempLen,"%4x",indexNewReq-2);
	
	packData(tempLen,(char*)&szResp[0]);

	memcpy(&szResp[2],&tempBuffer[2],indexNewReq-2);

	*rcvSize = indexNewReq;

	return 0;
}


/*****************************************************************************************************************
*Function Name : CommWithServer																				                                            *
*	Purpose		   : This function try to communicate with Server(maximum three times)							                  *
*	Input		     : message to send, response(output),size														                                *
*	Output		   : Return value for the success or failure			                                                    *
******************************************************************************************************************/
int CommWithServer(char *szReq, short lenRqst, char *szResp, short *rcvSize)
{
	int retVal = -1;
	unsigned char tempBuffer[800];
	unsigned char tempBufferdisp[800];//500 to 800 by (only for display)
	short len = 0;

	memset(tempBuffer, 0x00, sizeof(tempBuffer));

	if (LOG_STATUS == LOG_ENABLE)
	{
		LOG_PRINTF(("in the CommWithServer"));
	}
	//Suben no Encryption - uncommented below line and commented encryption block 
	retVal = StartCommunication(szReq, lenRqst, szResp, rcvSize);
	/*
	GetEncryptedData(szReq,lenRqst,(char*) &tempBuffer[0], &len);
	//	memcpy(tempBuffer,szResp,*rcvSize);

	LOG_PRINTF(("------------------------------------------------------len =%d",len));
	//
	bcd2a((char *)tempBufferdisp, (char *)tempBuffer,(short)len);
	LOG_PRINTF(("CommWithServer:After Encryption  %d and Data [%s]",len,tempBufferdisp));

	retVal = StartCommunication((char*)tempBuffer,len, szResp, rcvSize);
	*/
	if (retVal != _SUCCESS)
		return retVal;

	LOG_PRINTF(("StartCommunication:retVal [%d]", retVal));
	//Suben no Encryption - commented encryption block
	/*
	bcd2a((char *)tempBufferdisp, (char *)szResp, *rcvSize);
	LOG_PRINTF(("CommWithServer:Encrypted Response and Data [%s]",tempBufferdisp));
	LOG_PRINTF(("----------------------------------------------------------------"));

	memset(tempBuffer,0x00,sizeof(tempBuffer));
	memcpy(tempBuffer,szResp,*rcvSize);
	//len =(short) rcvSize;
	GetDecryptedData((char*)tempBuffer,*rcvSize,(char*)szResp,rcvSize);

	LOG_PRINTF(("----------------------------------------------------------------"));
	*/
	bcd2a((char *)tempBuffer, (char *)szResp, *rcvSize);
	LOG_PRINTF(("CommWithServer:Decrypted Response and Data [%s]", tempBuffer));

	return retVal;
}



/****************************************************************************************************************
*	Function Name : StartCommunication																			*
*	Purpose		  : To Create Socket and send data to Server and get response from server						*
*	Input		  : 1.message to send, response,size															*
*	Output		  : Return value for the success or failure														*
*****************************************************************************************************************/
int StartCommunication(char *Rqst,short lenRqst,char *Rsp,short *RvcSize)
{
	struct sockaddr_in	remote;
	int		hSocket = -1;
	int		shortRetval = -1;
	unsigned char hostip[IP_SIZE];
	unsigned char Company_Name[COMPANY_NAME_LEN]={0};
	char port[PORT_SIZE];
	 //char temp1Buffer[800];
	 char temp1Buff[800];
	struct timeval tv;
  
	tv.tv_sec = 20;  /* 20 Secs Timeout */
	tv.tv_usec = 0;  // Not init'ing this can cause strange errors

	window(1,1,30,20);
	clrscr();
	memset(hostip,0,sizeof(hostip));
	memset(port,0,sizeof(port));
	
	window(1,1,30,8);
	get_env("#COMPANYNAME",(char*)Company_Name,sizeof(Company_Name));
	write_at((char*)Company_Name, strlen((char*)Company_Name),(30-strlen((char*)Company_Name))/2, 2);
  
	SVC_WAIT(500);
	ClearKbdBuff();
	KBD_FLUSH();
        
	get_env("#HOSTIP",(char*)hostip,sizeof(hostip));
	get_env("#PORT",port,sizeof(port));
	if(LOG_STATUS == LOG_ENABLE)
	{
	  LOG_PRINTF(("hostip = %s",hostip));
	  LOG_PRINTF(("port = %s",port));
	}

	remote.sin_addr.s_addr = inet_addr((const char *)hostip);
	remote.sin_family = AF_INET;
	remote.sin_port = htons(atoi(port));
	window(1,2,30,20);
	clrscr();

	if((hSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{
		if(LOG_STATUS == LOG_ENABLE)
		{
		  LOG_PRINTF(("Socket Creation Failed: SOCKET_HANDLER = %d", hSocket));
		}
		return SOCKET_FAILED;
	}
	else
	{
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Socket Creation Success"));
		}
	}
	
	write_at(Dmsg[CONNECTING].dispMsg, strlen(Dmsg[CONNECTING].dispMsg),Dmsg[CONNECTING].x_cor, Dmsg[CONNECTING].y_cor);
	shortRetval = connect(hSocket , (struct sockaddr *)&remote , sizeof(struct sockaddr_in));
	if(shortRetval < 0)
	{
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Socket Connection Failed: RET_VALUE = %d", shortRetval));
			LOG_PRINTF(("errno = %d.\n", errno));
			LOG_PRINTF(("reason in connection = %s.\n", strerror(errno)));
		}
		write_at(Dmsg[CONNECTION_FAIL].dispMsg, strlen(Dmsg[CONNECTION_FAIL].dispMsg),Dmsg[CONNECTION_FAIL].x_cor, Dmsg[CONNECTION_FAIL].y_cor);
		error_tone();
		SVC_WAIT(1000);
		ClearKbdBuff();
		KBD_FLUSH();
        close(hSocket);
		return CONNET_FAILED;
	}
	else
	{
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Connected !!!"));
		}
		SVC_WAIT(100);
		ClearKbdBuff();
		KBD_FLUSH();   
	}
	clrscr();
	write_at(Dmsg[SENDING_MESSAGE].dispMsg, strlen(Dmsg[SENDING_MESSAGE].dispMsg),Dmsg[SENDING_MESSAGE].x_cor, Dmsg[SENDING_MESSAGE].y_cor);
  	SVC_WAIT(500);
	ClearKbdBuff();
	KBD_FLUSH();
	LOG_PRINTF(("Request message [%s]", Rqst));
    shortRetval = send(hSocket , Rqst , lenRqst , 0);
	if(shortRetval<0)
	{
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Socket Request Send Failed: RET_VALUE = %d", shortRetval));
			LOG_PRINTF(("errno = %d.\n", errno));
			LOG_PRINTF(("reason in send = %s.\n", strerror(errno)));
		}
		clrscr();
		write_at(Dmsg[SENDING_FAIL].dispMsg, strlen(Dmsg[SENDING_FAIL].dispMsg),Dmsg[SENDING_FAIL].x_cor, Dmsg[SENDING_FAIL].y_cor);
		error_tone();
		SVC_WAIT(1000);
		ClearKbdBuff();
		KBD_FLUSH();
		close(hSocket);
		return SEND_FAILED;
	}
	else
	{
		clrscr();
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Request Send Success length = %d",lenRqst));
		}
	}
	clrscr();
	write_at(Dmsg[RECEIVING_MESSAGE].dispMsg, strlen(Dmsg[RECEIVING_MESSAGE].dispMsg),Dmsg[RECEIVING_MESSAGE].x_cor, Dmsg[RECEIVING_MESSAGE].y_cor);
	setsockopt(hSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));		
	shortRetval = recv(hSocket, Rsp , *RvcSize , 0);
	bcd2a((char *)temp1Buffer, (char *)Rsp,shortRetval);
	LOG_PRINTF(("Test Balance Inq response [%s]",temp1Buffer));
	LOG_PRINTF(("Socket REsponse RET_VALUE = %d", shortRetval));
	SVC_WAIT(2000);
	if(shortRetval <= 0)
	{
		clrscr();
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Socket REsponse Receive Failed: RET_VALUE = %d", shortRetval));
		}
		write_at(Dmsg[RECEIVING_FAIL].dispMsg, strlen(Dmsg[RECEIVING_FAIL].dispMsg),Dmsg[RECEIVING_FAIL].x_cor, Dmsg[RECEIVING_FAIL].y_cor);
   		error_tone();
		SVC_WAIT(1000);
		ClearKbdBuff();
		KBD_FLUSH();
        close(hSocket);
		return RECV_FAILED;
	}
	else
	{
		clrscr();
		write_at(Dmsg[MESSAGE_RECEIVED].dispMsg, strlen(Dmsg[MESSAGE_RECEIVED].dispMsg),Dmsg[MESSAGE_RECEIVED].x_cor, Dmsg[MESSAGE_RECEIVED].y_cor);
   		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Recieve Message length = %d", shortRetval));
		}
		*RvcSize = shortRetval ;
	}
	close(hSocket);
	return _SUCCESS;
}
