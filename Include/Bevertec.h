/*-------------------------------------------------------------------
 *
 *	Filename	: Bevertec.H
 *
 *	Description	: This file contains prototypes for the functions
 *
 *  Modification History:
 *   #		Date        		Who				Comments
 *	1.0		12/25/2015 8:07:48 PM	   vikask3		Initial Implementation
 *
 *-------------------------------------------------------------------*/

/**********************************************************************
   Copyright (C) 2008-2010 by VeriFone Inc. All rights reserved.

 No part of this software may be used, stored, compiled, reproduced,
 modified, transcribed, translated, transmitted, or transferred, in
 any form or by any means  whether electronic, mechanical,  magnetic,
 optical, or otherwise, without the express prior written permission
                           of VeriFone, Inc.
************************************************************************/


// Function Prototypes
void    ReturnPrinter(void);
void    ReturnMagCard(void);
void    PrinterInit(void);
void    Print(char *str);
void    PrintCardData(void);
short    ShowIdle(void);
short   inEndTable(short state);
short   inActivateEvtResponder(void);
short   inDeactivateEvtResponder(void);
short   ReadCardHandler(short state);
short   appl_idle_loop(void);
short   slow_poll(void);
short   fast_poll(void);
short   F4KeyHandler(short state);
short   ResNotAvailableHandler(short state);
void    ReturnPrinter(void);
short   PRN_Request_Handler(short state);
short   MAG_Request_Handler(short state);
void    ReturnMagCard(void);
short   PRN_Release_Handler(short state);
short   MAG_Release_Handler(short state);
short   appl_coldboot(void);
short   F1KeyHandler(short state);
short   InitHandler(short state);
short   ActEventHandler(short state);
short   PipeHandler(short sh);
short   DeactivateEventHandler(short state);
short   DefaultActivateHandler(short state);
short   appl_coldboot(void);



