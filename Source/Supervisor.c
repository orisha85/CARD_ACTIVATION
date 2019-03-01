/***********************************************************************************************
  Filename		     : Supervisor.c
  Project		       : Bevertec 
  Developer Neame  : Vikas
  Module           : Change merchant password ,validate 
  Description	     : This is the implementation of validation of merchant ,supervisor password
 ***********************************************************************************************/
#include "..\include\Supervisor.h"
#include "..\Include\TouchScreen.h"
#include "..\Include\IPP_Key_injection.h"
#include "..\Include\ReciptPrint.h"
#include <svc.h>
#include <svctxo.h>
#include <applidl.h>
#include "..\include\Common.h"
 
extern short hClock;
extern unsigned char EncSessionKey[UNPACK_ENC_PR_KEY_LEN + 1];//33 
static const unsigned char hexdigits[] = "0123456789ABCDEF";
//extern pin_change;
//int pinmismatch;
extern unsigned char EncPinBlock[BITMAP_ARRAY_SIZE];//8
extern unsigned char PinBlockHolderOperator[PIN_BLOCK_LENGTH + 1];
extern char loggedinOper[9];
char new_pin[13];
char old_pin[13];
char confirm_pin[13];
char enter_pin[13];

/*char pwd_map[50][17] = { "CBDF17E435A64823","743B70C22B97E189","6849BDC44518522A"
,"3B45D1AD68FA225C"
,"8B8F93C49BD627BA"
,"620CD47AA890EC4F"
,"AFF1F5823C836CEF"
,"0A40CB81F94E9718"
,"C33A9819EB223734"
,"D020A3E2C50B2F83"
,"7296944BEDA86B41"
,"70B45C12E5E705FE"
,"61B72F7DAFE7A51F"
,"80E7D9DCE1C6407D"
,"25896F69325D2844"
,"A466FCF67D1B179C"
,"FE3B17145A169D57"
,"307B2B138F594CBB"
,"BEAA44C326502A3E"
,"9AFDBA1C3DBDE533"
,"1FA10CB24A4C0234"
,"5EFF52932B082DE3"
,"EBECB2D58020498B"
,"1962EBF48521377B"
,"52DCFB370790454B"
,"D302061361781AAC"
,"A0CDBC00F01025A4"
,"6C4C560DF112545D"
,"387325915E98A632"
,"CF28262485C87358"
,"05721C11E6400EFC"
,"69589DE5FEF2DF72"
,"29D5BEC11836B1FE"
,"9D3F30A553A34EF3"
,"6913C58286E07E36"
,"4DC87D660FDB98EC"
,"32C578CFD45E0E46"
,"3A3161DFD93C4C7E"
,"B0FB9DAB28F83A4D"
,"7B6AB4F00B96A126"
,"9E44CD9060F1EF54"
,"9C08DFB60F21236F"
,"B2FA7992BA323321"
,"42524EB37F82FA0C"
,"7566C36B92462D10"
,"0BFE886F15BEBEA1"
,"9A782C91FD0DAB5E"
};*/

char pwd_map[60][17] = { "00000000",
"000000000",
"0000000000",
"00000000000",
"000000000000",
"11111111",
"111111111",
"1111111111",
"11111111111",
"111111111111",
"22222222",
"222222222",
"2222222222",
"22222222222",
"222222222222",
"33333333",
"333333333",
"3333333333",
"33333333333",
"333333333333",
"44444444",
"444444444",
"4444444444",
"44444444444",
"444444444444",
"55555555",
"555555555",
"5555555555",
"55555555555",
"555555555555",
"66666666",
"666666666",
"6666666666",
"66666666666",
"666666666666",
"77777777",
"777777777",
"7777777777",
"77777777777",
"777777777777",
"88888888",
"888888888",
"8888888888",
"88888888888",
"888888888888",
"99999999",
"999999999",
"9999999999",
"99999999999",
"999999999999",
"23456789",
"0123456789",
"012345678",
"01234567",
"12345678",
"123456789"
};

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
	short superPwdLength = 0;
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

short getCustodianKey(char *key,short flag, int cust)
{
	short iRetVal = 0;
	short size = 32;
	if (LOG_STATUS == LOG_ENABLE)
	{
		LOG_PRINTF(("Cust Key"));
	}
	display_at(1, 2, "Custodian Key ", CLR_EOL);
	memset(key, 0, size);

	window(8, 9, 16, 8);
	clrscr();

	if (cust == 1)
		iRetVal = getkbd_entry(hClock, "Enter Custodian 1 Key", (signed char *)key, (unsigned int)100000, (unsigned int)flag, (char*)szKeyMap, sizeof(szKeyMap), size, 1);
	if (cust == 2)
		iRetVal = getkbd_entry(hClock, "Enter Custodian 2 Key", (signed char *)key, (unsigned int)100000, (unsigned int)flag, (char*)szKeyMap, sizeof(szKeyMap), size, 1);

	if (LOG_STATUS == LOG_ENABLE)
	{
		LOG_PRINTF(("#key %s", key));
	}
	if (iRetVal == BTN_CANCEL)
	{
		return KEY_CANCEL;
	}
	return _SUCCESS;
}

static int hexval(const unsigned char c)
{
	//assert(isxdigit(c));
	if (isdigit(c))
		return c - '0';
	else if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	else if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	else    // Only used if assert is disabled with -DNDEBUG
		return -1;
}


static const unsigned char hexdigit(int hexval)
{
	//assert(hexval >= 0 && hexval < 16);
	return hexdigits[hexval];
}

short Key_XOR(char* key1, char* key2, char* XOR_key)
{
	int i = 0;
	int length;
	char temp;
	const unsigned char *s1 = (const unsigned char *)key1;
	const unsigned char *s2 = (const unsigned char *)key2;
	LOG_PRINTF(("#key1: %s #key2 %s, key length: %d, %d", key1, key2, strlen(key1), strlen(key2)));
	if (strlen(key1) == strlen(key2))
	{
		LOG_PRINTF(("Equal Length keys"));
		length = strlen(key1);

		while (*s1 != '\0')
		{
			const unsigned char u1 = *s1++;
			const unsigned char u2 = *s2++;
			//assert(isxdigit(u1));
			//assert(isxdigit(u2));
			*XOR_key++ = hexdigit(hexval(u1) ^ hexval(u2));
		}

		*XOR_key = '\0';

		/*for (i = 0; i < length; i++)
		{
			XOR_key[i] = (char)(key1[i] ^ key2[i]);
			LOG_PRINTF(("XORKey: %s", XOR_key));
		}*/
	}
	return 0;
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
		//unsigned char key1[] = {"67B526322F169D26EF3D762FE361A731"};
		//unsigned char key2[] = {"D026B97A37BCD6CBF1D6AE2923ABA2D5"};
		unsigned char key1[33];
		unsigned char key2[33];
		//unsigned char testTMK[] = { "A1D0D31016A40BD649E52AD09EC716AB" };
		unsigned char XORKey[33];
		static const unsigned char hexdigits[] = "0123456789ABCDEF";
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
				case KEY_INJECTION://
					if (LOG_STATUS == LOG_ENABLE)
					{
						LOG_PRINTF(("in %s", "Key Injection"));
						write_at("PRESS ENTER TO INJECT KEYS", strlen("PRESS ENTER TO INJECT KEYS"), (30 - strlen("PRESS ENTER TO INJECT KEYS")) / 2, 10);
						if (get_char() == KEY_CR)
						{
							//LOG_PRINTF(("#key1: %s #key2 %s, key length: %d, %d", key1, key2, strlen(key1), strlen(key2)));
							//LOG_PRINTF(("XORKey %s", XORKey));

						
							getCustodianKey(key1, ALPHANUM, 1);
							getCustodianKey(key2, ALPHANUM, 2);
							Key_XOR(key1, key2, XORKey);
							//LOG_PRINTF(("#key1: %s #key2 %s, key length: %d, %d", key1, key2, strlen(key1), strlen(key2)));
							if (strlen(XORKey) == 32)
							{
								KeyInjection(XORKey);
								put_env("#XOR", XORKey, sizeof(XORKey));
								strcpy(EncSessionKey, "");
								window(1, 1, 30, 20);
								clrscr();
								write_at(Dmsg[KEYS_INJECTED].dispMsg, strlen(Dmsg[KEYS_INJECTED].dispMsg), Dmsg[KEYS_INJECTED].x_cor, Dmsg[PIN_NO_MATCH].y_cor);
								window(Dmsg[KEYS_INJECTED].x_cor, Dmsg[KEYS_INJECTED].y_cor + 2, 18, Dmsg[KEYS_INJECTED].y_cor + 2);
								SVC_WAIT(3000);
							}
							else
							{
								window(1, 1, 30, 20);
								clrscr();
								display_at(1, 9, "Key length different than 32", CLR_EOL);
								if (get_char() == KEY_CR)
									return econfiguration;
							}


							if ((iRetVal == _FAIL) || (iRetVal == KEY_STR) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL))
							{
								return econfiguration;
							}
						}						
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

short AddOperator()
{
	//Operator addOper;
	int i = 1;
	int x = 1;
	char oper_num[11] = {0};
	char oper_PWD[13] = {0};
	char oper_search[11] = { 0 };
	short iRetVal = 0;
	char operID[9] = { 0 };
	char searchoperID[9] = { 0 };
	char operPWD[17] = { 0 };
	int iret = 0;
	int pinreturn = 0;
	int checkpwd = 0;

	//strcpy(addOper.operID, "\0");
	//strcpy(addOper.operPWD, "\0");

	memset(new_pin, 0, sizeof(new_pin));
	memset(old_pin, 0, sizeof(old_pin));
	memset(confirm_pin, 0, sizeof(new_pin));
	memset(enter_pin, 0, sizeof(new_pin));
	

	for (i; i <= 10; i++)
	{
		sprintf(oper_num, "#OPER_%d_ID", i);
		LOG_PRINTF(("%s", oper_num));
		iret = get_env(oper_num, operID, sizeof(operID));
		LOG_PRINTF(("%s", operID));
		if (iret == 0)
		{
			LOG_PRINTF(("OPER_%d_ID is empty ", i));
			break;
		}
	}

	display_at(1, 2, "ENTER OPERATOR ID:", CLR_EOL);
	//memset(buffer, 0, size);
	window(1, 1, 30, 20);
	clrscr();

	window(5, 5, 25, 5);
	clrscr();
	iRetVal = getkbd_entry(hClock, "ENTER OPERATOR ID", (signed char *)operID, (unsigned int)100000, (unsigned int)NUMERIC, (char*)szKeyMap, sizeof(szKeyMap), 8, 8);
	if (LOG_STATUS == LOG_ENABLE)
	{
		LOG_PRINTF(("#OperID is %s", operID));
	}
	if (iRetVal == BTN_CANCEL)
	{
		return KEY_CANCEL;
	}
	else if (iRetVal == 0)
	{
		// Time Out
		window(1, 1, 30, 20);
		clrscr();
		error_tone();
		SVC_WAIT(190);
		error_tone();
		write_at(TIME_OUT, strlen(TIME_OUT), (30 - strlen(TIME_OUT)) / 2, 10);
		SVC_WAIT(4000);
		return _FAIL;
	}
	if (strlen(operID) != 8)
	{
		clrscr();
		write_at("OPERATOR ID", 11, 6, 9);
		write_at("SHOULD 8 DIGITS", 15, 6, 10);
		SVC_WAIT(2000);
		return _FAIL;
	}

	for (x; x <= 10; x++)
	{
		sprintf(oper_search, "#OPER_%d_ID", x);
		LOG_PRINTF(("%s", oper_search));
		iret = get_env(oper_search, searchoperID, sizeof(searchoperID));
		LOG_PRINTF(("%s", searchoperID));
		if (iret > 0)
		{
			LOG_PRINTF(("OPER_%d_ID ", x));
			if (!strcmp(searchoperID, operID))
			{
				clrscr();
				write_at("OPERATOR ID", 11, 6, 9);
				write_at("ALREADY EXISTS", 14, 6, 10);
				SVC_WAIT(2000);
				return _FAIL;
			}
		}
	}
	//else
	//	put_env(oper_num, operID, sizeof(operID));

	//pin_change = 2;
	//pinreturn = PinPrompt("6220151105029518");//pin
	pinreturn = promptpin(2);
	
	if (pinreturn != _SUCCESS)
		return _FAIL;
	checkpwd = checkpassword(new_pin);
	if (checkpwd == _FAIL)
	{
		window(1, 1, 30, 20);
		clrscr();
		write_at(Dmsg[PIN_INCORRECT_FORMAT].dispMsg, strlen(Dmsg[PIN_INCORRECT_FORMAT].dispMsg), Dmsg[PIN_INCORRECT_FORMAT].x_cor, Dmsg[PIN_INCORRECT_FORMAT].y_cor);
		window(Dmsg[PIN_INCORRECT_FORMAT].x_cor, Dmsg[PIN_INCORRECT_FORMAT].y_cor + 2, 18, Dmsg[PIN_INCORRECT_FORMAT].y_cor + 2);
		SVC_WAIT(3000);
		//pinmismatch = 0;   //reset pinmismatch flag
		return UserMgnt;
	}

	//pinreturn = PinPrompt("6220151105029518");//confirm pin
	//if (pinreturn == -3)
	//	return _FAIL;
	pinreturn = promptpin(3);

	if (pinreturn != _SUCCESS)
		return _FAIL;

	if (strcmp(new_pin, confirm_pin))
	{
		window(1, 1, 30, 20);
		clrscr();
		write_at(Dmsg[PIN_NO_MATCH].dispMsg, strlen(Dmsg[PIN_NO_MATCH].dispMsg), Dmsg[PIN_NO_MATCH].x_cor, Dmsg[PIN_NO_MATCH].y_cor);
		window(Dmsg[PIN_NO_MATCH].x_cor, Dmsg[PIN_NO_MATCH].y_cor + 2, 18, Dmsg[PIN_NO_MATCH].y_cor + 2);
		SVC_WAIT(3000);
		//pinmismatch = 0;   //reset pinmismatch flag
		return UserMgnt;
	}
	else
	{
		//strcpy(operPWD, PinBlockHolderOperator);
		//LOG_PRINTF(("Pinblock is %s", operPWD));
		sprintf(oper_PWD, "#OPER_%d_PWD", i);
		put_env(oper_PWD, new_pin, sizeof(new_pin));
		put_env(oper_num, operID, sizeof(operID));
	}

	pin_change = 0;

	clrscr();
	write_at("OPERATOR ADDED", 14, 5, 10);
	SVC_WAIT(2000);
	saveUserdtls('N', operID);
	return _SUCCESS;
}
short DeleteOperator()
{
	//Operator addOper;
	int i = 1;
	char oper_num[11] = { 0 };
	char oper_PWD[13] = { 0 };
	short iRetVal = 0;
	char operID[9] = { 0 };
	char operPWD[17] = { 0 };
	char tempoperID[9] = { 0 };
	int iret = 0;
	char ch = 0;

	//strcpy(addOper.operID, "\0");
	//strcpy(addOper.operPWD, "\0");

	display_at(1, 2, "ENTER OPERATOR ID:", CLR_EOL);
	//memset(buffer, 0, size);
	window(1, 1, 30, 20);
	clrscr();

	window(5, 5, 25, 5);
	clrscr();
	iRetVal = getkbd_entry(hClock, "ID TO DELETE", (signed char *)operID, (unsigned int)100000, (unsigned int)NUMERIC, (char*)szKeyMap, sizeof(szKeyMap), 8, 8);
	if (LOG_STATUS == LOG_ENABLE)
	{
		LOG_PRINTF(("#OperID is %s", operID));
	}
	if (iRetVal == BTN_CANCEL)
	{
		return KEY_CANCEL;
	}
	else if (iRetVal == 0)
	{
		// Time Out
		window(1, 1, 30, 20);
		clrscr();
		error_tone();
		SVC_WAIT(190);
		error_tone();
		write_at(TIME_OUT, strlen(TIME_OUT), (30 - strlen(TIME_OUT)) / 2, 10);
		SVC_WAIT(4000);
		return _FAIL;
	}

	for (i; i <= 10; i++)
	{
		sprintf(oper_num, "#OPER_%d_ID", i);
		LOG_PRINTF(("%s", oper_num));
		LOG_PRINTF(("OPER_%d_ID to delete ", i));
		iret = get_env(oper_num, tempoperID, sizeof(oper_num));
		if (iret > 0)
		{
			if (!strcmp(operID, tempoperID))
			{	
				window(1, 1, 30, 20);
				clrscr();
				error_tone();
				write_at("CONFIRM", strlen("CONFIRM"), (30 - strlen("CONFIRM")) / 2, 10);
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
					put_env(oper_num, "", 0);
					LOG_PRINTF(("OPER_%d_ID to delete ", i));
				}
				else
					return;
				break;
			}
		}
	}

	write_at("OPERATOR DELETED", 16, 5, 10);
	SVC_WAIT(2000);
	saveUserdtls('D', operID);
	return _SUCCESS;
}
short ChangeOperatorPassword()
{
	char Password[16 + 1] = { 0 };//7
	char tempPassword[16 + 1] = { 0 };//7
	char tempPassword1[16 + 1] = { 0 };//7
	short count = 0;
	char attamptLeft[13] = { 0 };
	short iRetVal = 0;
	char timeLimit[TIME_ARR_LEN] = { 0 };
	short MerPwdLength = 0;
	short time_limit = 0;
	char operID[9] = { 0 };
	char tempoperID[9] = { 0 };
	char operPWD[13] = { 0 };
	int i = 1;
	int iret = 0;
	char oper_num[11] = { 0 };
	char oper_PWD[13] = { 0 };
	int pinreturn = 0;
	int checkpwd = 0;

	memset(new_pin, 0, sizeof(new_pin));
	memset(old_pin, 0, sizeof(old_pin));
	memset(confirm_pin, 0, sizeof(new_pin));
	memset(enter_pin, 0, sizeof(new_pin));

	//get_env("#PASSWORD", Password, sizeof(Password));
	get_env("#TIME_OUT_LIMIT_IN_SECOND", timeLimit, sizeof(timeLimit));
	MerPwdLength = 8;
	time_limit = atoi(timeLimit);
	time_limit = time_limit * 100;
	//do //Password validation
	//{
		window(1, 1, 30, 20);
		clrscr();
		if (count != 0 && count != 3)
		{
			write_at(Dmsg[INVALID_PASSWORD].dispMsg, strlen(Dmsg[INVALID_PASSWORD].dispMsg), Dmsg[INVALID_PASSWORD].x_cor, Dmsg[INVALID_PASSWORD].y_cor);//invalid password
			sprintf(attamptLeft, "%d ATTEMPT LEFT", (3 - count));
			write_at(attamptLeft, strlen(attamptLeft), (30 - strlen(attamptLeft)) / 2, Dmsg[INVALID_PASSWORD].y_cor - 2);
		}
		else if (count == 3)
		{
			window(1, 1, 30, 20);
			clrscr();
			write_at(Dmsg[INVALID_PASSWORD].dispMsg, strlen(Dmsg[INVALID_PASSWORD].dispMsg), Dmsg[INVALID_PASSWORD].x_cor, Dmsg[INVALID_PASSWORD].y_cor);
			SVC_WAIT(1000);
			if (LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("Invalid Password"));
			}
			return _FAIL;
		}
		write_at(Dmsg[ENTER_OPERATOR_ID].dispMsg, strlen(Dmsg[ENTER_OPERATOR_ID].dispMsg), Dmsg[ENTER_OPERATOR_ID].x_cor, Dmsg[ENTER_OPERATOR_ID].y_cor);
		window(12, Dmsg[ENTER_OPERATOR_ID].y_cor + 2, 22, Dmsg[ENTER_OPERATOR_ID].y_cor + 2);
		clrscr();

		iRetVal = getkbd_entry(hClock, "", (signed char *)operID, (unsigned int)time_limit, (unsigned int)NUMERIC, (char*)szKeyMap, sizeof(szKeyMap), 8, 8);
		if (iRetVal == BTN_CANCEL)
		{
			return KEY_CANCEL;
		}
		else if (iRetVal == 0)
		{
			// Time Out
			window(1, 1, 30, 20);
			clrscr();
			error_tone();
			SVC_WAIT(190);
			error_tone();
			write_at(TIME_OUT, strlen(TIME_OUT), (30 - strlen(TIME_OUT)) / 2, 10);
			SVC_WAIT(4000);
			ClearKbdBuff();
			KBD_FLUSH();
			return _FAIL;
		}
		count++;
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("OperID:%s", operID));
		}
		//find operId in enviroment variables
		for (i; i <= 10; i++)
		{
			sprintf(oper_num, "#OPER_%d_ID", i);
			LOG_PRINTF(("%s", oper_num));
			iret = get_env(oper_num, tempoperID, sizeof(oper_num));
			if (iret > 0)
			{
				if (!strcmp(tempoperID, operID))
				{
					LOG_PRINTF(("OperID found"));
					break;
				}
			}
		}
		if (iret <= 0)
		{
			LOG_PRINTF(("OperID not found"));
			write_at("OPERATOR NOT FOUND", 18, 5, 10);
			SVC_WAIT(2000);
			return _FAIL;
		}

		sprintf(oper_PWD, "#OPER_%d_PWD", i);
		LOG_PRINTF(("%s", oper_PWD));
		iret = get_env(oper_PWD, tempPassword, sizeof(oper_PWD));
		if (iret <= 0)
		{
			LOG_PRINTF(("Password not found"));
			return _FAIL;
		}
		else
			LOG_PRINTF(("Password: %s", tempPassword));

		//pin_change = 1;
		//pinreturn = PinPrompt("6220151105029518");//pin
		//if (pinreturn == -3)

		pinreturn = promptpin(1);
		if(pinreturn != _SUCCESS)
			return _FAIL;

		if (strcmp(tempPassword, old_pin))
		{
			window(1, 1, 30, 20);
			clrscr();
			write_at(Dmsg[PIN_NO_MATCH].dispMsg, strlen(Dmsg[PIN_NO_MATCH].dispMsg), Dmsg[PIN_NO_MATCH].x_cor, Dmsg[PIN_NO_MATCH].y_cor);
			window(Dmsg[PIN_NO_MATCH].x_cor, Dmsg[PIN_NO_MATCH].y_cor + 2, 18, Dmsg[PIN_NO_MATCH].y_cor + 2);
			SVC_WAIT(3000);
			return UserMgnt;
		}
		//pin_change = 2;
		//pinreturn = PinPrompt("6220151105029518");//pin
		//if (pinreturn == -3)
		//	return _FAIL;
		pinreturn = promptpin(2);

		if (pinreturn != _SUCCESS)
			return _FAIL;

		checkpwd = checkpassword(new_pin);
		if (checkpwd == _FAIL)
		{
			window(1, 1, 30, 20);
			clrscr();
			write_at(Dmsg[PIN_INCORRECT_FORMAT].dispMsg, strlen(Dmsg[PIN_INCORRECT_FORMAT].dispMsg), Dmsg[PIN_INCORRECT_FORMAT].x_cor, Dmsg[PIN_INCORRECT_FORMAT].y_cor);
			window(Dmsg[PIN_INCORRECT_FORMAT].x_cor, Dmsg[PIN_INCORRECT_FORMAT].y_cor + 2, 18, Dmsg[PIN_INCORRECT_FORMAT].y_cor + 2);
			SVC_WAIT(3000);
			//pinmismatch = 0;   //reset pinmismatch flag
			return UserMgnt;
		}
		//pin_change = 3;
		//pinreturn = PinPrompt("6220151105029518");//pin
		//if (pinreturn == -3)
		//	return _FAIL;
		pinreturn = promptpin(3);
		if (pinreturn != _SUCCESS)
			return _FAIL;

		if (strcmp(new_pin, confirm_pin))
		{
			window(1, 1, 30, 20);
			clrscr();
			write_at(Dmsg[PIN_NO_MATCH].dispMsg, strlen(Dmsg[PIN_NO_MATCH].dispMsg), Dmsg[PIN_NO_MATCH].x_cor, Dmsg[PIN_NO_MATCH].y_cor);
			window(Dmsg[PIN_NO_MATCH].x_cor, Dmsg[PIN_NO_MATCH].y_cor + 2, 18, Dmsg[PIN_NO_MATCH].y_cor + 2);
			SVC_WAIT(3000);
			//pinmismatch = 0;   //reset pinmismatch flag
			return UserMgnt;
		}
		else
		{
			//strcpy(Password, PinBlockHolderOperator);
			//LOG_PRINTF(("Pinblock is %s", Password));
			sprintf(oper_PWD, "#OPER_%d_PWD", i);
			put_env(oper_PWD, new_pin, sizeof(Password));
			strcpy(tempPassword, Password);
		}

	//} while (strcmp(Password, tempPassword));
	clrscr();
	strcpy(loggedinOper, operID);
	pin_change = 0;
	write_at("PASSWORD CHANGED", 16, 3, 10);
	SVC_WAIT(2000);
	saveUserdtls('C', operID);
	return _SUCCESS;
}
short ChangeSupervisorPassword()
{
}
short ResetOperatorPassword()
{
	char Password[16 + 1] = { 0 };//7
	char tempPassword[16 + 1] = { 0 };//7
	char tempPassword1[16 + 1] = { 0 };//7
	short count = 0;
	char attamptLeft[13] = { 0 };
	short iRetVal = 0;
	char timeLimit[TIME_ARR_LEN] = { 0 };
	short MerPwdLength = 0;
	short time_limit = 0;
	char operID[9] = { 0 };
	char tempoperID[9] = { 0 };
	char operPWD[13] = { 0 };
	int i = 1;
	int iret = 0;
	char oper_num[11] = { 0 };
	char oper_PWD[13] = { 0 };
	int pinreturn = 0;
	int checkpwd = 0;

	memset(new_pin, 0, sizeof(new_pin));
	memset(old_pin, 0, sizeof(old_pin));
	memset(confirm_pin, 0, sizeof(new_pin));
	memset(enter_pin, 0, sizeof(new_pin));

	//get_env("#PASSWORD", Password, sizeof(Password));
	get_env("#TIME_OUT_LIMIT_IN_SECOND", timeLimit, sizeof(timeLimit));
	MerPwdLength = 8;
	time_limit = atoi(timeLimit);
	time_limit = time_limit * 100;

	//do //Password validation
	//{
		window(1, 1, 30, 20);
		clrscr();
		if (count != 0 && count != 3)
		{
			write_at(Dmsg[INVALID_PASSWORD].dispMsg, strlen(Dmsg[INVALID_PASSWORD].dispMsg), Dmsg[INVALID_PASSWORD].x_cor, Dmsg[INVALID_PASSWORD].y_cor);//invalid password
			sprintf(attamptLeft, "%d ATTEMPT LEFT", (3 - count));
			write_at(attamptLeft, strlen(attamptLeft), (30 - strlen(attamptLeft)) / 2, Dmsg[INVALID_PASSWORD].y_cor - 2);
		}
		else if (count == 3)
		{
			window(1, 1, 30, 20);
			clrscr();
			write_at(Dmsg[INVALID_PASSWORD].dispMsg, strlen(Dmsg[INVALID_PASSWORD].dispMsg), Dmsg[INVALID_PASSWORD].x_cor, Dmsg[INVALID_PASSWORD].y_cor);
			SVC_WAIT(1000);
			if (LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("Invalid Password"));
			}
			return _FAIL;
		}
		write_at(Dmsg[ENTER_OPERATOR_ID].dispMsg, strlen(Dmsg[ENTER_OPERATOR_ID].dispMsg), Dmsg[ENTER_OPERATOR_ID].x_cor, Dmsg[ENTER_OPERATOR_ID].y_cor);
		window(12, Dmsg[ENTER_OPERATOR_ID].y_cor + 2, 22, Dmsg[ENTER_OPERATOR_ID].y_cor + 2);
		clrscr();

		iRetVal = getkbd_entry(hClock, "", (signed char *)operID, (unsigned int)time_limit, (unsigned int)NUMERIC, (char*)szKeyMap, sizeof(szKeyMap), 8, 8);
		if (iRetVal == BTN_CANCEL)
		{
			return KEY_CANCEL;
		}
		else if (iRetVal == 0)
		{
			// Time Out
			window(1, 1, 30, 20);
			clrscr();
			error_tone();
			SVC_WAIT(190);
			error_tone();
			write_at(TIME_OUT, strlen(TIME_OUT), (30 - strlen(TIME_OUT)) / 2, 10);
			SVC_WAIT(4000);
			ClearKbdBuff();
			KBD_FLUSH();
			return _FAIL;
		}
		count++;
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("OperID:%s", operID));
		}
		//find operId in enviroment variables
		for (i; i <= 10; i++)
		{
			sprintf(oper_num, "#OPER_%d_ID", i);
			LOG_PRINTF(("%s", oper_num));
			iret = get_env(oper_num, tempoperID, sizeof(oper_num));
			if (iret > 0)
			{
				if (!strcmp(tempoperID, operID))
				{
					LOG_PRINTF(("OperID found"));
					break;
				}
			}
		}
		if (iret <= 0)
		{
			LOG_PRINTF(("OperID not found"));
			write_at("OPERATOR NOT FOUND", 18, 5, 10);
			SVC_WAIT(2000);
			return _FAIL;
		}

		sprintf(oper_PWD, "#OPER_%d_PWD", i);
		LOG_PRINTF(("%s", oper_PWD));
		iret = get_env(oper_PWD, tempPassword, sizeof(oper_PWD));
		if (iret <= 0)
		{
			LOG_PRINTF(("Password not found"));
			return _FAIL;
		}
		else
			LOG_PRINTF(("Password: %s", tempPassword));

		//pin_change = 2;
		//pinreturn = PinPrompt("6220151105029518");//pin
		//if (pinreturn == -3)
		//	return _FAIL;

		pinreturn = promptpin(2);
		if (pinreturn != _SUCCESS)
			return _FAIL;

		checkpwd = checkpassword(new_pin);
		if (checkpwd == _FAIL)
		{
			window(1, 1, 30, 20);
			clrscr();
			write_at(Dmsg[PIN_INCORRECT_FORMAT].dispMsg, strlen(Dmsg[PIN_INCORRECT_FORMAT].dispMsg), Dmsg[PIN_INCORRECT_FORMAT].x_cor, Dmsg[PIN_INCORRECT_FORMAT].y_cor);
			window(Dmsg[PIN_INCORRECT_FORMAT].x_cor, Dmsg[PIN_INCORRECT_FORMAT].y_cor + 2, 18, Dmsg[PIN_INCORRECT_FORMAT].y_cor + 2);
			SVC_WAIT(3000);
			//pinmismatch = 0;   //reset pinmismatch flag
			return UserMgnt;
		}
		//pin_change = 2;
		//pinreturn = PinPrompt("6220151105029518");//pin
		//if (pinreturn == -3)
		//	return _FAIL;
		//pin_change = 3;
		//PinPrompt("6220151105029518");//pin

		pinreturn = promptpin(3);
		if (pinreturn != _SUCCESS)
			return _FAIL;

		if (strcmp(new_pin, confirm_pin))
		{
			window(1, 1, 30, 20);
			clrscr();
			write_at(Dmsg[PIN_NO_MATCH].dispMsg, strlen(Dmsg[PIN_NO_MATCH].dispMsg), Dmsg[PIN_NO_MATCH].x_cor, Dmsg[PIN_NO_MATCH].y_cor);
			window(Dmsg[PIN_NO_MATCH].x_cor, Dmsg[PIN_NO_MATCH].y_cor + 2, 18, Dmsg[PIN_NO_MATCH].y_cor + 2);
			SVC_WAIT(3000);
			//pinmismatch = 0;   //reset pinmismatch flag
			return UserMgnt;
		}
		else
		{
			//strcpy(Password, PinBlockHolderOperator);
			//LOG_PRINTF(("Pinblock is %s", Password));
			sprintf(oper_PWD, "#OPER_%d_PWD", i);
			put_env(oper_PWD, new_pin, sizeof(operPWD));
			//strcpy(Password, tempPassword);
		}

	//} while (strcmp(Password, tempPassword));
	clrscr();
	strcpy(loggedinOper, operID);
	pin_change = 0;
	write_at("PASSWORD CHANGED", 16, 3, 10);
	SVC_WAIT(2000);
	saveUserdtls('R', operID);
	return _SUCCESS;
}
short ResetSupervisorPassword()
{
	char Password[6 + 1] = { 0 };//7
	char tempPassword[6 + 1] = { 0 };//7
	char tempPassword1[MERCHANT_PWD_LEN + 1] = { 0 };//7
	short count = 0;
	char attamptLeft[13] = { 0 };
	short iRetVal = 0;
	char timeLimit[TIME_ARR_LEN] = { 0 };
	short MerPwdLength = 0;
	short time_limit = 0;
	short superPwdLength = strlen(Password);
	get_env("#PASSWORD", Password, sizeof(Password));
	get_env("#TIME_OUT_LIMIT_IN_SECOND", timeLimit, sizeof(timeLimit));
	MerPwdLength = strlen(Password);
	time_limit = atoi(timeLimit);
	time_limit = time_limit * 100;

	clrscr();

	write_at(Dmsg[NEW_SUPERVISOR_PASSWORD].dispMsg, strlen(Dmsg[NEW_SUPERVISOR_PASSWORD].dispMsg), Dmsg[NEW_SUPERVISOR_PASSWORD].x_cor, Dmsg[NEW_SUPERVISOR_PASSWORD].y_cor);
	window(12, Dmsg[NEW_SUPERVISOR_PASSWORD].y_cor + 2, 22, Dmsg[NEW_SUPERVISOR_PASSWORD].y_cor + 2);
	clrscr();
	memset(tempPassword, 0, sizeof(tempPassword));

	iRetVal = getkbd_entry(hClock, "", (signed char *)tempPassword, (unsigned int)time_limit, (unsigned int)PASSWORD, (char*)szKeyMap, sizeof(szKeyMap), superPwdLength, superPwdLength);

	write_at(Dmsg[CONFIRM_SUPERVISOR_PASSWORD].dispMsg, strlen(Dmsg[CONFIRM_SUPERVISOR_PASSWORD].dispMsg), Dmsg[CONFIRM_SUPERVISOR_PASSWORD].x_cor, Dmsg[CONFIRM_SUPERVISOR_PASSWORD].y_cor);
	window(12, Dmsg[CONFIRM_SUPERVISOR_PASSWORD].y_cor + 2, 22, Dmsg[CONFIRM_SUPERVISOR_PASSWORD].y_cor + 2);
	clrscr();
	memset(tempPassword, 0, sizeof(tempPassword));

	iRetVal = getkbd_entry(hClock, "", (signed char *)tempPassword1, (unsigned int)time_limit, (unsigned int)PASSWORD, (char*)szKeyMap, sizeof(szKeyMap), superPwdLength, superPwdLength);

	if (!strcmp(tempPassword, tempPassword1))
	{
		put_env("#SUPER_PASSWORD", tempPassword, sizeof(tempPassword));
		LOG_PRINTF(("Password: %s", tempPassword));
		write_at("PASSWORD CHANGED", 16, 3, 10);
		SVC_WAIT(2000);
		return _SUCCESS;
	}
	else
	{
		window(1, 1, 30, 20);
		clrscr();
		write_at("PASSWORD NOT MATCHED", 20, 1, 10);
		SVC_WAIT(2000);
		return _FAIL;
	}
}
short ListOperators()
{
	int i = 1;
	char oper_num[11] = { 0 };
	char oper_PWD[13] = { 0 };
	short iRetVal = 0;
	char operID[9] = { 0 };
	char operPWD[13] = { 0 };
	char tempoperID[9] = { 0 };
	int iret = 0;
	short retPaperStatus = 0;
	char merchantName[MERCHANT_NAME_SIZE] = { 0 };
	char merchantAddress1[MERCHANT_ADDRESS_SIZE] = { 0 };
	char merchantAddress2[MERCHANT_ADDRESS_SIZE] = { 0 };
	char Merchant_Id[CARD_ACCEPTOR_ID_LEN + 1] = { 0 };
	char Terminal_Id[10] = { 0 };
	char timeBuffer[16] = { 0 };
	char sysDate[9] = { 0 };				// format date
	char sysTime[11] = { 0 };				// format time
	char countTrans[4] = { 0 };
	char tempbuf[100] = { 0 };

	retPaperStatus = Paperstatus();
	if (retPaperStatus == KEY_CANCEL)
		return KEY_CANCEL;


	


	get_env("#MERCHANT_NAME", merchantName, sizeof(merchantName));
	get_env("#MERCHANT_ADDRESS1", merchantAddress1, sizeof(merchantAddress1));
	get_env("#MERCHANT_ADDRESS2", merchantAddress2, sizeof(merchantAddress2));
	read_clock(timeBuffer); //reads the system clock time     
	sprintf(sysDate, "%c%c:%c%c:%c%c", timeBuffer[8], timeBuffer[9], timeBuffer[10], timeBuffer[11], timeBuffer[12], timeBuffer[13]);// hr:min:sec
	sprintf(sysTime, "%c%c/%c%c/%c%c", timeBuffer[4], timeBuffer[5], timeBuffer[6], timeBuffer[7], timeBuffer[2], timeBuffer[3]); // mm/dd/yy	
	window(1, 1, 30, 20);
	clrscr();
	write_at(Dmsg[PRINTING].dispMsg, strlen(Dmsg[PRINTING].dispMsg), Dmsg[PRINTING].x_cor, Dmsg[PRINTING].y_cor);//PRINTING
		//display_at (11, 10, "PRINTING ...", CLR_EOL);
		//print ("   ");
	printCenter((char *)merchantName, PRINT_BOLD);
	//printBold (merchantName);//print merchant Name
	print(_NEW_LINE);
	//print ("   ");
	printCenter((char *)merchantAddress1, PRINT_BOLD);
	print(_NEW_LINE);
	printCenter((char *)merchantAddress2, PRINT_BOLD);
	print(_NEW_LINE);
	get_env("#TERMINAL_ID", Terminal_Id, sizeof(Terminal_Id));
	print(TERMINALID);
	print(Terminal_Id);//print Terminal Id

	print(_NEW_LINE);
	print(sysTime);//Print System time
	print("            ");
	print(sysDate);//print System Date
	print(_NEW_LINE);
	print(_NEW_LINE);
	printCenter((char *)"OPERATOR LIST", PRINT_BOLD);
	print(_NEW_LINE);
	print(_NEW_LINE);
	for (i; i <= 10; i++)
	{
		sprintf(oper_num, "#OPER_%d_ID", i);
		LOG_PRINTF(("%s", oper_num));
		LOG_PRINTF(("OPER_%d_ID to delete ", i));
		iret = get_env(oper_num, operID, sizeof(oper_num));
		if (iret > 0)
		{
			print(operID);
			print(_NEW_LINE);
		}
	}

	print(_NEW_LINE);
	print(_NEW_LINE);
	print(_NEW_LINE);
	print(_NEW_LINE);
	print(_NEW_LINE);
	print(_NEW_LINE);
	print(_NEW_LINE);
	print(_NEW_LINE);
	clrscr();
}

short validateOperatorPassword()
{
	char Password[16 + 1] = { 0 };//7
	char tempPassword[16 + 1] = { 0 };//7
	char tempPassword1[16 + 1] = { 0 };//7
	short count = 0;
	char attamptLeft[13] = { 0 };
	short iRetVal = 0;
	char timeLimit[TIME_ARR_LEN] = { 0 };
	short MerPwdLength = 0;
	short time_limit = 0;
	char operID[9] = {0};
	char tempoperID[9] = { 0 };
	char operPWD[13] = { 0 };
	int i = 1;
	int iret = 0;
	char oper_num[11] = { 0 };
	char oper_PWD[13] = { 0 };
	int pinreturn = 0;


	//get_env("#PASSWORD", Password, sizeof(Password));
	get_env("#TIME_OUT_LIMIT_IN_SECOND", timeLimit, sizeof(timeLimit));
	MerPwdLength = 8;
	time_limit = atoi(timeLimit);
	time_limit = time_limit * 100;
	do //Password validation
	{
		window(1, 1, 30, 20);
		clrscr();
		if (count != 0 && count != 3)
		{
			write_at(Dmsg[INVALID_PASSWORD].dispMsg, strlen(Dmsg[INVALID_PASSWORD].dispMsg), Dmsg[INVALID_PASSWORD].x_cor, Dmsg[INVALID_PASSWORD].y_cor);//invalid password
			sprintf(attamptLeft, "%d ATTEMPT LEFT", (3 - count));
			write_at(attamptLeft, strlen(attamptLeft), (30 - strlen(attamptLeft)) / 2, Dmsg[INVALID_PASSWORD].y_cor - 2);
		}
		else if (count == 3)
		{
			window(1, 1, 30, 20);
			clrscr();
			write_at(Dmsg[INVALID_PASSWORD].dispMsg, strlen(Dmsg[INVALID_PASSWORD].dispMsg), Dmsg[INVALID_PASSWORD].x_cor, Dmsg[INVALID_PASSWORD].y_cor);
			SVC_WAIT(1000);
			if (LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("Invalid Password"));
			}
			return _FAIL;
		}
		write_at(Dmsg[ENTER_OPERATOR_ID].dispMsg, strlen(Dmsg[ENTER_OPERATOR_ID].dispMsg), Dmsg[ENTER_OPERATOR_ID].x_cor, Dmsg[ENTER_OPERATOR_ID].y_cor);
		window(12, Dmsg[ENTER_OPERATOR_ID].y_cor + 2, 22, Dmsg[ENTER_OPERATOR_ID].y_cor + 2);
		clrscr();
		
		iRetVal = getkbd_entry(hClock, "", (signed char *)operID, (unsigned int)time_limit, (unsigned int)NUMERIC, (char*)szKeyMap, sizeof(szKeyMap), 8, 8);
		if (iRetVal == BTN_CANCEL)
		{
			return KEY_CANCEL;
		}
		else if (iRetVal == 0)
		{
			// Time Out
			window(1, 1, 30, 20);
			clrscr();
			error_tone();
			SVC_WAIT(190);
			error_tone();
			write_at(TIME_OUT, strlen(TIME_OUT), (30 - strlen(TIME_OUT)) / 2, 10);
			SVC_WAIT(4000);
			ClearKbdBuff();
			KBD_FLUSH();
			return _FAIL;
		}
		count++;
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("OperID:%s", operID));
		}
		//find operId in enviroment variables
		for (i; i <= 10; i++)
		{
			sprintf(oper_num, "#OPER_%d_ID", i);
			LOG_PRINTF(("%s", oper_num));
			iret = get_env(oper_num, tempoperID, sizeof(oper_num));
			if (iret > 0)
			{
				if (!strcmp(tempoperID, operID))
				{
					LOG_PRINTF(("OperID found"));
					break;
				}
			}
		}
		if (iret <= 0)
		{
			LOG_PRINTF(("OperID not found"));
			return _FAIL;
		}

		sprintf(oper_PWD, "#OPER_%d_PWD", i);
		LOG_PRINTF(("%s", oper_PWD));
		iret = get_env(oper_PWD, tempPassword, sizeof(oper_PWD));
		if (iret <= 0)
		{
			LOG_PRINTF(("Password not found"));
			return _FAIL;
		}
		else
			LOG_PRINTF(("Password: %s", tempPassword));

		//pin_change = 0;
		//pinreturn = PinPrompt("6220151105029518");//pin
		pinreturn = promptpin(4);
		if (pinreturn != _SUCCESS )
			return _FAIL;

		strcpy(Password, enter_pin);
		LOG_PRINTF(("Pinblock is %s", Password));

	} while (strcmp(Password, tempPassword));
	clrscr();
	strcpy(loggedinOper, operID);
	pin_change = 0;
	return _SUCCESS;
}

short checkpassword(char *temppassword)
{
	int i = 0;

	LOG_PRINTF(("Checking password"));

	for ( i ; i < sizeof(pwd_map) / sizeof(pwd_map[0]); i++ )
	{
		LOG_PRINTF(("Pinblock is %s:%s", &pwd_map[i][0], temppassword ));

		if (!strcmp(&pwd_map[i][0], temppassword))
		{
			return _FAIL;
		}
	}

	return _SUCCESS;
}

short promptpin(int pin_set)
{
	int iRetVal = 0;
	if (pin_set == 1)
	{
		LOG_PRINTF(("pin_set == 1"))
		window(1, 1, 30, 20);
		clrscr();
		write_at(Dmsg[ENTER_OLD_PIN].dispMsg, strlen(Dmsg[ENTER_OLD_PIN].dispMsg), Dmsg[ENTER_OLD_PIN].x_cor, Dmsg[ENTER_OLD_PIN].y_cor);
		window(Dmsg[ENTER_OLD_PIN].x_cor, Dmsg[ENTER_OLD_PIN].y_cor + 2, 18, Dmsg[ENTER_OLD_PIN].y_cor + 2);
		
		iRetVal = getkbd_entry(hClock, "", (signed char *)old_pin, (unsigned int)1000, (unsigned int)PASSWORD, (char*)szKeyMap, sizeof(szKeyMap), 12, 8);
		if (iRetVal == BTN_CANCEL)
		{
			return KEY_CANCEL;
		}
		else if (iRetVal == 0)
		{
			// Time Out
			window(1, 1, 30, 20);
			clrscr();
			error_tone();
			SVC_WAIT(190);
			error_tone();
			write_at(TIME_OUT, strlen(TIME_OUT), (30 - strlen(TIME_OUT)) / 2, 10);
			SVC_WAIT(4000);
			ClearKbdBuff();
			KBD_FLUSH();
			return _FAIL;
		}
		if (iRetVal < 8 || iRetVal > 12)
		{
			window(1, 1, 30, 20);
			clrscr();
			write_at(Dmsg[PIN_INCORRECT_FORMAT].dispMsg, strlen(Dmsg[PIN_INCORRECT_FORMAT].dispMsg), Dmsg[PIN_INCORRECT_FORMAT].x_cor, Dmsg[PIN_INCORRECT_FORMAT].y_cor);
			window(Dmsg[PIN_INCORRECT_FORMAT].x_cor, Dmsg[PIN_INCORRECT_FORMAT].y_cor + 2, 18, Dmsg[PIN_INCORRECT_FORMAT].y_cor + 2);
			SVC_WAIT(3000);
			return _FAIL;
		}
	}

	if (pin_set == 2)
	{
		LOG_PRINTF(("pin_set == 2"))
		window(1, 1, 30, 20);
		clrscr();
		write_at(Dmsg[ENTER_NEW_PIN].dispMsg, strlen(Dmsg[ENTER_NEW_PIN].dispMsg), Dmsg[ENTER_NEW_PIN].x_cor, Dmsg[ENTER_NEW_PIN].y_cor);
		window(Dmsg[ENTER_NEW_PIN].x_cor, Dmsg[ENTER_NEW_PIN].y_cor + 2, 18, Dmsg[ENTER_NEW_PIN].y_cor + 2);

		iRetVal = getkbd_entry(hClock, "", (signed char *)new_pin, (unsigned int)1000, (unsigned int)PASSWORD, (char*)szKeyMap, sizeof(szKeyMap), 12, 8);
		if (iRetVal == BTN_CANCEL)
		{
			return KEY_CANCEL;
		}
		else if (iRetVal == 0)
		{
			// Time Out
			window(1, 1, 30, 20);
			clrscr();
			error_tone();
			SVC_WAIT(190);
			error_tone();
			write_at(TIME_OUT, strlen(TIME_OUT), (30 - strlen(TIME_OUT)) / 2, 10);
			SVC_WAIT(4000);
			ClearKbdBuff();
			KBD_FLUSH();
			return _FAIL;
		}
		if (iRetVal < 8 || iRetVal > 12)
		{
			window(1, 1, 30, 20);
			clrscr();
			write_at(Dmsg[PIN_INCORRECT_FORMAT].dispMsg, strlen(Dmsg[PIN_INCORRECT_FORMAT].dispMsg), Dmsg[PIN_INCORRECT_FORMAT].x_cor, Dmsg[PIN_INCORRECT_FORMAT].y_cor);
			window(Dmsg[PIN_INCORRECT_FORMAT].x_cor, Dmsg[PIN_INCORRECT_FORMAT].y_cor + 2, 18, Dmsg[PIN_INCORRECT_FORMAT].y_cor + 2);
			SVC_WAIT(3000);
			return _FAIL;
		}
	}

	if (pin_set == 3)
	{
		LOG_PRINTF(("pin_set == 3"))
		window(1, 1, 30, 20);
		clrscr();
		write_at(Dmsg[CONFIRM_PIN].dispMsg, strlen(Dmsg[CONFIRM_PIN].dispMsg), Dmsg[CONFIRM_PIN].x_cor, Dmsg[CONFIRM_PIN].y_cor);
		window(Dmsg[CONFIRM_PIN].x_cor, Dmsg[CONFIRM_PIN].y_cor + 2, 18, Dmsg[CONFIRM_PIN].y_cor + 2);

		iRetVal = getkbd_entry(hClock, "", (signed char *)confirm_pin, (unsigned int)1000, (unsigned int)PASSWORD, (char*)szKeyMap, sizeof(szKeyMap), 12, 8);
		if (iRetVal == BTN_CANCEL)
		{
			return KEY_CANCEL;
		}
		else if (iRetVal == 0)
		{
			// Time Out
			window(1, 1, 30, 20);
			clrscr();
			error_tone();
			SVC_WAIT(190);
			error_tone();
			write_at(TIME_OUT, strlen(TIME_OUT), (30 - strlen(TIME_OUT)) / 2, 10);
			SVC_WAIT(4000);
			ClearKbdBuff();
			KBD_FLUSH();
			return _FAIL;
		}
		if (iRetVal < 8 || iRetVal > 12)
		{
			window(1, 1, 30, 20);
			clrscr();
			write_at(Dmsg[PIN_INCORRECT_FORMAT].dispMsg, strlen(Dmsg[PIN_INCORRECT_FORMAT].dispMsg), Dmsg[PIN_INCORRECT_FORMAT].x_cor, Dmsg[PIN_INCORRECT_FORMAT].y_cor);
			window(Dmsg[PIN_INCORRECT_FORMAT].x_cor, Dmsg[PIN_INCORRECT_FORMAT].y_cor + 2, 18, Dmsg[PIN_INCORRECT_FORMAT].y_cor + 2);
			SVC_WAIT(3000);
			return _FAIL;
		}
	}

	if (pin_set == 4)
	{
		LOG_PRINTF(("pin_set == 1"))
			window(1, 1, 30, 20);
		clrscr();
		write_at(Dmsg[ENTER_PIN].dispMsg, strlen(Dmsg[ENTER_PIN].dispMsg), Dmsg[ENTER_OLD_PIN].x_cor, Dmsg[ENTER_PIN].y_cor);
		window(Dmsg[ENTER_PIN].x_cor, Dmsg[ENTER_PIN].y_cor + 2, 18, Dmsg[ENTER_PIN].y_cor + 2);

		iRetVal = getkbd_entry(hClock, "", (signed char *)enter_pin, (unsigned int)1000, (unsigned int)PASSWORD, (char*)szKeyMap, sizeof(szKeyMap), 12, 8);
		if (iRetVal == BTN_CANCEL)
		{
			return KEY_CANCEL;
		}
		else if (iRetVal == 0)
		{
			// Time Out
			window(1, 1, 30, 20);
			clrscr();
			error_tone();
			SVC_WAIT(190);
			error_tone();
			write_at(TIME_OUT, strlen(TIME_OUT), (30 - strlen(TIME_OUT)) / 2, 10);
			SVC_WAIT(4000);
			ClearKbdBuff();
			KBD_FLUSH();
			return _FAIL;
		}
		if (iRetVal < 8 || iRetVal > 12)
		{
			window(1, 1, 30, 20);
			clrscr();
			write_at(Dmsg[PIN_INCORRECT_FORMAT].dispMsg, strlen(Dmsg[PIN_INCORRECT_FORMAT].dispMsg), Dmsg[PIN_INCORRECT_FORMAT].x_cor, Dmsg[PIN_INCORRECT_FORMAT].y_cor);
			window(Dmsg[PIN_INCORRECT_FORMAT].x_cor, Dmsg[PIN_INCORRECT_FORMAT].y_cor + 2, 18, Dmsg[PIN_INCORRECT_FORMAT].y_cor + 2);
			SVC_WAIT(3000);
			return _FAIL;
		}
	}
	return _SUCCESS;
}


short saveUserdtls(char opt, char* oper)
{
	FILE *ifp = NULL;
	char timeBuffer[TIME_BUFFER_LEN] = { 0 };//16
	char sysDate[9] = { 0 };				// format date
	char sysTime[9] = { 0 };				// format time
	UserAudit userAudit;

	read_clock(timeBuffer); //reads the system clock time     
	sprintf(sysTime, "%c%c:%c%c:%c%c", timeBuffer[8], timeBuffer[9], timeBuffer[10], timeBuffer[11], timeBuffer[12], timeBuffer[13]);// hr:min:sec
	sprintf(sysDate, "%c%c/%c%c/%c%c", timeBuffer[4], timeBuffer[5], timeBuffer[6], timeBuffer[7], timeBuffer[2], timeBuffer[3]); // mm/dd/yy	

	memset(&userAudit, 0, sizeof(UserAudit));
	strcpy(userAudit.operID, oper);
	userAudit.code = opt;
	strcpy(userAudit.Date, sysDate);
	strcpy(userAudit.Time, sysTime);

	if (LOG_STATUS == LOG_ENABLE)
	{
		LOG_PRINTF(("=========Writing to the file %s\n", USER_AUDIT_FILE));
	}

	ifp = fopen(USER_AUDIT_FILE, "a");
	if (ifp == NULL)
	{
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Failed to open the file %s\n", USER_AUDIT_FILE));
		}
		return _FAIL;
	}
	else
	{
		if (fwrite(&userAudit, sizeof(userAudit), 1, ifp) != 1)
		{
			if (LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("Failed to write to %s\n", USER_AUDIT_FILE));
			}
			return _FAIL;
		}
		fclose(ifp);
	}
	return _SUCCESS;
}
