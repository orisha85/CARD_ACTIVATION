#ifndef TRANSACTIONS_H
#define TRANSACTIONS_H

#include "..\Include\Common.h"


#define MAX_TRACE_AUDIT_NO 999999 //MAXIMUM LIMIT FOR TRACE AUDIT NO 
#define MAX_BATCH_NO 999999 //MAXIMUM LIMIT FOR BATCH NO      
//MAXIMUM LIMIT FOR BATCH NO 
//FOR VOID
#define VOID_REQUEST_SIZE 108
#define VOID_RESPONSE_SIZE 200
#define VOID_UNAPACK_RES_SIZE 400
//FOR REFUND
#define REFUND_REQUEST_SIZE 96
#define REFUND_RESPONSE_SIZE 200
#define REFUND_UNAPACK_RES_SIZE 400
//FOR BALANCE ENQUIRY
#define BALANCE_ENQ_REQUEST_SIZE 200	//108
#define BALANCE_ENQ_RESPONSE_SIZE 400	//200
#define BALANCE_ENQ_UNAPACK_RES_SIZE 400


short InitBalanceInquiry(TransactionMsgStruc *);
short InitVoid(TransactionMsgStruc *);
short InitRefund(TransactionMsgStruc *);
short InitDeposit(TransactionMsgStruc *);
short InitBalance(TransactionMsgStruc *);
short InitTransfer(TransactionMsgStruc *);
short InitPINValidate(TransactionMsgStruc *);
short debitAccountType(TransactionMsgStruc *,int);
short BalanceEnquiryProcessing(TransactionMsgStruc *) ;
short RefundTransactionProcessing(TransactionMsgStruc *);
short BalanceTransactionProcessing(TransactionMsgStruc *);
short TransferTransactionProcessing(TransactionMsgStruc *);
short ProcessingVoidTransaction(TransactionMsgStruc *);
short PINValidateProcessing(TransactionMsgStruc *);
short InitCardActivation(TransactionMsgStruc *transMsg);
short InitPinChange(TransactionMsgStruc *transMsg);
short CardActivationTransactionProcessing(TransactionMsgStruc *transMsg);
short PinChangeTransactionProcessing(TransactionMsgStruc *transMsg);

#endif
