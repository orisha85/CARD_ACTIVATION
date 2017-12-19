#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <string.h>
#include <svc.h>
#include <svctxo.h>

//Include file for ACT2000
#include <applidl.h>
#include <acldev.h>
#include <aclstr.h>
#include <message.h>
#include <printer.h>

//Include file for EESL
#include "logsys.h"
#include "varrec.h"
#include "eeslapi.h"

//Include file for Device Manager
#include "devman.h"

#define SUP_CARD 1
#define AGENCY_BANK 1
//#define GOVT_APP 1		//Agency Bank
#define LOG_ENABLE 1
#define EMV_ENABLE 1

//int revstat=0;

#define LOG_STATUS 1 //1 FOR ON AND 0 FOR OFF
//#define BITMAP_ARRAY_SIZE 8
#define BITMAP_ARRAY_SIZE 16

#define MSG_CONFIG_FILE	"F:15/ConfiguredMessage.txt"
#define REVARSAL_DETAILS_FILE	"F:15/ReversalDetails.txt"


#define TPDU "60000103e8"
#define POS_ENTRY_MODE "021"
#define NETWORK_INTER_ID "000"
#define POS_COND_CODE "14"
#define TRACK2_DATA "4100780001164423=17041010873013000200"

#define DEFAULT_BUFLEN 512 
#define COMPANY_NAME_LEN 30
#define DISP_TEXT_BUFFER_LEN 50
#define RES_TEXT_BUFFER_LEN 50
#define BUFFER_SIZE 1024 
#define ASSEMBLE_BUFFER_SIZE 400
#define SOURCE_BUFFER_SIZE 100
#define DEST_BUFFER_SIZE 100
#define DISCARD_BUFFER_SIZE 200
#define _SUCCESS 0
#define _FAIL -1
#define BTN_CANCEL -3
#define FORCE_FOR_EMV -200
#define DEBIT_CARD_TYPE 1
#define CREDIT_CARD_TYPE 2
#define LOYALITY_CARD_TYPE 3
#define TRANSAMOUNT 4
#define CASHBACK_AMOUNT 5 
#define TIME_OUT										"TIME OUT"
#define DISP_ARR_SIZE 60 //size of array which store display message. 
#define MAX_AMT_DIGIT 99999999
////////////////////////////////////////////////////
#define HEX_RET_REF_ID_LEN 24 
#define HEX_AUTH_ID_LEN 12
#define HEX_TER_ID_LEN 16
#define HEX_RES_CODE_LEN 4 
//////////////////////////////////////////
#define TPDU_LEN 10
#define PAYMENT_ID_LEN 9
#define ACCOUNT_NO_LEN 12		//Agency Bank Account No length 12
#define AMOUNT_LEN 12
#define RETRIEVAL_REF_LEN 12
#define SECURITY_CONTROL_LEN 16
#define TERMINAL_ID_LEN 8
#define BIANRY_PIN_LEN 8
#define KSN_LENGTH 23   //Length + Tag 33 + KSN Value
#define POS_ENTRY_LEN 3
#define NII_LEN 3
#define POS_COND_LEN 2
#define CARD_ACCEPTOR_ID_LEN 15
#define CARD_HOLDER_NAME_LEN 50
#define TRANS_TYPE_STRING 20
#define TRANSACTION_ID_LEN 12
#define AUTH_ID_LEN 6
#define RESPONSE_CODE_LEN 2
#define TRACK2_DATA_LEN 37
#define TRACK1_DATA_LEN 76
#define TIME_ARR_LEN 4
#define ACC_NUM_ARR_LEN 19
#define TRANS_LOCALTIME_LEN 6
#define TRANS_LOCALDATE_LEN 4
#define EXPTIME_ARR_LEN 4
#define SERVICE_CODE_LEN 3
#define MSG_ID_LEN 4
#define PROC_CODE_LEN 6
#define TRACE_AUDIT_LEN 6
#define BATCH_ID_LEN 6
#define UNPACK_ENC_PR_KEY_LEN 32
#define UNPACK_ENC_PR_KCV_LEN 6
#define GISKE_KEY_LEN 120
#define RECONCILLATION_REQ_LEN 60 
//#define BAL_INQ_AMT_RES_LEN 120 
#define BAL_INQ_AMT_RES_LEN 240 
#define ICC_RELATED_DATA_LEN 151
#define UNPACK_ICC_RELATED_DATA_LEN 302
#define MAC_64_LEN 8 
#define _TVR_SIZE 10
#define _TSI_SIZE 4
#define CARD_TYPE_SIZE 20
//#define FROM_AC_LEN 19		//Agency Bank
///////////////////////////////

#define NEGATIVE_INITIALIZE		-1
#define UNPACK_SIZE_MSGLEN_OF_REQ_RES 4
#define BALANCE_AMOUNT_RES_LEN 84
#define HEADER_LENGTH_BEFORE_MK 56
#define PAD_MAC_LENGTH_AFTER_MK 32

//MessageTypes////////

#define MSGTYPE						"0200"
#define BALINQMSGTYPE								"0200"
#define SALEMSGTYPE									"0200"
#define CASHMSGTYPE									"0200"
#define VOIDMSGTYPE									"0200"
#define REFUNDMSGTYPE								"0200"
#define PINCHANGEMSGTYPE						"0100"
#define ACTIVATIONMSGTYPE						"0100"
#define MOBILETOPUPMSGTYPE					"0200"
#define SETTLEMENT_MSG_TYPE         "0500" 
#define BATCH_UPLOAD_MSG_TYPE       "0320" 
#define LOGON_MSG_TYPE              "0800" 
#define REVERSAL_MSG_TYPE           "0400" 


//MessageTypes for create request stream for bitmap////////
#define AUTHMSGTYPE_CASE									1
#define SALEMSGTYPE_CASE									2
#define BALINQMSGTYPE_CASE								3
#define VOIDMSGTYPE_CASE									4
#define REFUNDMSGTYPE_CASE								5
#define PINCHANGEMSGTYPE_CASE						  6
#define ACTIVATIONMSGTYPE_CASE						7
#define MOBILETOPUPMSGTYPE_CASE					  8
#define SETTLEMENT_MSG_TYPE_CASE          9 
#define LOGON_MSG_TYPE_CASE               10
#define REVERSAL_MSG_TYPE_CASE            11
#define WITHDRAWAL_MSG_TYPE_CASE		12
#define DEPOSIT_MSG_TYPE_CASE		14
#define TRANSFER_MSG_TYPE_CASE		15
#define PIN_VALIDATE_MSG_TYPE_CASE	16


//Processing codes for Saving ////////
#define SAVE_AUTHPROCODE									"001000"
#define SAVE_BALINQPROCODE								"311000"
#define SAVE_SALEPROCODE									"001000"
#define SAVE_WITHDRAWPROCODE							"011000"		//Agency Bank
//#define SAVE_SALEPROCODE									"011000"	//Agency Bank
#define SAVE_CASHPROCODE									"011000"
#define SAVE_VOIDPROCODE									"021000"
#define SAVE_REFUNDPROCODE								"200010"
#define SAVE_DEPOSITPROCODE								"210010"		//Agency Bank
#define SAVE_PINCHANGEPROCODE						  "910000"
#define SAVE_ACTIVATIONPROCODE						"920000"
#define SAVE_MOBILETOPUPPROCODE					  "810000"
#define SETTLEMENTPROCODE                 "920000"
#define SETTLEMENT_TRAILER_PROCODE        "960000"
#define LOGONPROCODE                      "920000"

//Processing codes for Checking ////////
#define CHECK_AUTHPROCODE									"002000"
#define CHECK_BALINQPROCODE								"312000"
#define CHECK_SALEPROCODE									"002000"
#define CHECK_WITHDRAWPROCODE									"012000"		//Agency Bank
#define CHECK_CASHPROCODE									"012000"
#define CHECK_VOIDPROCODE									"022000"
#define CHECK_REFUNDPROCODE								"200020"
#define CHECK_DEPOSITPROCODE								"210020"		//Agency Bank
#define CHECK_PINCHANGEPROCODE						"910000"
#define CHECK_ACTIVATIONPROCODE						"900000"
#define CHECK_MOBILETOPUPPROCODE					"810000"
#define CHECK_SETTLEMENTPROCODE           "920000"

//Processing codes for Credit ////////
#define CREDIT_AUTHPROCODE									"003000"
#define CREDIT_BALINQPROCODE								"313000"
#define CREDIT_SALEPROCODE									"003000"
#define CREDIT_CASHPROCODE									"013000"
#define CREDIT_VOIDPROCODE									"023000"
#define CREDIT_REFUNDPROCODE								"200030"
#define CREDIT_PINCHANGEPROCODE						  "910000"
#define CREDIT_ACTIVATIONPROCODE						"920000"
#define CREDIT_MOBILETOPUPPROCODE					  "810000"
#define CREDIT_SETTLEMENTPROCODE            "920000"

//Processing codes for Other ////////	Agency Bank
#define OTHER_AUTHPROCODE									"000000"
#define OTHER_BALINQPROCODE								"310000"
#define OTHER_SALEPROCODE									"000000"
#define OTHER_CASHPROCODE									"010000"
#define OTHER_VOIDPROCODE									"020000"
#define OTHER_REFUNDPROCODE								"200000"
#define OTHER_DEPOSITPROCODE								"210000"

//Processing codes TRANSFER ////////	Agency Bank
#define TRANSFER_SAV_TO_CHEPROCODE								"401020"
#define TRANSFER_CHE_TO_SAVPROCODE								"402010"
#define TRANSFER_SAV_TO_OTHPROCODE								"401000"
#define TRANSFER_CHE_TO_OTHPROCODE								"402000"
#define TRANSFER_OTH_TO_SAVPROCODE								"400010"
#define TRANSFER_OTH_TO_CHEPROCODE								"400020"
#define TRANSFER_OTH_TO_OTHPROCODE								"400000"

#define PIN_VALIDATE_PRCODE							"610000"

//POS Entry Mode
#define UNSPECIFIED                 "000"
#define MANUAL_PIN_CAPABLE          "011"
#define MSR_PIN_CAPABLE             "021"
#define MSR_PIN_NO_CAPABLE          "022"
#define ICC_PIN_CAPABLE             "051"
#define ICC_PIN_NO_CAPABLE          "052"
#define MSReICC_PIN_CAPABLE         "801"
#define MSReICC_PIN_NO_CAPABLE      "802"

//POS CONDITION Mode
#define NORMAL_PRESENTMENT            "00"//Normal presentment
#define CARD_HOLDER_NOT_PRESENT       "01"//Cardholder not present
#define MERCHANT_SUSPICIOUS           "03"//Merchant suspicious
#define CARD_IS_NOT_PRESENT           "05"//CARD_NOT_PRESENT
#define PRE_AUTHORIZE_REQUEST         "06"//Pre-authorized request
#define MAIL_OR_TELE_ORDER            "08"//Mail and/or telephone order
#define CARD_PRESENT_NOT_READ         "71"//Card present, magnetic stripe cannot be read

//Response code message
#define Normal_NO_ERROR							        "00" //Normal no error
#define REFER_TO_CARD_ISSUER				        "01" //Refer to Card Issuer
#define INVALID_MERCHANT						        "03" //Invalid Merchant
#define DO_NOT_HONOR								        "05" //Do Not Honor
#define HONOR_WITH_Id								        "08" //Honor with I.D. Only
#define INVALID_MESSAGE							        "12" //Invalid Message
#define INVALID_DOLLOR_AMOUNT				        "13" //Invalid Dollar Amount
#define INVALID_CARD_NUMBER  				        "14" //Invalid Card Number
#define NO_ACTION										        "21" //No Action Taken
#define NO_FILE_UPDATE_ALLOW				        "24" //No File Update Allowed
#define RECORD_NOT_FOUND						        "25" //Record Not Found in File
#define DUPLICATE_RECORD_FILE               "26" //Duplicate Record on File
#define RECORD_FILE_LOCKED					        "28" //Record or File Locked
#define FILE_UPDATE_DENIED					        "29" //File Update Denied
#define FORMAT_ERROR                        "30" //Format Error
#define INVALID_ACQIUIRER                   "31" //Invalid Acquirer Institution
#define CARD_EXPIRED						            "33" //Card Expired
#define CAPTURE_CARD						            "38" //Capture Card – PIN Retries Exceeded
#define NO_CREDIT_ACCOUNT					          "39" //No Credit Account
#define INVALID_FUNCTION                    "40" //Invalid Function
#define LOST_CARD                           "41" //Lost Card
#define CARD_IS_STOLEN						          "43" //Card is Hot/Stolen
#define INSUFFICIENT_FUNDS					        "51" //Insufficient Funds
#define CARD_EXPIRED_DNP                    "54" //Card Expired – Do not pickup
#define INVALID_PIN_CODE                    "55" //Invalid PIN – Retry
#define CARD_HOLDER_NOT_ONFILE              "56" //Card Holder not on File
#define TRANSACTION_NOT_ALLOWED			        "57" //Transaction not allowed for Card
#define EXCEEDS_WITHDRWAL_LIMIT			        "61" //Exceeds Withdrawal Limit
#define RESTRICTED_CARD                     "62" //Restricted Card
#define INVALID_MAC                         "63" //Invalid MAC
#define INVALID_ORIG_AMOUNT                 "64" //Invalid Original Amount
#define NO_OF_WITHDRAWAL_EXCEEDED           "65"//Number of Withdrawals Exceeded
#define FORCE_CAPTURE_CARD                  "67"//Force Capture of Card
#define PIN_RETRIES_EXCEEDED                "75"//PIN Retries exceeded
#define INVALID_INTER_CHANGE_AMOUNT         "76"//Invalid Interchange Amount
#define INVALID_BUSINESS_DATE               "77"//Invalid Business Date
#define DEACTIVATED_CARD                    "78"//Deactivated Card
#define INVALID_ACCOUNT                     "79"//Invalid Account
#define TRANSACTION_DENIED                  "80"//Transaction Denied
#define CANCELLED_CARD                      "81"//Cancelled Card
#define NO_ACKNOWLEDGEMENT                  "82"//No acknowledgement from ATM
#define HOST_REFUSE                         "83"//HOST REFUSE
#define ISSUER_DOWN                         "84"//ISSUER DOWN
#define INVALID_ORIGINATOR                  "85"//Invalid Originator or Processor
#define NOT_ALLOWED_ON_DEVICE               "86"//Not Allowed on Device
#define PIN_KEY_SYNC_ERROR                  "87"//PIN Key Sync Error
#define MAC_KEY_SYNC_ERROR                  "88"//MAC Key Sync Error
#define EXTERNAL_SWITCH_DECLINE             "89"//External Switch Decline
#define CUT_OFF_IN_PROGRESS                 "90"//Cutoff in Progress
#define MESSAGE_TIMED_OUT                   "91"//Message Timed Out
#define ISSUER_NOT_FOUND                    "92"//Issuer Not Found
#define POSSIBLE_DUPLICATED_TRANSACTION     "94"//Possible Duplicated Transaction
#define INTERNAL_SWITCH_ERROR               "96"//Internal Switch Error
#define KEY_EXCHANGE_IN_PROGRESS            "97"//Key Exchange in Progress
#define SWITCH_CONSIDER_ORIGINATOR_DOWN     "98"//Switch Considers Originator Down
#define INVALID_AUTHORIZATION_NUMBER        "99"//Invalid Authorization Number
#define UNMATCHED_TRANSACTION               "N0"//Unmatched Transaction
#define VALID_UNMATCHED_TRANSACTION         "N1"//Valid Unmatched Transaction
#define EXP_DATE_DOES_NOT_MATCH             "Q2"//Expiry Date does not Match with Database
#define INVALID_CVV                         "SK"//Invalid Card Verification Value (CVV)


//Configurable Message Index
#define SWIPE_YOUR_CARD					   0
#define ENTER_AMOUNT					     1
#define ENTER_CASHBACK_AMOUNT			 2 
#define CAN_NOT_READ_CARD_DATA		 3
#define ERROR_IN_CARD_PARSING			 4 
#define CARD_EXPIRY_DATE				   5
#define MMYY							         6
#define ENTER_PAYMENT_ID				   7
#define ENTER_TRANS_NO					   8
#define SENDING_MESSAGE					   9
#define RECEIVING_MESSAGE			     10
#define MESSAGE_RECEIVED			     11
#define PROCESSING					       12
#define ENTER_PIN					         13 
#define BEVERTEC					         14
#define CONNECTING					       15
#define CONNECTION_FAIL				     16
#define SENDING_FAIL				       17
#define RECEIVING_FAIL				     18
#define TRANSACTION_SUCCESSFULL		 19
#define TRANSACTION_FAILED			   20
#define GOING_FOR_REVERSAL			   21
#define CAN_NOT_DO_SETTLEMENT			 22
#define PENDING_REVERSAL			     23
#define CAN_NOT_DO_SALE				     24
#define CAN_NOT_DO_VOID				     25
#define CAN_NOT_DO_REFUND				   26
#define SETTLEMENT_SUCCESSFULL	   27
#define SETTLEMENT_DECLINE			   28
#define UPLOADING_BATCH				     29
#define NO_RECORD_FOUND				     30
#define INVALID_TRANS_REF_NO  		 31
#define INVALID_PAYMENT_ID			   32
#define INVALID_PIN_INDX					 33
#define INSUFFICIENT_FUND			     34
#define INVALID_PASSWORD			     35
#define ENTER_SUPERVISOR_PASSWORD	 36
#define PRINTING					         37
#define REVERSAL_SUCCESSFUL		   38
#define LOGON_SUCCESSFUL			     39
#define PLEASE_LOGON_FIRST			   40
#define DID_NOT_GET_KEY			       41
#define PLEASE_LOGON_AGAIN			   42
#define ENTER_MERCHNAT_PASSWORD		 43
#define SWIPE_ENTER_CARD					 44
#define GOING_FOR_FALLBACK			   45
#define SETTLEMENT_TRAILER_SUCCESS 46
#define SETTLEMENT_TRAILER_DECLINE 47
#define UPLOAD_NEXT_TRANSACTION    48
#define FALLBACK_OCCURRED          49
#define WANT_TO_SWIPE_CARD         50
#define PRESS_ENTER_FOR_YES        51
#define PRESS_CANCEL_FOR_NO        52
#define _INSERT_CARD               53
#define NO_TRANSACTION_TO_VOID     54
#define NO_TRANSACTION_TO_COPY     55
#define ENTER_FROM_ACCOUNT			56			//Agecny Bank From AC
#define ENTER_TO_ACCOUNT			57			//Agecny Bank To AC
#define CAN_NOT_DO_WITHDRAWAL				     58
#define CAN_NOT_DO_DEPOSIT				     59
#define CAN_NOT_DO_TRANSFER				     60
#define ENTER_OLD_PIN            61
#define ENTER_NEW_PIN            62


///////////////////////////////////////////////////////////
typedef struct BitmapStruct
{
	int packet_sz ;
	int trans_type;              
	unsigned char buffer [ASSEMBLE_BUFFER_SIZE];                 // to store packet //400
	unsigned char t_map[BITMAP_ARRAY_SIZE];		  				// core transaction map //8
	unsigned char source[SOURCE_BUFFER_SIZE], dest[DEST_BUFFER_SIZE];       // uses in convesion //100

	/***************************************************************************
	 8583 Interface Variables  
	 Note: size includes an extra byte for null- terminated strings.  
	****************************************************************************/ 
	unsigned char discard[ DISCARD_BUFFER_SIZE+1];    /* Variable to dump unwanted data into */ //201 
	unsigned char tpdu [TPDU_LEN+1]; 	    /* Transport_Protocol_Data_Unit */  //11
	unsigned char message_id [MSG_ID_LEN+1];   /* for message typeId  */ //5
	unsigned char field_02 [ACC_NUM_ARR_LEN+1]; 	/* Primary_Account_Number */ //20
	unsigned char field_03 [PROC_CODE_LEN+1]; 	/* Processing_Code */ //7
	unsigned char field_04 [AMOUNT_LEN+1]; 	/* Amount_Transaction */ //13
	 
	unsigned char field_11[ TRACE_AUDIT_LEN+1]; 	/* Systems_Trace_Audit_Number */ //7
	unsigned char field_12[ TRANS_LOCALTIME_LEN+1]; 	/* Time_Local_Transaction */ //7
	unsigned char field_13[ TRANS_LOCALDATE_LEN+1]; 	/* Date_Local_Transaction */ //5
	unsigned char field_14[EXPTIME_ARR_LEN+1 ]; 	/* Date_Expiration */ //5

	unsigned char field_22[POS_ENTRY_LEN+1 ];     /* Point_of_Service_Entry_Mode */ //4
	unsigned char field_24[NII_LEN+1 ];     /* Network_International_Identifier */ //4
	unsigned char field_25[POS_COND_LEN+1 ];     /* Point_of_Service_Condition_Code */ //3
	unsigned char field_35[TRACK2_DATA_LEN+1];    /* Track_2_Data */ //38
	
	unsigned char field_37[RETRIEVAL_REF_LEN+1 ];    /* Retrieval_Reference_Number */ //13
	unsigned char field_38[AUTH_ID_LEN+1 ];     /* Authorization_Identification_Response*/ //7
	unsigned char field_39[RESPONSE_CODE_LEN+1];     /* Response_Code *///3
	
	unsigned char field_41[TERMINAL_ID_LEN+1 ];     /* Terminal_Identification*/ //9
	unsigned char field_42[CARD_ACCEPTOR_ID_LEN+1 ];    /* Card_Acceptor_Identification_Code */ //16
	unsigned char field_48[PAYMENT_ID_LEN+1];    //Payment Id for Gov app payment //10
 	unsigned char field_102[ACCOUNT_NO_LEN+1];    //From Account Number //10		//Agency Bank
	unsigned char field_103[ACCOUNT_NO_LEN+1];    //To Account Number //10		//Agency Bank
	unsigned char field_52[BIANRY_PIN_LEN];     /* PIN data for debit transactions */ //8
	unsigned char field_53[SECURITY_CONTROL_LEN+1 ]; /*  Security__Control_Information */ //17
	unsigned char field_54[BAL_INQ_AMT_RES_LEN+1 ];   /*Balance amount for Balance inquiry  */ //121
	unsigned char field_55[ICC_RELATED_DATA_LEN ];   /* ICC Related data */ //55

	unsigned char field_60[BATCH_ID_LEN+1];   /* Batch no for Settlement and Settlemnet Trailor  */ //7
  
	unsigned char AmountForVoid[AMOUNT_LEN+1 ];   /* amount for void */ //13
	unsigned char field_63[26];   /* Reconcillation request for Settlement and Settlemnet Trailor*/ //61
	unsigned char field_64[MAC_64_LEN+1 ];     /* Message_Authentication_Code_Field */ //9
	//unsigned char field_102[FROM_AC_LEN+1];		//From Account Agency Bank
	
}BmapStruct;

// Structure which contain all transaction realted information
typedef struct TransactionMessageStruct         // to store data
{
	struct TRACK CardDetails;												//structure Megnetic card swipe data
	char MessageTypeInd[MSG_ID_LEN+1];												//Message type indicator 4 digit //5
	char PrimaryAccNum[ACC_NUM_ARR_LEN+1];												//Primary account number      //22 to 20
	char _transType[TRANS_TYPE_STRING+1];                          //store transaction type like sale ,void ,refund ... //21
	char CardHldrName[CARD_HOLDER_NAME_LEN];												//Card Holder Name //50
	char ProcessingCode[PROC_CODE_LEN+1];												//Processing code according to the transaction type //7
	char Amount[AMOUNT_LEN+1];															//Amount for transaction //13
	char CashBackAmt[AMOUNT_LEN+1];													//Cash back amount //13
	
	char TraceAuditNo[TRACE_AUDIT_LEN+1];													//Trace audit no //7
	char TransLocalTime[TRANS_LOCALTIME_LEN+1];												//Transaction time //7
	char TransLocalDate[TRANS_LOCALDATE_LEN+1];												//Transaction date //5
	char ExpiryDate[EXPTIME_ARR_LEN+1];														//Card expiry date //5
	char POSEntryMode[POS_ENTRY_LEN+1 ];													/*POS entry mode for how the account no has //4
                                                                been entered into the terminal*/
	char NetworkInternationalId[NII_LEN+1 ] ;							/*Network International Identifier to identify
																										the aquiring host*/ //4
	char POSConditionCode[POS_COND_LEN+1 ] ;										/*used to identify the condition under which the 
																								transaction takes place*/ //3
	char Track2Data[TRACK2_DATA_LEN+1];													//Track 2 data read by mag card //38
	char RetrievalReferenceNo[RETRIEVAL_REF_LEN+1] ;								//13
	char AuthIdResponse[AUTH_ID_LEN+1] ;											/*It is assigned by the authorizing host when the 
																								transaction is approved*/ //7
	char ResponseCode[RESPONSE_CODE_LEN+1];													/*This is returned to the terminal from the authorization 
																								host to indicate the status of the transaction*/ //3
	char TerminalID[TERMINAL_ID_LEN+1];														//The terminal id used to uniquely identify the terminal. //9
	char CardAcceptorID[CARD_ACCEPTOR_ID_LEN+1];											//The merchant number assigned to the terminal. //16
	
	char Track1Data[TRACK1_DATA_LEN+1];													/*The Track 1 data is present when valid track 1 is 
																								used to initiate the transaction.*/ //77
  
	unsigned char PinData[BIANRY_PIN_LEN];															//Cardholder entered PIN in ANSI X9.8 PIN block format //8
	char NewPinData[BIANRY_PIN_LEN];
	char PaymentId[PAYMENT_ID_LEN+1];                             //Bit 48 for payment //10
	char FromAcNo[ACCOUNT_NO_LEN+1];		//Agency Bank
	char ToAcNo[ACCOUNT_NO_LEN+1];		//Agency Bank
	char ResponseText[RES_TEXT_BUFFER_LEN];                            //response text print on the reciept in some cases according to the response code//50
 
	char rqstConsilation[RECONCILLATION_REQ_LEN+1];                       //Reconclillation request for settlement //61
	char EMV_Tag_Data[ICC_RELATED_DATA_LEN];                       //EMV tag data//55
	char BatchNo[BATCH_ID_LEN+1];                              //batch no for settlemnt //7
	char TVR[_TVR_SIZE+1];                                  //Terminal verification result(tvr) for EMV 
	char TSI[_TSI_SIZE+1];                           //Transaction status information for EMV 
	char CARD_TYPE[CARD_TYPE_SIZE+1];               //Card type information for EMV 
	char Verify_Method[RES_TEXT_BUFFER_LEN];               //Card type information for EMV 
	short TrMethordFlag;                            //transaction method like debit,credit ,loyality
	short TrTypeFlag;                               //transaction type like paymnet ,void ,refund ,balance inquiry etc
	short EMV_Flag; 
	char KSN[KSN_LENGTH+1];		//Key Serial Number                 

}TransactionMsgStruc;


typedef struct parsedResponse
{
	char TraceAuditNo[TRACE_AUDIT_LEN+1];
	char responseCode[HEX_RES_CODE_LEN+1];
  char retrivalReferenceNo[HEX_RET_REF_ID_LEN+1];
  char AuthIdResponse[HEX_AUTH_ID_LEN+1];
	char Amount[AMOUNT_LEN+1];
  char AddAmount[BAL_INQ_AMT_RES_LEN+1];
	char orgTime[TRANS_LOCALTIME_LEN+1];
	char orgDate[TRANS_LOCALDATE_LEN+1];
	char Nii[NII_LEN+1];
	char PosEntryMod[POS_ENTRY_LEN+1];
  char TerId[HEX_TER_ID_LEN+1];
  char FromAc[ACCOUNT_NO_LEN+1];
  char ToAc[ACCOUNT_NO_LEN+1];
}parsedRes;

typedef struct
{
		char traceAuditNo[TRACE_AUDIT_LEN+1];//bit field 11
		char cardHolderName[CARD_HOLDER_NAME_LEN];
		char cardNumber[ACC_NUM_ARR_LEN+1]; //22 to 20 
		char expDate[EXPTIME_ARR_LEN+1]; //
		char trans_Type[TRANS_TYPE_STRING+1];
		char Payment_Id[PAYMENT_ID_LEN+1];
		char FromAc_No[ACCOUNT_NO_LEN+1];		//Agency Bank
		char ToAc_No[ACCOUNT_NO_LEN+1];		//Agency Bank
		char trResponse[RES_TEXT_BUFFER_LEN];
		char amount[AMOUNT_LEN+1];//bit field 4
		char NII[NII_LEN+1];//bit field 24
		char orgTransTime[TRANS_LOCALTIME_LEN+1];//bit field 12
		char orgTransDate[TRANS_LOCALDATE_LEN+1];//bit field 13
		char AuthIdResponse[AUTH_ID_LEN+1]; //bit field 38
		char RetrievalRefNo[RETRIEVAL_REF_LEN+1]; //bit field 37    
    char processingCode[PROC_CODE_LEN+1];//bit field 3
		char PosEntryMode[POS_ENTRY_LEN+1]; //bit field 22
    char PosCondCode[POS_COND_LEN+1]; //bit field 25
    char TerminalId[TERMINAL_ID_LEN+1]; //bit field 38
		char CardAcceptorId[CARD_ACCEPTOR_ID_LEN+1]; //bit field 37
    char TVR[_TVR_SIZE+1]; //Terminal verification result
    char TSI[_TSI_SIZE+1]; //
    char CARD_TYPE[CARD_TYPE_SIZE+1];               //Card type information for EMV 
    
    
    short EMV_Flag; 
		short TransTypeFlg; 
		short TransMethord_Flag;
		
}TrDetails;
typedef struct
{
    char cardNumber[ACC_NUM_ARR_LEN+1];  //22 TO 20
		char expDate[EXPTIME_ARR_LEN+1]; // 
		char Payment_Id[PAYMENT_ID_LEN+1];//Bit field 48
		char FromAc_No[ACCOUNT_NO_LEN+1];			//Agency Bank
		char ToAc_No[ACCOUNT_NO_LEN+1];			//Agency Bank
		char amount[AMOUNT_LEN+1];//bit field 4
		char NII[NII_LEN+1];//bit field 24
		char processingCode[PROC_CODE_LEN+1];//bit field 3
		char TerminalId[TERMINAL_ID_LEN+1]; //bit field 41
		char CardAcceptorId[CARD_ACCEPTOR_ID_LEN+1]; //bit field 42    
    char PosEntryMode[POS_ENTRY_LEN+1]; //bit field 22
    char PosCondCode[POS_COND_LEN+1]; //bit field 25
		char Track2Data[TRACK2_DATA_LEN+1];	//bit field 35
    char PinData[BIANRY_PIN_LEN];	//bit field 35
    char RetrievalRefNo[RETRIEVAL_REF_LEN+1]; //bit field 37
    char trans_Type[TRANS_TYPE_STRING+1]; //saving txn string like sale ,void and refund 
   
    short TransTypeFlg; //for case of txn
    short TransMethord_Flag; //txn method like debit ,credit and loyality 

}RevDetails;

//-------------------
typedef struct 
{
		short x_cor;
		short y_cor;
		char dispMsg[DISP_TEXT_BUFFER_LEN];

}dispMsg;
//------------------
extern char szKeyMap[MAX_ALPNUM_KEYS][CHAR_PER_KEY];
extern dispMsg *Dmsg;
extern short paperOutFlag;
extern short OTHER_AC;
extern char AC_TYPE;
extern short Sup_Login;
extern short BI;
//extern char tempBuff[800];
extern unsigned char temp1Buffer[800];
extern short FROM_OTHER_AC;
extern short pin_enc;
extern short key_injected;
extern short pin_change;
int mainMenu(TransactionMsgStruc *);

void readKbdBuff(void);
void ClearKbdBuff(void);
void KBD_FLUSH(void);
void getTraceAuditNumber(TransactionMsgStruc *transMsg);//getting and update trace audit id
void updateBatchNumber(TransactionMsgStruc *transMsg);
void resetMsgDetails(TransactionMsgStruc *);
short readDisplaygMsg(void);
int checkPaperStatus(void);

extern unsigned char KeySessionKeyEncripted[GISKE_KEY_LEN + 1];//Giske Block specs for session key

#endif
