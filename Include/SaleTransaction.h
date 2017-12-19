#ifndef _SALE_TRANS_
#define _SALE_TRANS_
#include "..\Include\Common.h"


#define CARD_SIZE 230
//#define BITMAP_ARRAY_SIZE 8
#define BITMAP_ARRAY_SIZE 16		//Agency Bank

//#define UNPACK_BITMAP_ARRAY_SIZE 16
#define UNPACK_BITMAP_ARRAY_SIZE 32		//Agency Bank
//#define BITMAP_SIZE 64
#define BITMAP_SIZE 64	//Agency Bank

//#define PAYMENT_REQUEST_SIZE 96
#define PAYMENT_REQUEST_SIZE 200	//108
#define PAYMENT_RESPONSE_SIZE 200
//#define PAYMENT_RESPONSE_SIZE 512
#define PAYMENT_UNAPACK_RES_SIZE 400


int packData(char *src,char *dst);
short getTransNo(TransactionMsgStruc *);
short getPaymentId(TransactionMsgStruc *);
short getAccountNo(TransactionMsgStruc *);
void  responseErrorReason(TransactionMsgStruc *transMsg);
int swipeOrEMVCardEntry(TransactionMsgStruc *);
int swipeCardEntry(TransactionMsgStruc *transMsg);
int SavingAccount(void);
int CreditAccount(void);
int ChequeAccount(void);

int GetInputFromUser(TransactionMsgStruc *,int);
short readMagCard(TransactionMsgStruc *);
short getCardNumber(TransactionMsgStruc *,char);
void showCardData(TransactionMsgStruc *);
short InitSaleTransaction(TransactionMsgStruc *,short,short);
short SaleTransactionProcessing(TransactionMsgStruc *);
short PaymentTransactionProcessing(TransactionMsgStruc *);
void resetBitmapStructure(BmapStruct *ptrBitmapStructOb);

void display_TVR_TSI(TransactionMsgStruc *transMsg);//Added for EMV
void CloseEMVSLot(void);
int CheckServiceCode(char *track2data);
int EMVCardEntry(TransactionMsgStruc *transMsg);
#endif
