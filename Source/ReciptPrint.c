/***********************************************************************************************
  Filename		     : ReciptPrint.c
  Project		       : Bevertec 
  Developer Neame  : Amar
  Module           : Print Reciept
  Description	     : This is the implementation of printing transaction reciept,
                     Total report reciept ,close batch reciept and transaction details reciept
 ***********************************************************************************************/




#include "..\Include\ReciptPrint.h"
#include "..\Include\Settlement.h"
#include "..\Include\ISo8583Map.h"
#include "..\Include\Supervisor.h"
#include <time.h>


short paperOutFlag = 0;
short PinFlag = 0; // 19-08-2016
/*********************************************************************************************************
*	Function Name : print																																										*
*	Purpose		    : print String in paper																																		*
*	Input					:	pointer of char buffer																														 		 *
*	Output		    : returns success or failure 																															*
																																						*
**********************************************************************************************************/

short print(char *bufferToPrint)
{
		unsigned char   printBuf[PRINT_BUF_SIZE]= {0} ;
		short retVal=_FAIL;
		disable_hot_key ();
		if(paperOutFlag == 0)
		{
				memset (printBuf, 0, sizeof (printBuf));
				printBuf[0] = PRINT_NORM;

				strcpy ((char *) &printBuf[1], bufferToPrint);

				retVal = p3300_status(hPrinter,1000);
				if(retVal == PAPER_OUT && paperOutFlag==0)
				{
					window(1,1,30,20);
					clrscr();
					error_tone();
					write_at("PAPER OUT", strlen("PAPER OUT"),(30-strlen("PAPER OUT"))/2, 10);
					write_at("INSERT ROLL", strlen("INSERT ROLL"),(30-strlen("INSERT ROLL"))/2, 12);
					SVC_WAIT(1000);
					paperOutFlag=1;
					clrscr();
					return retVal;
				}
				p3300_print (hPrinter, printBuf);
			//LOG_PRINTF (("Print a buffer "));
				return _SUCCESS;
		}
		return _FAIL;
}

/*********************************************************************************************************
*	Function Name : print																																										*
*	Purpose		    : print Bold String in paper																																		*
*	Input					:	pointer of char buffer																														 		 *
*	Output		    : returns success or failure 																															*
*	Date		      : 12-feb-2016																																							*
**********************************************************************************************************/

short printBold(char *bufferToPrint)
{
    unsigned char   printBuf[PRINT_BUF_SIZE]={0};
		short retVal;
    disable_hot_key ();
    memset (printBuf, 0, sizeof (printBuf));
		printBuf[0] = DBL_WIDTH;
    strcpy ((char *) &printBuf[1], bufferToPrint);

		retVal = p3300_status(hPrinter,1000);
		if(retVal == PAPER_OUT && paperOutFlag==0)
		{
				window(1,1,30,20);
				clrscr();
				error_tone();
				write_at("PAPER OUT", strlen("PAPER OUT"),(30-strlen("PAPER OUT"))/2, 10);
				write_at("INSERT ROLL", strlen("INSERT ROLL"),(30-strlen("INSERT ROLL"))/2, 12);
				SVC_WAIT(1000);
				paperOutFlag=1;
				clrscr();
				return retVal;
		}
    p3300_print (hPrinter, printBuf);
	//LOG_PRINTF (("Print a buffer "));
		return _SUCCESS;
		
}


short printMainHeader(char* szBuffertoPrint)
{
	short shRetVal=NEGATIVE_INITIALIZE;
  unsigned char	szPrintBuf[LOCAL_PRINTER_BUFFER_SIZE]={0};
	
	memset(szPrintBuf,NULL_CHAR,sizeof(szPrintBuf));
	szPrintBuf[ZERO]= PRINT_NORM;
	/*szPrintBuf[ONE] = DBL_HEIGHT;
	szPrintBuf[TWO] = DBL_WIDTH;*/

	//LOG_PRINTF(("In print Bold szPrintBuff is [%s]",szBuffertoPrint));	

	shRetVal = p3300_status(hPrinter,PRINTER_STATUS);

	// check whether PrinterPaper is present or not
	if ( shRetVal == PAPER_OUT )
	{
		//LOG_PRINTF(("PRINTER IS OUT OF PAPER"));
		return(shRetVal);	
	}
	
    strcpy((char *)&szPrintBuf[THREE], szBuffertoPrint);
	shRetVal = p3300_print(hPrinter, szPrintBuf);
	//LOG_PRINTF(("In print Bold szPrintBuff is [%s] shRetVal is [%d]",szPrintBuf));
	
	return _SUCCESS;
}

short printCenter(char *szBuffertoPrint,short shFontFlag)
{
	short shIloopCnt=ZERO,shJloopCnt=ZERO;
	short shRetVal= NEGATIVE_INITIALIZE;

	if(strlen(szBuffertoPrint) > PRINTER_WIDTH_BOLD)
		shFontFlag = PRINT_NORMAL;

	if(shFontFlag == PRINT_NORMAL)
	{
		shIloopCnt = (PRINTER_WIDTH - strlen(szBuffertoPrint))/TWO;
		for(shJloopCnt=ZERO ; shJloopCnt<shIloopCnt ; shJloopCnt++)
		{
			shRetVal = print(PRINT_SPACE);
		}
		shRetVal = print(szBuffertoPrint);
	}
	else if (shFontFlag == PRINT_BOLD)
	{
		shIloopCnt = (PRINTER_WIDTH - (TWO*strlen(szBuffertoPrint)))/TWO;
		for(shJloopCnt=ZERO ; shJloopCnt < shIloopCnt ; shJloopCnt++)
		{
			shRetVal = print(PRINT_SPACE);
		}
		shRetVal = printBold(szBuffertoPrint);
	}
	else  if(shFontFlag == CENTER_BOLD)
	{
		shIloopCnt = (PRINTER_WIDTH - (THREE*strlen(szBuffertoPrint)))/TWO;
		for(shJloopCnt=ZERO ; shJloopCnt < shIloopCnt ; shJloopCnt++)
		{
			shRetVal = print(PRINT_SPACE);
		}
		shRetVal = printMainHeader(szBuffertoPrint);
 
	}

	return(shRetVal);
}



/*********************************************************************************************************
*	Function Name : printRecipt																																							*
																																									                        *
*	Purpose		    : print Recipt for customer																																*
*	Input					:	Address of main transaction message structure							                              *
*	Output		    : returns success or failure 																															*
																																						                              *
**********************************************************************************************************/
short printRecipt(TransactionMsgStruc *transMsg)
{
	//Transaction time
	short Pos = 0;
	short counter = 0,i=0;
	char Soft_Version[SOFT_VER_LEN]={0};
	char timeBuffer[TIME_BUFFER_LEN]={0};//16
	char accountNo[ACC_NUMBER_LEN]={0};//22
	char merchantName[MERCHANT_NAME_SIZE]={0};
	char merchantAddress1[MERCHANT_ADDRESS_SIZE]={0};
	char merchantAddress2[MERCHANT_ADDRESS_SIZE]={0};
    char Merchant_Id[CARD_ACCEPTOR_ID_LEN+1]={0};//10 
	char Terminal_Id[TERMINAL_ID_LEN+1]={0};//9
    char printExpDate[PRINT_EXPDATE]={0};//6
	char printflag[2]={0};
	char curr_code[4];
	char ch=0;
    //int RetVal =0;
    char sysDate[9] = {0};				// format date
    char sysTime[9] = {0};				// format time
    short retPaperStatus = 0;
	LOG_PRINTF(("==============printReceipt==========="));
	retPaperStatus =Paperstatus();
    if(retPaperStatus == KEY_CANCEL)
		return KEY_CANCEL;
    get_env("#PRINT_FLAG",printflag,sizeof(printflag));
	if(strcmp(printflag,"1")==0)
	{
        memset(merchantName,0,MERCHANT_NAME_SIZE);
		memset(merchantAddress1,0,MERCHANT_ADDRESS_SIZE);
		memset(merchantAddress2,0,MERCHANT_ADDRESS_SIZE);
		memset(curr_code,0,sizeof(curr_code));
  
		get_env("#MERCHANT_NAME",merchantName,sizeof(merchantName));
		get_env("#MERCHANT_ADDRESS1",merchantAddress1,sizeof(merchantAddress1));
		get_env("#MERCHANT_ADDRESS2",merchantAddress2,sizeof(merchantAddress2));
		get_env("#CURRENCY_CODE",curr_code,sizeof(curr_code));
		read_clock(timeBuffer); //reads the system clock time     
		sprintf(sysTime, "%c%c:%c%c:%c%c"   , timeBuffer[8], timeBuffer[9], timeBuffer[10], timeBuffer[11], timeBuffer[12], timeBuffer[13]);// hr:min:sec
		sprintf(sysDate,"%c%c/%c%c/%c%c", timeBuffer[4], timeBuffer[5], timeBuffer[6],  timeBuffer[7],  timeBuffer[2],  timeBuffer[3]); // mm/dd/yy	
		strncpy(printExpDate,transMsg->ExpiryDate,2);
        strcat(printExpDate,"/");
        strcat(printExpDate,transMsg->ExpiryDate+2);
        do
		{
    		window(1,1,30,20);
			clrscr();
			write_at(Dmsg[PRINTING].dispMsg, strlen(Dmsg[PRINTING].dispMsg),Dmsg[PRINTING].x_cor, Dmsg[PRINTING].y_cor);//PRINTING
			ClearKbdBuff();
			KBD_FLUSH();
			printCenter((char *)merchantName,PRINT_BOLD);
			print (_NEW_LINE);
			printCenter((char *)merchantAddress1,PRINT_BOLD);
			print (_NEW_LINE);
			printCenter((char *)merchantAddress2,PRINT_BOLD);
			print (_NEW_LINE);
			print (_NEW_LINE);
			//get_env("#SOFTWARE_VERSION",Soft_Version,sizeof(Soft_Version));
			//print ("Version : ");
			//print(Soft_Version);
		//	print (_NEW_LINE);
		//	print (_NEW_LINE);
			get_env("#CARDACCEPTERID",Merchant_Id,sizeof(Merchant_Id));
			//print (MERCHANTID);
			print ("AID:");
			print (Merchant_Id);//print Merchant Id
			print (" ");
			get_env("#TERMINAL_ID",Terminal_Id,sizeof(Terminal_Id));
			print(TERMINALID);
			print(Terminal_Id);//print Terminal Id
			print (_NEW_LINE);
			print(sysDate);//Print System time
			print("            ");
			print(sysTime);//print System Date
			print (_NEW_LINE);

			// print the name
			print (transMsg->CardHldrName);
			print (_NEW_LINE);

			// print Account
		//	if(counter == 0)
			//{
				strcpy(accountNo,transMsg->PrimaryAccNum);
				for(Pos=strlen(transMsg->PrimaryAccNum)-10;Pos<strlen(transMsg->PrimaryAccNum)-4;Pos++)
					accountNo[Pos]='*';
				accountNo[strlen(accountNo)-1]='*';
				print (accountNo);
				print ("    ");
		/*	}
			else
			{
				print (transMsg->PrimaryAccNum);
				print ("    ");
			}*/
				
			// print the expiry Date
	    	//print (printExpDate);
				print(_NEW_LINE);

			// print the Total Amount
			//if(!strcmp(transMsg->ResponseText,APPROVED_MSG))
			//{
			//	if (!strcmp(transMsg->_transType, "BALANCE INQUIRY"))
			//		printBold("BALANCE");
			//	else
			//		printBold(_AMOUNT);
			//	print("      ");
			//	printBold(transMsg->Amount);
			//	print("  ");
			//	printBold(curr_code);
		//	print(_NEW_LINE);
			//}
		
			
			if(transMsg->EMV_Flag == 1)
			{
				if(strcmp((char *)transMsg->CARD_TYPE,"")!=0)//if card type is not empty
				{
					print(transMsg->CARD_TYPE);
					for(i =0;i<20-strlen(transMsg->CARD_TYPE);i++)
						print (" ");
				}
			}
			print(transMsg->_transType);//sale
			if((!strcmp(transMsg->ResponseText,APPROVED_MSG)))
			{
				print (_NEW_LINE);
				if(!strcmp(transMsg->_transType,"WITHDRAW") || !strcmp(transMsg->_transType,"BALANCE INQUIRY") || !strcmp(transMsg->_transType,"TRANSFER"))
				{
					print("FROM A/C #: ");
					print(transMsg->FromAcNo);
					print (_NEW_LINE);
				}
				else if(!strcmp(transMsg->_transType,"DEPOSIT") || !strcmp(transMsg->_transType,"TRANSFER"))
				{
					print("TO A/C   #: ");
					print(transMsg->ToAcNo);
				}
				/*else if(!strcmp(transMsg->_transType,"REVERSAL") || !strcmp(transMsg->_transType,"VOID"))
				{
					print("FROM A/C #: ");
					print(transMsg->FromAcNo);
					print(_NEW_LINE);
					print("TO A/C   #: ");
					print(transMsg->ToAcNo);
				}*/
				print (_NEW_LINE);
			//if((!strcmp(transMsg->ResponseText,APPROVED_MSG)))
			//{
			}
				if(strcmp(transMsg->MessageTypeInd,REVERSAL_MSG_TYPE)!=0)
				{
					print ("AUTH #:");
					print (transMsg->AuthIdResponse);
					print ("     ");
					print("REF #: ");
					print(transMsg->RetrievalReferenceNo);
					print (_NEW_LINE);
				}
			//	printBold(transMsg->ResponseText);
			//	print (_NEW_LINE);
			//}
			//else
			//{
			print (_NEW_LINE);
			//printCenter((char *)"TRANSACTION DECLINED",PRINT_BOLD);
			printCenter(transMsg->ResponseText,PRINT_BOLD);
			print (_NEW_LINE);
			
				//print (transMsg->ResponseText);
			//print (_NEW_LINE);
			//}
			if(transMsg->EMV_Flag == 1)//if transaction is an EMV transaction
			{
				print ("TVR: ");
				print (transMsg->TVR);
				print ("     ");
				print("TSI: ");
				print(transMsg->TSI);
				print (_NEW_LINE);
			}
			/*if(transMsg->TrTypeFlag != BALINQMSGTYPE_CASE)
			{
				//print ("PAYMENT ID: ");
				//print (transMsg->PaymentId);
				print (_NEW_LINE);
			}*/
       
			print (_NEW_LINE);
			print (_NEW_LINE);
			print (_NEW_LINE);
			//Agency Bank Prinit Signatur always
			if((!strcmp(transMsg->ResponseText,APPROVED_MSG))&&((strcmp((char *)transMsg->MessageTypeInd,REVERSAL_MSG_TYPE)!=0)))
			{
					printCenter("______________________", PRINT_NORMAL);
					print(_NEW_LINE);
					printCenter("Signature:", PRINT_NORMAL);
					print(_NEW_LINE);
					print(_NEW_LINE);
			}

			if((transMsg->EMV_Flag == 1 ) && (!strcmp(transMsg->ResponseText,APPROVED_MSG)))//if transaction is an EMV transaction
			{
				if((!strcmp(transMsg->Verify_Method,"Sig:")))
				{
					/*printCenter("Signature:",PRINT_NORMAL);
					print (_NEW_LINE);
					printCenter("______________________",PRINT_NORMAL);
					print (_NEW_LINE);*/
				}
				else
				{
					printCenter(transMsg->Verify_Method,PRINT_NORMAL);
					print (_NEW_LINE);          
				}
			}
			if(counter == 1)
				printCenter((char *)MERCHANT_RCPT,PRINT_NORMAL);
			else
				printCenter((char *)CLIENT_RCPT_TO_KEEP,PRINT_NORMAL);
       		print (_NEW_LINE);
			printCenter((char *)THANK_YOU,PRINT_BOLD);
			for(i=0;i<8;i++)
				print (_NEW_LINE);
				
			clrscr();
			ClearKbdBuff();
			KBD_FLUSH();
			if((!strcmp(transMsg->ResponseText,APPROVED_MSG)))
			{
				if(counter < 1 &&(paperOutFlag!=1)&&((strcmp((char *)transMsg->MessageTypeInd,REVERSAL_MSG_TYPE)!=0)))
				{
					counter++;
            		display_at (5, 10, "PRESS ENTER KEY FOR", CLR_EOL);
					//display_at (5, 11, "MERCHANT RECEIPT", CLR_EOL);
					display_at (8, 11, "MERCHANT RECEIPT", CLR_EOL);
            		ch = get_char();
				}
				else
				{       
					return _SUCCESS;
				}
			}
       
		}while(ch == KEY_CR  && counter <2);//press green key to print merchant receipt(2nd copy)
	}
    clrscr();
	return _SUCCESS; 
}

/*********************************************************************************************************
*	Function Name : RetriveDataForTotalReportReciept																												*
																																									                        *
*	Purpose		    : Retrive Data For TotalReport Reciept for merchant																				*
*	Input					:	Address of main transaction details structure and Address of main transaction details 
                  structure	                                                                              *
*	Output		    : returns success or failure or no data in the file																				*
																																						                              *
**********************************************************************************************************/
short RetriveDataForTotalReportReciept(TrDetails *transDetails,TotalReportDetails *settlDetails)
{

	FILE *ifp=NULL;
    short count = 0 ;
		
	char temp[13]={0};
		
	//float fSalAmount = 0.00;
	//float fWithAmount = 0.00;
	//float fTraAmount = 0.00;
	//float fRefAmount = 0.00;
	//float fDepAmount = 0.00;
	//float fVoidAmount = 0.00;
    //float fCreditAmount = 0.00;
    //float fDebitAmount = 0.00;
    //float fSumTransAmount = 0.00;
    
	//settlDetails->SalesCount = 0;
	//settlDetails->WithdrawCount = 0;
	//settlDetails->RefundCount = 0;
	//settlDetails->DepositCount = 0;
	//settlDetails->TransferCount = 0;
	//settlDetails->CreditCount = 0;
	//settlDetails->DebitCount = 0;
	//settlDetails->SumTransCount = 0;
		
	memset(transDetails,0,sizeof(TrDetails));
	settlDetails->activation_approved = 0;
	settlDetails->activation_declined = 0;
	settlDetails->activation_total = 0;
	settlDetails->reset_approved = 0;
	settlDetails->reset_declined = 0;
    settlDetails->reset_total = 0;
	settlDetails->change_approved = 0;
	settlDetails->change_declined = 0;
	settlDetails->change_total = 0;
	settlDetails->approved_total = 0;
	settlDetails->decline_total = 0;
	settlDetails->total_txns = 0;


	
	//settlDetails->SaleAmount[12]=0;//putting null char
	//settlDetails->RefundAmount[12]=0;//putting null char
    //settlDetails->CreditdAmount[12]=0;//putting null char
	//settlDetails->DebitAmount[12]=0;//putting null char
	//settlDetails->SumOfTrAmount[12]=0;//putting null char
		
	ifp = fopen(TRANS_DETAILS_FILE, "r");
	if (ifp == NULL)
	{
        if(LOG_STATUS == LOG_ENABLE)
        {
			LOG_PRINTF (("Failed to open the file %s\n", TRANS_DETAILS_FILE));	
        }
			return _FAIL;
	}
		
	while (fread(transDetails, sizeof(TrDetails), 1, ifp) != 0)
	{
		count++;
		//Activation
		if(transDetails->TransTypeFlg == ACTIVATIONMSGTYPE_CASE)//if transaction is activation
		{
			if (!strcmp(transDetails->trResponse, APPROVED_MSG))
			//if (!strcmp(transDetails->trResponse, "DECLINE"))
			{
				settlDetails->activation_approved++;
				settlDetails->activation_total++;
				settlDetails->total_txns++;
			}
			else
			{
				settlDetails->activation_declined++;
				settlDetails->activation_total++;
				settlDetails->total_txns++;
			}
		}
		//Pin change
		else if(transDetails->TransTypeFlg == PINCHANGEMSGTYPE_CASE)
		{
			if (!strcmp(transDetails->trResponse, APPROVED_MSG))
			{
				settlDetails->change_approved++;
				settlDetails->change_total++;
				settlDetails->total_txns++;
			}
			else
			{
				settlDetails->change_declined++;
				settlDetails->change_total++;
				settlDetails->total_txns++;
			}
		}
		//Pin reset
		else if (transDetails->TransTypeFlg == PINRESETMSGTYPE_CASE)
		{
			if (!strcmp(transDetails->trResponse, APPROVED_MSG))
			{
				settlDetails->reset_approved++;
				settlDetails->reset_total++;
				settlDetails->total_txns++;
			}
			else
			{
				settlDetails->reset_declined++;
				settlDetails->reset_total++;
				settlDetails->total_txns++;
			}
		}
	}
	if(count == 0)//if no data in file
    {
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("There is no transaction in file for total report"));
		}
		fclose(ifp);
		return (-2);
    }
    else
    {
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("In  else counter =%d",count));
		}
    }

	
	
		//--------------- ----------------- ---------------------

	fclose(ifp);
	return _SUCCESS;

}

short ClearLogs()
{
	int ret;
	char ch = 0;

	window(1, 1, 30, 20);
	clrscr();
	error_tone();
	write_at("CLEAR LOGS", strlen("CLEAR LOGS"), (30 - strlen("CLEAR LOGS")) / 2, 10);
	//write_at("CONTINUE WITHOUT PAPER?", strlen("CONTINUE WITHOUT PAPER?"), (30 - strlen("CONTINUE WITHOUT PAPER?")) / 2, 11);
	write_at(Dmsg[PRESS_ENTER_FOR_YES].dispMsg, strlen(Dmsg[PRESS_ENTER_FOR_YES].dispMsg), Dmsg[PRESS_ENTER_FOR_YES].x_cor, Dmsg[PRESS_ENTER_FOR_YES].y_cor);
	write_at(Dmsg[PRESS_CANCEL_FOR_NO].dispMsg, strlen(Dmsg[PRESS_CANCEL_FOR_NO].dispMsg), Dmsg[PRESS_CANCEL_FOR_NO].x_cor, Dmsg[PRESS_CANCEL_FOR_NO].y_cor);
	ClearKbdBuff();
	// KBD_FLUSH();
	do
	{
		ch = get_char();
	} while ((ch != KEY_CR) && (ch != KEY_CANCEL) && (ch != KEY_STR));
	LOG_PRINTF(("ch = %d", ch));
	if (ch == KEY_CR)
	{
		ret = CleanFileData();

		if (ret == _SUCCESS)
		{
			clrscr();
			write_at(Dmsg[LOG_CLEARED].dispMsg, strlen(Dmsg[LOG_CLEARED].dispMsg), Dmsg[LOG_CLEARED].x_cor, Dmsg[LOG_CLEARED].y_cor);
			SVC_WAIT(2000);
			ClearKbdBuff();
			KBD_FLUSH();
			return _SUCCESS;
		}
		else
		{
			clrscr();
			write_at(Dmsg[LOG_NOT_CLEARED].dispMsg, strlen(Dmsg[LOG_NOT_CLEARED].dispMsg), Dmsg[LOG_NOT_CLEARED].x_cor, Dmsg[LOG_NOT_CLEARED].y_cor);
			SVC_WAIT(2000);
			ClearKbdBuff();
			KBD_FLUSH();
			return _FAIL;
		}
	}
	return _SUCCESS;
}

/*********************************************************************************************************
*	Function Name : TotalReportReciept																																			*
*	Purpose		    : To Print TotalReport Reciept for merchant																				        *
*	Input					:	Pointer to the buffer which decides the recipet is print as close batch or total report *
*	Output		    : returns success or failure 																															*
**********************************************************************************************************/
short TotalReportReciept(char *recieptType)
{
	char merchantName[MERCHANT_NAME_SIZE]={0};
	char merchantAddress1[MERCHANT_ADDRESS_SIZE]={0};
	char merchantAddress2[MERCHANT_ADDRESS_SIZE]={0};
	char Merchant_Id[CARD_ACCEPTOR_ID_LEN+1]={0};
	char Terminal_Id[10]={0};
	char timeBuffer[16]={0};
	char sysDate[9] = {0};				// format date
    char sysTime[11] = {0};				// format time
	char countTrans[4]={0};
	char tempbuf[100] = { 0 };
	char ch = 0;
	
    short iRet =0 ;
	short i=0;
	TrDetails transDetails={0};
	TotalReportDetails settlDetails={0};
    char Batch_No[BATCH_ID_LEN+1]={0}; //added 
    short retPaperStatus = 0;

	retPaperStatus =Paperstatus();
    if(retPaperStatus == KEY_CANCEL)
		return KEY_CANCEL;
	iRet =	RetriveDataForTotalReportReciept(&transDetails,&settlDetails);
    if(iRet == _FAIL)
    {
		return _FAIL;
    }
    else if(iRet==-2)
    {     
		window(1,1,30,20);
		clrscr();
		display_at (5, 11, "NOTHING TO PRINT", CLR_EOL);
		SVC_WAIT(2000);
		ClearKbdBuff();
		KBD_FLUSH();
        return _SUCCESS;
    }
		
	get_env("#MERCHANT_NAME",merchantName,sizeof(merchantName));
	get_env("#MERCHANT_ADDRESS1",merchantAddress1,sizeof(merchantAddress1));
	get_env("#MERCHANT_ADDRESS2",merchantAddress2,sizeof(merchantAddress2));
	read_clock(timeBuffer); //reads the system clock time     
	sprintf(sysDate, "%c%c:%c%c:%c%c"   , timeBuffer[8], timeBuffer[9], timeBuffer[10], timeBuffer[11], timeBuffer[12], timeBuffer[13]);// hr:min:sec
	sprintf(sysTime,"%c%c/%c%c/%c%c", timeBuffer[4], timeBuffer[5], timeBuffer[6],  timeBuffer[7], timeBuffer[2], timeBuffer[3]); // mm/dd/yy	
    window(1,1,30,20);
	clrscr();
	write_at(Dmsg[PRINTING].dispMsg, strlen(Dmsg[PRINTING].dispMsg),Dmsg[PRINTING].x_cor, Dmsg[PRINTING].y_cor);//PRINTING
		//display_at (11, 10, "PRINTING ...", CLR_EOL);
		//print ("   ");
	printCenter((char *)merchantName,PRINT_BOLD);
		//printBold (merchantName);//print merchant Name
	print (_NEW_LINE);
		//print ("   ");
	printCenter((char *)merchantAddress1,PRINT_BOLD);
	print (_NEW_LINE);
	printCenter((char *)merchantAddress2,PRINT_BOLD);
	print (_NEW_LINE);
	print (_NEW_LINE);
	printCenter((char *)recieptType,PRINT_BOLD);
    //printBold ("     SUB TOTAL");
	print (_NEW_LINE);
	print (_NEW_LINE);
	get_env("#CARDACCEPTERID",Merchant_Id,sizeof(Merchant_Id));
	print (MERCHANTID);
	print (Merchant_Id);//print Merchant Id
	print (" ");
	get_env("#TERMINAL_ID",Terminal_Id,sizeof(Terminal_Id));
	print(TERMINALID);
	print(Terminal_Id);//print Terminal Id
		
	print (_NEW_LINE);
	print(sysTime);//Print System time
	print("            ");
	print(sysDate);//print System Date
	print (_NEW_LINE);
	print (_NEW_LINE);
	print (_NEW_LINE);
	get_env("#TRACE_BATCH_NO",Batch_No,sizeof(Batch_No));//getting batch no
    print (BATCH_ID);
	print(Batch_No);
	print (_NEW_LINE);
	print (_NEW_LINE);
	print (_NEW_LINE);
	printCenter((char *)"ACTIVITY SUMMARY",PRINT_BOLD);
		
	print (_NEW_LINE);
	print (_NEW_LINE);
				
	print ("CARD ACTIVATION");
	print(_NEW_LINE);
	sprintf(tempbuf, "APPROVED: %d ", settlDetails.activation_approved);
	print (tempbuf);
	print(_NEW_LINE);
	sprintf(tempbuf, "DECLINED: %d ", settlDetails.activation_declined);
	print(tempbuf);
	print(_NEW_LINE);
	sprintf(tempbuf, "TOTAL ACTIVATIONS: %d ", settlDetails.activation_total);
	print(tempbuf);
	print(_NEW_LINE);
	print(_NEW_LINE);
	print("PIN CHANGE");
	print(_NEW_LINE);
	sprintf(tempbuf, "APPROVED: %d ", settlDetails.change_approved);
	print(tempbuf);
	print(_NEW_LINE);
	sprintf(tempbuf, "DECLINED: %d ", settlDetails.change_declined);
	print(tempbuf);
	print(_NEW_LINE);
	sprintf(tempbuf, "TOTAL PIN CHANGE: %d ", settlDetails.change_total);
	print(tempbuf);
	print(_NEW_LINE);
	print(_NEW_LINE);
	print("PIN RESET");
	print(_NEW_LINE);
	sprintf(tempbuf, "APPROVED: %d ", settlDetails.reset_approved);
	print(tempbuf);
	print(_NEW_LINE);
	sprintf(tempbuf, "DECLINED: %d ", settlDetails.reset_declined);
	print(tempbuf);
	print(_NEW_LINE);
	sprintf(tempbuf, "TOTAL PIN RESET: %d ", settlDetails.reset_total);
	print(tempbuf);
	print(_NEW_LINE);
	print(_NEW_LINE);
	sprintf(tempbuf, "GRAND TOTAL APPROVED: %d ", settlDetails.activation_approved + settlDetails.change_approved + settlDetails.reset_approved);
	print(tempbuf);
	print(_NEW_LINE);
	sprintf(tempbuf, "GRAND TOTAL DECLINED: %d ", settlDetails.activation_declined + settlDetails.change_declined + settlDetails.reset_declined);
	print(tempbuf);
	//paddCount(settlDetails.SalesCount,countTrans);
	//print (countTrans);
	//print ("            ");
	//print (settlDetails.SaleAmount);
	//print (_NEW_LINE);//for Withdraw
	//print ("WITHDRAW");
	//print ("          ");
	//paddCount(settlDetails.WithdrawCount,countTrans);
	//print (countTrans);
	//print ("            ");
	//print (settlDetails.WithdrawAmount);
	//print (_NEW_LINE);//for refund
	//print ("REFUND");
	//print ("            ");
	//paddCount(settlDetails.RefundCount,countTrans);
	//print (countTrans);
	//print ("            ");
	//print (settlDetails.RefundAmount);
	//print (_NEW_LINE);//for Deposit
	//print ("DEPOSIT");
	//print ("           ");
	//paddCount(settlDetails.DepositCount,countTrans);
	//print (countTrans);
	//print ("            ");
	//print (settlDetails.DepositAmount);
	//print (_NEW_LINE);//for Transfer
	//print ("TRANSFER");
	//print ("          ");
	//paddCount(settlDetails.TransferCount,countTrans);
	//print (countTrans);
	//print ("            ");
	//print (settlDetails.TransferAmount);

	//print (_NEW_LINE);
	//printCenter((char *)CREATE_LINE,PRINT_BOLD);

	//print (_NEW_LINE);//for Sum
	//print ("SUM");
	//print ("               ");
	//paddCount(settlDetails.SumTransCount,countTrans);
	//print (countTrans);
	//print ("            ");
	/*
	print (_NEW_LINE);//for void
	print ("VOID");
	print ("              ");
	paddCount(settlDetails.VoidCount,countTrans);
	print (countTrans);
	print ("            ");
	print (settlDetails.VoidAmount);	
    //print (settlDetails.SumOfTrAmount);		//Commented for Agency Bank 
	*/
    //print (_NEW_LINE);
	//print (_NEW_LINE);
		
	//printCenter((char *)"TOTAL",PRINT_BOLD);
	//print (_NEW_LINE);
	//print (_NEW_LINE);

	//print ("DEBIT");//for Debit
	//print ("             ");
	//paddCount(settlDetails.DebitCount,countTrans);
	//print (countTrans);
	//print ("            ");
	
    //print (settlDetails.DebitAmount);
    //print (_NEW_LINE);//for Credit
	//print ("CREDIT");
	//print ("            ");
	//paddCount(settlDetails.CreditCount,countTrans);
	//print (countTrans);
	//print ("            ");
		
    //print (settlDetails.CreditdAmount);
	//print (_NEW_LINE);
	//print (CREATE_LINE);
    //print (_NEW_LINE);//for Sum
	//print ("SUM");
	//print ("               ");
	//paddCount(settlDetails.SumTransCount,countTrans);
	//print (countTrans);
	//print ("            ");
		
    //print (settlDetails.SumOfTrAmount);
	//print (_NEW_LINE);
	//print (_NEW_LINE);

	printCenter((char *)CREATE_LINE,PRINT_BOLD);
	printCenter((char *)"END",PRINT_BOLD);

    for(i=0;i<8;i++)
		print (_NEW_LINE);

	window(1, 1, 30, 20);
	clrscr();
	error_tone();
	write_at("CLEAR LOGS", strlen("CLEAR LOGS"), (30 - strlen("CLEAR LOGS")) / 2, 10);
	//write_at("CONTINUE WITHOUT PAPER?", strlen("CONTINUE WITHOUT PAPER?"), (30 - strlen("CONTINUE WITHOUT PAPER?")) / 2, 11);
	write_at(Dmsg[PRESS_ENTER_FOR_YES].dispMsg, strlen(Dmsg[PRESS_ENTER_FOR_YES].dispMsg), Dmsg[PRESS_ENTER_FOR_YES].x_cor, Dmsg[PRESS_ENTER_FOR_YES].y_cor);
	write_at(Dmsg[PRESS_CANCEL_FOR_NO].dispMsg, strlen(Dmsg[PRESS_CANCEL_FOR_NO].dispMsg), Dmsg[PRESS_CANCEL_FOR_NO].x_cor, Dmsg[PRESS_CANCEL_FOR_NO].y_cor);
	ClearKbdBuff();
	// KBD_FLUSH();
	do
	{
		ch = get_char();
	} while ((ch != KEY_CR) && (ch != KEY_CANCEL) && (ch != KEY_STR));
	LOG_PRINTF(("ch = %d", ch));
	if (ch == KEY_CR)
	{
		ClearLogs();
	}
		
	clrscr();
    return _SUCCESS;
}
/*********************************************************************************************************
*	Function Name : RetriveDataForTransDetailsReciept																												*
*	Purpose		    : Retrive Data For Transaction Details Reciept for merchant																*
*	Input					:	Pointer to the transaction details structue and Total report details structure          *
*	Output		    : returns success or failure  or no data in the file																			*
**********************************************************************************************************/
short RetriveDataForTransDetailsReciept(TrDetails *transDetails,TotalReportDetails *settlDetails)
{

		FILE *ifp=NULL;
		short count  = 0;
		char temp[13]={0};
		float fSalAmount = 0.00;
		float fRefAmount = 0.00;
		float fVoidAmount = 0.00;
    float fCreditAmount = 0.00;
    float fDebitAmount = 0.00;
    float fSumTransAmount = 0.00;
    
		//settlDetails->SalesCount = 0;
		//settlDetails->RefundCount = 0;
    //settlDetails->VoidCount = 0;
    //settlDetails->CreditCount = 0;
		//settlDetails->DebitCount = 0;

//settlDetails->SumTransCount = 0;
		
		memset(transDetails,0,sizeof(TrDetails));
   // memset(settlDetails->SaleAmount,'0',sizeof(settlDetails->SaleAmount));
	//	memset(settlDetails->RefundAmount,'0',sizeof(settlDetails->RefundAmount));
   // memset(settlDetails->DebitAmount,'0',sizeof(settlDetails->DebitAmount));
	//	memset(settlDetails->CreditdAmount,'0',sizeof(settlDetails->CreditdAmount));

   // memset(settlDetails->SumOfTrAmount,'0',sizeof(settlDetails->SumOfTrAmount));
	  
	//	settlDetails->SaleAmount[12]=0;//putting null char
	//	settlDetails->RefundAmount[12]=0;//putting null char
   // settlDetails->CreditdAmount[12]=0;//putting null char
	//	settlDetails->DebitAmount[12]=0;//putting null char

  //  settlDetails->SumOfTrAmount[12]=0;//putting null char
		
		ifp = fopen(TRANS_DETAILS_FILE, "r");
		if (ifp == NULL)
		{
        if(LOG_STATUS == LOG_ENABLE)
        {
				  LOG_PRINTF (("Failed to open the file %s\n", TRANS_DETAILS_FILE));	
        }
				return _FAIL;
		}
		
		while (fread(transDetails, sizeof(TrDetails), 1, ifp) != 0)
		{
        count++;
      
				//if((transDetails->TransTypeFlg == SALEMSGTYPE_CASE || transDetails->TransTypeFlg == WITHDRAWAL_MSG_TYPE_CASE))//if trans is debit sale type
				if((transDetails->TransTypeFlg == SALEMSGTYPE_CASE))//if trans is debit sale type
				{
						
						fSalAmount = fSalAmount + (float)atof(transDetails->amount);//need to put void also
						//settlDetails->SalesCount++;
          
            fSumTransAmount = fSumTransAmount + (float)atof(transDetails->amount);//need to put void also
						//settlDetails->SumTransCount++;
            
            if(transDetails->TransMethord_Flag==edebit)
            {
              //settlDetails->DebitCount++;
              fDebitAmount = fDebitAmount + (float)atof(transDetails->amount);     
             
            }
            else if(transDetails->TransMethord_Flag==ecredit)
            {
              //settlDetails->CreditCount++;
              fCreditAmount = fCreditAmount + (float)atof(transDetails->amount);     
            }
						
				}
				else if((transDetails->TransTypeFlg== REFUNDMSGTYPE_CASE))//if trans is debit refund type
				{
						
						fRefAmount = fRefAmount + (float)atof(transDetails->amount);
						//settlDetails->RefundCount++;

            fSumTransAmount = fSumTransAmount - (float)atof(transDetails->amount);//need to put void also
						//settlDetails->SumTransCount++;
             if(transDetails->TransMethord_Flag==edebit)
            {
              // settlDetails->DebitCount++;
               fDebitAmount = fDebitAmount - (float)atof(transDetails->amount);     
              
            }
            else if(transDetails->TransMethord_Flag==ecredit)
            {
             // settlDetails->CreditCount++;
              fCreditAmount = fCreditAmount - (float)atof(transDetails->amount);     
            }
						
				}
        else if((transDetails->TransTypeFlg == VOIDMSGTYPE_CASE))//if trans is debit refund type
				{
						
						fVoidAmount = fVoidAmount + (float)atof(transDetails->amount);
						//settlDetails->VoidCount++;
						
            fSumTransAmount = fSumTransAmount -  (float)atof(transDetails->amount);//need to put void also
						//settlDetails->SumTransCount--;
            if(transDetails->TransMethord_Flag==edebit)
            {
             //settlDetails->DebitCount--;
              fDebitAmount = fDebitAmount - (float)atof(transDetails->amount);     
              
            }
            else if(transDetails->TransMethord_Flag==ecredit)
            {
              //settlDetails->CreditCount--;
              fCreditAmount = fCreditAmount - (float)atof(transDetails->amount);     
            }
				}
		    
		}
	  if(count == 0)//if no data in file
    {
      if(LOG_STATUS == LOG_ENABLE)
      {
        LOG_PRINTF(("There is no transaction in file for Trans details"));
      }
      fclose(ifp);
      return (-2);
    }
    else
    {
      if(LOG_STATUS == LOG_ENABLE)
      {
        LOG_PRINTF(("In  else counter =%d",count));
      }
    }
		sprintf(temp,"%f",fSalAmount);
		//sprintf(settlDetails->SaleAmount,"%8.02f",fSalAmount);
    if(LOG_STATUS == LOG_ENABLE)
    {
		//  LOG_PRINTF(("sale amount =%s",settlDetails->SaleAmount));
		//  LOG_PRINTF(("sale count =%d",settlDetails->SalesCount));
    }

		memset(temp,0,sizeof(temp));
		sprintf(temp,"%f",fRefAmount);
		//sprintf(settlDetails->RefundAmount,"%8.02f",fRefAmount);
    if(LOG_STATUS == LOG_ENABLE)
    {
		//  LOG_PRINTF(("refund amount =%s",settlDetails->RefundAmount));
		//  LOG_PRINTF(("refund count =%d",settlDetails->RefundCount));
    }


    memset(temp,0,sizeof(temp));
		sprintf(temp,"%f",fVoidAmount);
		//sprintf(settlDetails->VoidAmount,"%8.02f",fVoidAmount);
    if(LOG_STATUS == LOG_ENABLE)
    {
    //  LOG_PRINTF(("Void amount =%s",settlDetails->VoidAmount));
		//  LOG_PRINTF(("Void count =%d",settlDetails->VoidCount));
    }

    memset(temp,0,sizeof(temp));
		sprintf(temp,"%f",fDebitAmount);
		//sprintf(settlDetails->DebitAmount,"%8.02f",fDebitAmount);
    if(LOG_STATUS == LOG_ENABLE)
    {
		//  LOG_PRINTF(("Debit amount =%s",settlDetails->DebitAmount));
		 // LOG_PRINTF(("Debit count =%d",settlDetails->DebitCount));
    }

   
    memset(temp,0,sizeof(temp));
		sprintf(temp,"%f",fCreditAmount);
		//sprintf(settlDetails->CreditdAmount,"%8.02f",fCreditAmount);
    if(LOG_STATUS == LOG_ENABLE)
    {
		  //LOG_PRINTF(("Credit amount =%s",settlDetails->CreditdAmount));
		  //LOG_PRINTF(("Credit count =%d",settlDetails->CreditCount));
    }

     memset(temp,0,sizeof(temp));
     sprintf(temp,"%f",fSumTransAmount);
		// sprintf(settlDetails->SumOfTrAmount,"%8.02f",fSumTransAmount);
     if(LOG_STATUS == LOG_ENABLE)
     {
		  //LOG_PRINTF(("Sum of Tr amount =%s",settlDetails->SumOfTrAmount));
		  //LOG_PRINTF(("Sum of Tr count =%d",settlDetails->SumTransCount));
     }

		//--------------- ----------------- ---------------------

		fclose(ifp);
		return _SUCCESS;

}
/*********************************************************************************************************
*	Function Name : TransactionDetailsReciept																																*
*	Purpose		    : To Print Transaction Details Reciept for merchant																				*
*	Input					:	void                                                                                    *
*	Output		    : returns short success or failure 																												*
**********************************************************************************************************/

short TransactionDetailsReciept(void)
{
		char merchantName[MERCHANT_NAME_SIZE]={0};
		char merchantAddress1[MERCHANT_ADDRESS_SIZE]={0};
		char merchantAddress2[MERCHANT_ADDRESS_SIZE]={0};
		char Merchant_Id[CARD_ACCEPTOR_ID_LEN+1]={0};
		char Terminal_Id[10]={0};
		char timeBuffer[16]={0};
		char sysDate[9] = {0};				// format date
    char sysTime[9] = {0};				// format time
		char countTrans[4]={0};
		char ch = 0;
	
    short iRet =0,i=0 ;
    TrDetails transDetails={0};
		TotalReportDetails settlDetails={0};


    char Batch_No[BATCH_ID_LEN+1]={0}; //added 
    short retPaperStatus = 0;
		retPaperStatus =Paperstatus();
    if(retPaperStatus == KEY_CANCEL)
      return KEY_CANCEL;
    //iRet =	RetriveDataForTransDetailsReciept(&transDetails,&settlDetails);
	iRet =	RetriveDataForTotalReportReciept(&transDetails,&settlDetails);
    if(iRet==_FAIL)
    {
      return _FAIL;
    }
    else if(iRet == -2)
    {     
      window(1,1,30,20);
			clrscr();
      display_at (5, 11, "NO TRANSACTION", CLR_EOL);
      SVC_WAIT(2000);
      ClearKbdBuff();
			KBD_FLUSH();
                    
      return _SUCCESS;
    }
		
		get_env("#MERCHANT_NAME",merchantName,sizeof(merchantName));
		get_env("#MERCHANT_ADDRESS1",merchantAddress1,sizeof(merchantAddress1));
		get_env("#MERCHANT_ADDRESS2",merchantAddress2,sizeof(merchantAddress2));
		read_clock(timeBuffer); //reads the system clock time     
		sprintf(sysTime, "%c%c:%c%c:%c%c"   , timeBuffer[8], timeBuffer[9], timeBuffer[10], timeBuffer[11], timeBuffer[12], timeBuffer[13]);// hr:min:sec
		sprintf(sysDate,"%c%c/%c%c/%c%c", timeBuffer[4], timeBuffer[5], timeBuffer[6],  timeBuffer[7],  timeBuffer[2],  timeBuffer[3]); // mm/dd/yy	
    window(1,1,30,20);
		clrscr();
		write_at(Dmsg[PRINTING].dispMsg, strlen(Dmsg[PRINTING].dispMsg),Dmsg[PRINTING].x_cor, Dmsg[PRINTING].y_cor);//PRINTING
	
		printCenter((char *)merchantName,PRINT_BOLD);
	
		print (_NEW_LINE);
		printCenter((char *)merchantAddress1,PRINT_BOLD);
		print (_NEW_LINE);
		printCenter((char *)merchantAddress2,PRINT_BOLD);
		print (_NEW_LINE);
		print (_NEW_LINE);
    printCenter((char *)"TRANSACTIONS DETAILS",PRINT_BOLD);
	
		print (_NEW_LINE);
		print (_NEW_LINE);
		get_env("#CARDACCEPTERID",Merchant_Id,sizeof(Merchant_Id));
		print (MERCHANTID);
		print (Merchant_Id);//print Merchant Id
		print (" ");
		get_env("#TERMINAL_ID",Terminal_Id,sizeof(Terminal_Id));
		print(TERMINALID);
		print(Terminal_Id);//print Terminal Id
		print (_NEW_LINE);
	  print(sysDate);//Print System time 
	  print("            ");
		print(sysTime);//print System Date
		print (_NEW_LINE);
		print (_NEW_LINE);
		print (_NEW_LINE);
		get_env("#TRACE_BATCH_NO",Batch_No,sizeof(Batch_No));//getting batch no
    print (BATCH_ID);
		print(Batch_No);
		print (_NEW_LINE);
		print (_NEW_LINE);
		print (_NEW_LINE);
		printCenter((char *)"TRANSACTION SUMMARY",PRINT_BOLD);
		print (_NEW_LINE);
		print (_NEW_LINE);
		//Sale
		//print ("SALE");
		//print ("              ");
		//paddCount(settlDetails.SalesCount,countTrans);
		//print (countTrans);
		//print ("            ");
		//print (settlDetails.SaleAmount);
		//Withdraw
		//print (_NEW_LINE);
		//print ("WITHDRAW");
		//print ("          ");
		//paddCount(settlDetails.WithdrawCount,countTrans);
		//print (countTrans);
		//print ("            ");
		//print (settlDetails.WithdrawAmount);
		//Deposit
		//print (_NEW_LINE);
		//print ("DEPOSIT");
		//print ("           ");
		//paddCount(settlDetails.DepositCount,countTrans);
		//print (countTrans);
		//print ("            ");
		//print (settlDetails.DepositAmount);
		//Transfer
		//print (_NEW_LINE);
		//print ("TRANSFER");
		//print ("          ");
		//paddCount(settlDetails.TransferCount,countTrans);
		//print (countTrans);
		//print ("            ");
		//print (settlDetails.TransferAmount);
		
		//print (_NEW_LINE);//for refund
		//print ("REFUND");
		//print ("            ");
		//paddCount(settlDetails.RefundCount,countTrans);
		//print (countTrans);
		//print ("            ");
   // print (settlDetails.RefundAmount);
    /*print (_NEW_LINE);//for void
		print ("VOID");
		print ("              ");
		paddCount(settlDetails.VoidCount,countTrans);
		print (countTrans);
		print ("            ");
    print (settlDetails.VoidAmount);
	*/
    //print (_NEW_LINE);//for Sum
	//	print ("SUM");
	//	print ("               ");
		//paddCount(settlDetails.SumTransCount,countTrans);
	//	print (countTrans);
	//	print ("            ");
    //print (settlDetails.SumOfTrAmount);
		 print (_NEW_LINE);
//    print (_NEW_LINE);
		
		printCenter((char *)CREATE_LINE,PRINT_BOLD);

    printReciptForTransDetails(&transDetails,sysTime,sysDate);

		print (_NEW_LINE);
		print (_NEW_LINE);
//		printCenter((char *)"TOTAL",PRINT_BOLD);
//    print (_NEW_LINE);
//		print (_NEW_LINE);

//		print ("DEBIT");//for Debit
//		print ("             ");
		//paddCount(settlDetails.DebitCount,countTrans);
//		print (countTrans);
//		print ("            ");
    //print (settlDetails.DebitAmount);
 //   print (_NEW_LINE);//for Credit
	//	print ("CREDIT");
	//	print ("            ");
		//paddCount(settlDetails.CreditCount,countTrans);
	//	print (countTrans);
	//	print ("            ");
   // print (settlDetails.CreditdAmount);

   // print (_NEW_LINE);//for Sum
	//	print ("SUM");
	//	print ("               ");
		//paddCount(settlDetails.SumTransCount,countTrans);
	//	print (countTrans);
	//	print ("            ");
		
    //print (settlDetails.SumOfTrAmount);
	//	print (_NEW_LINE);
	//	print (_NEW_LINE);

    printCenter((char *)CREATE_LINE,PRINT_BOLD);
		printCenter((char *)"END",PRINT_BOLD);
    
		clrscr();
		for(i=0;i<8;i++)
				print (_NEW_LINE);



	window(1, 1, 30, 20);
	clrscr();
	error_tone();
	write_at("CLEAR LOGS", strlen("CLEAR LOGS"), (30 - strlen("CLEAR LOGS")) / 2, 10);
	//write_at("CONTINUE WITHOUT PAPER?", strlen("CONTINUE WITHOUT PAPER?"), (30 - strlen("CONTINUE WITHOUT PAPER?")) / 2, 11);
	write_at(Dmsg[PRESS_ENTER_FOR_YES].dispMsg, strlen(Dmsg[PRESS_ENTER_FOR_YES].dispMsg), Dmsg[PRESS_ENTER_FOR_YES].x_cor, Dmsg[PRESS_ENTER_FOR_YES].y_cor);
	write_at(Dmsg[PRESS_CANCEL_FOR_NO].dispMsg, strlen(Dmsg[PRESS_CANCEL_FOR_NO].dispMsg), Dmsg[PRESS_CANCEL_FOR_NO].x_cor, Dmsg[PRESS_CANCEL_FOR_NO].y_cor);
	ClearKbdBuff();
	// KBD_FLUSH();
	do
	{
		ch = get_char();
	} while ((ch != KEY_CR) && (ch != KEY_CANCEL) && (ch != KEY_STR));
	LOG_PRINTF(("ch = %d", ch));
	if (ch == KEY_CR)
	{
		ClearLogs();
	}

    return _SUCCESS;
}

/*********************************************************************************************************
*	Function Name : printReciptForTransDetails																															*
*	Purpose		    : To Print Each Transaction Details on Reciept stored in the file													*
*	Input					:	void                                                                                    *
*	Output		    : returns short success or failure 																												*
**********************************************************************************************************/

short printReciptForTransDetails(TrDetails *transDetails,char *sysTime,char *sysDate)
{
		FILE *ifp=NULL;
		int i = 0,Pos=0;
		char accountNo[ACC_NUMBER_LEN]={0};//22
		char printExpDate[6]={0};
		char curr_code[4];
	
		memset(transDetails,0,sizeof(TrDetails));
		get_env("#CURRENCY_CODE",curr_code,sizeof(curr_code));
		ifp = fopen(TRANS_DETAILS_FILE, "r");
		if (ifp == NULL)
		{
			if(LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF (("Failed to open the file %s\n", TRANS_DETAILS_FILE));	
			}
		return _FAIL;
		}

		while (fread(transDetails, sizeof(TrDetails), 1, ifp) != 0)
		{
    		print(transDetails->orgTransDate);//Print System time
			print("        ");
			print(transDetails->orgTransTime);//print System Date
			print (_NEW_LINE);

				// print the name
			print (transDetails->cardHolderName);
			print (_NEW_LINE);

				// print Account
				//print (transDetails->cardNumber);
			strcpy(accountNo,transDetails->cardNumber);
			for(Pos=strlen(transDetails->cardNumber)-10;Pos<strlen(transDetails->cardNumber)-4;Pos++)
					accountNo[Pos]='*';
			accountNo[strlen(accountNo)-1]='*';
			print (accountNo);
			print ("    ");
				
			memset(printExpDate,0,sizeof(printExpDate));
			strncpy(printExpDate,transDetails->expDate,2);
			strcat(printExpDate,"/");
			strcat(printExpDate,transDetails->expDate+2);
				// print the expiry Date
			
				print (printExpDate);
				print (_NEW_LINE);

				// print the Total Amount
				//print (_AMOUNT);
				//print ("             ");
				//print (transDetails->amount);
				//print (" ");
				//print (curr_code);
				//print ("  ");
				
				//print (_NEW_LINE);
	
        //if(transDetails->EMV_Flag == 1)
        //{
        //  if(strcmp((char *)transDetails->CARD_TYPE,"")!=0)//if card type is not empty
        //  {
		//		    print(transDetails->CARD_TYPE);
        //    for(i =0;i<20-strlen(transDetails->CARD_TYPE);i++)
		//		    print (" ");
        //  }
        //}
				
        print(transDetails->trans_Type);//sale
				print (_NEW_LINE);
				
        //print ("AUTHOR #:");
        //print (transDetails->AuthIdResponse);
				print ("     ");
        print("Trace: ");
        print(transDetails->traceAuditNo);
				print ("\n");
        //if(transDetails->EMV_Flag == 1)//if transaction is an EMV transaction
        //{
		//		  print ("TVR: ");
		//		  print (transDetails->TVR);
        //  print ("     ");
		//			print("TSI: ");
		//			print(transDetails->TSI);
		//			print (_NEW_LINE);
        //}
		print("Response #: ");
		print(transDetails->trResponse);
		print (_NEW_LINE);
		print("Operator ID: ");
		print(transDetails->operatorID);
		print(_NEW_LINE);
        printCenter((char *)CREATE_LINE,PRINT_BOLD);
		}
		fclose(ifp);
		return _SUCCESS;

}

short Paperstatus()
{
  short stat = _FAIL;
  short RetVal = _FAIL;
  int mask = _FAIL ;
  LOG_PRINTF(("in the PaperStatus"));
  
  window(1,1,30,20);
	clrscr();
	
	write_at("INSERT PAPER ROLL", strlen("INSERT PAPER ROLL"),(30-strlen("INSERT PAPER ROLL"))/2, 10);
	write_at("FEED PAPER OR", strlen("FEED PAPER OR"),(30-strlen("FEED PAPER OR"))/2, 12);
	write_at("PRESS CANCEL TO EXIT", strlen("PRESS CANCEL TO EXIT"),(30-strlen("PRESS CANCEL TO EXIT"))/2, 13);			
  do
  {
    LOG_PRINTF(("in the do while"));
    if(!p3300_status(hPrinter,1000))
    {
      paperOutFlag = 0;
      LOG_PRINTF(("paper feeded"));
      return _SUCCESS;
    }
    ClearKbdBuff(); //clearing keyboard buffer
	  
    mask = read_event();					//Collect read interrupt information 	
		mask = wait_event();//waiting for event
     

    if(mask & EVT_KBD) //checking for keyboard event
		{
			
        RetVal = get_char();
				if(LOG_STATUS == LOG_ENABLE)
        {
          LOG_PRINTF (("KBD...%d",RetVal));
        }
        if(RetVal == KEY_CANCEL)
          return KEY_CANCEL ;
				
		}
    error_tone();
    SVC_WAIT(300);
  }while(stat != 0);
  return _FAIL ;
}


short printUserAudit()
{
	FILE *ifp = NULL;
	int i = 0, Pos = 0;
	UserAudit useraudit;
	char timeBuffer[16] = { 0 };
	char sysDate[9] = { 0 };				// format date
	char sysTime[11] = { 0 };				// format time


	printCenter((char *)"End of Day Audit Report", PRINT_BOLD);
	read_clock(timeBuffer); //reads the system clock time     
	sprintf(sysDate, "%c%c:%c%c:%c%c", timeBuffer[8], timeBuffer[9], timeBuffer[10], timeBuffer[11], timeBuffer[12], timeBuffer[13]);// hr:min:sec
	sprintf(sysTime, "%c%c/%c%c/%c%c", timeBuffer[4], timeBuffer[5], timeBuffer[6], timeBuffer[7], timeBuffer[2], timeBuffer[3]); // mm/dd/yy	
	

	print(_NEW_LINE);
	print("Run Date#:");
	print(sysTime);//Print System time
	print("     ");
	print(sysDate);//print System Date
	print(_NEW_LINE);
	print(_NEW_LINE);
	print(_NEW_LINE);

	printCenter((char *)CREATE_LINE, PRINT_BOLD);
	print(_NEW_LINE);
	print(_NEW_LINE);
	
	memset(&useraudit, 0, sizeof(UserAudit));
	
	ifp = fopen(USER_AUDIT_FILE, "r");
	if (ifp == NULL)
	{
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Failed to open the file %s\n", TRANS_DETAILS_FILE));
		}
		return _FAIL;
	}

	while (fread(&useraudit, sizeof(UserAudit), 1, ifp) != 0)
	{
		print(useraudit.Time);//Print System time
		print("        ");
		print(useraudit.Date);//print System Date
		print(_NEW_LINE);

		// print the name
		print("Operator #:");
		print(useraudit.operID);
		print(_NEW_LINE);
		print("Action:");
		switch (useraudit.code)
		{
		case 'C':
			print("Password Change");
			break;
		case 'N':
			print("New Operator");
			break;
		case 'D':
			print("Operator Deleted");
			break;
		case 'R':
			print("Password Reset");
			break;
		}
		
		print(_NEW_LINE);
		
		print(_NEW_LINE);
		printCenter((char *)CREATE_LINE, PRINT_BOLD);
	}
	fclose(ifp);

	print(_NEW_LINE);
	print(_NEW_LINE);
	print(_NEW_LINE);


	return _SUCCESS;
}
