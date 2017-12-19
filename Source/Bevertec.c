/******************************************************************
  Filename   :Bevertec.C
  Product    :Bevertec APPLICATION
  Version    :2.0
  Module     :Bevertec
  Description:	This is the sample implementation of VMAC enabled application
				using the ACT2000 library

  Modification History:
    #		Date       		 Who				Comments
	1.0		12/25/2015 8:07:48 PM	   vikask3		Initial Implementation
*******************************************************************/

/******************************************************************
   Copyright (C) 2008-2010 by VeriFone Inc. All rights reserved.

 No part of this software may be used, stored, compiled, reproduced,
 modified, transcribed, translated, transmitted, or transferred, in
 any form or by any means  whether electronic, mechanical,  magnetic,
 optical, or otherwise, without the express prior written permission
                          of VeriFone, Inc.
*******************************************************************/
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

#include <aclconio.h>
//Include file for EESL
#include "logsys.h"
#include "varrec.h"
#include "eeslapi.h"

//Include file for Device Manager
#include "devman.h"

//Application specific Include files
#include "..\include\Bevertec.h"
#include "..\include\Appevent.h"
#include "..\include\BVTC_EMV.h"
#include "..\include\TouchScreen.h"
#include "..\include\SaleTransaction.h"

//emv includes
#include <cardslot.h>
#include <EMVCWrappers.h>
#include <EMVCStructs.h>
#include <vxemvap.h>
#include <emvlib.h>
#include <estfuncs.h>
#include <mvtfuncs.h>
#include <mvt.h>
#include <est.h>

#include <VXEMVAP_confproto.h>
#include <proto.h>

#include "..\include\Merchant.h"
#include "..\include\Supervisor.h"
#include "..\include\Transactions.h"
#include "..\Include\BevertecClient.h"
#include "..\Include\Common.h"
#include "..\Include\Settlement.h"
#include "..\Include\ReciptPrint.h"
#include "..\Include\Logon.h"


//extern unsigned char SessionKey[33]; 
char szKeyMap[MAX_ALPNUM_KEYS][CHAR_PER_KEY]= {"0- +%", "1QZ.\\","2ABC&", "3DEF%", "4GHI*", "5JKL/", "6MNO~", "7PRS^", "8TUV[", "9WXY]","*,'\":", "#=:$?" };
//char szKeyMap[MAX_ALPNUM_KEYS][CHAR_PER_KEY_VX680]={"0- +%", "1QZqz.\\", "2ABCabc&", "3DEFdef%","4GHIghi*","5JKLjkl/", "6MNOmno~", "7PRSprs^", "8TUVtuv[", "9WXYwxy]", "*,'\":","#=:$?" };
char LOGFlag[2]={0};
// Global Variables for the Application
int CheckPaperFlag = 0 ;
short Sup_Login = 1;
short pin_change = 0;
extern unsigned char EncSessionKey[UNPACK_ENC_PR_KEY_LEN+1];//33 
char            LogicalName[EESL_APP_LOGICAL_NAME_SIZE];    // To store the logical name of this Appl
dispMsg *Dmsg = NULL;//pointer of display message structure
short						hClock = -1;
static short    hConsole = -1;      // Console	 handle
short						hPrinter = -1;      // Printer	 handle
short						hMagReader = -1;    // MagReader handle
static short    Magflag = -1;       // 0 if the MagReader was obtained by INIT_EVENT 1 if obtained by MAGCARD_EVENT
static int      SendFlag = 0;       // Set to 0 if request is already not sent to DevMan for MagCard Request
unsigned char   ucEventData[250];   // CustomEvent Data Buffer.  INCREASE it for your requirement
#ifdef _TARG_68000
unsigned int    uiDataSize;         // Size of the EventData Received
#elif __thumb
unsigned short  uiDataSize;         // Size of the EventData Received
#endif
short           shInputEvent;       // Input Event which could not be processed
char            cSenderName[10];    // Logical Name of the Appl which sent the Event
struct TRACK    stParsedCardData;   // Card Data Holder
void
ReturnPrinter(void) {
    short   iEvent;
    if(hPrinter != -1) {
        LOG_PRINTF (("Releasing Printer Device from %s", LogicalName));
        close (hPrinter);
        hPrinter = -1;
        iEvent = PRINTER_EVENT;
        EESL_send_event ("DEVMAN", RES_COMPLETE_EVENT, (unsigned char *) &iEvent, sizeof (iEvent));
    }
}

void
ReturnMagCard(void) {
    short   iEvent;
    if(hMagReader != -1) {
        LOG_PRINTF (("Releasing MagCard Device from %s", LogicalName));
        close (hMagReader);
        hMagReader = -1;
        if(Magflag == 0) {
            iEvent = INIT_EVENT;    // MagHandle was earlier obtained by the INIT EVENT
        }
        else {
            iEvent = MAGCARD_EVENT; // MagHandle was earlier obtained by the MAGCARD_EVENT
        }

        Magflag = -1;               // MagCard is now not available
        SendFlag = 0;               // Now the app can send an request event if required
        EESL_send_event ("DEVMAN", RES_COMPLETE_EVENT, (unsigned char *) &iEvent, sizeof (iEvent));
    }
}

//initialise the printer
void
PrinterInit(void) {
    open_block_t    parm;
    memset (&parm, 0, sizeof (parm));
    parm.rate = Rt_19200;   // Paprika ITP is always set to 19200 baud
    parm.format = Fmt_A8N1 | Fmt_auto | Fmt_RTS;    // Paprika ITP is always set at 8N1
    parm.protocol = P_char_mode;
    parm.parameter = 0;
    set_opn_blk (hPrinter, &parm);
    SVC_WAIT (200);
    p3300_init (hPrinter, 6);
	LOG_PRINTF (("Printer init"));
    SVC_WAIT (100);
}

// Print a buffer
void
Print(char *str) {
    unsigned char   printBuf[50];
    disable_hot_key ();
    memset (printBuf, 0, sizeof (printBuf));
    printBuf[0] = PRINT_NORM;
    strcpy ((char *) &printBuf[1], str);
    p3300_print (hPrinter, printBuf);
	//LOG_PRINTF (("Print a buffer "));
}

void
PrintCardData(void) {
    LOG_PRINTF (("Printing the Card Data from %s", LogicalName));

    // print the name
    Print ("Name : ");
    Print (stParsedCardData.name);
    Print ("\n");

    // print Account
    Print ("A/C  : ");
    Print (stParsedCardData.acct);
    Print ("\n");

    // print the expiry Date
    Print ("Exp  : ");
    Print (stParsedCardData.exp);
    Print ("\n\n");
}

void KBD_FLUSH(void)
{
  //Clearing KeyBoard Buffer
	
  char buff[3] = {'\0'};
  do
  {
        memset(buff, '\0', sizeof(buff)); 
        getkbd_entry(hClock, NULL,(signed char *)buff,1,KEYMAP,(char *)szKeyMap, sizeof((char *)szKeyMap),1, NULL);
  }while (KBHIT() > 0);
}



// display the idle menu
short
ShowIdle(void) {

		short iRetVal =0;
    //TransactionMsgStruc *ptrTransMsg = NULL;
    TransactionMsgStruc TransMsg ={0};//for static
		disable_hot_key();
   
		iRetVal = mainMenu(&TransMsg);//calls main Application
    //inVXEMVAPCloseCardslot();
    //  if(ptrTransMsg != NULL)
	  //		free(ptrTransMsg);//free transaction structure pointer
		if(Dmsg != NULL)
			free(Dmsg);//free display message array
		if(iRetVal == KEY_STR)
		{
			ClearKbdBuff();
			F4KeyHandler(1);
			ReturnPrinter();
			enable_hot_key ();
			close(hConsole);	
			return BRANCH_EXIT;
		}

	return 0;
}

//default handler defined for aie_main
short
appl_coldboot(void) {
	//LOG_PRINTF (("I am in cool boot "));

// Application specific initialization
    return TRUE;    // successful initialization
}

//default handler defined for aie_main
short
appl_idle_loop(void) {
	//LOG_PRINTF (("I am in idle loop "));
// Application specific idle loop processing
    return BRANCH_EXIT;
}

//default handler defined for aie_main
short
slow_poll(void) {
    return BRANCH_EXIT;
}

//default handler defined for aie_main
short
fast_poll(void) {
    return BRANCH_EXIT;
}

//default handler defined for application table
short
inEndTable(short state) {
    return(BRANCH_EXIT);
}

//default handler defined for aie_main
short
inActivateEvtResponder(void) {
    return(BRANCH_EXIT);
}

//default handler defined for aie_main
short
inDeactivateEvtResponder(void) {
    return(BRANCH_EXIT);
}

// clear keyboard buffer
void ClearKbdBuff(void)
{
    
	unsigned char Key;
	while(read(hConsole, (char*)&Key, 1) != 0);
}

// function to enable or disable the F1 key to get the magcard device

// This is required if some other high priority event grabs the mag device from this application
short
F1KeyHandler(short state) {
    if(hMagReader == -1 && SendFlag == 0) {
        LOG_PRINTF (("Sending MAGCARD Request to DevMan"));

        //	 EESL_send_event("DEVMAN", MAGCARD_REQUEST_EVENT, NULL,0);
        EESL_send_devman_event (LogicalName, MAGCARD_REQUEST_EVENT, NULL, 0);
        SendFlag = 1;   //disable app from sending more requests
    }
    else {
        LOG_PRINTF (("Did Not Sent event to Devman as hMagReader = %d & SendFlag = %d", hMagReader, SendFlag));
    }

    return(BRANCH_EXIT);
}

// function to process the exit of the application (Forceful deactivation process by the user)
short
F4KeyHandler(short state) {
    short   iEvent;
    short   retVal = BRANCH_EXIT;
    ReturnPrinter ();
    ReturnMagCard ();
    close (hConsole);
    hConsole = -1;  // need to close the console only on an exit event
    iEvent = ACT_EVENT;
    retVal = EESL_send_event ("DEVMAN", RES_COMPLETE_EVENT, (unsigned char *) &iEvent, sizeof (iEvent));
    LOG_PRINTF (("SENT RES_COMPLETE_EVENT to DEVMAN from %s", LogicalName));
    return retVal;
}

// function which gets called when DEVMAN sends an INIT_EVENT to the application
short
InitHandler(short state) {
    //hMagReader = open (DEV_CARD, 0);
    LOG_PRINTF (("Acquired MagCard Device in %s", LogicalName));
    Magflag = 0;    // magcard is obtained by the application init event
    return(BRANCH_EXIT);
}

// function which handles the ACT_EVENT from DevMan when a application selected from the FrontEnd Menu
short
ActEventHandler(short state) {

		if(hConsole <=0)
          hConsole = open(DEV_CONSOLE, 0);

     if(hPrinter <=0)
     {
          hPrinter = open(DEV_COM4, 0);
          PrinterInit();
     }
		 if(hClock <=0)
          hClock = open(DEV_CLOCK , 0 );
				LOG_PRINTF (("clock open hClock = %d", hClock));
	
  if(!LoadMVTFunction())
	{
		 LOG_PRINTF(("emvstart successfully."));
	}
	else
	{
		LOG_PRINTF(("emvstart failed."));
	}

  ShowIdle ();
	LOG_PRINTF (("Activated the Application %s", LogicalName));
		
  return(BRANCH_EXIT);

}

// function to read the card data after a card swipe happens
short
ReadCardHandler(short state) {
    short   shRetVal = 0;
    char    szRawCardData[CARD_SIZE];

    //Read card details from mag card reader
    shRetVal = read (hMagReader, szRawCardData, sizeof (szRawCardData));
    if(shRetVal < 0) {
        clrscr ();
        write_at ("Bevertec APPLICATION", strlen ("Bevertec APPLICATION"), 1, 1);
        write_at ("Can't read Card Data", 19, 1, 2);
        pause (25);     // wait for a keypress or a 250 millisecond wait
        ShowIdle ();
        return BRANCH_EXIT;
    }

    //parse the card data
    shRetVal = card_parse (szRawCardData, &stParsedCardData, "1");
    if(shRetVal != 1) {
        clrscr ();
        write_at ("Bevertec APPLICATION", strlen ("Bevertec APPLICATION"), 1, 1);
        write_at ("Error in Card Parsing", 21, 1, 2);
        pause (25);     // wait for a keypress or a 250 millisecond wait
        ShowIdle ();
        return BRANCH_EXIT;
    }

    clrscr ();

    //Show the card data
    display_at (1, 1, "A/C:", CLR_EOL);
    display_at (5, 1, stParsedCardData.acct, CLR_EOL);
    display_at (1, 2, "Exp on :", CLR_EOL);
    display_at (9, 2, stParsedCardData.exp, CLR_EOL);
    display_at (1, 3, "Name :", CLR_EOL);
    display_at (6, 3, stParsedCardData.name, CLR_EOL);
    pause (25);         // wait for a keypress or a 250 millisecond wait
    if(hPrinter == -1) {
        LOG_PRINTF (("Sending Request for Printer Device from %s", LogicalName));

        //	EESL_send_event("DEVMAN",PRINTER_REQUEST_EVENT,NULL,0) ;
        EESL_send_devman_event (LogicalName, PRINTER_REQUEST_EVENT, NULL, 0);
        SVC_WAIT (0);   // yield control so that by the app returns back
        // either handle is obtained or error procedure is executed
    }
    else {
        PrintCardData ();
    }

    ShowIdle ();
    return(BRANCH_EXIT);
}

// function to process error conditons when a request event to devman is not processed per requirement
short
ResNotAvailableHandler(short state) {
    switch(shInputEvent)
    {
        case ACT_EVENT:
            LOG_PRINTF (("Application %s failed the ACT_EVENT", LogicalName));
            break;

        case PRINTER_EVENT:
            clrscr ();
            write_at ("Can't Acquire PRINTER", 20, 1, 2);
            SVC_WAIT (2000);
            ShowIdle ();
            break;

        case MAGCARD_EVENT:
            clrscr ();
            write_at ("Can't Acquire MAGCARD", 20, 1, 2);
            SVC_WAIT (2000);

            SendFlag = 0;   //enable app from sending more requests
            ShowIdle ();
            break;

        case INIT_EVENT:
            LOG_PRINTF (("Application %s failed the INIT_EVENT", LogicalName));
            break;

        // Application can extend this SWITCH if needed specifically
        default:
            LOG_PRINTF (("Application %s failed the Event ID %d", LogicalName, shInputEvent));
            break;
    }

    return(BRANCH_EXIT);
}

// function to open the Printer Device
short
PRN_Request_Handler(short state) {
    hPrinter = open (DEV_COM4, 0);
    LOG_PRINTF (("Acquird Printer Device in %s", LogicalName));
    PrinterInit ();
    PrintCardData ();
    return(BRANCH_EXIT);
}

// function to open the MAGCARD device
short
MAG_Request_Handler(short state) {
    hMagReader = open (DEV_CARD, 0);
    LOG_PRINTF (("Acquired MagCard Device in %s", LogicalName));
    Magflag = 1;    // MAGCARD device is now acquired by MAGCARD_EVENT
    ShowIdle ();
    return(BRANCH_EXIT);
}

// handler which gets called when DEVMAN posts a COMM_4_REQUEST_EVENT to the application
short
PRN_Release_Handler(short state) {
    ReturnPrinter ();
    return BRANCH_EXIT;
}

// handler which gets called when DEVMAN posts a MAG_READER_REQUEST_EVENT to the application
short
MAG_Release_Handler(short sh) {
    ReturnMagCard ();
    ShowIdle ();
    return BRANCH_EXIT;
}

// Generic Handler for any custom Pipe Messages
short
PipeHandler(short sh) {
    short   shRetVal = 0;
    disable_hot_key ();
    if(EESL_queue_count () == 0) {

        // Check if the application is having messages
        return BRANCH_EXIT;
    }

    //LOG_PRINTF (("Reading PIPE event in Bevertec"));
    shRetVal = EESL_read_cust_evt (ucEventData, sizeof (ucEventData), &uiDataSize, cSenderName);
    if(shRetVal == APP_RES_SET_UNAVAILABLE) {
        memcpy ((void *) &shInputEvent, (void *) ucEventData, uiDataSize);
        LOG_PRINTF (("The Input Event %d caused DEVMAN to send APP_RES_SET_UNAVAILABLE", shInputEvent));
    }

    LOG_PRINTF (("EESL_read_cust_evt: Received Event ID %d & EventData size is %d", shRetVal, uiDataSize));
    return shRetVal;    // This will appropriately call the Custom Event handler
}

// This function is provided for completeness as this never occurs under VMAC environment
short
DefaultActivateHandler(short state) {
    //LOG_PRINTF (("In Default ACTIVATE IN"));
    return(BRANCH_EXIT);
}

//This function is called by the application whenever an user presses the HOTKEY in the terminal
short
DeactivateEventHandler(short state) {
    short   iEvent;
    short   retVal = BRANCH_EXIT;
    LOG_PRINTF (("DeActivating Application %s", LogicalName));
    ReturnPrinter ();
    ReturnMagCard ();
    iEvent = ACT_EVENT;
    retVal = EESL_send_event ("DEVMAN", RES_COMPLETE_EVENT, (unsigned char *) &iEvent, sizeof (iEvent));
    return(retVal);
}

/**************************************************************************/

/*                    Idle transaction Process table                      */

/**************************************************************************/
PF_TABLE            appl_table[] =
{
#define FT_PIPE_EVENT   0
    PipeHandler,
#define FT_EVENT_EKEY   1
    F1KeyHandler,
#define FT_EVENT_HKEY   2
    F4KeyHandler,
#define FT_INIT_EVENT   3
    InitHandler,
#define FT_ACT_EVENT    4
    ActEventHandler,
#define FT_PRN_REQ_EVENT    5
    PRN_Request_Handler,
#define FT_MAG_REQ_EVENT    6
    MAG_Request_Handler,
#define FT_READCARD_EVENT   7
    ReadCardHandler,
#define FT_UNAVAILABLE_EVENT    8
    ResNotAvailableHandler,
#define FT_ACT_IN   9
    DefaultActivateHandler,
#define FT_DEACT_IN 10
    DeactivateEventHandler,
#define FT_PRN_RELEASE_EVENT    11
    PRN_Release_Handler,
#define FT_MAG_RELEASE_EVENT    12
    MAG_Release_Handler,
#define FT_ENDTBL   13
    inEndTable,
    (PF_TABLE) END_TABLE
};

/**************************************************************************/

/*                    Main Idle State Branch Table                        */

/*            (this table is required for every application)              */

/**************************************************************************/
BRANCH_TBL_ENTRY    idle_table[] =
{
    { PIPE_IN, FT_PIPE_EVENT, idle_table },
    { KEY_e, FT_EVENT_EKEY, idle_table },
    { KEY_h, FT_EVENT_HKEY, idle_table },
    { INIT_EVENT, FT_INIT_EVENT, idle_table },
    { ACT_EVENT, FT_ACT_EVENT, idle_table },
    { PRINTER_EVENT, FT_PRN_REQ_EVENT, idle_table },
    { MAGCARD_EVENT, FT_MAG_REQ_EVENT, idle_table },
    { APPL_CARD, FT_READCARD_EVENT, idle_table },
    { APP_RES_SET_UNAVAILABLE, FT_UNAVAILABLE_EVENT, idle_table },
    { SLOW_POLL_IN, 0, idle_table },
    { FAST_POLL_IN, 0, idle_table },
    { ACTIVATE_IN, FT_ACT_IN, idle_table },
    { DEACTIVATE_IN, FT_DEACT_IN, idle_table },
    { COMM_4_REQUEST_EVENT, FT_PRN_RELEASE_EVENT, idle_table },
    { MAG_READER_REQUEST_EVENT, FT_MAG_RELEASE_EVENT, idle_table },
    { END_TABLE, FT_ENDTBL, idle_table }
};
short OTHER_AC=0;
char AC_TYPE='F';
short pin_enc = '1';
short FROM_OTHER_AC =0;
short BI=0;
unsigned char temp1Buffer[800];
short key_injected;
//char tempBuff[800];
int
main(int argc, char *argv[]) {
    ///long                error;
    EESL_CONTROL_STR    gEeslData;  // global memory for EESL
    strcpy (LogicalName, "Bevertec");
    LOG_INIT (
      LogicalName, LOGSYS_COMM, LOGSYS_PRINTF_FILTER | EESL_DEBUG_GLOBAL | EESL_DEBUG_SEND_SUCCESS | EESL_DEBUG_RECEIVE_SUCC);
    /*error =*/ EESL_Initialise (argv, argc, LogicalName, &gEeslData);
    //kLOG_PRINTF (("EESL_Initialise Returned :%d", error));
    aie_main (idle_table, appl_table, NULL, NULL, NULL, NULL, NULL);
    return 0;
}


void resetMsgDetails(TransactionMsgStruc *transMsg)
{
    memset(transMsg, '\0', sizeof(TransactionMsgStruc));
    if(LOG_STATUS == LOG_ENABLE)
    {
      LOG_PRINTF(("Reset Structure->>>>>>>>>>>>>>>>>>>>>>"));
    }
}

/////////////////////////////////////// E N D /////////////////////////////////////////////////////////

///////////////////////////////////////read display message //////////////////////////////////////////
//this readDisplaygMsg function read the display message string from ConfiguredMessage.txt file
/*********************************************************************************************************
*	Function Name : readDisplaygMsg																																					*
*	Purpose		    : read the display message string from ConfiguredMessage.txt file 												*
*	Input					:	void																																										*
*	Output		    : store all message available for display 																								*
*	Date		      : 20-apr-2016																																							*
**********************************************************************************************************/

short readDisplaygMsg(void)
{
		FILE *ifp=NULL;
		int i=0,j=0;
		char buff[50]={0};
		char temp[50]={0};
		
		char *lptr = NULL;
		char *rptr = NULL;
		
		ifp = fopen(MSG_CONFIG_FILE, "r");
		if (ifp == NULL)
		{
      if(LOG_STATUS == LOG_ENABLE)
      {
				LOG_PRINTF (("Failed to open the file %s",MSG_CONFIG_FILE));	
      }
			return -1;
		}
		for(i = 0;i>=0;i++)
		{
				lptr = buff;
				j=0;
				memset(buff,0,sizeof(buff));
				if(fgets(buff,sizeof(buff),ifp) == NULL)
				{
          if(LOG_STATUS == LOG_ENABLE)
          {
						LOG_PRINTF (("EOF reached"));
          }
					fclose(ifp);
					break;
				}
        if(strstr(buff,"//"))
        {
          if(LOG_STATUS == LOG_ENABLE)
          {
            LOG_PRINTF (("// reached"));
          }
          fclose(ifp);
          break;
        }

				rptr = strchr(buff,',');
				rptr--;
				memset(temp,0,sizeof(temp));
				
				for(j=0;lptr<=rptr;j++,lptr++)
				{
						temp[j]=*lptr;
				}

				Dmsg[i].x_cor = atoi(temp);
				lptr = rptr +2;
				rptr = strchr(lptr,',');
				rptr--;
				j=0;
				memset(temp,0,sizeof(temp));
				for(j=0;lptr<=rptr;j++,lptr++)
				{
						temp[j]=*lptr;
				}
				Dmsg[i].y_cor = atoi(temp);
				lptr = rptr +2;
				rptr = strchr(lptr,13);
				rptr = rptr-1;
				j=0;
				memset(temp,0,sizeof(temp));
				
				for(j=0;lptr<=rptr;j++,lptr++)
				{
						temp[j]=*lptr;
				}
				strcpy(Dmsg[i].dispMsg,temp);
				getc(ifp);//for increment one char 
		}
		return _SUCCESS;
}

short swipeCard(short *swipeCardFlag,TransactionMsgStruc *ptrTransMsgDetails,short *Selectionstatus,short *getKey)
{
      int mask = 0;
      short iRetVal = 0;
      char track2data[TRACK2_DATA_LEN+1]={0};//stores the track2 data for extracting the PAN
 
      *swipeCardFlag = 0; 
      
      do
			{
        if( hMagReader == -1)
	      {
          ClearKbdBuff(); //clearing keyboard buffer
				  KBD_FLUSH();
			    hMagReader = open (DEV_CARD, 0);
			    if(LOG_STATUS == LOG_ENABLE)
          {
            LOG_PRINTF (("megnatic reader open = %d",hMagReader));	
          }
	      }
			  mask = read_event();					//Collect read interrupt information 	
				mask = wait_event();//waiting for event
      
				if(mask & EVT_MAG) //checking for magnetic card swipe
				{
            if(!strcmp((char *)EncSessionKey,"")) 
            {
                window(1,1,30,20);
                clrscr();
				write_at("MERCHANT",8,13,8);
		        write_at(Dmsg[PLEASE_LOGON_FIRST].dispMsg, strlen(Dmsg[PLEASE_LOGON_FIRST].dispMsg),Dmsg[PLEASE_LOGON_FIRST].x_cor, Dmsg[PLEASE_LOGON_FIRST].y_cor);
     
                SVC_WAIT(2000);
                ClearKbdBuff();
			    KBD_FLUSH();
                *Selectionstatus = eTransaction; 
                break ;
            }
            if(CheckPaperFlag == 0)
            {
              iRetVal = checkPaperStatus();
              if(iRetVal == _FAIL)
              {
                *Selectionstatus = eTransaction; 
                 *swipeCardFlag = -1;  
                  return _FAIL;
              }
            }
						iRetVal = readMagCard(ptrTransMsgDetails); //reading mag card
            if(iRetVal == _FAIL)
            {
              return _FAIL;
            }

            strcpy((char *)track2data,ptrTransMsgDetails->Track2Data);             //22 POS Entry Mode
	          iRetVal =  CheckServiceCode(track2data);
            if(iRetVal != _SUCCESS)
		        {
			        iRetVal =  FORCE_FOR_EMV;
              
		        }
						*swipeCardFlag = 1; 
						*Selectionstatus = eTransaction; 
            strcpy((char *)ptrTransMsgDetails->POSEntryMode,"021");
            if(LOG_STATUS == LOG_ENABLE)
            {
              LOG_PRINTF (("MAG CARD..."));//22 POS Entry Mode
            }
					  break;
				}
				else if(mask & EVT_KBD) //checking for keyboard event
				{
						*swipeCardFlag = 0;
            *getKey = get_char();
						if(LOG_STATUS == LOG_ENABLE)
            {
              LOG_PRINTF (("KBD..."));
            }
            if(*getKey== KEY_CANCEL) //if * key pressed
            {
              *Selectionstatus  = SelectEvent ;
             
            }
            else
            {
              *Selectionstatus  = *getKey ;
            }
   
						break;
				}
        else if(mask & EVT_ICC1_INS) //checking for EMV event
        {
          //if(!strcmp((char *)EncSessionKey,"")) 
          //{
          //    window(1,1,30,20);
          //    clrscr();
		        //  write_at(Dmsg[PLEASE_LOGON_FIRST].dispMsg, strlen(Dmsg[PLEASE_LOGON_FIRST].dispMsg),Dmsg[PLEASE_LOGON_FIRST].x_cor, Dmsg[PLEASE_LOGON_FIRST].y_cor);//LOGON SUCCESSFULL
     
          //    SVC_WAIT(1000);
          //    ClearKbdBuff();
			       // KBD_FLUSH();
          //    *Selectionstatus = eTransaction; 
          //    break ;
          //}
          if(CheckPaperFlag == 0)
          {
            iRetVal = checkPaperStatus();
            LOG_PRINTF(("checkPaperStatus = %d",iRetVal));
            if(iRetVal == _FAIL)
            {
              *Selectionstatus = eTransaction; 
              *swipeCardFlag = -1;  
              return _FAIL;
            }
          }
            
 				  *swipeCardFlag = 1;
          *Selectionstatus = eTransaction; 
          strcpy((char *)ptrTransMsgDetails->POSEntryMode,"051");//22 POS Entry Mode
          if(LOG_STATUS == LOG_ENABLE)
          {
            LOG_PRINTF (("EMV CARD..."));
          }
          //ptrTransMsgDetails->TrTypeFlag = SALEMSGTYPE_CASE;
		  ptrTransMsgDetails->TrTypeFlag = WITHDRAWAL_MSG_TYPE_CASE;		//AGENCY BANK
          iRetVal = inRunEMVTransaction(ptrTransMsgDetails->TrTypeFlag);
          if(iRetVal == _SUCCESS)
          {
				iRetVal = GetInputFromUser(ptrTransMsgDetails,TRANSAMOUNT);
				if((iRetVal == _FAIL ) || (iRetVal == KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL))
				{
					*Selectionstatus = KEY_CANCEL;
					CloseEMVSLot();
					return _FAIL;
				}
				if(iRetVal == _SUCCESS)
				{
					iRetVal = emvFirstGenerationFlow(ptrTransMsgDetails);//first generation flow
					if(iRetVal == _SUCCESS)
					{
						ptrTransMsgDetails->EMV_Flag = 1 ;
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF (("After emvFirstGenerationFlow success ret =%d",iRetVal));
						}
					}
					else if(iRetVal != FALLBACK)
					{
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF (("After emvFirstGenerationFlow fail ret =%d",iRetVal));
						}
						CloseEMVSLot();
                return _FAIL;
              }
              
            }
          }
          else if(iRetVal != FALLBACK)
          {
            if(LOG_STATUS == LOG_ENABLE)
            {
		          LOG_PRINTF (("After emvFirstGenerationFlow fail ret =%d",iRetVal));
            }
            CloseEMVSLot();
            return _FAIL;
          }
          else
          {
            if(LOG_STATUS == LOG_ENABLE)
            {
              LOG_PRINTF (("iFallback occurred = %d",iRetVal));
            }
            
          }
				  break;
        }
        
			}while (1);
      if((iRetVal == FALLBACK)/*&&(ptrTransMsgDetails->EMV_Flag == 1)*/)
      {
        LOG_PRINTF(("Main screen SaleTransaction:FALLBACK"));
        iRetVal = swipeCardEntry(ptrTransMsgDetails);//Read card data 
				if((iRetVal == (KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL) || (iRetVal == _FAIL)))
				{
            *Selectionstatus = KEY_CANCEL;
            
						return _FAIL;;
				}
        strcpy((char *)ptrTransMsgDetails->POSEntryMode,MSReICC_PIN_CAPABLE);             //22 POS Entry Mode
      }
      else if(iRetVal == FORCE_FOR_EMV)
      {
        LOG_PRINTF(("Main screen SaleTransaction:FORCE_FOR_EMV"));
        iRetVal = GetInputFromUser(ptrTransMsgDetails,TRANSAMOUNT);
        if((iRetVal == _FAIL ) || (iRetVal == KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL))
				{
					*Selectionstatus = KEY_CANCEL;
          CloseEMVSLot();
					return _FAIL;
				}
        //ptrTransMsgDetails->TrTypeFlag = SALEMSGTYPE_CASE;
		ptrTransMsgDetails->TrTypeFlag = WITHDRAWAL_MSG_TYPE_CASE;		//Agency Bank
        iRetVal = EMVCardEntry(ptrTransMsgDetails);//Read card data
        if(iRetVal == FALLBACK)
        {
          iRetVal = swipeCardEntry(ptrTransMsgDetails);//Read card data 
				  if((iRetVal == (KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL) || (iRetVal == _FAIL)))
				  {
              *Selectionstatus = KEY_CANCEL;
            
						  return _FAIL;;
				  }
          strcpy((char *)ptrTransMsgDetails->POSEntryMode,MSReICC_PIN_CAPABLE);             //22 POS Entry Mode
                
        }
				else if((iRetVal == (KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL) || (iRetVal == _FAIL)))
				{
            *Selectionstatus = KEY_CANCEL;
            CloseEMVSLot();
						return _FAIL;;
				}
        //strcpy((char *)ptrTransMsgDetails->POSEntryMode,MSReICC_PIN_CAPABLE); 
      }
			/////////////////////////////////////////////////
			close(hMagReader);
      hMagReader = -1;
      if(ptrTransMsgDetails->EMV_Flag == 1)
      {
        CloseEMVSLot();
      }
      return iRetVal;
}


int mainMenu(TransactionMsgStruc *ptrTransMsgDetails)
{
	short Selectionstatus = SelectEvent;
	short iRetVal = 0;
	
	short mainFlag = GoToMainScreen;
  //char ch =0 ;
	short swipeCardFlag = 0;
	short getKey = -1;
  	int /*mask = 0,*/i = 0;
	resetMsgDetails(ptrTransMsgDetails);						//reset All Data which is present in Transaction Message Structure
	OTHER_AC=0;
	Dmsg = NULL;//inttilizing with NULL
	Dmsg = (dispMsg*)malloc(DISP_ARR_SIZE*sizeof(dispMsg));
	for(i= 0;i<DISP_ARR_SIZE;i++)
		memset(&Dmsg[i],0,sizeof(dispMsg));
	readDisplaygMsg();
	
	KBD_FLUSH();
		 
	while(Selectionstatus != KEY_STR)
	{
        set_chars_per_key_value(5);         //set 5 character per key
		if(mainFlag == GoToMainScreen)
		{
			if(!StartEMV())
			{
				LOG_PRINTF(("emvstart successfully."));
			}
			else
			{
				LOG_PRINTF(("emvstart failed."));
			}
			iRetVal = p3300_status(hPrinter,1000);
			LOG_PRINTF(("p3300_status iRetVal = %d",iRetVal));
			if(iRetVal == PAPER_OUT)
			{
				paperOutFlag=1;
			}
			else
			{
				paperOutFlag=0;//reset print flag when paper is out
			}
			
			if(inVXEMVAPCardPresent() == _SUCCESS)
			{
			  //message display  "REMOVE CARD"
				window(1,1,30,20);
				clrscr();
				write_at("REMOVE YOUR CARD", strlen("REMOVE YOUR CARD"),7, 10);					
			  //display_at(1,8,"REMOVE CHIP CARD",CLR_LINE);
        //sound(95,5000);
        
				while(!inVXEMVAPCardPresent())
				{
					error_tone();
					//normal_tone();
					SVC_WAIT(200);
				}
				ClearKbdBuff();
				KBD_FLUSH();
			}
			clrscr();
			if(LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("mainFlag get_char"));
			}
			resetMsgDetails(ptrTransMsgDetails);//reset transaction structrure every time when control goes to main menu.
			
			SetImage(MainMenu_BMP);
			EnblTouchScreen(mainMenuS);
			iRetVal = p3300_status(hPrinter,1000);
			LOG_PRINTF(("p3300_status iRetVal = %d",iRetVal));
			if(iRetVal == PAPER_OUT)
			{
				paperOutFlag=1;
			}
			else
			{
				paperOutFlag=0;//reset print flag when paper is out
			}
			
			ptrTransMsgDetails->TrMethordFlag = -1;
			Selectionstatus = SelectEvent;
      
			if(LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("mainFlag get_char = %d " ,mainFlag));
			}
			mainFlag = 0;

			if(LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("getKey  12233  =====%d",getKey));
			}

			//iRetVal = swipeCard(&swipeCardFlag,ptrTransMsgDetails,&Selectionstatus,&getKey);	//Agency Bank
      
			if(LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("After getKey  12233 get_char =====%d",getKey));
			}
      
/*			if(iRetVal == _FAIL)
			{
				mainFlag = GoToMainScreen;
				Selectionstatus = KEY_CANCEL;
			}
*/
			CheckPaperFlag = 0 ;
		}
/*		if((swipeCardFlag == 0) && ((getKey == 8) || (getKey == 14) || (getKey == 13)|| (getKey == 35) ||((getKey >= 48)&&(getKey <= 57)) ))
		{ 
			if(LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("getKey    =====%d",getKey));
			}
			iRetVal = swipeCard(&swipeCardFlag,ptrTransMsgDetails,&Selectionstatus,&getKey);
			if(iRetVal == _FAIL)
			{
				mainFlag = GoToMainScreen;
				Selectionstatus = KEY_CANCEL;
			}
			if(LOG_STATUS == LOG_ENABLE)
			{
				LOG_PRINTF(("$$$$$$$$$$$$$$$$$$$Selectionstatus=%d After getKey  =%d",Selectionstatus,getKey));
			}
		}
	*/
		if(LOG_STATUS == LOG_ENABLE)
		{
			LOG_PRINTF(("Waiting for event getKey-get_char%d",getKey));
		}

		if (Selectionstatus == SelectEvent)
		{
			Selectionstatus =  get_char();
		}
		if(Selectionstatus == KEY_STR) //if * key pressed
			return KEY_STR;
		else if(Selectionstatus == KEY_CANCEL) //if * key pressed
		{
			Selectionstatus = SelectEvent ;
			mainFlag = GoToMainScreen ;
		}
		LOG_PRINTF(("Selection Status %d",Selectionstatus));
		AC_TYPE='F';
		switch(Selectionstatus)
		{
			case eTransaction: //transaction touch		
				clrscr();
				ptrTransMsgDetails->TrMethordFlag = -1;
                if(!strcmp((char *)EncSessionKey,"")) 
                {
                     window(1,1,30,20);
                     clrscr();
		             write_at("MERCHANT",8,13,8);
					 write_at(Dmsg[PLEASE_LOGON_FIRST].dispMsg, strlen(Dmsg[PLEASE_LOGON_FIRST].dispMsg),Dmsg[PLEASE_LOGON_FIRST].x_cor, Dmsg[PLEASE_LOGON_FIRST].y_cor);//LOGON SUCCESSFULL
     
                     SVC_WAIT(1000);
                     ClearKbdBuff();
			               KBD_FLUSH();
                     mainFlag = GoToMainScreen;
                     break ;
				}
				LOG_PRINTF(("SUP_CARD = %d " ,SUP_CARD));
				if(Sup_Login==0)
				{
					window(1,1,30,20);
                    clrscr();
					write_at("SUPERVISOR NOT LOGGED IN",24,3,10);//LOGON SUCCESSFULL
		             //write_at(Dmsg[PLEASE_LOGON_FIRST].dispMsg, strlen(Dmsg[PLEASE_LOGON_FIRST].dispMsg),Dmsg[PLEASE_LOGON_FIRST].x_cor, Dmsg[PLEASE_LOGON_FIRST].y_cor);//LOGON SUCCESSFULL
                    SVC_WAIT(1000);
                    ClearKbdBuff();
					KBD_FLUSH();
                    mainFlag = GoToMainScreen;
                    break ;
				}
                LOG_PRINTF(("swipeCardFlag:     %d " ,swipeCardFlag));
                if(swipeCardFlag == 0 && CheckPaperFlag == 0)
				{
                    iRetVal = checkPaperStatus();
                    if(iRetVal == _FAIL)
                    {
						mainFlag = GoToMainScreen;
						break ;
                    }
				}
				SetImage(TransMethord_BMP);//display transaction Methord like debit,credit,loyality screen
				EnblTouchScreen(TransMethord);
									
                if(LOG_STATUS == LOG_ENABLE)
                {
                    LOG_PRINTF (("eTransaction:PrimaryAccNum %%%%%%% = %s",ptrTransMsgDetails->PrimaryAccNum));
                }
				Selectionstatus = SelectEvent;

				break;
			case  edebit:		//debit button touch
					clrscr();
					ptrTransMsgDetails->TrMethordFlag = edebit;
					if(LOG_STATUS == LOG_ENABLE)
					{
						LOG_PRINTF (("-----IPP Communication session key =%s\n",EncSessionKey));
					}
                 
					if(swipeCardFlag!=1)
					{
						SetImage(TransType_BMP);//display transaction type (sale,balance inq,void,refund) screen for debit transaction
						EnblTouchScreen(TransType);
						Selectionstatus = SelectEvent;
					}
					else
					{
						Selectionstatus = eSale ;                  
					}
					LOG_PRINTF (("Selection Status = %d",Selectionstatus));
					if(LOG_STATUS == LOG_ENABLE)
					{
						LOG_PRINTF (("edebit:PrimaryAccNum ^^^^^^^^^^^ = %s",ptrTransMsgDetails->PrimaryAccNum));
					}
					break;
				case  ecredit:	//credit button touch		
						clrscr();
						//ReadFromDatabase();
						ptrTransMsgDetails->TrMethordFlag = ecredit;
						if(swipeCardFlag!=1)
						{
							SetImage(Credit_TransType_BMP);//display transaction type (sale,balance inq,void,refund) screen for credit transaction
							EnblTouchScreen(CreditTransMenu);
							Selectionstatus = SelectEvent;
						}
						else
						{
							Selectionstatus = eSale ;
						}
									
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("Credit -------%d",Selectionstatus));
						}
						break;
				case  eLoyality:			//loyality button touch		
						clrscr();
						ptrTransMsgDetails->TrMethordFlag = eLoyality;
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("loyality -------"));
						}
						write_at("loyality", strlen("loyality"),1, 10);
						SVC_WAIT(1000);
						ClearKbdBuff();
			            KBD_FLUSH();
						mainFlag = GoToMainScreen;
						break;
				case  eTrMethordBack:	//back button touch from transaction scrren
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("Back from Transaction Methord -------"));
						}
						mainFlag = GoToMainScreen;
						break;
		
				case  eWithdrawal:
					iRetVal  = InitReversal();//check reversal data and if any go for reversal
					if(iRetVal == _SUCCESS)
					{
						ptrTransMsgDetails->TrTypeFlag = WITHDRAWAL_MSG_TYPE_CASE;
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("eWithdrawal-------"));
						}
						iRetVal = InitDeposit(ptrTransMsgDetails);
/*						//if(ptrTransMsgDetails->POSEntryMode != 1)
						if((strcmp((char *)ptrTransMsgDetails->POSEntryMode,ICC_PIN_CAPABLE)!=0)&& (strcmp((char *)ptrTransMsgDetails->POSEntryMode,MSReICC_PIN_CAPABLE)!=0))
						{
							iRetVal = GetInputFromUser(ptrTransMsgDetails,TRANSAMOUNT);//getting amount from user input
						    if((iRetVal == _FAIL ) || (iRetVal == KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL))
						    {
								mainFlag = GoToMainScreen;
								break;
							}
						}*/
					}
					else
					{
						window(1,1,30,20);
			            clrscr();
						write_at(Dmsg[CAN_NOT_DO_WITHDRAWAL].dispMsg, strlen(Dmsg[CAN_NOT_DO_WITHDRAWAL].dispMsg),Dmsg[CAN_NOT_DO_WITHDRAWAL].x_cor, Dmsg[CAN_NOT_DO_WITHDRAWAL].y_cor);
				        write_at(Dmsg[PENDING_REVERSAL].dispMsg, strlen(Dmsg[PENDING_REVERSAL].dispMsg),Dmsg[PENDING_REVERSAL].x_cor, Dmsg[PENDING_REVERSAL].y_cor);
						SVC_WAIT(2000);
						ClearKbdBuff();
			            KBD_FLUSH();
                        mainFlag = GoToMainScreen;
						break;
					}
				/*	if(ptrTransMsgDetails->TrMethordFlag == edebit )
					{
						SetImage(ACC_TYPE_BMP);//setting image for Account type 
						EnblTouchScreen(Agency_AccountType);
						Selectionstatus = SelectEvent;
					}
					else //for credit card
					{
          				Selectionstatus = eCashBackNo;
					}*/
					mainFlag = GoToMainScreen;
					break;
				/*case ePINValidate:
						ptrTransMsgDetails->TrTypeFlag = PIN_VALIDATE_MSGTYPE_CASE;
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("ePINValidate-------"));
						}
						iRetVal = InitPINValidate(ptrTransMsgDetails);
						if(iRetVal == KEY_STR)
							Selectionstatus = KEY_STR;
						else
							mainFlag = GoToMainScreen;
						break;
						*/
				case  eSale:			//sale button (paymnent button for government )
					iRetVal  = InitReversal();//check reversal data and if any go for reversal
					if(iRetVal == _SUCCESS)
					{
						ptrTransMsgDetails->TrTypeFlag = SALEMSGTYPE_CASE;
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("eSale-------"));
						}
						//if(ptrTransMsgDetails->POSEntryMode != 1)
						if((strcmp((char *)ptrTransMsgDetails->POSEntryMode,ICC_PIN_CAPABLE)!=0)&& (strcmp((char *)ptrTransMsgDetails->POSEntryMode,MSReICC_PIN_CAPABLE)!=0))
						{
							iRetVal = GetInputFromUser(ptrTransMsgDetails,TRANSAMOUNT);//getting amount from user input
						    if((iRetVal == _FAIL ) || (iRetVal == KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL))
						    {
								mainFlag = GoToMainScreen;
								break;
							}
						}
					}
					else
					{
						window(1,1,30,20);
			            clrscr();
						write_at(Dmsg[CAN_NOT_DO_SALE].dispMsg, strlen(Dmsg[CAN_NOT_DO_SALE].dispMsg),Dmsg[CAN_NOT_DO_SALE].x_cor, Dmsg[CAN_NOT_DO_SALE].y_cor);
				        write_at(Dmsg[PENDING_REVERSAL].dispMsg, strlen(Dmsg[PENDING_REVERSAL].dispMsg),Dmsg[PENDING_REVERSAL].x_cor, Dmsg[PENDING_REVERSAL].y_cor);
						SVC_WAIT(2000);
						ClearKbdBuff();
			            KBD_FLUSH();
                        mainFlag = GoToMainScreen;
						break;
					}
					#ifdef GOVT_APP //if application is government type
						iRetVal = getPaymentId(ptrTransMsgDetails);//getting Payment Id from user for Government
						if((iRetVal == (KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL) || (iRetVal == _FAIL)))
						{
							mainFlag = GoToMainScreen;
							break;
						}
					#endif
					if(ptrTransMsgDetails->TrMethordFlag == edebit )
					{
						SetImage(ACC_TYPE_BMP);//setting image for Account type 
						EnblTouchScreen(AccountType);
						Selectionstatus = SelectEvent;
					}
					else //for credit card
					{
          				Selectionstatus = eCashBackNo;
					}
					break;
				case  eBalanceInquiry:	//balance inquiry		
						clrscr();
						BI=1;
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("eBalanceInquiry -------"));
						}
						ptrTransMsgDetails->TrTypeFlag = BALINQMSGTYPE_CASE;
						iRetVal = InitBalanceInquiry(ptrTransMsgDetails);
						//iRetVal = InitBalance(ptrTransMsgDetails);
						if(iRetVal == KEY_STR)
							Selectionstatus = KEY_STR;
						else
							mainFlag = GoToMainScreen;
						break;
				case  eVoid:	//Void a Transaction
						iRetVal  = InitReversal();//check reversal data and if any go for reversal
						if(iRetVal == _SUCCESS)
						{
							ptrTransMsgDetails->TrTypeFlag = VOIDMSGTYPE_CASE;
							iRetVal  = InitVoid(ptrTransMsgDetails);
						}
						else
						{
							window(1,1,30,20);
							clrscr();
							write_at(Dmsg[CAN_NOT_DO_VOID].dispMsg, strlen(Dmsg[CAN_NOT_DO_VOID].dispMsg),Dmsg[CAN_NOT_DO_VOID].x_cor, Dmsg[CAN_NOT_DO_VOID].y_cor);
				            write_at(Dmsg[PENDING_REVERSAL].dispMsg, strlen(Dmsg[PENDING_REVERSAL].dispMsg),Dmsg[PENDING_REVERSAL].x_cor, Dmsg[PENDING_REVERSAL].y_cor);
							SVC_WAIT(2000);
							ClearKbdBuff();
							KBD_FLUSH();
							mainFlag = GoToMainScreen;
							break;
						}
						mainFlag = GoToMainScreen;	
						break;
				case  eRefund: //refund transaction	
						iRetVal  = InitReversal();//check reversal data and if any go for reversal
						if(iRetVal == _SUCCESS)
						{	
							ptrTransMsgDetails->TrTypeFlag = REFUNDMSGTYPE_CASE;
							iRetVal = InitRefund(ptrTransMsgDetails);
						}
						else
						{
							window(1,1,30,20);
							clrscr();
							write_at(Dmsg[CAN_NOT_DO_REFUND].dispMsg, strlen(Dmsg[CAN_NOT_DO_REFUND].dispMsg),Dmsg[CAN_NOT_DO_REFUND].x_cor, Dmsg[CAN_NOT_DO_REFUND].y_cor);
							write_at(Dmsg[PENDING_REVERSAL].dispMsg, strlen(Dmsg[PENDING_REVERSAL].dispMsg),Dmsg[PENDING_REVERSAL].x_cor, Dmsg[PENDING_REVERSAL].y_cor);
							SVC_WAIT(2000);
							ClearKbdBuff();
							KBD_FLUSH();
							mainFlag = GoToMainScreen;
							break;
						}
						mainFlag = GoToMainScreen;
						break;
				
				case  eDeposit: //Agency Bank Deposit transaction	
						iRetVal  = InitReversal();//check reversal data and if any go for reversal
						if(iRetVal == _SUCCESS)
						{	
							ptrTransMsgDetails->TrTypeFlag = DEPOSIT_MSG_TYPE_CASE;
							iRetVal = InitDeposit(ptrTransMsgDetails);
						}
						else
						{
							window(1,1,30,20);
							clrscr();
							write_at(Dmsg[CAN_NOT_DO_DEPOSIT].dispMsg, strlen(Dmsg[CAN_NOT_DO_DEPOSIT].dispMsg),Dmsg[CAN_NOT_DO_DEPOSIT].x_cor, Dmsg[CAN_NOT_DO_DEPOSIT].y_cor);
							write_at(Dmsg[PENDING_REVERSAL].dispMsg, strlen(Dmsg[PENDING_REVERSAL].dispMsg),Dmsg[PENDING_REVERSAL].x_cor, Dmsg[PENDING_REVERSAL].y_cor);
							SVC_WAIT(2000);
							ClearKbdBuff();
							KBD_FLUSH();
							mainFlag = GoToMainScreen;
							break;
						}
						mainFlag = GoToMainScreen;
						break;

				case  eTransfer: //Agency Bank Deposit transaction	
						iRetVal  = InitReversal();//check reversal data and if any go for reversal
						if(iRetVal == _SUCCESS)
						{	
							ptrTransMsgDetails->TrTypeFlag = TRANSFER_MSG_TYPE_CASE;
							iRetVal = InitTransfer(ptrTransMsgDetails);
						}
						else
						{
							window(1,1,30,20);
							clrscr();
							write_at(Dmsg[CAN_NOT_DO_TRANSFER].dispMsg, strlen(Dmsg[CAN_NOT_DO_TRANSFER].dispMsg),Dmsg[CAN_NOT_DO_TRANSFER].x_cor, Dmsg[CAN_NOT_DO_TRANSFER].y_cor);
							write_at(Dmsg[PENDING_REVERSAL].dispMsg, strlen(Dmsg[PENDING_REVERSAL].dispMsg),Dmsg[PENDING_REVERSAL].x_cor, Dmsg[PENDING_REVERSAL].y_cor);
							SVC_WAIT(2000);
							ClearKbdBuff();
							KBD_FLUSH();
							mainFlag = GoToMainScreen;
							break;
						}
						mainFlag = GoToMainScreen;
						break;

				case  eTrTypeBack:		//back button on screen
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("Back from Transaction Type -------"));
						}
						Selectionstatus = eTransaction;
						break;
				case  eTrTypeNext:		//back button on screen
						clrscr();
						SetImage(TransType1_BMP);//display transaction type 1 (void) screen for debit transaction
						EnblTouchScreen(TransType1);
						Selectionstatus = SelectEvent;
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("Transaction Type 1 screen -------"));
						}
						Selectionstatus = SelectEvent;
						break;
				case  eTrType1Back:
						clrscr();
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("Transaciton Type 1 screen Back -------"));
						}
						Selectionstatus = edebit;
						break;
				case  eChequeAccount:	//cheque account
						clrscr();
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("eChequeAccount -------"));
						}
						debitAccountType(ptrTransMsgDetails,Selectionstatus);
						Selectionstatus = eCashBackNo;			//Agency Bank
/*                 
#ifndef GOVT_APP //if application is government type
										SetImage(Cashback_BMP);//setting image for Account type 
										EnblTouchScreen(CashBack);
#endif
#ifdef GOVT_APP
										Selectionstatus = eCashBackNo;
#else
						//			Selectionstatus = SelectEvent;		//Agency Bank
#endif
		*/							
						break;
				case  eSavingAccount:	//saving account
						clrscr();
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("eSavingAccount -------"));
						}
						debitAccountType(ptrTransMsgDetails,Selectionstatus);
						Selectionstatus = eCashBackNo;		//Agency Bank
/*
#ifndef GOVT_APP //if application is government type
										SetImage(Cashback_BMP);//setting image for Account type 
										EnblTouchScreen(CashBack);
#endif
#ifdef GOVT_APP
										Selectionstatus = eCashBackNo;
#else
									//Selectionstatus = SelectEvent;		//Agency Bank
#endif
									*/
									
						break;
				case  eCreditAccount:	//credit type account
						clrscr();
						debitAccountType(ptrTransMsgDetails,Selectionstatus);
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("eCreditAccount -------"));
						}
									
#ifndef GOVT_APP //if application is government type
										SetImage(Cashback_BMP);//setting image for Account type 
										EnblTouchScreen(CashBack);
#endif
#ifdef GOVT_APP
										Selectionstatus = eCashBackNo;
#else
						//			Selectionstatus = SelectEvent;
#endif
									
						break;
				case  eOtherAccount:	//Other account
						OTHER_AC=1;
						clrscr();
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("eOtherAccount -------"));
							LOG_PRINTF(("OTHER_AC %d -------",OTHER_AC));
						}
						debitAccountType(ptrTransMsgDetails,Selectionstatus);
								//Agency Bank

						//Get Account number from Customer
						//iRetVal = getPaymentId(ptrTransMsgDetails);//getting Payment Id from user for Government
						iRetVal = getAccountNo(ptrTransMsgDetails);//getting Payment Id from user for Government
						if((iRetVal == (KEY_STR ) || (iRetVal == KEY_CANCEL) || (iRetVal == BTN_CANCEL) || (iRetVal == _FAIL)))
						{
							mainFlag = GoToMainScreen;
							break;
						}
					
						Selectionstatus = eCashBackNo;						

/*
#ifdef GOVT_APP
										Selectionstatus = eCashBackNo;
#else
						//			Selectionstatus = SelectEvent;		//Agency Bank
#endif
	*/								
						break;
				case  eCashBackYes:		//if taking cashback
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("amount type = %d",CASHBACK_AMOUNT));
						}
						mainFlag = InitSaleTransaction(ptrTransMsgDetails,CASHBACK_AMOUNT,swipeCardFlag);
						break;
				case  eCashBackNo:	//if no to taking cashback
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("amount type = %d",TRANSAMOUNT));
                        }
									
						mainFlag = InitSaleTransaction(ptrTransMsgDetails,TRANSAMOUNT,swipeCardFlag);
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("Getchar= %d",TRANSAMOUNT));
                        }
						break;
				case	eCashBack:	
						clrscr();
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("eCashBack Back -------"));
						}
						Selectionstatus = eTransaction;
						break;
				case  eAccTypeBack:	
						clrscr();
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("Back from Transaction Type ---"));
						}
						Selectionstatus = eTransaction;
						break;
				case  eMerchent:	//merchant menu
						SetImage(MERCHANT_MENU_BMP);
						EnblTouchScreen(MerchentMenu1);
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("eMerchent ---"));
						}
						Selectionstatus = SelectEvent;
						break;
				case  eLogOn:		//logon function
						clrscr();
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("Log On function --"));
						}
						InitLogon(ptrTransMsgDetails);
						mainFlag = GoToMainScreen;
						break;
				case  eSattlement:	//settlement 
						clrscr();
						iRetVal  = InitReversal();//check reversal data and if any go for reversal
						if(iRetVal == _SUCCESS)
						{
							ptrTransMsgDetails->TrTypeFlag = SETTLEMENT_MSG_TYPE_CASE;   
							InitSettlement(ptrTransMsgDetails); //send request for settlement of each and every transaction.
						}
						else
						{
							window(1,1,30,20);
							clrscr();
          		            write_at(Dmsg[CAN_NOT_DO_SETTLEMENT].dispMsg, strlen(Dmsg[CAN_NOT_DO_SETTLEMENT].dispMsg),Dmsg[GOING_FOR_REVERSAL].x_cor, Dmsg[CAN_NOT_DO_SETTLEMENT].y_cor);
				            write_at(Dmsg[PENDING_REVERSAL].dispMsg, strlen(Dmsg[PENDING_REVERSAL].dispMsg),Dmsg[PENDING_REVERSAL].x_cor, Dmsg[PENDING_REVERSAL].y_cor);
			                SVC_WAIT(2000);
					        ClearKbdBuff();
						    KBD_FLUSH();
				            mainFlag = GoToMainScreen;
							break;
			            }
						mainFlag = GoToMainScreen;
						break;
				case  ecopy:		
						clrscr();
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("Copy function -------"));
						}
						iRetVal = printAnotherCopy(ptrTransMsgDetails);//print another copy
						mainFlag = GoToMainScreen;
						break;
				case  eTotalReport:		
						clrscr();
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("TotalReport function -------"));
						}
						iRetVal =  TotalReportReciept("SUB TOTAL");
						mainFlag = GoToMainScreen;
						break;
				case  eMerchant1Back:		
						clrscr();
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("Merchant Back screen -------"));
						}
						mainFlag = GoToMainScreen;
						break;
				case  eMerchant1Next:	
						clrscr();
						SetImage(MERCHANT_MENU_BMP2);
						EnblTouchScreen(MerchentMenu2);
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("Merchant next screen -------"));
						}
						Selectionstatus = SelectEvent;
						break;
				case  eTransactionDetail:		
						clrscr();
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("Transaction Detail function -------"));
						}
						iRetVal =  TransactionDetailsReciept();
						mainFlag = GoToMainScreen;
						break;
				case  eKeyExchange:		
						clrscr();
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("KeyExchange function -------"));
						}
						write_at("KeyExchange function --", strlen("KeyExchange function --"),1, 10);
						mainFlag = GoToMainScreen;
						SVC_WAIT(1000);
						ClearKbdBuff();
			            KBD_FLUSH();
						break;
				case  eMerchant2Back:		
						clrscr();
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("Merchant next screen back-------"));
						}
						Selectionstatus = eMerchent;
						break;
				case  eSuperviser:		
						SetImage(Super_MENU_BMP);
						EnblTouchScreen(SupervisorMenu1);
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("eSuperviser -------"));
						}
						Selectionstatus = SelectEvent;
						break;
				case  ePassword:		
						clrscr();
						iRetVal = changePassword();//change password
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("Password function -------"));
						}
						mainFlag = GoToMainScreen;
						break;
				case  econfiguration:										
						Selectionstatus = dldConfiguration();
						if(Selectionstatus == KEY_STR)
							return KEY_STR;
						break;
				case ePINValidate:
						window(1,1,30,20);
						clrscr();
						if(!strcmp((char *)EncSessionKey,"")) 
						{
							write_at("AGENT",5,13,8);
							write_at(Dmsg[PLEASE_LOGON_FIRST].dispMsg, strlen(Dmsg[PLEASE_LOGON_FIRST].dispMsg),Dmsg[PLEASE_LOGON_FIRST].x_cor, Dmsg[PLEASE_LOGON_FIRST].y_cor);//LOGON SUCCESSFULL
							SVC_WAIT(2000);
							
						}
						else if (Sup_Login==1)
						{
							clrscr();
							write_at("SUPERVISOR",10 ,10, 9);
							write_at("ALREADY LOGGED IN.",18 ,6, 10);
							SVC_WAIT(2000);
						}
						else
						{
							ptrTransMsgDetails->TrMethordFlag = edebit;
							if(LOG_STATUS == LOG_ENABLE)
							{
								LOG_PRINTF(("Supervisor Login -------"));
							}
							iRetVal=InitPINValidate(ptrTransMsgDetails);
							if (iRetVal==_SUCCESS)
								Sup_Login=1;
						}
						ClearKbdBuff();
						KBD_FLUSH();
						mainFlag = GoToMainScreen;
						break;
				case eLogout:
					clrscr();
					if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("Supervisor LOGOUT -------"));
						}
						Sup_Login=0;
						write_at("SUPERVISOR LOGING OUT....",24 ,3, 10);
						mainFlag = GoToMainScreen;
						SVC_WAIT(2000);
						ClearKbdBuff();
			            KBD_FLUSH();
						break;

				/*case  eDownload:		
						clrscr();
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("Download function -------"));
						}
						write_at("Download function --", strlen("Download function --"),1, 10);
						mainFlag = GoToMainScreen;
						SVC_WAIT(1000);
						ClearKbdBuff();
			            KBD_FLUSH();
						break;*/
				/*

				case  eKeyDownload:		
						clrscr();
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("KeyDownload function -------"));
						}
						write_at("KeyDownload function --", strlen("KeyDownload function --"),1, 10);
						mainFlag = GoToMainScreen;
						SVC_WAIT(1000);
						ClearKbdBuff();
			            KBD_FLUSH();
						break;
						*/
				case  eSuper1Back:		
						clrscr();	
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("Supervisor  Back screen ---"));
						}
						mainFlag = GoToMainScreen;
						break;
				case  eSuper1Next:	
						clrscr();
						SetImage(Super_MENU_BMP2);
						EnblTouchScreen(SupervisorMenu2);
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("Supervisor Next screen -------"));
						}
						Selectionstatus = SelectEvent;		
						break;
				case  eHelpDesk:		
						clrscr();
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("HelpDesk function -------"));
						}
						write_at("HelpDesk function --", strlen("HelpDesk function --"),1, 10);
						mainFlag = GoToMainScreen;
						SVC_WAIT(1000);
						ClearKbdBuff();
			            KBD_FLUSH();
						break;
				case  eSuper2Back:	
						clrscr();
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("Supervisor Next screen Back -------"));
						}
						Selectionstatus = eSuperviser;
						break;
				case eCardActivation:
						clrscr();
						if (LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("Card Activation -------"));
						}
						//iRetVal = InitReversal();//check reversal data and if any go for reversal
						if (iRetVal == _SUCCESS)
						{
							ptrTransMsgDetails->TrMethordFlag = edebit;
							ptrTransMsgDetails->TrTypeFlag = ACTIVATIONMSGTYPE_CASE;
							iRetVal = InitCardActivation(ptrTransMsgDetails);
						}
						else
						{
							window(1, 1, 30, 20);
							clrscr();
							write_at(Dmsg[CAN_NOT_DO_REFUND].dispMsg, strlen(Dmsg[CAN_NOT_DO_REFUND].dispMsg), Dmsg[CAN_NOT_DO_REFUND].x_cor, Dmsg[CAN_NOT_DO_REFUND].y_cor);
							write_at(Dmsg[PENDING_REVERSAL].dispMsg, strlen(Dmsg[PENDING_REVERSAL].dispMsg), Dmsg[PENDING_REVERSAL].x_cor, Dmsg[PENDING_REVERSAL].y_cor);
							SVC_WAIT(2000);
							ClearKbdBuff();
							KBD_FLUSH();
							mainFlag = GoToMainScreen;
							break;
						}
						mainFlag = GoToMainScreen;
						break;
				case ePinChange:
						clrscr();
						if (LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("Pin Change -------"));
						}
						//iRetVal = InitReversal();//check reversal data and if any go for reversal
						if (iRetVal == _SUCCESS)
						{
							pin_change = 1;
							ptrTransMsgDetails->TrMethordFlag = edebit;
							ptrTransMsgDetails->TrTypeFlag = PINCHANGEMSGTYPE_CASE;
							iRetVal = InitPinChange(ptrTransMsgDetails);
						}
						else
						{
							window(1, 1, 30, 20);
							clrscr();
							write_at(Dmsg[CAN_NOT_DO_REFUND].dispMsg, strlen(Dmsg[CAN_NOT_DO_REFUND].dispMsg), Dmsg[CAN_NOT_DO_REFUND].x_cor, Dmsg[CAN_NOT_DO_REFUND].y_cor);
							write_at(Dmsg[PENDING_REVERSAL].dispMsg, strlen(Dmsg[PENDING_REVERSAL].dispMsg), Dmsg[PENDING_REVERSAL].x_cor, Dmsg[PENDING_REVERSAL].y_cor);
							SVC_WAIT(2000);
							ClearKbdBuff();
							KBD_FLUSH();
							mainFlag = GoToMainScreen;
							break;
						}
						mainFlag = GoToMainScreen;
						break;

				default:
						getKey = Selectionstatus;
						if(LOG_STATUS == LOG_ENABLE)
						{
							LOG_PRINTF(("defauktk -------%d",getKey));
						}
						if((Selectionstatus != KEY_STR) && (Selectionstatus != KEY_CANCEL))
						{
							Selectionstatus = SelectEvent;
						}
						else if(Selectionstatus == KEY_STR)
							Selectionstatus = KEY_STR;
						else if(Selectionstatus == KEY_CANCEL)
							mainFlag = GoToMainScreen;

		}		//End if Switch
		ClearKbdBuff();
		KBD_FLUSH();
	}
	return Selectionstatus;
}

int checkPaperStatus()
{
  char ch = 0;
  int iRetVal =0 ;
  LOG_PRINTF(("paperOutFlag = %d",paperOutFlag));
  iRetVal = p3300_status(hPrinter,1000);
  LOG_PRINTF(("p3300_status iRetVal = %d",iRetVal));
  if(iRetVal == PAPER_OUT)
  {
    paperOutFlag=1;
  }
  else
  {
    paperOutFlag=0;//reset print flag when paper is out
  } 

  if(paperOutFlag == 1) 
  {
      window(1,1,30,20);
			clrscr();
			error_tone();
			write_at("INSERT PAPER ROLL", strlen("INSERT PAPER ROLL"),(30-strlen("INSERT PAPER ROLL"))/2, 10);
      write_at("CONTINUE WITHOUT PAPER?", strlen("CONTINUE WITHOUT PAPER?"),(30-strlen("CONTINUE WITHOUT PAPER?"))/2, 11);
			write_at(Dmsg[PRESS_ENTER_FOR_YES].dispMsg, strlen(Dmsg[PRESS_ENTER_FOR_YES].dispMsg),Dmsg[PRESS_ENTER_FOR_YES].x_cor, Dmsg[PRESS_ENTER_FOR_YES].y_cor);
      write_at(Dmsg[PRESS_CANCEL_FOR_NO].dispMsg, strlen(Dmsg[PRESS_CANCEL_FOR_NO].dispMsg),Dmsg[PRESS_CANCEL_FOR_NO].x_cor, Dmsg[PRESS_CANCEL_FOR_NO].y_cor);
      ClearKbdBuff();
     // KBD_FLUSH();
      do
	    {
		    ch =get_char();
	    }while((ch != KEY_CR) && (ch!= KEY_CANCEL) && (ch!= KEY_STR));
			LOG_PRINTF(("ch = %d",ch));	           
      if(ch == KEY_CR)
      {
        CheckPaperFlag =  1 ;
        return _SUCCESS ;          
      }
      else
      {
        return _FAIL ;          
      }
  }

  return _SUCCESS ;      
}
