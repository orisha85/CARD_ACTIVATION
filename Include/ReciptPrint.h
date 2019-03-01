#ifndef	RECIPTPRINT_H
#define RECIPTPRINT_H

#include <string.h>
#include <printer.h>
#include "..\Include\Common.h"

extern short hPrinter;

//Macro Constants 
#define PRINT_NORMAL				1
#define PRINT_BOLD                  2
#define CENTER_BOLD					3
#define PRINTER_WIDTH               42
#define PRINTER_WIDTH_BOLD          21
#define PRINT_NEWLINE               "\n"
#define PRINT_SPACE                 " " 
#define LOCAL_PRINTER_BUFFER_SIZE   300
#define PRINTER_OPEN_WAIT           200
#define PRINTER_INIT_WAIT           100


//Printer status error codes:
#define NO_STATUS												-4
#define PAPER_LOW												-5
#define RAM_ERROR												-10
#define PRINTER_FAILURE									-20
#define PAPER_OUT												-21//if paper is not present
#define PRINTER_ESC_SEQUENCE_NOT_FOUND	-23
#define PRINTER_NOT_INITIALIZED					-24
#define PRINTER_FIRMWARE_CORRUPTED			-27


#define PRINT_BUF_SIZE									200
#define MERCHANT_NAME_SIZE							50
#define MERCHANT_ADDRESS_SIZE						40

//printing Message
#define _NEW_LINE												"\n"
#define _AMOUNT													"AMOUNT:"
#define MERCHANTID											"MID:"
#define BATCH_ID											"BATCH No:"
#define TERMINALID											"TID:"
#define USD															"USD"
#define CREATE_LINE											"_____________________"
//#define ANOTHER_COPY										"Press green key for merchant copy"
#define CLIENT_RCPT_TO_KEEP							"Client receipt"
#define MERCHANT_RCPT							"Agent receipt"
#define THANK_YOU												"...Thank You..."


//Response code message
#define APPROVED_MSG							"Approved" //Normal no error
#define UNAPPROVED_MSG							"Unapproved" //Saved for reversal
#define REFER_TO_CARD_ISSUER_MSG				"Refer to Card Issue" //Refer to Card Issuer
#define INVALID_MERCHANT_MSG						"Invalid Merchant" //Invalid Merchant
#define DO_NOT_HONOR_MSG								"Do Not Honor" //Do Not Honor
#define HONOR_WITH_ID_MSG								"Honor with I.D. Only" //Honor with I.D. Only
#define INVALID_MESSAGE_MSG							"Invalid Message" //Invalid Message
#define INVALID_DOLLOR_AMOUNT_MSG				"Invalid Dollar Amount" //Invalid Dollar Amount
#define INVALID_CARD_NUMBER_MSG  				"Invalid Card Number" //Invalid Card Number
#define NO_ACTION_MSG					"No Action Taken" //No Action Taken
#define NO_FILE_UPDATE_ALLOW_MSG					"No File Update Allowed" //No File Update Allowed
#define RECORD_NOT_FOUND_MSG         "Record Not Found in File" //Record Not Found in File
#define DUPLICATE_RECORD_FILE_MSG              "Duplicate Record on File" //Duplicate Record on File
#define RECORD_FILE_LOCKED_MSG						"Record or File Locked" //Record or File Locked
#define FILE_UPDATE_DENIED_MSG					"File Update Denied" //File Update Denied
#define FORMAT_ERROR_MSG         "Format Error" //Format Error
#define INVALID_ACQIUIRER_MSG              "Invalid Acquirer Institution" //Invalid Acquirer Institution
#define CARD_EXPIRED_MSG						"Card Expired" //Card Expired
#define CAPTURE_CARD_MSG						"PIN Retries Exceeded" //Capture Card – PIN Retries Exceeded
#define NO_CREDIT_ACCOUNT_MSG					"No Credit Account" //No Credit Account
#define INVALID_FUNCTION_MSG         "Invalid Function" //Invalid Function
#define LOST_CARD_MSG              "Lost Card" //Lost Card
#define CARD_IS_STOLEN_MSG						"Card is Hot/Stolen" //Card is Hot/Stolen
#define INSUFFICIENT_FUNDS_MSG					"Insufficient Funds" //Insufficient Funds
#define CARD_EXPIRED_MSG         "Card Expired" //Card Expired – Do not pickup
#define INVALID_PIN_MSG              "Invalid PIN" //Invalid PIN – Retry
#define CARD_HOLDER_NOT_ONFILE_MSG              "Card Holder not on File" //Card Holder not on File
#define TRANSACTION_NOT_ALLOWED_MSG						"Transaction not allowed for Card" //Transaction not allowed for Card
#define EXCEEDS_WITHDRWAL_LIMIT_MSG					"Exceeds Withdrawal Limit" //Exceeds Withdrawal Limit
#define RESTRICTED_CARD_MSG         "Restricted Card" //Restricted Card
#define INVALID_MAC_MSG              "Invalid MAC" //Invalid MAC


#define PRINTER_STATUS          1000
//----------------------------------
#define ALWAYS_TRUE				1
#define NULL_CHAR				'\0'

#define EXPIRED_CARD			-3 
#define INVALID_CARD			-1
#define CARD_NOT_FOUND			-1

#define SOFT_VER_LEN 9
#define TIME_BUFFER_LEN 16
#define ACC_NUMBER_LEN 22
#define MER_ID_LEN 9
#define PRINT_EXPDATE 6
//---------------------------------
#define NEGATIVE_INITIALIZE		-1
#define ZERO					0
#define ONE						1
#define TWO						2	
#define THREE					3	
#define FOUR					4	
#define FIVE					5
#define SIX						6
#define SEVEN					7 
#define EIGHT					8
#define NINE					9 
#define TEN						10
#define ELEVEN					11
#define SIXTEEN					16
#define TWENTY					20
#define FIFTY 50
#define HUNDRED 100
///////////////////////////
typedef struct
{
	//char SaleAmount[13];
	//char WithdrawAmount[13];
	//char RefundAmount[13];
	//char DepositAmount[13];
	//char TransferAmount[13];
    //char VoidAmount[13];
	//char DebitAmount[13];
	//char CreditdAmount[13];
    //char SumOfTrAmount[13];
    // char SumOfDebCredAmount[13];
   
    //short SalesCount;
	//short WithdrawCount;
	//short RefundCount;
	//short DepositCount;
	//short TransferCount;
    //short VoidCount;
   	//short DebitCount;
    //short CreditCount;
    //short SumTransCount;
    //short SumDebitCreditCount;
	short activation_approved;
	short activation_declined;
	short activation_total;
	short reset_approved;
	short reset_declined;
	short reset_total;
	short change_approved;
	short change_declined;
	short change_total;
	short approved_total;
	short decline_total;
	short total_txns;
}TotalReportDetails;

short print(char *);
short printRecipt(TransactionMsgStruc *);
short TotalReportReciept(char *);
short TransactionDetailsReciept(void);
short printReciptForTransDetails(TrDetails *,char *,char*);
short RetriveDataForTransDetailsReciept(TrDetails *,TotalReportDetails *);
short RetriveDataForTotalReportReciept(TrDetails *transDetails,TotalReportDetails *settlDetails);
short Paperstatus(void);
short ClearLogs();
short printUserAudit();
#endif
