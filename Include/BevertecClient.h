/***********************************************************************************************************************
   File			: COMMUNICATION.H
   Project		: ParadigmServices-FI 
   Module		: BankTech RETAIL
   Description	: This file contains communication specific function declaration.
***********************************************************************************************************************/

#ifndef BEVERTECCLIENT_H
#define BEVERTECCLIENT_H

#define SSL_LOAD_SUCCESS		1
#define _SUCCESS				0
#define	SOCKET_FAILED			-1
#define	CONNET_FAILED			-2
#define	SEND_FAILED				-3
#define	RECV_FAILED				-4
#define SSL_LOAD_ERROR			-5
#define INIT_CTX_ERROR			-6
#define SSL_CONNECT_ERROR		-7
#define CONN_TIMEOUT_ERROR		-8
#define VCS_SOCKET_ERROR		-9
#define VCS_SSL_ERROR			-10

#define RECOMMENDED_HEAP 90000
#define RECOMMENDED_STACK 60000
#define RECOMMENDED_RAM 2048
#define RECOMMENDED_FLASH 2048
#define IP_SIZE	50
#define PORT_SIZE 8

//#define SENDING_MSG "Sending Message ..."
//#define RECEIVING_MSG "Receiving Message ..."
//#define MESSAGE_RECEIVED "Message Received"
//SOCKET Communication
int CommWithServer(char *,short ,char *,short *);

short SendRecvEther(char *reqBuff, char *resBuff,short rcvSize );
int StartCommunication(char *Rqst,short lenRqst,char *Rsp,short *RvcSize);

int ComWithSsl(char *Rqst, char *Rsp, short RvcSize);
int DoSendReceive(char *szSendBuff, char *szRecvBuff, short rcvSize);

void CloseSocket(short);
void isServerNotPresent(void);
int CheckMemoryUsage(void);

#endif //BEVERTECCLIENT_H

