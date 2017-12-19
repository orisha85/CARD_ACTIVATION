/***************************************************************************
  Filename		     : IPP_Key_injection.c
  Project		       : Bevertec 
  Developer Neame  : Amar
  Module           : PIN encription
  Description	     : This is the implementation of master-session encryption
*****************************************************************************/


#include <ssl.h> 
#include <err.h>
#include <des.h>

#include "..\Include\Common.h"
#include "..\Include\IPP_Key_injection.h"
#include "..\Include\SaleTransaction.h" 
#include "..\Include\ISo8583Map.h"

#include <errno.h>


unsigned char EncPinBlock[BITMAP_ARRAY_SIZE]={0}; //Pin data
unsigned char EncOldPinBlock[BITMAP_ARRAY_SIZE] = { 0 }; //Old Pin data. Used for pin change
unsigned char KSN_pp[KSN_LENGTH] = { 0 }; //KSN data
unsigned char EncSessionKey[UNPACK_ENC_PR_KEY_LEN+1]=""; //for logon response session key 
unsigned char EncSessionKeyKCV[UNPACK_ENC_PR_KCV_LEN+1]="";
unsigned char DataEncKey[UNPACK_ENC_PR_KEY_LEN+1]=""; 
unsigned char DataEncKeyKCV[UNPACK_ENC_PR_KCV_LEN+1]="";

/****************************************************************************************************************
*	Function Name : KeyInjection																													              *
*	Purpose		  : Process key injection										              *
*	Input		  : char buffer pointer to the primary account no										                              *
*	Output		  : Return value for the success or failure																						              *
*****************************************************************************************************************/

int KeyInjection(char *PrimaryAccNO)
{
	int iIPPHandle;
	short iRet = -1, iRet1 = -1, iRetVal = -1;
	char OutBuff[IPP_BUFFER_SIZE] = { 0 };//OUTPUT BUFFER
	char InBuff[IPP_BUFFER_SIZE] = { 0 };//input buffer
	unsigned char KeyMasterKey[UNPACK_ENC_PR_KEY_LEN + 1] = { 0 };//unpack master key	
	unsigned char AftterMasterKey[PAD_MAC_LENGTH_AFTER_MK + 1] = { 0 };//56 UNCRYPTED BYTES BEFORE THE MASTER KEY
	unsigned char BeforeMasterKey[HEADER_LENGTH_BEFORE_MK + 1] = { 0 };//32 BYTES UNCRYPTED AFTER MASTER KEY (PADDING + MAC) 
	unsigned char packMasterKey[PACK_KEY_SIZE + 1] = { 0 };//packed master key 
	unsigned char Key[PACK_KEY_SIZE] = { 0 };//for packed encrypted key
	unsigned char OutKey[PACK_KEY_SIZE] = { 0 };//for packed decrypted key

	unsigned char Decr_Key1[PACK_KEY_SIZE + 1] = { 0 };//key part 1
	unsigned char Decr_Key2[PACK_KEY_SIZE + 1] = { 0 };//key part 2

	unsigned char temp_Buff[UNPACK_ENC_PR_KEY_LEN + 1] = { 0 };

	//Added a #PIN_ENC to select DUKPT or MS
	//DUKPT = 0
	//MS = 1	
	char pin_enc_char;
	get_env("#PIN_ENC", (char *)pin_enc_char, sizeof(pin_enc));

	//pin_enc = atoi(pin_enc_char);

	if (LOG_STATUS == LOG_ENABLE)
	{
		LOG_PRINTF(("-----IPP Communication session key =%s\n", EncSessionKey));
	}

	//Check if there is an active session
	if (!strcmp((char *)EncSessionKey, ""))
	{
		window(1, 1, 30, 20);
		clrscr();
		write_at(Dmsg[PLEASE_LOGON_FIRST].dispMsg, strlen(Dmsg[PLEASE_LOGON_FIRST].dispMsg), Dmsg[PLEASE_LOGON_FIRST].x_cor, Dmsg[PLEASE_LOGON_FIRST].y_cor);//LOGON SUCCESSFULL

		SVC_WAIT(2000);
		ClearKbdBuff();
		KBD_FLUSH();
		return _FAIL;
	}

	//Open IPP port 
	iIPPHandle = open_ipp_port();
	if (iIPPHandle < 0)
	{
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Open IPP OK Error hdl %d errno %d", iIPPHandle, errno));
		}
		clrscr();

		return 1;
	}
	else
	{
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Open IPP OK "));
		}
	}

	// Gets IPP info. Packet 06 - Request Pin Pad serial number
	memset(InBuff, 0, sizeof(InBuff));
	strcpy(InBuff, "06");
	iRet = ipp_communicate(iIPPHandle, 0x0F, (unsigned char *)InBuff, (unsigned char *)OutBuff, TRUE);
	if (LOG_STATUS == LOG_ENABLE)
	{
		LOG_PRINTF(("Get IPP Info return %d", iRet));
		LOG_PRINTF(("Packet 06 %s", InBuff));
	}

	if (iRet != PP_COMM_OK)
	{
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("ipp_communicate fail"));
		}
		return _FAIL;
	}
	
	// Set IPP Key Managment Mode. Packet 15, VISA value cover MS/DUKPT management modes 
	memset(InBuff, 0, sizeof(InBuff));
	strcpy(InBuff, "15VISA");
	iRet = ipp_communicate(iIPPHandle, SI, (unsigned char *)InBuff, (unsigned char *)OutBuff, TRUE);
	if (LOG_STATUS == LOG_ENABLE)
	{
		LOG_PRINTF(("Get IPP Key Managment Mode return %d", iRet));
		LOG_PRINTF(("Packet 15 %s", InBuff));
	}
	if (iRet != PP_COMM_OK)
	{
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("ipp_communicate fail"));
		}
		return _FAIL;
	}

	//Set Key Management Mode. Packet 17 
	memset(InBuff, 0, sizeof(InBuff));


	if (pin_enc)
	{
		if (c3DES == '1')
		{
			// Set all MSSequence engines to 3DES
			strcpy(InBuff, "17020");
			iRet = ipp_communicate(iIPPHandle, 0x0F, (unsigned char *)InBuff, (unsigned char *)OutBuff, TRUE);
		}
		else
		{
			// Sets  all MSSequence engines to 1DES
			strcpy(InBuff, "17000");
			iRet = ipp_communicate(iIPPHandle, 0x0F, (unsigned char *)InBuff, (unsigned char *)OutBuff, TRUE);
		}
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Set MSSequence %s return %d", (c3DES == '1') ? "3DES" : "1DES", iRet));
		}
		//--------------19-4-2016---------------
		if (iRet != PP_COMM_OK)
		{
			if (LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("ipp_communicate fail"));
			}
			return _FAIL;
		}
	}
	else
	{
		//17080 enables 3DES DUKPT mode
		strcpy(InBuff, "17080");
		iRet = ipp_communicate(iIPPHandle, 0x0F, (unsigned char *)InBuff, (unsigned char *)OutBuff, TRUE);

		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Packet 17 %s", InBuff));
		}
		if (iRet != PP_COMM_OK)
		{
			if (LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("ipp_communicate fail"));
			}
			return _FAIL;
		}
	}


	//Gets IPP Key Management mode. Packet 18
	LOG_PRINTF(("Get IPP Key Management mode"));
	memset(InBuff, 0, sizeof(InBuff));
	strcpy(InBuff, "18");
	iRet = ipp_communicate(iIPPHandle, 0x0F, (unsigned char *)InBuff, (unsigned char *)OutBuff, TRUE);

	if (LOG_STATUS == LOG_ENABLE)
	{
		LOG_PRINTF(("Get IPP Mode return %d", iRet));
		LOG_PRINTF(("Packet 18 %s", InBuff));
	}
	if (iRet != PP_COMM_OK)
	{
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("ipp_communicate fail"));
		}
		return _FAIL;
	}	

	//LOG_PRINTF(("KEY INJECT STARTS"));
	if (pin_enc)
	{
		LOG_PRINTF(("MS KEY INJECT STARTS"));
		//Select Master Key
		memset(InBuff, 0, sizeof(InBuff));
		sprintf(InBuff, "080");
		iRet = ipp_communicate(iIPPHandle, 0x0F, (unsigned char *)InBuff, (unsigned char *)NULL, TRUE);
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Set MSSequence Engine return %d", iRet));
		}
		//--------------19-4-2016---------------
		if (iRet != PP_COMM_OK)
		{
			if (LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("ipp_communicate fail"));
			}
			return _FAIL;
		}
		// Check Master Key.-----------------------------------
		memset(InBuff, 0, sizeof(InBuff));
		sprintf(InBuff, "040");
		iRet = ipp_communicate(iIPPHandle, 0x0F, (unsigned char *)InBuff, (unsigned char *)OutBuff, TRUE);
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Check MSSequence Engine return %d", iRet));
		}
		//--------------19-4-2016---------------
		if (iRet != PP_COMM_OK)
		{
			if (LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("ipp_communicate fail"));
			}
			return _FAIL;
		}

		// Transfer Master Key.doc
		strcpy((char *)BeforeMasterKey, BEFORE_MASTER_KEY_PART);//Header+length
		strcpy((char *)KeyMasterKey, MASTER_KEY);//clear component
		strcpy((char *)AftterMasterKey, AFTER_MASTER_KEY_PART);//PAD+MAc

		memset(InBuff, 0, sizeof(InBuff));

		sprintf(InBuff, "020%s%s%s", BeforeMasterKey, KeyMasterKey, AftterMasterKey);
		iRet = ipp_communicate(iIPPHandle, 0x0F, (unsigned char *)InBuff, (unsigned char *)OutBuff, TRUE);
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Transfer Master Key Check MSSequence Engine return %d", iRet));
		}

		//--------------19-4-2016---------------
		if (iRet != PP_COMM_OK)
		{
			if (LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("ipp_communicate fail"));
			}
			return _FAIL;
		}

		packData((char*)KeyMasterKey, (char*)packMasterKey);//packing master key
		packData((char*)EncSessionKey, (char*)Key);//packing encrypted logon session key
		iRetVal = TDES_Decrypt_Logon_key(Key, OutKey, packMasterKey);////decrypting encrypted session key 
																	 //--------------19-4-2016---------------
		if (iRetVal != _SUCCESS)
		{
			if (LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("TDES_Decrypt_Logon_key fail"));
			}
			return _FAIL;
		}
		//--------------19-4-2016---------------

		if (LOG_STATUS == LOG_ENABLE)
		{
			bcd2a((char*)temp_Buff, (char*)OutKey, sizeof(OutKey));
			LOG_PRINTF(("-----Decrypted sestrhhjysion  =%s\n", temp_Buff));
		}

		strncpy((char*)Decr_Key1, (char*)temp_Buff, PACK_KEY_SIZE); //copying first 16 bytes to key1
		strncpy((char*)Decr_Key2, (char*)temp_Buff + PACK_KEY_SIZE, PACK_KEY_SIZE); //copying first 16 bytes to key1

		memset(InBuff, 0, sizeof(InBuff));
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Key Injection :transMsg->PrimaryAccNum = %s", PrimaryAccNO));
		}
		MakingGiske(Decr_Key1, Decr_Key2, KeySessionKeyEncripted, packMasterKey);
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("KeySessionKeyEncripted %s", KeySessionKeyEncripted));
		}

		LOG_PRINTF(("pin_change: %d", pin_change));
		if (pin_change != 0)
		{
			window(1, 1, 30, 20);
			clrscr();
			write_at(Dmsg[ENTER_OLD_PIN].dispMsg, strlen(Dmsg[ENTER_OLD_PIN].dispMsg), Dmsg[ENTER_OLD_PIN].x_cor, Dmsg[ENTER_OLD_PIN].y_cor);
			window(Dmsg[ENTER_OLD_PIN].x_cor, Dmsg[ENTER_OLD_PIN].y_cor + 2, 18, Dmsg[ENTER_OLD_PIN].y_cor + 2);
			pin_change++;
		}
		else
		{
			window(1, 1, 30, 20);
			clrscr();
			write_at(Dmsg[ENTER_PIN].dispMsg, strlen(Dmsg[ENTER_PIN].dispMsg), Dmsg[ENTER_PIN].x_cor, Dmsg[ENTER_PIN].y_cor);
			window(Dmsg[ENTER_PIN].x_cor, Dmsg[ENTER_PIN].y_cor + 2, 18, Dmsg[ENTER_PIN].y_cor + 2);
		}


		//strncpy(track2data,TRACK2_DATA,strlen(TRACK2_DATA));        //35 Sample Track 2 Data provided by Client 


		//ptr = strtok((char*)track2data, delim);//extracting account no from track2 data
		//strncpy(transMsg->PrimaryAccNum,ptr,strlen(transMsg->PrimaryAccNum));  
		/////////----Sending Packet Z63: Accept and Encrypt PIN..
		sprintf(InBuff, "Z63.%s%c%s%s%s%c%c", PrimaryAccNO, 0x1C, KeySessionKeyEncripted, MIN_PIN_LEN, MAX_PIN_LEN, NULL_PIN_NO, ECHO_CHAR);

		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF((" acc no =%s", PrimaryAccNO));
			LOG_PRINTF(("_________Z63 Pakcet=%s", InBuff));
		}
		memset(OutBuff, 0x00, sizeof(OutBuff));
		iRet1 = ipp_communicate(iIPPHandle, 0x02, (unsigned char*)InBuff, (unsigned char *)OutBuff, FALSE);
		window(1, 1, 30, 20);
		clrscr();
		write_at(Dmsg[PROCESSING].dispMsg, strlen(Dmsg[PROCESSING].dispMsg), Dmsg[PROCESSING].x_cor, Dmsg[PROCESSING].y_cor);

		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Enter PIN return  = %d", iRet1));

		}

		memset(InBuff, 0, sizeof(InBuff));
		strcpy(InBuff, "72");
		iRet = ipp_communicate(iIPPHandle, 0x02, (unsigned char *)InBuff, (unsigned char *)OutBuff, FALSE);
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Terminate Sessionnn return %d", iRet));
		}
		//--------------19-4-2016---------------
		if (iRet != PP_COMM_OK)
		{
			if (LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("ipp_communicate fail"));
			}
			return _FAIL;
		}
	}
	else
	{
		LOG_PRINTF(("DUKPT KEY INJECT STARTS"));
		memset(InBuff, 0, sizeof(InBuff));
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Key Injection :transMsg->PrimaryAccNum = %s", PrimaryAccNO));
		}

		//Injecting DUKPT Keys. Packet 19 -  Select a DUKPT Engine
		//First step is selecting a DUKPT Engine, not necessary but recommended before sending any DUKPT packet
		memset(InBuff, 0, sizeof(InBuff));
		strcpy(InBuff, "190");
		iRet = ipp_communicate(iIPPHandle, 0x0F, (unsigned char *)InBuff, (unsigned char *)OutBuff, TRUE);

		//TODO: For debugging purposes, delete later
		//
		if (LOG_STATUS == LOG_ENABLE)
		{
			//LOG_PRINTF(("Packet 19 %s", InBuff));
		}
		//Check for failed comm
		if (iRet != PP_COMM_OK)
		{
			if (LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("ipp_communicate fail"));
			}
			return _FAIL;
		}

		//TODO: Key loading. Takes IPEK and KSN(currently hardcoded). Packet 90 - Load Initial Key Request 
		memset(InBuff, 0, sizeof(InBuff));
		sprintf(InBuff, "90%s%s", IPEK, KSN);
		iRet = ipp_communicate(iIPPHandle, 0x02, (unsigned char *)InBuff, (unsigned char *)OutBuff, TRUE);

		//TODO: For debugging purposes, delete later
		//
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Packet 90 %s", InBuff));
		}
		//Check for failed comm
		if (iRet != PP_COMM_OK)
		{
			if (LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("ipp_communicate fail"));
			}
			return _FAIL;
		}

		window(1, 1, 30, 20);
		clrscr();
		write_at(Dmsg[ENTER_PIN].dispMsg, strlen(Dmsg[ENTER_PIN].dispMsg), Dmsg[ENTER_PIN].x_cor, Dmsg[ENTER_PIN].y_cor);
		window(Dmsg[ENTER_PIN].x_cor, Dmsg[ENTER_PIN].y_cor + 2, 18, Dmsg[ENTER_PIN].y_cor + 2);

		/////////----Sending Packet Z63: Accept and Encrypt PIN..
		sprintf(InBuff, "Z63.%s%c%s%s%s%c%c", PrimaryAccNO, 0x1C, "DUKPT ENCRYPTION", MIN_PIN_LEN, MAX_PIN_LEN, NULL_PIN_NO, ECHO_CHAR);

		LOG_PRINTF(("Inbuff %s", InBuff));

		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF((" acc no =%s", PrimaryAccNO));
			LOG_PRINTF(("_________Z63 Pakcet=%s", InBuff));
		}
		memset(OutBuff, 0x00, sizeof(OutBuff));
		iRet1 = ipp_communicate(iIPPHandle, 0x02, (unsigned char*)InBuff, (unsigned char *)OutBuff, FALSE);
		window(1, 1, 30, 20);
		clrscr();
		write_at(Dmsg[PROCESSING].dispMsg, strlen(Dmsg[PROCESSING].dispMsg), Dmsg[PROCESSING].x_cor, Dmsg[PROCESSING].y_cor);

		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Enter PIN return  = %d", iRet1));

		}
	}

	//Close the device
	vdPIN_Close(iIPPHandle);

	if (iRet1 == -3)
	{
		window(1, 1, 30, 20);
		clrscr();
		error_tone();
		write_at(TIME_OUT, strlen(TIME_OUT), (30 - strlen(TIME_OUT)) / 2, 10);

		error_tone();
		error_tone();
		SVC_WAIT(3000);

	}
	key_injected = 1;
	return iRet1;
}


int PinPrompt(char *PrimaryAccNO)
{
	int iIPPHandle;
	short iRet = -1, iRet1 = -1, iRetVal = -1;
	char OutBuff[IPP_BUFFER_SIZE] = { 0 };//OUTPUT BUFFER
	char InBuff[IPP_BUFFER_SIZE] = { 0 };//input buffer

	if (key_injected == 0) //Extra injection check
	{
		LOG_PRINTF(("Init:KeyInjection"));
		iRet = KeyInjection(PrimaryAccNO);
		return iRet;
	}

	iIPPHandle = open_ipp_port();
	if (iIPPHandle < 0)
	{
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Open IPP OK Error hdl %d errno %d", iIPPHandle, errno));
		}
		clrscr();

		return PP_COMM_FAILURE;
	}
	else
	{
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Open IPP OK "));
		}
	}

	if (pin_enc)
	{
		LOG_PRINTF(("pin_change = %d", pin_change));
		if (pin_change != 0)
		{
			if (pin_change == 1)
			{
				LOG_PRINTF(("enter new pin, pin_change == 1"))
				window(1, 1, 30, 20);
				clrscr();
				write_at(Dmsg[ENTER_OLD_PIN].dispMsg, strlen(Dmsg[ENTER_OLD_PIN].dispMsg), Dmsg[ENTER_OLD_PIN].x_cor, Dmsg[ENTER_OLD_PIN].y_cor);
				window(Dmsg[ENTER_OLD_PIN].x_cor, Dmsg[ENTER_OLD_PIN].y_cor + 2, 18, Dmsg[ENTER_OLD_PIN].y_cor + 2);
				pin_change++;
			}
			else
			{
				LOG_PRINTF(("enter new pin, pin_change == 2"))
				window(1, 1, 30, 20);
				clrscr();
				write_at(Dmsg[ENTER_NEW_PIN].dispMsg, strlen(Dmsg[ENTER_NEW_PIN].dispMsg), Dmsg[ENTER_NEW_PIN].x_cor, Dmsg[ENTER_NEW_PIN].y_cor);
				window(Dmsg[ENTER_NEW_PIN].x_cor, Dmsg[ENTER_NEW_PIN].y_cor + 2, 18, Dmsg[ENTER_NEW_PIN].y_cor + 2);
				pin_change = 1;
			}

		}
		else
		{
			window(1, 1, 30, 20);
			clrscr();
			write_at(Dmsg[ENTER_PIN].dispMsg, strlen(Dmsg[ENTER_PIN].dispMsg), Dmsg[ENTER_PIN].x_cor, Dmsg[ENTER_PIN].y_cor);
			window(Dmsg[ENTER_PIN].x_cor, Dmsg[ENTER_PIN].y_cor + 2, 18, Dmsg[ENTER_PIN].y_cor + 2);
		}


		//strncpy(track2data,TRACK2_DATA,strlen(TRACK2_DATA));        //35 Sample Track 2 Data provided by Client 


		//ptr = strtok((char*)track2data, delim);//extracting account no from track2 data
		//strncpy(transMsg->PrimaryAccNum,ptr,strlen(transMsg->PrimaryAccNum));  
		/////////----Sending Packet Z63: Accept and Encrypt PIN..
		sprintf(InBuff, "Z63.%s%c%s%s%s%c%c", PrimaryAccNO, 0x1C, KeySessionKeyEncripted, MIN_PIN_LEN, MAX_PIN_LEN, NULL_PIN_NO, ECHO_CHAR);

		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF((" acc no =%s", PrimaryAccNO));
			LOG_PRINTF(("_________Z63 Pakcet=%s", InBuff));
		}
		memset(OutBuff, 0x00, sizeof(OutBuff));
		iRet1 = ipp_communicate(iIPPHandle, 0x02, (unsigned char*)InBuff, (unsigned char *)OutBuff, FALSE);
		window(1, 1, 30, 20);
		clrscr();
		write_at(Dmsg[PROCESSING].dispMsg, strlen(Dmsg[PROCESSING].dispMsg), Dmsg[PROCESSING].x_cor, Dmsg[PROCESSING].y_cor);

		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Enter PIN return  = %d", iRet1));

		}

		memset(InBuff, 0, sizeof(InBuff));
		strcpy(InBuff, "72");
		iRet = ipp_communicate(iIPPHandle, 0x02, (unsigned char *)InBuff, (unsigned char *)OutBuff, FALSE);
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Terminate Sessionnn return %d", iRet));
		}
		//--------------19-4-2016---------------
		if (iRet != PP_COMM_OK)
		{
			if (LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("ipp_communicate fail"));
			}
			return _FAIL;
		}
	}
	else
	{

		if (pin_change = !0)
		{
			if (pin_change == 1)
			{
				window(1, 1, 30, 20);
				clrscr();
				write_at(Dmsg[ENTER_OLD_PIN].dispMsg, strlen(Dmsg[ENTER_OLD_PIN].dispMsg), Dmsg[ENTER_OLD_PIN].x_cor, Dmsg[ENTER_OLD_PIN].y_cor);
				window(Dmsg[ENTER_OLD_PIN].x_cor, Dmsg[ENTER_OLD_PIN].y_cor + 2, 18, Dmsg[ENTER_OLD_PIN].y_cor + 2);
				pin_change++;
			}
			else
			{
				window(1, 1, 30, 20);
				clrscr();
				write_at(Dmsg[ENTER_NEW_PIN].dispMsg, strlen(Dmsg[ENTER_NEW_PIN].dispMsg), Dmsg[ENTER_NEW_PIN].x_cor, Dmsg[ENTER_NEW_PIN].y_cor);
				window(Dmsg[ENTER_NEW_PIN].x_cor, Dmsg[ENTER_NEW_PIN].y_cor + 2, 18, Dmsg[ENTER_NEW_PIN].y_cor + 2);
				pin_change = 0;
			}

		}
		else
		{
			window(1, 1, 30, 20);
			clrscr();
			write_at(Dmsg[ENTER_PIN].dispMsg, strlen(Dmsg[ENTER_PIN].dispMsg), Dmsg[ENTER_PIN].x_cor, Dmsg[ENTER_PIN].y_cor);
			window(Dmsg[ENTER_PIN].x_cor, Dmsg[ENTER_PIN].y_cor + 2, 18, Dmsg[ENTER_PIN].y_cor + 2);
		}

		/////////----Sending Packet Z63: Accept and Encrypt PIN..
		sprintf(InBuff, "Z63.%s%c%s%s%s%c%c", PrimaryAccNO, 0x1C, "DUKPT ENCRYPTION", MIN_PIN_LEN, MAX_PIN_LEN, NULL_PIN_NO, ECHO_CHAR);

		LOG_PRINTF(("Inbuff %s", InBuff));

		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF((" acc no =%s", PrimaryAccNO));
			LOG_PRINTF(("_________Z63 Pakcet=%s", InBuff));
		}
		memset(OutBuff, 0x00, sizeof(OutBuff));
		iRet1 = ipp_communicate(iIPPHandle, 0x02, (unsigned char*)InBuff, (unsigned char *)OutBuff, FALSE);
		window(1, 1, 30, 20);
		clrscr();
		write_at(Dmsg[PROCESSING].dispMsg, strlen(Dmsg[PROCESSING].dispMsg), Dmsg[PROCESSING].x_cor, Dmsg[PROCESSING].y_cor);

		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Enter PIN return  = %d", iRet1));

		}
	}

	//Close the device
	vdPIN_Close(iIPPHandle);

	if (iRet1 == -3)
	{
		window(1, 1, 30, 20);
		clrscr();
		error_tone();
		write_at(TIME_OUT, strlen(TIME_OUT), (30 - strlen(TIME_OUT)) / 2, 10);

		error_tone();
		error_tone();
		SVC_WAIT(3000);

	}

	LOG_PRINTF(("iRet1 PinPromt = %d", iRet1));
	return iRet1;
}


int ProcessingKeyInjection(char *PrimaryAccNO)
{
  int iIPPHandle ;
  
  short iRet=-1,iRet1=-1,iRetVal = -1;
  //char *ptr =NULL ;
  //const char delim[2] = "=";
  //char track2data[TRACK2_DATA_LEN+1]={0};//stores the track2 data for extracting the PAN
  char OutBuff[IPP_BUFFER_SIZE]  = {0};//OUTPUT BUFFER
	char InBuff[IPP_BUFFER_SIZE] = {0};//input buffer
  unsigned char KeyMasterKey[UNPACK_ENC_PR_KEY_LEN+1]= {0};//unpack master key
  unsigned char KeySessionKeyEncripted[GISKE_KEY_LEN+1]= {0};//Giske Block specs for session key
  unsigned char AftterMasterKey[PAD_MAC_LENGTH_AFTER_MK+1]= {0};//56 UNCRYPTED BYTES BEFORE THE MASTER KEY
  unsigned char BeforeMasterKey[HEADER_LENGTH_BEFORE_MK+1]= {0};//32 BYTES UNCRYPTED AFTER MASTER KEY (PADDING + MAC) 
  unsigned char packMasterKey[PACK_KEY_SIZE+1]={0};//packed master key 
  unsigned char Key[PACK_KEY_SIZE]={0};//for packed encrypted key
  unsigned char OutKey[PACK_KEY_SIZE]={0};//for packed decrypted key

  unsigned char Decr_Key1[PACK_KEY_SIZE+1]={0};//key part 1
  unsigned char Decr_Key2[PACK_KEY_SIZE+1]={0};//key part 2

  unsigned char temp_Buff[UNPACK_ENC_PR_KEY_LEN+1]={0}; 
  
  if(LOG_STATUS == LOG_ENABLE)
  {
    LOG_PRINTF (("-----IPP Communication session key =%s\n",EncSessionKey));
  }
  if(!strcmp((char *)EncSessionKey,"")) 
  {
     window(1,1,30,20);
     clrscr();
		 write_at(Dmsg[PLEASE_LOGON_FIRST].dispMsg, strlen(Dmsg[PLEASE_LOGON_FIRST].dispMsg),Dmsg[PLEASE_LOGON_FIRST].x_cor, Dmsg[PLEASE_LOGON_FIRST].y_cor);//LOGON SUCCESSFULL
     
     SVC_WAIT(2000);
     ClearKbdBuff();
		 KBD_FLUSH();
     return _FAIL;
  }
  

	iIPPHandle = open_ipp_port();
	if (iIPPHandle < 0) 
	{
    if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF(("Open IPP OK Error hdl %d errno %d", iIPPHandle, errno));
    }
		clrscr();
		
		return 1;
	}
  else
  {
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("Open IPP OK "));
    }
  }
    // Gets IPP info -----------------------------------------------------------
  memset(InBuff,0,sizeof(InBuff));
  strcpy (InBuff, "06");
  iRet = ipp_communicate ( iIPPHandle, 0x0F, (unsigned char *)InBuff, ( unsigned char *)OutBuff, TRUE);
	if(LOG_STATUS == LOG_ENABLE)
  {
    LOG_PRINTF(("Get IPP Info return %d", iRet));
  }
	//--------------19-4-2016---------------
	if(iRet != PP_COMM_OK)
	{
      if(LOG_STATUS == LOG_ENABLE)
      {
			  LOG_PRINTF (("ipp_communicate fail"));
      }
			return _FAIL;
	}
	//-------------- END ---------------
   // Set IPP Key Managment Mode-----------------------------------------------
	memset(InBuff,0,sizeof(InBuff));
  strcpy (InBuff, "15VISA");
  iRet = ipp_communicate (iIPPHandle, SI, (unsigned char *)InBuff, ( unsigned char *)OutBuff, TRUE);
	if(LOG_STATUS == LOG_ENABLE)
  {
    LOG_PRINTF(("Get IPP Key Managment Mode return %d", iRet));
  }
	//--------------19-4-2016---------------
	if(iRet != PP_COMM_OK)
	{
      if(LOG_STATUS == LOG_ENABLE)
      {
			  LOG_PRINTF (("ipp_communicate fail"));
      }
			return _FAIL;
	}
	//-------------- END ---------------

  // Set MSSequence Engine DES
	memset(InBuff,0,sizeof(InBuff));
 
  if(c3DES == '1') 
  {
    // Set all MSSequence engines to 3DES
    strcpy (InBuff, "17020"); 
    iRet = ipp_communicate (iIPPHandle, 0x0F, (unsigned char *)InBuff, ( unsigned char *)OutBuff, TRUE);
  }
  else 
	{
    // Sets  all MSSequence engines to 1DES
    strcpy (InBuff, "17000"); 
    iRet = ipp_communicate (iIPPHandle, 0x0F, (unsigned char *)InBuff, ( unsigned char *)OutBuff, TRUE);
  }  
  if(LOG_STATUS == LOG_ENABLE)
  {
	  LOG_PRINTF(("Set MSSequence %s return %d", (c3DES=='1') ? "3DES" : "1DES", iRet));
  }
	//--------------19-4-2016---------------
	if(iRet != PP_COMM_OK)
	{
      if(LOG_STATUS == LOG_ENABLE)
      {
			  LOG_PRINTF (("ipp_communicate fail"));
      }
			return _FAIL;
	}
	//-------------- END ---------------
  // gets IPP mode -----------------------------------------------------------
	memset(InBuff,0,sizeof(InBuff));
	strcpy (InBuff, "18");
  iRet = ipp_communicate ( iIPPHandle, 0x0F, (unsigned char *)InBuff, ( unsigned char *)OutBuff, TRUE);
	
  if(LOG_STATUS == LOG_ENABLE)
  {
    LOG_PRINTF(("Get IPP Mode return %d", iRet));       
  }
  //--------------19-4-2016---------------
	if(iRet != PP_COMM_OK)
	{
      if(LOG_STATUS == LOG_ENABLE)
      {
			  LOG_PRINTF (("ipp_communicate fail"));
      }
			return _FAIL;
	}
	//-------------- END ---------------


  //Select Master Key
  memset(InBuff,0,sizeof(InBuff));
	sprintf (InBuff, "080" );
  iRet = ipp_communicate ( iIPPHandle, 0x0F, (unsigned char *)InBuff, (unsigned char *)NULL, TRUE);
	if(LOG_STATUS == LOG_ENABLE)
  {
    LOG_PRINTF(("Set MSSequence Engine return %d", iRet));                    
  }
	//--------------19-4-2016---------------
	if(iRet != PP_COMM_OK)
	{
      if(LOG_STATUS == LOG_ENABLE)
      {
			  LOG_PRINTF (("ipp_communicate fail"));
      }
			return _FAIL;
	}
  // Check Master Key.-----------------------------------
	memset(InBuff,0,sizeof(InBuff));
  sprintf (InBuff, "040" );
  iRet = ipp_communicate ( iIPPHandle, 0x0F, (unsigned char *)InBuff, (unsigned char *)OutBuff, TRUE);
	if(LOG_STATUS == LOG_ENABLE)
  {
    LOG_PRINTF(("Check MSSequence Engine return %d", iRet)); 
  }
	//--------------19-4-2016---------------
	if(iRet != PP_COMM_OK)
	{
      if(LOG_STATUS == LOG_ENABLE)
      {
			  LOG_PRINTF (("ipp_communicate fail"));
      }
			return _FAIL;
	}
	
  // Transfer Master Key.doc
  strcpy((char *)BeforeMasterKey, BEFORE_MASTER_KEY_PART);//Header+length
  strcpy((char *)KeyMasterKey, MASTER_KEY);//clear component
  strcpy((char *)AftterMasterKey, AFTER_MASTER_KEY_PART);//PAD+MAc

  memset(InBuff,0,sizeof(InBuff));

  sprintf (InBuff, "020%s%s%s",BeforeMasterKey,KeyMasterKey,AftterMasterKey);
  iRet = ipp_communicate ( iIPPHandle, 0x0F, (unsigned char *)InBuff, (unsigned char *)OutBuff, TRUE);
	if(LOG_STATUS == LOG_ENABLE)
  {
    LOG_PRINTF(("Transfer Master Key Check MSSequence Engine return %d", iRet)); 
  }
  
	//--------------19-4-2016---------------
	if(iRet != PP_COMM_OK)
	{
      if(LOG_STATUS == LOG_ENABLE)
      {
			  LOG_PRINTF (("ipp_communicate fail"));
      }
			return _FAIL;
	}
  
  packData((char*)KeyMasterKey,(char*)packMasterKey);//packing master key
  packData((char*)EncSessionKey,(char*)Key);//packing encrypted logon session key
  iRetVal = TDES_Decrypt_Logon_key(Key,OutKey,packMasterKey);////decrypting encrypted session key 
	//--------------19-4-2016---------------
	if(iRetVal != _SUCCESS)
	{
      if(LOG_STATUS == LOG_ENABLE)
      {
			  LOG_PRINTF (("TDES_Decrypt_Logon_key fail"));
      }
			return _FAIL;
	}
	//--------------19-4-2016---------------
  
  if(LOG_STATUS == LOG_ENABLE)
  {
    bcd2a((char*)temp_Buff,(char*)OutKey,sizeof(OutKey));
    LOG_PRINTF (("-----Decrypted sestrhhjysion  =%s\n",temp_Buff));
  }

  strncpy((char*)Decr_Key1,(char*)temp_Buff,PACK_KEY_SIZE); //copying first 16 bytes to key1
  strncpy((char*)Decr_Key2,(char*)temp_Buff+PACK_KEY_SIZE,PACK_KEY_SIZE); //copying first 16 bytes to key1
 
  memset(InBuff,0,sizeof(InBuff));
  if(LOG_STATUS == LOG_ENABLE)
  {
    LOG_PRINTF(("Key Injection :transMsg->PrimaryAccNum = %s",PrimaryAccNO));
  }
  MakingGiske(Decr_Key1,Decr_Key2,KeySessionKeyEncripted,packMasterKey);
  if(LOG_STATUS == LOG_ENABLE)
  {
    LOG_PRINTF(("KeySessionKeyEncripted %s", KeySessionKeyEncripted)); 
  }
 
  window(1,1,30,20);
	clrscr();
	write_at(Dmsg[ENTER_PIN].dispMsg, strlen(Dmsg[ENTER_PIN].dispMsg),Dmsg[ENTER_PIN].x_cor, Dmsg[ENTER_PIN].y_cor);
	window(Dmsg[ENTER_PIN].x_cor,Dmsg[ENTER_PIN].y_cor+2,18,Dmsg[ENTER_PIN].y_cor+2);
  
  
  //strncpy(track2data,TRACK2_DATA,strlen(TRACK2_DATA));        //35 Sample Track 2 Data provided by Client 
    
   
  //ptr = strtok((char*)track2data, delim);//extracting account no from track2 data
  //strncpy(transMsg->PrimaryAccNum,ptr,strlen(transMsg->PrimaryAccNum));  
  /////////----Sending Packet Z63: Accept and Encrypt PIN..
  sprintf (InBuff, "Z63.%s%c%s%s%s%c%c",PrimaryAccNO,0x1C, KeySessionKeyEncripted,MIN_PIN_LEN,MAX_PIN_LEN,NULL_PIN_NO,ECHO_CHAR);
	
  if(LOG_STATUS == LOG_ENABLE)
  {
    LOG_PRINTF((" acc no =%s",PrimaryAccNO));
    LOG_PRINTF(("_________Z63 Pakcet=%s",InBuff));
  }
  memset(OutBuff, 0x00, sizeof(OutBuff));		
  iRet1 = ipp_communicate ( iIPPHandle, 0x02, (unsigned char*)InBuff, (unsigned char *)OutBuff, FALSE);
	window(1,1,30,20);
	clrscr();
	write_at(Dmsg[PROCESSING].dispMsg, strlen(Dmsg[PROCESSING].dispMsg),Dmsg[PROCESSING].x_cor, Dmsg[PROCESSING].y_cor);
	
  if(LOG_STATUS == LOG_ENABLE)
  {
    LOG_PRINTF(("Enter PIN return  = %d", iRet1));
   
  }
	
	memset(InBuff, 0, sizeof(InBuff));
	strcpy (InBuff, "72");
	iRet = ipp_communicate (iIPPHandle, 0x02, (unsigned char *)InBuff, (unsigned char *)OutBuff, FALSE);
	if(LOG_STATUS == LOG_ENABLE)
  {
    LOG_PRINTF(("Terminate Sessionnn return %d", iRet));
  }
	//--------------19-4-2016---------------
	if(iRet != PP_COMM_OK)
	{
    if(LOG_STATUS == LOG_ENABLE)
    {
			LOG_PRINTF (("ipp_communicate fail"));
    }
	  return _FAIL;
	}
	//-------------- END ---------------
	// Close the device
	vdPIN_Close (iIPPHandle);

  if(iRet1 == -3)
  {
    window(1,1,30,20);
    clrscr();
		error_tone();
		write_at (TIME_OUT, strlen(TIME_OUT), (30-strlen(TIME_OUT))/2, 10);
		
    error_tone();
    error_tone();
		SVC_WAIT(3000);
	
  }
  return iRet1;
}


/****************************************************************************************************************
*	Function Name : open_ipp_port																				*
*	Purpose		    : Open inetrnakl pinpad port 	and testing with packet 11	 								*
*	Input		      : void											                                        *
*	Output		    : Return value for the success or failure													*
*****************************************************************************************************************/

static int open_ipp_port (void)
{
	int iRet;
	int iHdl;
  if(LOG_STATUS == LOG_ENABLE)
  {
	  LOG_PRINTF(("<open_ipp_port>"));
  }

	iRet = iPIN_Open (PINPAD_INTERNAL, Rt_1200, Fmt_A7E1, &iHdl);
	if (iRet != PP_COMM_OK) 
  {
    if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF(("iPIN_Open 1200 iRet=%d", iRet));
    }
  }	
	iRet = iPIN_Send (iHdl, SI, (unsigned char *)"11", 2, 4);
  if(LOG_STATUS == LOG_ENABLE)
  {
	  LOG_PRINTF(("iPIN_Send 1200 iRet=%d", iRet));
  }
	
	if (iRet != PP_COMM_OK)
	{
    if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF(("iPIN_Send 1200 iRet=%d", iRet));
    }
		vdPIN_Close (iHdl);
		iHdl = -1;

		iRet = iPIN_Open (PINPAD_INTERNAL, Rt_19200, Fmt_A8N1, &iHdl);
		if (iRet != PP_COMM_OK) 
		{
      if(LOG_STATUS == LOG_ENABLE)
      {
			  LOG_PRINTF(("iPIN_Open 19200 iRet=%d", iRet));
      }
			return -1;
		}

		iRet = iPIN_Send (iHdl, STX, (unsigned char *)"11", 2, 4);
		if (iRet != PP_COMM_OK)
		{
			if(LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("iPIN_Send 19200 iRet=%d", iRet));
			}
		vdPIN_Close (iHdl);
		return -1;
		}
	}
  if(LOG_STATUS == LOG_ENABLE)
  {
	  LOG_PRINTF(("<open_ipp_port> IPP Handle=%d", iHdl));
  }
	
	return iHdl;
}

/****************************************************************************************************************
*	Function Name : iPIN_Open																					*
*	Purpose		    : Open inetrnakl pinpad port 			 												    *
*	Input		      : int device falg,int baud rate,format and handle										    *
*	Output		    : Return value for the success or failure													*
*****************************************************************************************************************/

int iPIN_Open (int iDevice, int iBaud, int iFormat, int *piHandle)
{
	int iRet;
	open_block_t rs232_Opn_Blk;

	memset (&rs232_Opn_Blk, 0, sizeof (rs232_Opn_Blk));

	rs232_Opn_Blk.protocol  = P_char_mode;
	// rs232_Opn_Blk.parameter = 0;
	rs232_Opn_Blk.rate      = iBaud;
	rs232_Opn_Blk.format    = iFormat;

	if (iDevice == COM1_DEVICE)
	{
		*piHandle = open (DEV_COM1, 0);
		
		if (*piHandle < 0)
			return PP_COMM_FAILURE;
	}
	else
	{
		if (iDevice == PINPAD_INTERNAL)
		{
			*piHandle = open (DEV_COM5, 0);
			
			if (*piHandle < 0)
				return PP_COMM_FAILURE;
      if(LOG_STATUS == LOG_ENABLE)
      {
			  LOG_PRINTF(("PINPAD_INTERNAL DEV_COM5 %d", *piHandle));
      }
		}

		else
		{
			*piHandle = open (DEV_COM2, 0);

			if (*piHandle < 0)
				return PP_COMM_FAILURE;
      if(LOG_STATUS == LOG_ENABLE)
      {
			  LOG_PRINTF(("PINPAD_INTERNAL DEV_COM2 %d", *piHandle));
      }
		}
	}

	iRet = set_opn_blk (*piHandle, &rs232_Opn_Blk);
	if (iRet < 0)
	{
		close (*piHandle);
		return PP_COMM_FAILURE;
	}
	
	if (iDevice == PINPAD_INTERNAL)
		select_pinpad (iDevice);

	return PP_COMM_OK;
}


/****************************************************************************************************************
*	Function Name : vdPIN_Close													   	                            *
*	Purpose		    : close inetrnakl pinpad port 			 												    *
*	Input		      : integer handle										                                    *
*	Output		    : void																					    *
*****************************************************************************************************************/
void vdPIN_Close (int iHandle)
{
	if (iHandle >= 0) 
		close (iHandle);
}
/****************************************************************************************************************
*	Function Name : iPIN_Send																					*
*	Purpose		    : send packet to the ipp			 												        *
*	Input		      : integer handle	,char pointer to the pakcet ,size of packet and timeout 				*
*	Output		    : int																					    *
*****************************************************************************************************************/
int iPIN_Send (int iHandle, unsigned char ucPcktStart, unsigned char *pucMessage, int iSize, int iTimeout)
{
	int i, inRet, inTry, iLen;
	int len;

	unsigned char  gvbyRecPcktStart = {0};
	unsigned char cBegPkt = STX, cEndPkt = ETX;
	unsigned char vucBuffer[IPP_BUFFER_SIZE]    = {0};
	unsigned char vuSendBuffer[IPP_BUFFER_SIZE] = {0};
	unsigned char  vbyCalcCRC = 0;
	
	if (ucPcktStart == STX || ucPcktStart == SI) 
		gvbyRecPcktStart = ucPcktStart;

	if (gvbyRecPcktStart == SI) 
	{
		cBegPkt = SI;
		cEndPkt = SO;
	}

	// Make the Packet <Start><Data><End>
	vuSendBuffer[0] = cBegPkt;
	memcpy(&vuSendBuffer[1], pucMessage, iSize);
	vuSendBuffer[1 + iSize] = cEndPkt;
	
	// Calculate LRC
	len = iSize + 2;			// To include sentinel: is 2 because we are skipping the first one and including the last one.
	
	for (i = 1; i < len; i++)	// Skip Start centinel but include End sentinel
        vbyCalcCRC ^= vuSendBuffer[i];

	// Add LRC
    vuSendBuffer[iSize + 2] = vbyCalcCRC;
	iSize += 3; 
  
	SerialClear (iHandle);
	for (inTry = 0; inTry < 3; inTry++)
	{
       	if (write (iHandle, (char *)vuSendBuffer, iSize) != iSize)
			return PP_COMM_FAILURE;

		iLen = 1;
		inRet = iPIN_Receive (iHandle, vucBuffer, &iLen, iTimeout);
      
		if (inRet == PP_COMM_OK)
		{
			if (vucBuffer[0] == ACK) return PP_COMM_OK;
			if (vucBuffer[0] == NAK) continue;
		}
		break;
	}
	return PP_COMM_FAILURE;   
}
/****************************************************************************************************************
*	Function Name : iPIN_Receive																													                       *
*	Purpose		    : Receive packet to the ipp			 												                                       *
*	Input		      : integer handle	,char pointer to the pakcet ,size of packet and timeout 										 *
*	Output		    : int																					                                                 *
*****************************************************************************************************************/
int iPIN_Receive (int iHandle, unsigned char *pucMessage, int *piLen, int iTimeout)
{
	int  iCount, iRequestedLen, iExpectLen;
	long lTimer, lInterTimer;

	iRequestedLen = *piLen;

	// To manage timeouts, ACT SVC_TICKS function is used.
	// This is just a simple tick read/comparison. 
	// We wait up to 1 second for the input to start
	// NOTE: another approach is to use the OS set_timer and wait for events.
	SVC_TICKS (COPY_TICKS, &lTimer);
	lTimer += (long)((long)iTimeout * 1000L);

	lInterTimer = 0L;

	*piLen = 0;

	for (;;)
	{
		iExpectLen = iRequestedLen - *piLen;
		if (iExpectLen <= 0)
			break;

		iCount = read (iHandle, (char *)&pucMessage[*piLen], iExpectLen);
		if (iCount < 0)
			return PP_COMM_FAILURE;

		if (iCount > 0)
			lInterTimer = 0L;

		*piLen += iCount;

		/* Exit if reception has began (*piLen > 0) and then stopped (iCount == 0) */
		if (*piLen > 0 && iCount == 0)
		{
			if (lInterTimer == 0L)
			{
				// since we've started to receive the input, we now go to a
				// smaller timeout
				SVC_TICKS (COPY_TICKS, &lInterTimer);
				lInterTimer += 300L;
			}
			else
			{
				if (SVC_TICKS (CHECK_TICKS, &lInterTimer) == 0)
					break;
			}
		}
		SVC_WAIT (50);

		if (SVC_TICKS (CHECK_TICKS, &lTimer) == 0)
			break;
	}
	if(LOG_STATUS == LOG_ENABLE)
	{
		LOG_HEX_PRINTF("iPINReceive", pucMessage, *piLen);
	}

	return PP_COMM_OK;
}


/****************************************************************************************************************
*	Function Name : SerialClear																													              *
*	Purpose		    : Receive packet to the ipp			 												              *
*	Input		      : integer handle										                              *
*	Output		    : void																					              *
*****************************************************************************************************************/
static void SerialClear (int iHandle)
{
	char c;
	Bool first = TRUE;
	for (;;) 
	{
		if (read (iHandle, &c, 1) <= 0) 
			break;
		
		if (first)
		{
      if(LOG_STATUS == LOG_ENABLE)
      {
			  LOG_PRINTF(("Serial Clear"));
      }
			first = FALSE;
		}
    if(LOG_STATUS == LOG_ENABLE)
    {
		  LOG_PRINTF(("0x%x", c));
    }
	}
	return;
}

/****************************************************************************************************************
*	Function Name : ipp_communicate																													              *
*	Purpose		    : send and recieve packet to the ipp			 												                              *
*	Input		    : integer handle,input pakcet and output pakcet  									                              *
*	Output		    : int																					                                                  *
*****************************************************************************************************************/
int ipp_communicate(int iHdl, unsigned char byPcktStart, unsigned char *pucMsgIn, unsigned char *pucMsgOut, Bool bEOT)
{
	int iRet = 0;
	int iSize = -99;

	unsigned char vbyAux[100];

	if (pucMsgOut != NULL) pucMsgOut[0] = 0x00;


	iRet = iPIN_Send(iHdl, byPcktStart, pucMsgIn, strlen((char *)pucMsgIn), 4);

	if ((iRet == PP_COMM_OK) && (strncmp((const char *)pucMsgIn, "72", 2) != 0))
	{
		if (pucMsgOut != NULL)
		{
			iSize = 60;
			iRet = iPIN_ReceivePacket(iHdl, pucMsgOut, &iSize, 20);
			if (LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("<ipp_communicate> pucMsgOut(%d)=%s", strlen((char *)pucMsgOut), pucMsgOut));
			}

			if ((strncmp((const char *)pucMsgIn, "Z63", 3) == 0) && (strlen((char *)pucMsgOut) == 0))
				return BTN_CANCEL;

			if ((iRet == PP_COMM_OK) && (strncmp((const char *)pucMsgOut, "71", 2) == 0))
			{
				if (LOG_STATUS == LOG_ENABLE)
				{
					LOG_PRINTF(("<ipp_communicate> 71 packet pucMsgOut(%d)=%s", strlen((char *)pucMsgOut), pucMsgOut));
				}
				parseEncryptedPinBlock(pucMsgOut);
			}
			else
			{
				if (LOG_STATUS == LOG_ENABLE)
				{
					LOG_PRINTF(("<ipp_communicate> iPIN_ReceivePacket iRet=%d", iRet));
				}
			}
		}
	}

	else
	{
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("<ipp_communicate>  PP_COMM_NOT OK iPIN_Send iRet=%d", iRet));
		}
	}

	if (iRet == PP_COMM_OK)
	{

		if (bEOT)
		{
			iSize = 1;
			iRet = iPIN_ReceivePacket(iHdl, vbyAux, &iSize, 20);
			if (LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("<ipp_communicate> iPIN_ReceivePacket = %s", vbyAux));
			}

			if (iRet == PP_COMM_CANCELLED) /* CANCELLED mean EOT */
				iRet = PP_COMM_OK;

			else
			{
				if (LOG_STATUS == LOG_ENABLE)
				{
					LOG_PRINTF(("<ipp_communicate> iPIN_ReceivePacket EOT iRet=%d", iRet));
				}
				iRet = PP_COMM_FAILURE;
			}
		}
	}
	else
	{
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("PP_COMM_NOT_OK"));
		}
	}
	return iRet;
}

/*int ipp_communicate ( int iHdl, unsigned char byPcktStart, unsigned char *pucMsgIn, unsigned char *pucMsgOut, Bool bEOT)
{
	int iRet  =  0;
	int iSize = -99;
	
	unsigned char vbyAux[100];

	if (pucMsgOut != NULL) pucMsgOut[0] = 0x00;


	iRet = iPIN_Send(iHdl, byPcktStart, pucMsgIn, strlen((char *)pucMsgIn), 4);

	if ((iRet == PP_COMM_OK) && (strncmp((const char *)pucMsgIn, "72", 2) != 0))
	{    
		if (pucMsgOut != NULL)
		{
			iSize = 60;
			iRet = iPIN_ReceivePacket (iHdl, pucMsgOut, &iSize, 20);
      if(LOG_STATUS == LOG_ENABLE)
      {
			  LOG_PRINTF(("<ipp_communicate> pucMsgOut(%d)=%s", strlen ((char *)pucMsgOut), pucMsgOut));
      }

			if((strncmp((const char *)pucMsgIn, "Z63", 3) == 0) && (strlen((char *)pucMsgOut) == 0) )
				return BTN_CANCEL;

			if((iRet == PP_COMM_OK) && (strncmp((const char *)pucMsgOut,"73",2) == 0))
			{
				if(LOG_STATUS == LOG_ENABLE)
				{
					LOG_PRINTF(("<ipp_communicate> 73 packet pucMsgOut(%d)=%s", strlen ((char *)pucMsgOut), pucMsgOut));				
				}
				parseEncryptedPinBlock(pucMsgOut);
			}
			else
			{
				if(LOG_STATUS == LOG_ENABLE)
				{
					LOG_PRINTF(("<ipp_communicate> iPIN_ReceivePacket iRet=%d", iRet));
				}
			}
		}
	}

	else
	{
		if (LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("<ipp_communicate>  PP_COMM_NOT OK iPIN_Send iRet=%d", iRet));
		}
	}
	if (strncmp((const char *)pucMsgOut, "91", 2 != 0))
	{
		if (iRet == PP_COMM_OK)
		{

			if (bEOT)
			{
				iSize = 1;
				iRet = iPIN_ReceivePacket(iHdl, vbyAux, &iSize, 20);
				if (LOG_STATUS == LOG_ENABLE)
				{
					LOG_PRINTF(("<ipp_communicate> iPIN_ReceivePacket = %s", vbyAux));
				}

				if (iRet == PP_COMM_CANCELLED) // CANCELLED mean EOT 
				{
					iRet = PP_COMM_OK;
				}
				else
				{
					if (LOG_STATUS == LOG_ENABLE)
					{
						LOG_PRINTF(("<ipp_communicate> iPIN_ReceivePacket EOT iRet=%d", iRet));
					}
					iRet = PP_COMM_FAILURE;
				}
			}
		}
		else
		{
			if (LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("PP_COMM_NOT_OK"));
			}
		}
	}
 
	return iRet ;
}*/

/****************************************************************************************************************
*	Function Name : iPIN_ReceivePacket																										      		              *
*	Purpose		    : Receive packet to the ipp			 												                                         *
*	Input		      : integer handle,char pointer to the pacKet ,size of packet and timeout 										                                                             *
*	Output		    : int																					                                                 *
*****************************************************************************************************************/
int iPIN_ReceivePacket (int iHandle, unsigned char *pucMessage, int *piLen, int iTimeout)
{
	int iRet, iTries;

	for (iTries = 0; iTries < 3; iTries++)
	{
		iRet = iPIN_Receive(iHandle, pucMessage, piLen, iTimeout);

		if (iRet != PP_COMM_OK)
		{
			return iRet;
		}
		if (*piLen == 1 && pucMessage[0] == EOT)
		{
			return PP_COMM_CANCELLED;
		}
		if (*piLen > 0)
		{
			// Verifies format and LRC/CRC
			iRet = iPIN_VerifyPacket (iHandle, pucMessage, piLen);
			LOG_PRINTF(("iPIN_Receive response %d", iRet));

			if (iRet == PP_COMM_OK || iRet == PP_COMM_OK_NO_ACK) 
			{
				if (*piLen > 0 && iRet == PP_COMM_OK) iPIN_SendByte (iHandle, ACK);
				break;
			}

			else 
				iPIN_SendByte (iHandle, NAK);
		}

		else
			break;
	}

	if (iTries > 2)
		iPIN_SendByte (iHandle, EOT);

	return PP_COMM_OK;
}

/****************************************************************************************************************
*	Function Name : iPIN_VerifyPacket																													                  *
*	Purpose		    : verify the recieve packet to the ipp			 												                            *
*	Input		      : integer handle	,char pointer to the pakcet ,size of packet									                                                            *
*	Output		    : int																					                                                *
*****************************************************************************************************************/
static int iPIN_VerifyPacket (int iHandle, unsigned char *pucMessage, int *piLen)
{
	int inLoop, inGarbage;
	byte vbyMsgCRC[2], vbyCalcCRC[2];
	byte gvbyRecPcktStart = {0};
   
	// Ignore undesired protocol characteres
	inGarbage = 0;
	while (pucMessage[inGarbage] != STX && pucMessage[inGarbage] != SI && inGarbage < *piLen) 
		inGarbage++;

	if (inGarbage >= *piLen)
	{
		*piLen = 0;
		return PP_COMM_OK;
	}

	gvbyRecPcktStart = pucMessage[inGarbage];
	
	vbyCalcCRC[0] = 0x00;

	for (inLoop = 1; inLoop < *piLen - inGarbage; inLoop++) 
	{
		vbyCalcCRC[0] ^= pucMessage[inLoop + inGarbage];
			
		if (gvbyRecPcktStart == STX && pucMessage[inLoop + inGarbage] == ETX) break;
		if (gvbyRecPcktStart == SI  && pucMessage[inLoop + inGarbage] == SO ) break;
	
		/* strips Protocol characters from message */
		pucMessage[inLoop - 1] = pucMessage[inLoop + inGarbage];
	}

	/* makes the clean message NULL terminated */
	pucMessage[inLoop - 1] = 0;

	vbyMsgCRC[0] = pucMessage[++inLoop + inGarbage];

    return ( (vbyMsgCRC[0] == vbyCalcCRC[0]) ? PP_COMM_OK : PP_COMM_FAILURE);
}
/****************************************************************************************************************
*	Function Name : iPIN_SendByte																													                  *
*	Purpose		    : sends the packet to the ipp			 												                            *
*	Input		      : integer handle										                                                            *
*	Output		    : int																					                                                *
*****************************************************************************************************************/
int iPIN_SendByte (int iHandle, unsigned char ucByte)
{
	if (write (iHandle, (char *)&ucByte, 1) != 1)
		return PP_COMM_FAILURE;
	if(LOG_STATUS == LOG_ENABLE)
  {
	  LOG_PRINTF(("iPIN_SendByte 0x%x", ucByte));
  }

	return PP_COMM_OK;
}

/****************************************************************************************************************
*	Function Name : TDES_CBC16																											        		                  *
*	Purpose		    : encrypts the 16 byte data using 3 des CBC encryption          		                            *
*	Input		      : unsigned char pointer to the input buffer,length of input data,output buffer,encription key 
                  and intialization vector							                                                        *
*	Output		    : short																					                                                  *
*****************************************************************************************************************/
short TDES_CBC16(unsigned char *Input_Data,int iInputlen,unsigned char *Output_Data,int *iOutLen,unsigned char *Iv,unsigned char *Key,int enc)
{
	 const_DES_cblock TDES_Key1;                          /* KEY1 for Triple DES */
     const_DES_cblock TDES_Key2;                          /* KEY2 for Triple DES */
	 unsigned char outtempBuffer[500];
	 unsigned char LastEncBlock[DATA_BLOCK_LEN+1];
	 unsigned char LastEncOutBlock[DATA_BLOCK_LEN+1];
	 
	 

     DES_key_schedule SchKey1,SchKey2;
	 int len=0;
	 int lenrem=0;
     
	 memset(outtempBuffer,0x00,sizeof(outtempBuffer));
	 memset(LastEncOutBlock,0x00,sizeof(LastEncOutBlock));
	 
	

     /** Copy in 8 byte chunks from base64DecodeOutput1 array to the array TDES_Key1, TDES_Key2,TDES_Key3  which are using as a key for TDES in ecb mode **/
     memcpy(TDES_Key1, Key+KEY_1, sizeof(TDES_Key1)); 
     memcpy(TDES_Key2, Key+KEY_2, sizeof(TDES_Key2)); 
	 
	 
    
     /* Clear the DES_key_schedule key*/
     memset((DES_key_schedule*)&SchKey1, 0, sizeof(SchKey1));
     memset((DES_key_schedule*)&SchKey2, 0, sizeof(SchKey2));
  
     /* DES_set_key() works like DES_set_key_checked() if the DES_check_key flag is non-zero, otherwise like DES_set_key_unchecked(). */
     DES_set_key((const_DES_cblock *)&TDES_Key1, &SchKey1);
     DES_set_key((const_DES_cblock *)&TDES_Key2, &SchKey2);
	 lenrem = iInputlen % 8;
	 
		 if(lenrem == 0)   // LENGTH is MULTIPLE of 8 BYTE
		 {
			DES_ede2_cbc_encrypt(Input_Data,outtempBuffer,iInputlen,&SchKey1,&SchKey2,(DES_cblock *)Iv,enc);
			len = iInputlen;
		 }
		 else // LENGTH is not MULTIPLE of 8 BYTE
		 {
			 len = (iInputlen/8) * 8;

			
			 DES_ede2_cbc_encrypt(Input_Data,outtempBuffer,len,&SchKey1,&SchKey2,(DES_cblock *)Iv,enc);

			 memset(LastEncBlock,0xFF,DATA_BLOCK_LEN);
			 memcpy((char*)LastEncBlock,(char*)&Input_Data[len],lenrem);
			
			 
			 DES_ede2_cbc_encrypt(LastEncBlock,LastEncOutBlock,8,&SchKey1,&SchKey2,(DES_cblock *)Iv,enc);
			

			 memcpy((char*)&outtempBuffer[len],(char*)LastEncOutBlock,DATA_BLOCK_LEN);
			 len +=DATA_BLOCK_LEN;
		 }
		*iOutLen = len;
		memcpy(Output_Data,outtempBuffer,len); // COPY FINAL Encrypted Data to OUTPUT Buffer

		
     return _SUCCESS;

}
/****************************************************************************************************************
*	Function Name : TDES_CBC16																											        		                  *
*	Purpose		    : encrypts the 8 byte data using 3 des CBC encryption          		                            *
*	Input		      : unsigned char pointer to the input buffer,length of input data,output buffer ,encription key 
                  and intialization vector							                                                        *
*	Output		    : short																					                                                  *
*****************************************************************************************************************/
short TDES_CBC(unsigned char *Input_Data,unsigned char *Output_Data,unsigned char *Iv,unsigned char *Key) 
{   
     unsigned char Out_Data[DATA_BLOCK_LEN]= {0};  //to store the output block
    
     const_DES_cblock TDES_Key1;                          /* KEY1 for Triple DES */
     const_DES_cblock TDES_Key2;                          /* KEY2 for Triple DES */
     const_DES_cblock TDES_Key3;                          /* KEY3 for Triple DES */

     DES_key_schedule SchKey1,SchKey2,SchKey3;
     
     /** Copy in 8 byte chunks from base64DecodeOutput1 array to the array TDES_Key1, TDES_Key2,TDES_Key3  which are using as a key for TDES in ecb mode **/
     memcpy(TDES_Key1, Key+KEY_1, sizeof(TDES_Key1)); 
     memcpy(TDES_Key2, Key+KEY_2, sizeof(TDES_Key2)); 
     memcpy(TDES_Key3, Key+KEY_1, sizeof(TDES_Key3));

     /* Clear the DES_key_schedule key*/
     memset((DES_key_schedule*)&SchKey1, 0, sizeof(SchKey1));
     memset((DES_key_schedule*)&SchKey2, 0, sizeof(SchKey2));
     memset((DES_key_schedule*)&SchKey3, 0, sizeof(SchKey3));
    
     /* DES_set_key() works like DES_set_key_checked() if the DES_check_key flag is non-zero, otherwise like DES_set_key_unchecked(). */
     DES_set_key((const_DES_cblock *)&TDES_Key1, &SchKey1);
     DES_set_key((const_DES_cblock *)&TDES_Key2, &SchKey2);
     DES_set_key((const_DES_cblock *)&TDES_Key3, &SchKey3);
    
     DES_ncbc_encrypt(Input_Data,Output_Data,8,&SchKey1,(DES_cblock *)Iv,DES_ENCRYPT);//encryption with kety part1
     DES_ncbc_encrypt(Output_Data,Out_Data,8,&SchKey2,(DES_cblock *)Iv,DES_DECRYPT);//decryption with key part2
     memset(Output_Data,0,sizeof(Output_Data));
     DES_ncbc_encrypt(Out_Data,Output_Data,8,&SchKey3,(DES_cblock *)Iv,DES_ENCRYPT);//encryption with key part3
    
     return _SUCCESS;
}
/****************************************************************************************************************
*	Function Name : MakeXOR																											        		                        *
*	Purpose		    : calculate xor value for 8 byte                            		                                  *
*	Input		      : unsigned char pointer to the input buffer,output buffer , flag                                  *
*	Output		    : short																					                                                  *
*****************************************************************************************************************/
//making XOR for encryption and mac 
static void MakeXOR(unsigned char *str1, unsigned char *out, int len,short Encflag)
{
    int i =0;
    char Xor_Value1 = 0x45  ;
    char Xor_Value2 = 0x4d  ;
    unsigned char temp[PACK_KEY_SIZE+1]={0}; //temp buffer to print the output
		 
    char ch =0;
     if(Encflag == 1)
     {
      ch = Xor_Value1;
     }
     else if(Encflag == 2)
     {
       ch = Xor_Value2;
     }
     for(i=0; i<len; ++i)
        out[i] = (char)(ch ^ str1[i]);
    if(LOG_STATUS == LOG_ENABLE)
    {
      bcd2a((char*)temp,(char*)out,len);
      LOG_PRINTF(("Key after xor =%s",temp));
    }
} 

/****************************************************************************************************************
*	Function Name : MakingGiske																											        		                    *
*	Purpose		    : make the GISKE block of 120 bytes using the encryption keys ,session key and master key         *
*	Input		      : unsigned char pointer to the key1 ,key2,session key and pack master key                         *
*	Output		    : short																					                                                  *
*****************************************************************************************************************/
short MakingGiske(unsigned char *Key1, unsigned char *Key2,unsigned char *SessionKey,unsigned char *packMasterKey) 
{   
     unsigned char iVecArr[INIT_VECTOR_LEN]={'2','0','1','2','0','P','0','T'}; //intial vector for Encryptiion (first 8 btyes of GISKe header)
     unsigned char Mac_iVecArr[INIT_VECTOR_LEN]={'2','0','1','2','0','P','0','T'};//intial vector for MAC genration (first 8 btyes of GISKe header)
     
     unsigned char HexOutput[DATA_BLOCK_LEN]={0};//output for CBC function 
     unsigned char MacHexOutput[MAC_BLOCK_LEN]={0};//Output for MAC genration 
     unsigned char TempBuff[PACK_KEY_SIZE+1]={0}; //temp buffer to print the output
		 
     unsigned char EncKey1[PACK_KEY_SIZE+1]={0};//encryption key used for encryption
     unsigned char EncKey2[PACK_KEY_SIZE+1]={0};//encryption key used for MAC
     int offset =0;
		 
     MakeXOR(packMasterKey,EncKey1,PACK_KEY_SIZE,1);//Getting key1 used in encrypting the key 
     MakeXOR(packMasterKey,EncKey2,PACK_KEY_SIZE,2);//Getting key2 used in Mac generation
           
     strncpy((char*)SessionKey,(char*)iVecArr,INIT_VECTOR_LEN);//Copying first 8 btye of intial vector 
     offset = INIT_VECTOR_LEN;


     //Encrypting first unencrypted 16 bytes 
     EncryptingKeyBlocks((unsigned char*)UNENC_AFTER_HEADER,HexOutput,MacHexOutput,EncKey1,EncKey2,iVecArr,Mac_iVecArr,TempBuff,DATA_BLOCK_LEN);
     strncpy((char*)SessionKey+offset,(char*)TempBuff,PACK_KEY_SIZE);
     offset = offset + PACK_KEY_SIZE;
     memset(TempBuff,0,sizeof(TempBuff));
     
     //Encrypting reserved unencrypted 16 bytes 
     EncryptingKeyBlocks((unsigned char*)UNENC_RESERVED,HexOutput,MacHexOutput,EncKey1,EncKey2,iVecArr,Mac_iVecArr,TempBuff,DATA_BLOCK_LEN);
     strncpy((char*)SessionKey+offset,(char*)TempBuff,PACK_KEY_SIZE);
     offset = offset + PACK_KEY_SIZE;
     memset(TempBuff,0,sizeof(TempBuff));
     
     
     //Encrypting unencrypted 16 bytes which contains the length of key
     EncryptingKeyBlocks((unsigned char*)UNENC_LENGTH_PART,HexOutput,MacHexOutput,EncKey1,EncKey2,iVecArr,Mac_iVecArr,TempBuff,DATA_BLOCK_LEN);
     strncpy((char*)SessionKey+offset,(char*)TempBuff,PACK_KEY_SIZE);
     offset = offset + PACK_KEY_SIZE;
     memset(TempBuff,0,sizeof(TempBuff));
     
     //Encrypting unencrypted unpacked 16 bytes of key 1
     EncryptingKeyBlocks(Key1,HexOutput,MacHexOutput,EncKey1,EncKey2,iVecArr,Mac_iVecArr,TempBuff,DATA_BLOCK_LEN);
     strncpy((char*)SessionKey+offset,(char*)TempBuff,PACK_KEY_SIZE);
     offset = offset + PACK_KEY_SIZE;
     memset(TempBuff,0,sizeof(TempBuff));
    
     //Encrypting unencrypted unpacked 16 bytes of key 2
     EncryptingKeyBlocks(Key2,HexOutput,MacHexOutput,EncKey1,EncKey2,iVecArr,Mac_iVecArr,TempBuff,DATA_BLOCK_LEN);
     strncpy((char*)SessionKey+offset,(char*)TempBuff,PACK_KEY_SIZE);
     offset = offset + PACK_KEY_SIZE;
     memset(TempBuff,0,sizeof(TempBuff));     
     
     //Encrypting padded unpacked 16 bytes of key 3
     EncryptingKeyBlocks((unsigned char*)UNENC_PADDEDKEY3_PART,HexOutput,MacHexOutput,EncKey1,EncKey2,iVecArr,Mac_iVecArr,TempBuff,DATA_BLOCK_LEN);
     strncpy((char*)SessionKey+offset,(char*)TempBuff,PACK_KEY_SIZE);
     offset = offset + PACK_KEY_SIZE;
     memset(TempBuff,0,sizeof(TempBuff));


     bcd2a((char*)TempBuff,(char*)MacHexOutput,sizeof(MacHexOutput));
     if(LOG_STATUS == LOG_ENABLE)
     {
      LOG_PRINTF(("TempBuff =%s",TempBuff));
     }
     strncpy((char*)SessionKey+offset,(char*)TempBuff,PACK_KEY_SIZE);
     memset(TempBuff,0,sizeof(TempBuff));
     
     return _SUCCESS;
}
/****************************************************************************************************************
*	Function Name : TDES_CBC_MAC																											        		                  *
*	Purpose		    : encrypts the 8 byte data using 3 des CBC encryption and calculate the MAC                       *
*	Input		      : unsigned char pointer to the input buffer,output buffer,ininital vector and encryption key      *
*	Output		    : short																					                                                  *
*****************************************************************************************************************/
short TDES_CBC_MAC(unsigned char *Input_Data,unsigned char *Output_Data,unsigned char *Iv,unsigned char *Key) 
{   
     
     unsigned char Out_Data[DATA_BLOCK_LEN]= {0};  
    
     const_DES_cblock TDES_Key1;                          /* KEY1 for Triple DES */
     const_DES_cblock TDES_Key2;                          /* KEY2 for Triple DES */
     const_DES_cblock TDES_Key3;                          /* KEY3 for Triple DES */

     DES_key_schedule SchKey1,SchKey2,SchKey3;
    
     /** Copy in 8 byte chunks from base64DecodeOutput1 array to the array TDES_Key1, TDES_Key2,TDES_Key3  which are using as a key for TDES in ecb mode **/
     memcpy(TDES_Key1, Key+KEY_1, sizeof(TDES_Key1)); 
     memcpy(TDES_Key2, Key+KEY_2, sizeof(TDES_Key2)); 
     memcpy(TDES_Key3, Key+KEY_1, sizeof(TDES_Key3));

     /* Clear the DES_key_schedule key*/
     memset((DES_key_schedule*)&SchKey1, 0, sizeof(SchKey1));
     memset((DES_key_schedule*)&SchKey2, 0, sizeof(SchKey2));
     memset((DES_key_schedule*)&SchKey3, 0, sizeof(SchKey3));
    
     /* DES_set_key() works like DES_set_key_checked() if the DES_check_key flag is non-zero, otherwise like DES_set_key_unchecked(). */
     DES_set_key((const_DES_cblock *)&TDES_Key1, &SchKey1);
     DES_set_key((const_DES_cblock *)&TDES_Key2, &SchKey2);
     DES_set_key((const_DES_cblock *)&TDES_Key3, &SchKey3);
    
     DES_ncbc_encrypt(Input_Data,Output_Data,DATA_BLOCK_LEN,&SchKey1,(DES_cblock *)Iv,DES_ENCRYPT);
     DES_ncbc_encrypt(Output_Data,Out_Data,DATA_BLOCK_LEN,&SchKey2,(DES_cblock *)Iv,DES_DECRYPT);
     memset(Output_Data,0,sizeof(Output_Data));
     DES_ncbc_encrypt(Out_Data,Output_Data,DATA_BLOCK_LEN,&SchKey3,(DES_cblock *)Iv,DES_ENCRYPT); 
     
     return _SUCCESS;
}


/********************************************************************************************************************
*	Function Name : parseEncryptedPinBlock							   							                    *
*	Author		    : 																								*
*	Purpose		    : Parse encrypted pin block																        *
*	Input		      : a pointer to the Transaction structure and poniter to the response buffer		            *
*	Output		    : Return value for the success or failure														*
*	Date		      : 15-Jan-2016																					*
*********************************************************************************************************************/
void parseEncryptedPinBlock(unsigned char *pucMsgOut)
{
  int i = 0;
  unsigned char PinBlockHolder[PIN_BLOCK_LENGTH+1]={0};
  unsigned char *ptr = PinBlockHolder ;
   
  int ret=0 ;
  char testch[2]={0};
  
  /*
  //Extracting the last 16 hex digits pin block in a  buffer    
  strncpy((char*)PinBlockHolder,((char*)(pucMsgOut)+8),PIN_BLOCK_LENGTH);
  */
  //Parsing incoming Packet 73.
  if (pin_enc)
  {
	  //Extracting the last 16 hex digits pin block in a  buffer    
	  strncpy((char*)PinBlockHolder, ((char*)(pucMsgOut)+8), PIN_BLOCK_LENGTH);
  }
  else
  {
	  strncpy(KSN_pp, ((char*)(pucMsgOut)+8), KSN_LENGTH);
	  strncpy((char*)PinBlockHolder, ((char*)(pucMsgOut)+28), PIN_BLOCK_LENGTH);
  }

  memset(EncPinBlock,0,sizeof(EncPinBlock));
  if(LOG_STATUS == LOG_ENABLE)
  {
    LOG_PRINTF(("parseEncryptedPinBlock PinBlock Header (%d)=%s", strlen ((char *)PinBlockHolder), PinBlockHolder));	
	LOG_PRINTF(("parseEncryptedPinBlock KSN (%d)=%s", strlen((char *)KSN_pp), KSN_pp));
  }
  
  LOG_PRINTF(("Parse encripted block pin_change: %d", pin_change ));

  if (pin_change == 0)
  {	  
	  memset(EncPinBlock, 0, sizeof(EncPinBlock));
	  //copying 16 chars in 64 bit
	  for (i = 0; i < strlen((char*)PinBlockHolder); i += 2)
	  {
		  memset(testch, 0, sizeof(testch));
		  strncpy(testch, (char*)ptr, 1);
		  ret = (int)strtol(testch, NULL, 16);
		  ptr++;
		  EncPinBlock[i / 2] = ret;
		  EncPinBlock[i / 2] = EncPinBlock[i / 2] << 4;
		  memset(testch, 0, sizeof(testch));
		  strncpy(testch, (char*)ptr, 1);
		  ret = (int)strtol(testch, NULL, 16);

		  EncPinBlock[i / 2] |= ret;
		  ptr++;
	  }
	  LOG_PRINTF(("new pin : %s", EncPinBlock));
	  return;
  }

  if (pin_change == 2 || pin_change == 3)
  {
	  memset(EncOldPinBlock, 0, sizeof(EncOldPinBlock));
	  //copying 16 chars in 64 bit 
	  for (i = 0; i < strlen((char*)PinBlockHolder); i += 2)
	  {
		  memset(testch, 0, sizeof(testch));
		  strncpy(testch, (char*)ptr, 1);
		  ret = (int)strtol(testch, NULL, 16);
		  ptr++;
		  EncOldPinBlock[i / 2] = ret;
		  EncOldPinBlock[i / 2] = EncOldPinBlock[i / 2] << 4;
		  memset(testch, 0, sizeof(testch));
		  strncpy(testch, (char*)ptr, 1);
		  ret = (int)strtol(testch, NULL, 16);

		  EncOldPinBlock[i / 2] |= ret;
		  ptr++;
	  }
	  LOG_PRINTF(("old pin : %s", EncOldPinBlock));
	  return;
  }

  if (pin_change == 1)
  {
	  strncpy((char*)EncPinBlock, (char*)PinBlockHolder, PIN_BLOCK_LENGTH);
	  LOG_PRINTF(("new pin : %s", EncPinBlock));
	  pin_change = 0;
	  return;
  }  
}

/****************************************************************************************************************
*	Function Name : TDES_Decrypt_Logon_key																		*
*	Purpose		    : decrypts the logon key recieved during the logon transaction                              *
*	Input		      : unsigned char pointer to the input buffer,output buffer and encryption key              *
*	Output		    : short																					    *
*****************************************************************************************************************/
short TDES_Decrypt_Logon_key(unsigned char *Input_Data,unsigned char *Output_Data,unsigned char *Key) 
{              
     const_DES_cblock TDES_Key1;                          /* KEY1 for Triple DES */
     const_DES_cblock TDES_Key2;                          /* KEY2 for Triple DES */
    
     DES_key_schedule SchKey1,SchKey2;
     unsigned char outdata[DATA_BLOCK_LEN+1]={0};   // Add this declaration beginning of the function TDES_Decrypt_Logon_key
   
     /** Copy in 8 byte chunks from base64DecodeOutput1 array to the array TDES_Key1, TDES_Key2,TDES_Key3  which are using as a key for TDES in ecb mode **/
     memcpy(TDES_Key1, Key+KEY_1, sizeof(TDES_Key1)); 
     memcpy(TDES_Key2, Key+KEY_2, sizeof(TDES_Key2)); 
    
     /* Clear the DES_key_schedule key*/
     memset((DES_key_schedule*)&SchKey1, 0, sizeof(SchKey1));
     memset((DES_key_schedule*)&SchKey2, 0, sizeof(SchKey2));
  
     /* DES_set_key() works like DES_set_key_checked() if the DES_check_key flag is non-zero, otherwise like DES_set_key_unchecked(). */
     DES_set_key((const_DES_cblock *)&TDES_Key1, &SchKey1);
     DES_set_key((const_DES_cblock *)&TDES_Key2, &SchKey2);
  
     

     DES_ecb2_encrypt((const_DES_cblock *)Input_Data,(DES_cblock *)outdata,&SchKey1,&SchKey2,DES_DECRYPT);
     memcpy(Output_Data,outdata,DATA_BLOCK_LEN);

     memset(outdata,0x00,sizeof(outdata));

     DES_ecb2_encrypt((const_DES_cblock *)&Input_Data[8],(DES_cblock *)outdata,&SchKey1,&SchKey2,DES_DECRYPT);
     memcpy(&Output_Data[KEY_2],outdata,DATA_BLOCK_LEN);
     
    
     return _SUCCESS;
}
/****************************************************************************************************************
*	Function Name : EncryptingKeyBlocks																											        		            *
*	Purpose		    : a function to perform the TDES CBC and TDES CBC MAC                                             *
*	Input		      : unsigned char pointer to the input buffer,output buffer,ininital vector and encryption key                        *
*	Output		    : short																					                                                  *
*****************************************************************************************************************/
short EncryptingKeyBlocks(unsigned char *Input_Data,unsigned char *Output_Data,unsigned char *MAC_Output_Data,unsigned char *Key1,unsigned char *Key2,unsigned char *IVec,unsigned char *MAC_IVec,unsigned char *Temp,int size) 
{              
     unsigned char HexInput[DATA_BLOCK_LEN]={0}; //input for CBC function 
     
     memset(MAC_Output_Data,0,size);
     packData((char*)Input_Data,(char*)HexInput);
     TDES_CBC(HexInput,Output_Data,IVec,Key1);
     TDES_CBC_MAC(Output_Data,MAC_Output_Data,MAC_IVec,Key2);
     bcd2a((char*)Temp,(char*)Output_Data,size);
     if(LOG_STATUS == LOG_ENABLE)
     {
      LOG_PRINTF(("EncryptingKeyBlocks TempBuff =%s",Temp));
     }

     memset(IVec,0,size);
     memcpy(IVec,Output_Data,size);
     memset(Output_Data,0,size);
     memset(MAC_IVec,0,size);
     memcpy(MAC_IVec,MAC_Output_Data,size);     
     
     return _SUCCESS;
}
