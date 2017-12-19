#ifndef SATTLEMENT_H
#define SATTLEMENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\Include\Common.h"
#include "..\Include\SaleTransaction.h"
#include "..\Include\TouchScreen.h"
#include "..\Include\ReciptPrint.h"


#define TRANS_DETAILS_FILE	"F:15/TransactionDetails.txt"
#define TMP_TRANS_DETAILS_FILE	"TmpTransactionDetails.txt"
#define LOG_FILE	"F:15/Log.txt"

//FOR SETLLEMENT
#define SETLLEMENT_REQUEST_SIZE 122
#define SETLLEMENT_RESPONSE_SIZE 200 //200 TO 120
#define SETLLEMENT_UNAPACK_RES_SIZE 400
//FOR BATCH UPLOAD
#define BATCH_UPLOAD_REQUEST_SIZE 122
#define BATCH_UPLOAD_RESPONSE_SIZE 200 //200 TO 120
#define BATCH_UPLOAD_UNAPACK_RES_SIZE 400
//FOR SETLLEMENT TRAILOR
#define SETLLEMENT_TRAILOR_REQUEST_SIZE 122
#define SETLLEMENT_TRAILOR_RESPONSE_SIZE 200//200 TO 120
#define SETLLEMENT_TRAILOR_UNAPACK_RES_SIZE 400




#define FOUR 4
#define FIVE 5
#define SEVEN 7
#define NINE 9
#define TEN 10
#define THIRTEEN 13
#define SIXTEEN 16


typedef struct
{
		char creditSaleAmount[13];//for credit tranx
		char creditRefundAmount[13];
		short creditSalesCount;
		short creditrefundCount;


		char saleAmount[13];//for debit tranx
		char withdrawAmount[13];
		char depositAmount[13];
		char transferAmount[13];
		char refundAmount[13];
		short saleCount;
		short withdrawCount;
		short depositCount;
		short transferCount;
		short refundCount;

}StlDetails;

short InitSettlement(TransactionMsgStruc *transMsg);
short processingSettlement(TransactionMsgStruc *);
short SaveTransDetails(TransactionMsgStruc *);
short deleteRecordFromFile(TransactionMsgStruc *);
short ReadTransDetails(TransactionMsgStruc *);
short ReadLastTransDetails(TransactionMsgStruc *);
short printAnotherCopy(TransactionMsgStruc *);
short RetriveDataForSettlement(TransactionMsgStruc *transMsg);
short paddCount(short data,char *Res);

short processingSettlementTrailer(char *);
short processingBatchUpload(void);

void ResetVariablesForBatchUpload(TrDetails *ptrTransDetails,int size_TransDetails,BmapStruct *ptrbmpstruct,unsigned char *testbuff,int size_testBuff,char *tempbuff,char *BitmapBuff,char *UnpackBitmapBuff,unsigned char *reqBuff,int size_reqBuff,unsigned char *resBuff,int size_resBuff);
short CleanFileData(void);

//For saving log in release mode
//short SaveLOG(char *ptr,int size);

#endif

