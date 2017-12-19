#ifndef ISO_8583_MAP_H
#define ISO_8583_MAP_H
#include <iso8583.h>
#include "..\Include\TouchScreen.h"

#define UNPACK_ASSEMBLE_ARR_SIZE 500 

#define AMOUNT_TYPE1_LEN 2 
#define ACCOUNT_TYPE1_LEN 2
#define ACCOUNT_CURRENCY1_LEN 3
#define AMOUNT1_LEN 13 

#define AMOUNT_TYPE2_LEN 2 
#define ACCOUNT_TYPE2_LEN 2 
#define ACCOUNT_CURRENCY2_LEN 3
#define AMOUNT2_LEN 13

typedef struct parseAmount
{
	char LedgerAmount[13]; //LEDGER AMOUNT
  char AvailAmount[13]; //AVAILABL amount

}parsedAmount;

int main8583(void);
void map_man(char *, ...);
//unsigned int return_variant1();
//unsigned int return_variant2(char *);
void map_clear(unsigned char *map, int max_fn);
//int process_8583(int how, field struct *field_tbl, unsigned char *map,
//unsigned char *buffer, int limit);
////////////////////////////////////////////////////////////////////


void bcd2a(char *,char *,short);
int compute_02 (int,unsigned char *,int *,int *);
short ProcessingISOBitmapEngine(field_struct * ,TransactionMsgStruc *);
void CreateRequestStream(TransactionMsgStruc *);
void SetingBitMap(TransactionMsgStruc *);
int assemble_packet(field_struct *,unsigned char*) ;
int disassemble_packet(field_struct *);
int set_tpdu_length(int );// Variables for the Application
void PrintAllBits(unsigned char *);
int ParseAndFillBitFields(char * ,char *) ;
void CopyAllBits(char *,char *) ;
short parseAmountResponse(parsedAmount *);
short paddAmount(char *,char *);
#endif
