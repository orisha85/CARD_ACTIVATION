
#ifndef EMV_H
#define EMV_H
#include "..\Include\Common.h"

#define SALE					0x00
#define WITHDRAWAL				0x01
#define BAL_INQUIRY				0x31
#define SALE_WITH_CASHBACK		0x09
#define VOID					0x02
#define REFUND					0x04
#define DEPOSIT					0x21
#define TRANSFER				0x40
#define PIN_VALIDATE			0x61
#define CARD_ACTIVATE			0x90
#define PIN_CHANGE				0x91


#define APPL_NOT_AVALBL			0x6985
#define EMV_FAILED_V400			0x9481
#define ASCII_RID_LEN			10

#define TAG8E00_SIZE			64  
#define TAG9500_SIZE			7

#define MSG_YES                 56
#define MSG_NO	                57
#define MSG_FORCE_ONLINE        68
#define EMVMODULE_RET_OK		0
#define EMVMODULE_RET_NOT_OK	-1
#define BYTE2048       2048

#define FALLBACK				15 

#define AAC 0x00
#define TC 0x40
#define ARQC 0x80
#define TAG_9F5B_ISSUER_SCRIPT_RESULTS 0x9f5b
#define MAX_DE55_SIZE 1024

#define EMV_PROCESSING			"EMV Processing..."


///////////For lang pref/////////////////////////

#define INVALID_MSG_FILE   299
//Interac specific changes are done for language selection
//by introducing support for 3rd language

#ifdef __thumb
#define MAX_DISP_MSG_LENGTH   24
#endif

#ifdef  _TARG_68000

 #define MAX_DISP_MSG_LENGTH   22

#endif

typedef struct TagMSG_REC
{
    char szDisplayMessages[MAX_DISP_MSG_LENGTH];		 
}MSG_REC;



////////////////////////////
typedef struct sEMV_AUTH_RESP sEMV_AUTH_RESP;
typedef struct sEMV_DATA sEMV_DATA;
/*  ******************************************************************************************
	  Structure for storing EMV transaction Data
    ******************************************************************************************/
	struct sEMV_DATA
	{
		char ApplicationId[15];
		char TransactionAmount[15];
		char DE55[BYTE2048];
		int  De55len;
		int  Track2Len;
		char Track2[40];
    //char Track2[TRACK2_DATA_LEN+1];
		int  Track1Len;
		char Track1[50];
    char TVR[_TVR_SIZE+1];// 
    char TSI[_TSI_SIZE+1];// 
    char CardType[CARD_TYPE_SIZE+1];// 
		//char Track1[TRACK1_DATA_LEN+1];
    char PinBlock[50];
		char IsFallback;
		char TxnStatus;
		unsigned long  HostTxnType;
		unsigned long  DeviceTxnType;
		unsigned char accountType;
		char m_chArrCardHolderName[100];
    char VerificationMethod[50];// 
	};

//EMV RESPOSE DATA 
struct sEMV_AUTH_RESP{
	char AuthResponseCode[10];
	int  AuthResponseCodeLen;
	char IssuerAuthData[500];
	int  IssuerAuthDataLen;
	char IssuerScript71[500];
	int  IssuerScript71Len;
	char IssuerScript72[500];
	int  IssuerScript72Len;
};


short StartEMV(void);
void vSetDefaultFunctionPointers(void);

int inSetTransactionType(int TxnType);
int inRunEMVTransaction(int TxnType);
void parseSerialBuf(char *);
void initiateFallbackTxn(void);
int ValidateICCCard(TransactionMsgStruc *,sEMV_DATA * emvdata);
int emvDataAuthentication(void);
int emvPRandTRM(void);
int emvCardholderVerification(void);
int emvFirstGenerateAC(void);
int inGetTerminalDecision(void);
int  EMVModuleOnlineAuthorization(sEMV_DATA * emvdata);
int BuildDE55( sEMV_DATA * emv_data);
short emvFirstGenerationFlow(TransactionMsgStruc *transMsg);
int ParseAuthResponseDe55(unsigned char * emv_data,sEMV_AUTH_RESP * sAuthRespn);
int Scriptprocessing( char*  /*chnged short to char* */, sEMV_AUTH_RESP * sAuthResp);
int emvUseHostDataFlow(char*);
int createScriptBuffers(byte *scriptBuf, int iLen, short iType);


short vdSelPreferredLang(void);
int LangMenuFunc(char *labels, int usLen);
short LoadMVTFunction(void);
#endif //EMV_H
