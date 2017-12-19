#ifndef TOUCH_SCREEN_H
#define TOUCH_SCREEN_H

#define MainMenu_BMP "I:mainMenu.BMP"
#define TransMethord_BMP "I:TransMethord.BMP"
#define TransType_BMP "I:TransType.BMP"  
#define TransType1_BMP "I:TransType1.BMP"  
#define Credit_TransType_BMP "I:CreditTransType.BMP" 
#define ACC_TYPE_BMP	"I:AccountType.BMP"
#define MERCHANT_MENU_BMP "I:Merchant1.BMP"
#define MERCHANT_MENU_BMP2	"I:Merchant2.BMP"
#define Super_MENU_BMP "I:Super1.BMP"
#define Super_MENU_BMP2	"I:Super2.BMP"
#define Cashback_BMP	"I:Cashback.BMP"
#define Config_BMP	"I:Config.BMP"
#define TO_ACC_TYPE_BMP_SO	"I:ToAccountTypeSO.BMP"
#define TO_ACC_TYPE_BMP_CO	"I:ToAccountTypeCO.BMP"

#define mainMenuS						1
#define TransMethord				2
#define TransType						3
#define TransType1						23
#define AccountType					4	
#define MerchentMenu1				5
#define MerchentMenu2				6
#define SupervisorMenu1			7
#define SupervisorMenu2			8
#define CashBack						9
#define ConfigMenu					10
#define CreditTransMenu					11
#define MultiAPPSectionMenu			12
#define DisableTouch			13
#define GoToMainScreen			1
#define Agency_AccountType					14	
#define Agency_Sav_Oth_AccountType					15	
#define Agency_Che_Oth_AccountType					16	
#define Agency_ToAccountType					17	


/////////////////////////////////////////////////////////////
#define MultiApp1   			1
#define MultiApp2					2
#define MultiApp3					3
#define MultiApp4					4
#define MultiAppNext			5


#define MultiAppPrev1   			6
#define MultiAppPrev2					7
#define MultiAppPrev3					8
#define MultiAppPrev4					9

///////////////////////////////////////////////////////////////////
#define MultiLang1   			  1
#define MultiLang2					2
#define MultiLang3					3
#define MultiLang4					4
/////////////////////////////////////////////
//for configuration
#define COMPNAME								1
#define HOST_IP									2
#define HOST_PORT								3
#define MERCHANT_ID							4
#define MERCHANT_NAME						5
#define MERCHANT_ADDR						6
#define TERMINAL_ID							7
#define SUPER_PASSWORD					8
#define CONGIF_BACK							9

#define App1								      1
#define App2									    2
#define App3			    					  3
#define App4      							  4
#define AppNext      							5
#define AppPrev      							6


#define CASE_SPN_NAME	 1
#define CASE_FRN_NAME	 2
#define CASE_GER_NAME	 3

int SetImage(char *ImageName);
void DisplayMsgOnScreen(char *,short,short);
void EnblTouchScreen(int);
void ModifyLangName(char * buff);
void EnblMultiAppTouchScreen(int Key);   // set up keymap and display prompt
void EnblMultiLangTouchScreen(int Key);   // set up keymap and display prompt
//this Enum is written in order to recognize event on screen and perform according to event 
//generated on screen 
enum EnumFlowScreen
{
// Main Menu screen event values 
		eTransaction = 1,							  // 1 
		eMerchent,											// 2
		eSuperviser,										// 3

//event values for transaction Methord screen (second screen---debit credit and loyality)
		edebit,													//4
		ecredit,												//5
		eLoyality,											//6


////event values for transaction Type screen (third screen Sale,Bal Inq,Void and Refund) 
		eSale,													//7
		eWithdrawal,								//8		Agency Bank
		eBalanceInquiry = 9,						//9
		eVoid,													//10
		eRefund,												//11
								
////event values for Debit transaction (Cheque,Saving and Credit)
		eChequeAccount,									//12
		eSavingAccount	= 15,						//15
		eCreditAccount, 								//16


//event values for Merchant first screen
		eLogOn,													//17
		eSattlement,										//18
		eTransactionDetail,				//Agency Bank
		//ecopy,													//19
		eTotalReport,										//20

//event values for Merchant Second screen
		ecopy,		
		//eTransactionDetail,							//21
		eKeyExchange,										//22


//event values for Supervisor first screen
		ePassword,											//23
		econfiguration,									//24
		ePINValidate,
		//eDownload,											//25
		eLogout,
		//eKeyDownload,										//26

//	event values for Supervisor Second screen
		eHelpDesk = 28,									//28

// event values for CashBack screen
		eCashBackYes,										//29
		eCashBackNo,										//30

//values for Back and Next Screen Button
		eTrMethordBack,									//31
		eTrTypeBack,										//32
		eAccTypeBack,										//33
		eMerchant1Back,									//34
		eMerchant1Next = 36,						//36
		eMerchant2Back ,								//37
		eSuper1Back,										//38
		eSuper1Next,										//39
		eSuper2Back,										//40
		eCashBack,											//41
		eOtherAccount = 45,											//45
		eDeposit,									//46	Agency Bank
		eTransfer = 48,									//48	Agency Bank
//Transfer Account screen buttons
		eSavToCheAccount =60,
		eSavToOthAccount,
		eCheToSavAccount,
		eCheToOthAccount,
		eOthToCheAccount,
		eOthToSavAccount,
		eOthToOthAccount,
		eTrTypeNext,
		eTrType1Back,
		eCardActivation,
		ePinChange,
		SelectEvent =											-99
		
};	

struct FlowScreen
{
	enum EnumFlowScreen test;
};


#endif 
