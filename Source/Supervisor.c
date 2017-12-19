/***********************************************************************************************
  Filename		     : Supervisor.c
  Project		       : Bevertec 
  Developer Neame  : Vikas
  Module           : Change merchant password ,validate 
  Description	     : This is the implementation of validation of merchant ,supervisor password
 ***********************************************************************************************/
#include "..\include\Supervisor.h"
#include "..\Include\TouchScreen.h"
#include <svc.h>
#include <svctxo.h>
#include <applidl.h>
#include "..\include\Common.h"
 
extern short hClock;

/****************************************************************************************************************
*	Function Name : validateSupervisorPassword																																		*
*	Purpose		  : This feature allows the validate Supervisor Password          																	*
*	Input				: int variable to check transaction is debit or not																								*
*	Output		  : Return value for the success or failure																													*
*****************************************************************************************************************/

short validateSupervisorPassword()
{
    char Password[SUPER_PWD_LEN+1]={0};//7
	char tempPassword[SUPER_PWD_LEN+1]={0};//7
    char tempPassword1[SUPER_PWD_LEN+1]={0};//7
	short count = 0;
    char attamptLeft[13]={0};
    short iRetVal =0 ;
    char timeLimit[TIME_ARR_LEN]={0};
	short superPwdLength=0;
    short time_limit=0;
	get_env("#SUPER_PASSWORD",Password,sizeof(Password));
    get_env("#TIME_OUT_LIMIT_IN_SECOND",timeLimit,sizeof(timeLimit));
	superPwdLength = strlen(Password);
    time_limit = atoi(timeLimit);
	time_limit = time_limit*100;
	do //Password validation
	{
		window(1,1,30,20);
		clrscr ();
		if(count !=0 && count != 3)
        {
			write_at(Dmsg[INVALID_PASSWORD].dispMsg, strlen(Dmsg[INVALID_PASSWORD].dispMsg),Dmsg[INVALID_PASSWORD].x_cor, Dmsg[INVALID_PASSWORD].y_cor);//invalid password
            sprintf(attamptLeft,"%d ATTEMPT LEFT",(3-count));
			write_at(attamptLeft,strlen(attamptLeft),(30-strlen(attamptLeft))/2,Dmsg[INVALID_PASSWORD].y_cor-2);
		}
        else if(count ==3)
        {
			window(1,1,30,20);
			clrscr ();
			write_at(Dmsg[INVALID_PASSWORD].dispMsg, strlen(Dmsg[INVALID_PASSWORD].dispMsg),Dmsg[INVALID_PASSWORD].x_cor, Dmsg[INVALID_PASSWORD].y_cor);
			SVC_WAIT(1000);
			if(LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("Invalid Password"));
			}
			return _FAIL;
        }
		write_at(Dmsg[ENTER_SUPERVISOR_PASSWORD].dispMsg, strlen(Dmsg[ENTER_SUPERVISOR_PASSWORD].dispMsg),Dmsg[ENTER_SUPERVISOR_PASSWORD].x_cor, Dmsg[ENTER_SUPERVISOR_PASSWORD].y_cor);
 		window(12,Dmsg[ENTER_SUPERVISOR_PASSWORD].y_cor+2,22,Dmsg[ENTER_SUPERVISOR_PASSWORD].y_cor+2);
		clrscr ();
		memset(tempPassword,0,sizeof(tempPassword));
		memset(tempPassword1,0,sizeof(tempPassword1));
		iRetVal = getkbd_entry (hClock, "",(signed char *)tempPassword, (unsigned int) time_limit,(unsigned int) PASSWORD, (char*) szKeyMap,sizeof(szKeyMap),superPwdLength, superPwdLength);
		if(iRetVal == BTN_CANCEL)
		{
		    return KEY_CANCEL;
		}
		else if(iRetVal == 0)
		{
		    // Time Out
			window(1,1,30,20);
		    clrscr();
		    error_tone();
		    SVC_WAIT(190);
		    error_tone();
		    write_at(TIME_OUT, strlen(TIME_OUT),(30-strlen(TIME_OUT))/2, 10);
		    SVC_WAIT(4000);
		    ClearKbdBuff();
			KBD_FLUSH();
		    return _FAIL;
		}
        count++;
        strncpy(tempPassword1,tempPassword,SUPER_PWD_LEN);
		if(LOG_STATUS == LOG_ENABLE)
        {
			LOG_PRINTF(("Entered Supervisor Password = %s",tempPassword));
			LOG_PRINTF(("Entered Supervisor Password1 = %s",tempPassword1));
			LOG_PRINTF(("Supervisor Password = %s",Password));
        }
	}while(strcmp(Password,tempPassword1));
	clrscr();
	return _SUCCESS;
}
/****************************************************************************************************************
*	Function Name : validateMerchantPassword																																		*
*	Purpose		    : This feature allows the validate Merchant Password          																	*
*	Input				  : void                                																								*
*	Output		    : Return value for the success or failure																													*
*****************************************************************************************************************/

short validateMerchantPassword()
{
	char Password[MERCHANT_PWD_LEN+1]={0};//7
	char tempPassword[MERCHANT_PWD_LEN+1]={0};//7
    char tempPassword1[MERCHANT_PWD_LEN+1]={0};//7
	short count = 0;
    char attamptLeft[13]={0};
    short iRetVal =0 ;
    char timeLimit[TIME_ARR_LEN]={0};
	short MerPwdLength=0;
    short time_limit=0;
	get_env("#PASSWORD",Password,sizeof(Password));
    get_env("#TIME_OUT_LIMIT_IN_SECOND",timeLimit,sizeof(timeLimit));
	MerPwdLength = strlen(Password);
    time_limit = atoi(timeLimit);
	time_limit = time_limit*100;
	do //Password validation
	{
		window(1,1,30,20);
		clrscr ();
        if(count !=0 && count != 3)
        {
			write_at(Dmsg[INVALID_PASSWORD].dispMsg, strlen(Dmsg[INVALID_PASSWORD].dispMsg),Dmsg[INVALID_PASSWORD].x_cor, Dmsg[INVALID_PASSWORD].y_cor);//invalid password
            sprintf(attamptLeft,"%d ATTEMPT LEFT",(3-count));
			write_at(attamptLeft,strlen(attamptLeft),(30-strlen(attamptLeft))/2,Dmsg[INVALID_PASSWORD].y_cor-2);
		}
        else if(count ==3)
        {
			window(1,1,30,20);
			clrscr ();
			write_at(Dmsg[INVALID_PASSWORD].dispMsg, strlen(Dmsg[INVALID_PASSWORD].dispMsg),Dmsg[INVALID_PASSWORD].x_cor, Dmsg[INVALID_PASSWORD].y_cor);
			SVC_WAIT(1000);
			if(LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("Invalid Password"));
			}
				return _FAIL;
		}
		write_at(Dmsg[ENTER_MERCHNAT_PASSWORD].dispMsg, strlen(Dmsg[ENTER_MERCHNAT_PASSWORD].dispMsg),Dmsg[ENTER_SUPERVISOR_PASSWORD].x_cor, Dmsg[ENTER_SUPERVISOR_PASSWORD].y_cor);
    	window(12,Dmsg[ENTER_MERCHNAT_PASSWORD].y_cor+2,22,Dmsg[ENTER_MERCHNAT_PASSWORD].y_cor+2);
		clrscr ();
		memset(tempPassword,0,sizeof(tempPassword));
		memset(tempPassword1,0,sizeof(tempPassword1));
		iRetVal = getkbd_entry (hClock, "",(signed char *)tempPassword, (unsigned int) time_limit,(unsigned int) PASSWORD, (char*) szKeyMap,sizeof(szKeyMap),MerPwdLength, MerPwdLength);
		if(iRetVal == BTN_CANCEL)
		{
			return KEY_CANCEL;
		}
		else if(iRetVal == 0)
		{
		    // Time Out
			window(1,1,30,20);
		    clrscr();
		    error_tone();
		    SVC_WAIT(190);
		    error_tone();
		    write_at(TIME_OUT, strlen(TIME_OUT),(30-strlen(TIME_OUT))/2, 10);
		    SVC_WAIT(4000);
		    ClearKbdBuff();
			KBD_FLUSH();
		    return _FAIL;
		}
		count++;
		strncpy(tempPassword1,tempPassword,MERCHANT_PWD_LEN);
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Entered Merchant Password1 = %s",tempPassword1));
			LOG_PRINTF(("   Password = %s",Password));
			LOG_PRINTF(("Entered Merchant Password = %s",tempPassword));
		}
	}while(strcmp(Password,tempPassword1));
	clrscr();
	return _SUCCESS;
}


/****************************************************************************************************************
*	Function Name : changePassword																																						*
*	Purpose		  : This feature allows the supervisor to change merchant password																	*
*	Input				: int variable to check transaction is debit or not																								*
*	Output		  : Return value for the success or failure																													*
*****************************************************************************************************************/
short changePassword(void)
{
	char tempPassword[MERCHANT_PWD_LEN+1]={0};//5
	char reEnterPassword[MERCHANT_PWD_LEN+1]={0};//5
    char tempPassword1[MERCHANT_PWD_LEN+1]={0};//5
	char reEnterPassword1[MERCHANT_PWD_LEN+1]={0};//5
	int size = 4;
	short iRetVal = 0;
	short count = 0;
	
	if(validateSupervisorPassword() != _SUCCESS)
		return _FAIL;
	do
	{
		if(count <=2)
		{
			window(1,1,30,20);
			clrscr ();
			display_at (1, 9, "ENTER MERCHANT", CLR_EOL);
			display_at (1, 10, "NEW PASSWORD: ", CLR_EOL);
			window(24,10,27,10);
			clrscr ();
			memset(tempPassword,0,sizeof(tempPassword));
            memset(tempPassword1,0,sizeof(tempPassword1));
			iRetVal = getkbd_entry (hClock, "",(signed char *)tempPassword, (unsigned int) 12000,(unsigned int) PASSWORD, (char*) szKeyMap,sizeof(szKeyMap),size, size);
			if(iRetVal == BTN_CANCEL)
			{
				return KEY_CANCEL;
			}
			else if(iRetVal == 0)
			{
			    // Time Out
				window(1,1,30,20);
				clrscr();
				error_tone();
				SVC_WAIT(190);
				error_tone();
				write_at(TIME_OUT,strlen(TIME_OUT),(30-strlen(TIME_OUT))/2,10);
				SVC_WAIT(4000);
				return _FAIL;
			}
            strncpy(tempPassword1,tempPassword,MERCHANT_PWD_LEN);
			window(1,12,26,12);
			clrscr ();
			display_at (1, 12, "RE-ENTER NEW PASSWORD: ", CLR_EOL);
			window(24,12,27,12);
			clrscr ();
			memset(reEnterPassword,0,sizeof(reEnterPassword));
            memset(reEnterPassword1,0,sizeof(reEnterPassword1));
			iRetVal = getkbd_entry (hClock, "",(signed char *)reEnterPassword, (unsigned int) 12000,(unsigned int) PASSWORD, (char*) szKeyMap,sizeof(szKeyMap),size, size);
			if(iRetVal == BTN_CANCEL)
			{
				return KEY_CANCEL;
			}
			else if(iRetVal == 0)
			{
				// Time Out
				window(1,1,30,20);
				clrscr();
				error_tone();
				SVC_WAIT(190);
				error_tone();
				write_at(TIME_OUT,strlen(TIME_OUT),(30-strlen(TIME_OUT))/2,10);
				SVC_WAIT(4000);
				return _FAIL;
			}
            strncpy(reEnterPassword1,reEnterPassword,MERCHANT_PWD_LEN);
            if(LOG_STATUS == LOG_ENABLE)
            {
				LOG_PRINTF(("New Entered Password1 = %s",tempPassword1));
				LOG_PRINTF(("Re Entered Password1 = %s",reEnterPassword1));

				LOG_PRINTF(("New Entered Password = %s",tempPassword));
				LOG_PRINTF(("Re Entered Password = %s",reEnterPassword));
            }
						
		}
		else
		{
			return _FAIL;
		}
		
		if(!strcmp(tempPassword1,reEnterPassword1))
			break;
		window(1,1,30,20);
		clrscr ();
		write_at("PASSWORD NOT MATCHED",20,5,10);
		SVC_WAIT(2000);
		count++;
	}while(1);
	put_env("#PASSWORD",reEnterPassword1,sizeof(reEnterPassword1));
	window(1,1,30,20);
	clrscr ();
	write_at("SUCCESS !!!",strlen("SUCCESS !!!"),(30-strlen("SUCCESS !!!"))/2,10);
	if(LOG_STATUS == LOG_ENABLE)
    {
		LOG_PRINTF(("Password Reset Successful"));
    }
	SVC_WAIT(1000);
	clrscr();
	return _SUCCESS;
}
/****************************************************************************************************************
*	Function Name : changeEnv																																				      *
*	Purpose				: it sets the environment variabler                                                     				*
*	Input					: input buffer and flag																																												*
*	Output				: Return value for the success or failure																												*
*****************************************************************************************************************/
short changeEnv(char *buffer,short flag,short size)
{
		short iRetVal = 0;
    if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF(("changeEnv %d " ,flag));
    }
		display_at (1, 2, "ENTER NEW ", CLR_EOL);
		memset(buffer,0,size);
		window(1,1,30,20);
		clrscr();
	
		window(5,5,25,5);
		clrscr();
		iRetVal = getkbd_entry (hClock, "",(signed char *)buffer, (unsigned int) 100000,(unsigned int) flag, (char*) szKeyMap,sizeof(szKeyMap),size, 1);
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("#buffer %s",buffer));
    }
    if(iRetVal == BTN_CANCEL)
	{
		return KEY_CANCEL;
	}
	else if(iRetVal == 0)
	{
	  // Time Out
		window(1,1,30,20);
		clrscr();
		error_tone();
		SVC_WAIT(190);
		error_tone();
		write_at(TIME_OUT,strlen(TIME_OUT),(30-strlen(TIME_OUT))/2,10);
		SVC_WAIT(4000);
		return _FAIL;
	}
	return _SUCCESS;
}
/****************************************************************************************************************
*	Function Name : dldConfiguration																																				      *
*	Purpose				: This feature allows the supervisor to display, edit and print the terminal parameters					*
*	Input					: void																																													*
*	Output				: Return value for the success or failure																												*
*****************************************************************************************************************/
short dldConfiguration(void)
{
		short Selectionstatus = SelectEvent;
		char compName[COMPANY_NAME_LEN]={0};//30
		char host_ip[HOST_IP_LEN]={0};//25
		char portNo[PORT_NO_LEN]={0};//6
		char Mid[CARD_ACCEPTOR_ID_LEN+1]={0};//15+1
		char MerchantName[MERCHANT_NAME_LEN]={0};//30
    char Tid[TID_LEN]={0};//11
		short iRetVal =0;
	  if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF(("in dldConfiguration fun"));
    }
		if(Selectionstatus != KEY_STR)
		{
				window(1,1,30,20);
				clrscr();
				SetImage(Config_BMP);//setting image for display or edit environment veriable
				EnblTouchScreen(ConfigMenu);
				if(Selectionstatus == SelectEvent)
						Selectionstatus =  get_char();
				if(Selectionstatus == KEY_STR)
						return KEY_STR;
				clrscr();
				
				switch(Selectionstatus)
				{
				case COMPNAME://
								get_env("#COMPANYNAME",compName,sizeof(compName));
                if(LOG_STATUS == LOG_ENABLE)
                {
								  LOG_PRINTF(("in %s","COMPNAME"));
                }
								display_at (1, 2, compName, CLR_EOL);
								write_at("PRESS ENTER TO CHANGE",strlen("PRESS ENTER TO CHANGE"),(30-strlen("PRESS ENTER TO CHANGE"))/2,10);
								if((get_char()) == KEY_CR)
								{
                    if(LOG_STATUS == LOG_ENABLE)
                    {
										  LOG_PRINTF(("key = %d",KEY_CR));
                    }
										iRetVal = changeEnv(compName,ALPHANUM,sizeof(compName));//change company name
										if((iRetVal == _FAIL ) || (iRetVal == KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL))
										{
												return econfiguration;
										}
										put_env("#COMPANYNAME",compName,sizeof(compName));
								}
								break;
				case HOST_IP:
								get_env("#HOSTIP",host_ip,sizeof(host_ip));
                if(LOG_STATUS == LOG_ENABLE)
                {
								  LOG_PRINTF(("in %s","HOST_IP"));
                }
								display_at (1, 2, host_ip, CLR_EOL);
								write_at("PRESS ENTER TO CHANGE",strlen("PRESS ENTER TO CHANGE"),(30-strlen("PRESS ENTER TO CHANGE"))/2,10);
								if(get_char() == KEY_CR)
								{
										iRetVal = changeEnv(host_ip,ALPHANUM,sizeof(host_ip));//change Host ip
										if((iRetVal == _FAIL ) || (iRetVal == KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL))
										{
												return econfiguration;
										}
										put_env("#HOSTIP",host_ip,sizeof(host_ip));
								}
								break;
				case HOST_PORT:
								get_env("#PORT",portNo,sizeof(portNo));
                if(LOG_STATUS == LOG_ENABLE)
                {
								  LOG_PRINTF(("in %s","HOST_PORT"));
                }
								display_at (1, 2, portNo, CLR_EOL);
								write_at("PRESS ENTER TO CHANGE",strlen("PRESS ENTER TO CHANGE"),(30-strlen("PRESS ENTER TO CHANGE"))/2,10);
								if(get_char() == KEY_CR)
								{
                    if(LOG_STATUS == LOG_ENABLE)
                    {
										  LOG_PRINTF(("key = %d",KEY_CR));
                    }
										iRetVal = changeEnv(portNo,NUMERIC,sizeof(portNo));//change Host Port
										if((iRetVal == _FAIL ) || (iRetVal == KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL))
										{
												return econfiguration;
										}
										put_env("#PORT",portNo,sizeof(portNo));
								}
								break;
				case MERCHANT_ID:
								get_env("#CARDACCEPTERID",Mid,sizeof(Mid));
                if(LOG_STATUS == LOG_ENABLE)
                {
								  LOG_PRINTF(("in %s","MERCHANT_ID"));
                }
								display_at (1, 2, Mid, CLR_EOL);
								write_at("PRESS ENTER TO CHANGE",strlen("PRESS ENTER TO CHANGE"),(30-strlen("PRESS ENTER TO CHANGE"))/2,10);
								if((get_char()) == KEY_CR)
								{
                    if(LOG_STATUS == LOG_ENABLE)
                    {
										  LOG_PRINTF(("key = %d",KEY_CR));
                    }
										iRetVal = changeEnv(Mid,NUMERIC,sizeof(Mid));//change Merchant Id
										if((iRetVal == _FAIL ) || (iRetVal == KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL))
										{
												return econfiguration;
										}
										put_env("#CARDACCEPTERID",Mid,sizeof(Mid));
								}
								break;
				case MERCHANT_NAME:
								get_env("#MERCHANT_NAME",MerchantName,sizeof(MerchantName));
                if(LOG_STATUS == LOG_ENABLE)
                {
								  LOG_PRINTF(("in %s","MERCHANT_NAME"));
                }
								display_at (1, 2, MerchantName, CLR_EOL);
								write_at("PRESS ENTER TO CHANGE",strlen("PRESS ENTER TO CHANGE"),(30-strlen("PRESS ENTER TO CHANGE"))/2,10);
								if((get_char()) == KEY_CR)
								{
                    if(LOG_STATUS == LOG_ENABLE)
                    {
										  LOG_PRINTF(("key = %d",KEY_CR));
                    }
										iRetVal = changeEnv(MerchantName,ALPHANUM,sizeof(MerchantName));//change Merchant Name
										if((iRetVal == _FAIL ) || (iRetVal == KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL))
										{
												return econfiguration;
										}
										put_env("#MERCHANT_NAME",MerchantName,sizeof(MerchantName));
								}
								break;
				case TERMINAL_ID:
								get_env("#TERMINAL_ID",Tid,sizeof(Tid));
								if(LOG_STATUS == LOG_ENABLE)
                {
                  LOG_PRINTF(("in %s","TERMINAL_ID"));
                }
								display_at (1, 2, Tid, CLR_EOL);
								write_at("PRESS ENTER TO CHANGE",strlen("PRESS ENTER TO CHANGE"),(30-strlen("PRESS ENTER TO CHANGE"))/2,10);
								if((get_char()) == KEY_CR)
								{
                    if(LOG_STATUS == LOG_ENABLE)
                    {
										  LOG_PRINTF(("key = %d",KEY_CR));
                    }
										iRetVal = changeEnv(Tid,NUMERIC,sizeof(Tid));
										if((iRetVal == _FAIL ) || (iRetVal == KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL))
										{
												return econfiguration;
										}
										put_env("#TERMINAL_ID",Tid,sizeof(Tid));
								}
								break;
				case CONGIF_BACK:
            if(LOG_STATUS == LOG_ENABLE)
						LOG_PRINTF(("in %s","CONGIF_BACK"));
								return eSuperviser;//
								
				default:
          break;
				}
				Selectionstatus = econfiguration;
				clrscr();
		}
		return Selectionstatus;
}

