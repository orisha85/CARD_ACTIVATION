#ifndef LOGON_H
#define LOGON_H

#include "..\Include\Common.h"

//FOR LOGON
#define LOGON_REQUEST_SIZE 96
#define LOGON_RESPONSE_SIZE 200 
#define LOGON_UNAPACK_RES_SIZE 400


//FOR REVERSAL
#define REVERSAL_REQUEST_SIZE 108
#define REVERSAL_RESPONSE_SIZE 200 
#define REVERSAL_UNAPACK_RES_SIZE 400

typedef struct parsedLogonResponse
{
	char responseCode[5];
  char TerPrivateKey[32];
}parsedLogonRes;

short InitLogon(TransactionMsgStruc *);
short InitReversal(void);
short LogonProcessing(TransactionMsgStruc *) ;
short ReversalProcessing(TransactionMsgStruc *);
short ReadReversalDetails(TransactionMsgStruc *) ;
short SaveReversalDetails(TransactionMsgStruc *);
#endif
