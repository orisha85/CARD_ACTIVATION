//#include "..\include\Common.h"
//#include "..\include\Merchant.h"
//#include "..\include\TouchScreen.h"
//#include <svc.h>
//#include <svctxo.h>
//#include <applidl.h>
//
///****************************************************************************************************************
//*	Function Name : TransactionDetailsfun																			                                    *
//*																						*
//*	Purpose		  : Allow you to print the report in details of transactions in the terminal.						 														*
//*	Input		  : int variable to check transaction is debit or not											*
//*	Output		  : Return value for the success or failure														*
//*																						*
//*****************************************************************************************************************/
//
//int TransactionDetailsfun(void)
//{
//	int Event_selected =-1;
//	clrscr();
//	write_at("TransactionDetailsfun", strlen("TransactionDetailsfun"),1, 10);
//	Event_selected = get_char();
//	if(Event_selected == KEY_CANCEL)
//		return KEY_CANCEL;
//	return 0;
//}
//
///****************************************************************************************************************
//*	Function Name : KeyExchangefun																			*
//*																						*
//*	Purpose		  : Allow you to print the report in details of transactions in the terminal.						 														*
//*	Input		  : int variable to check transaction is debit or not											*
//*	Output		  : Return value for the success or failure														*
//*																						*
//*****************************************************************************************************************/
//int KeyExchangefun(void)
//{
//	int Event_selected =-1;
//	clrscr();
//	write_at("KeyExchangefun", strlen("KeyExchangefun"),1, 10);
//	Event_selected = get_char();
//	if(Event_selected == KEY_CANCEL)
//		return KEY_CANCEL;
//	return 0;
//}
//
//
///****************************************************************************************************************
//*	Function Name : LogOnFun																					*
//*																						*
//*	Purpose		  : This feature allows the merchant to send a logon message to the bank in order to open a     *
//					day and change the pin working Key.						 									*
//*	Input		  : int variable to check transaction is debit or not											*
//*	Output		  : Return value for the success or failure														*
//*																						*
//*****************************************************************************************************************/
//int LogOnFun(void)
//{
//	int Event_selected =-1;
//	clrscr();
//	write_at("LogOnFun", strlen("LogOnFun"),1, 10);
//	Event_selected = get_char();
//	if((Event_selected == KEY_CANCEL) || (Event_selected == KEY_STR))
//		return Event_selected;
//	return 0;
//}
///****************************************************************************************************************
//*	Function Name : SattlementFun																			*
//*																						*
//*	Purpose		  : This feature allows the merchant to send the transactions contained in the terminal 		*
//*	Input		  : int variable to check transaction is debit or not											*
//*	Output		  : Return value for the success or failure														*
//*																						*
//*****************************************************************************************************************/
//int SattlementFun(void)
//{
//	int Event_selected =-1;
//	clrscr();
//	write_at("SattlementFun", strlen("SattlementFun"),1, 10);
//	Event_selected = get_char();
//	if((Event_selected == KEY_CANCEL) || (Event_selected == KEY_STR))
//		return Event_selected;
//	return 0;
//}
///****************************************************************************************************************
//*	Function Name : CopyFun																			                                          *
//*																						                *
//*	Purpose		  : Will print the Copy of any transaction (client ticket only)									*
//*	Input		  : int variable to check transaction is debit or not											*
//*	Output		  : Return value for the success or failure														*
//*																						*
//*****************************************************************************************************************/
//int CopyFun(void)
//{
//	int Event_selected =-1;
//	clrscr();
//	write_at("CopyFun", strlen("CopyFun"),1, 10);
//	Event_selected = get_char();
//	if((Event_selected == KEY_CANCEL) || (Event_selected == KEY_STR))
//		return Event_selected;
//	return 0;
//}
///****************************************************************************************************************
//*	Function Name : TotalReportFun																			                                          *
//*	Purpose		  : This menu allows you to print a ticket containing											                          *
//*					* The total number of transactions and the amount											                                *
//*					* The total number of reversal operations (cancellation) and the amount						                    *
//*					*Total net (differential)																	                                            *
//*	Input		  : int variable to check transaction is debit or not											                            *
//*	Output		  : Return value for the success or failure														                              *
//*****************************************************************************************************************/
//int TotalReportFun(void)
//{
//	int Event_selected =-1;
//	clrscr();
//	write_at("TotalReportFun", strlen("TotalReportFun"),1, 10);
//	Event_selected = get_char();
//	if((Event_selected == KEY_CANCEL) || (Event_selected == KEY_STR))
//		return Event_selected;
//	return 0;
//}
//
