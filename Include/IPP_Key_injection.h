#ifndef _IPP_KEY_INJECTION_
#define _IPP_KEY_INJECTION_

#define COM1_DEVICE			2 /* It is an alternative for PINPAD_INTERNAL & PINPAD_EXTERNAL */


/* PP_COMM possible return codes */
#define PP_COMM_OK_NO_ACK	1
#define PP_COMM_OK			0
#define PP_COMM_FAILURE		-1
#define PP_COMM_CANCELLED	-2
#define PP_COMM_TIMEOUT     -3

#define CRC_MASK        0x1021   /* x^16 + x^12 + x^5 + x^0 */


// Timer Constants
#define COPY_TICKS		1
#define CHECK_TICKS		0 
///

/* Protocol Characters */
#ifndef SI
#define SI			(unsigned char)0x0F	/* shift in */
#endif
#ifndef SO
#define SO			(unsigned char)0x0E	/* shift out */
#endif
#ifndef STX
#define STX			(unsigned char)0x02
#endif
#ifndef ETX
#define ETX			(unsigned char)0x03
#endif
#ifndef EOT
#define EOT			(unsigned char)0x04	/* end of transaction */
#endif
#ifndef ACK
#define ACK			(unsigned char)0x06	/* acknowledge */
#endif
#ifndef NAK
#define NAK			(unsigned char)0x15	/* not acknowledge */
#endif

#define KEY_RELEASE 0
#define KEY_PRESS 1
#define KEY_KEEPING_PRESSED 2

//#define APP_LOGIC_FIL	1<<1
//#define IPP_COMM_FIL	1<<2
//#define	TRUE     1
//#define	FALSE    0
#define IPP_BUFFER_SIZE         1024

typedef	unsigned char	Bool;
#define c3DES '1' //Change to '0' for DES and '1' for 3DES
/* PP_COMM function prototypes */
int  iPIN_Open          (int iDevice, int iBaud, int iFormat, int *piHandle);
int iPIN_Send (int iHandle, unsigned char ucPcktStart, unsigned char *pucMessage, int iSize, int iTimeout);

//Breaking the Key Injection and Pin prompt in two different functions
int KeyInjection(char *PrimaryAccNO);
int PinPrompt(char *PrimaryAccNO);


//int ProcessingKeyInjection(char *PrimaryAccNO); //Not being used anymore

void vdRunIppMSSequence(void);
void vdPIN_Close (int iHandle);
int open_ipp_port (void);
int iPIN_Receive (int iHandle, unsigned char *pucMessage, int *piLen, int iTimeout);
static void SerialClear (int iHandle);
int ipp_communicate ( int iHdl, unsigned char byPcktStart, unsigned char *pucMsgIn, unsigned char *pucMsgOut, Bool bEOT);
int iPIN_ReceivePacket (int iHandle, unsigned char *pucMessage, int *piLen, int iTimeout);
static int iPIN_VerifyPacket (int iHandle, unsigned char *pucMessage, int *piLen);
int iPIN_SendByte (int iHandle, unsigned char ucByte);

void parseEncryptedPinBlock(unsigned char *pucMsgOut);

//Session key Related (Working Key)
#define MIN_PIN_LEN "04"
#define MAX_PIN_LEN "12"
#define NULL_PIN_YES 'Y'
#define NULL_PIN_NO 'N'
#define ECHO_CHAR '*'
#define PIN_BLOCK_LENGTH 16
#define KSN_PP_LENGTH 24

///////////////////////////////////////////
#define INIT_VECTOR_LEN 8
#define MAC_BLOCK_LEN 8
#define DATA_BLOCK_LEN 8
//After 3des cbc 

#define  KEY_1     0  // Use in key generation
#define  KEY_2     8
#define  KEY_3     16 

#define PACK_KEY_SIZE               16
#define KEY_SIZE                    32
///////////////////////////////////////////////
#define BEFORE_MASTER_KEY_PART "20120K0T443030303030303030303030303030303030304E30303332"

#define MASTER_KEY             "A1D0D31016A40BD649E52AD09EC716AB"

//#define MASTER_KEY             "67B526322F169D26EF3D762FE361A731"
#define AFTER_MASTER_KEY_PART  "FFFFFFFFFFFFFFFF0000000000000000" 
#define INITIAL_VECTOR         "3230313230503054"
#define UNENC_AFTER_HEADER     "4530303030303030"
#define UNENC_RESERVED         "3030303030303030"
#define UNENC_LENGTH_PART      "3030304E30303332"
#define UNENC_PADDEDKEY3_PART  "FFFFFFFFFFFFFFFF"
////////////////////////////////////////////////
//TODO: Hardcoded IPEK and KSN added for DUKPT support, needs to be replaced with values matching Host config
//This IPEK needs to be derived from KSN+BDK
//YS - 08-17-2017
#define IPEK "0D1BEB6B6D7B3234CAFF937786E3F124"
#define KSN "9500030000000D200516"



short MakingGiske(unsigned char *, unsigned char *,unsigned char *,unsigned char *) ;
short TDES_CBC(unsigned char *,unsigned char *,unsigned char *,unsigned char *) ;
short TDES_CBC_MAC(unsigned char *,unsigned char *,unsigned char *,unsigned char *);
short EncryptingKeyBlocks(unsigned char *Input_Data,unsigned char *Output_Data,unsigned char *MAC_Output_Data,unsigned char *Key1,unsigned char *Key2,unsigned char *IVec,unsigned char *MAC_IVec,unsigned char *Temp,int size) ;
short TDES_Decrypt_Logon_key(unsigned char *Input_Data,unsigned char *Output_Data,unsigned char *) ;
short TDES_CBC16(unsigned char *Input_Data,int iInputlen,unsigned char *Output_Data,int *iOutLen,unsigned char *Iv,unsigned char *Key,int enc);

#endif
