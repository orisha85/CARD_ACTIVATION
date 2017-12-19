
#include <EMVCWrappers.h>

typedef struct TagEMV_NEW_LABEL
{
	char  szLabel[1+MAX_LABEL_LEN];		
	int   inOrgLabelPos;					
	int   inAppPriority;				
} EMV_NEW_LABEL;

#define MAX_MENU_ITEMS			4

void EMVDisplayFunc(char *);
void vdPromptManager(unsigned short);
int inGetPTermID(char *ptid);
EMVResult usEMVIssAcqCVM(unsigned short issacq, unsigned short *code);
unsigned short usEMVPerformSignature(void);
EMVResult getLastTxnAmt(Ulong *amt);
void usEMVDisplayErrorPrompt(unsigned short errorID);
short getUsrPin(unsigned char *pin);
unsigned short usGetExtAuthDecision(unsigned short usStatusWord);
unsigned short usEMVPerformOnlinePIN(void);
int inMenuFunc(char **labels, int numLabels);
void sortApplnsListOnPriority(char **labels, int inNumOfLabels, EMV_NEW_LABEL srNewMenuArray[]);
int emvRemoveCard(void);
void hex2asc(unsigned char *outp, unsigned char *inp, int length);
void vdPromptRemoveManager(unsigned short);
void displayPINPrompt(void);
void vdSetPinParams (void);
short fEMVIsItUnrecognizedCVM(byte bCVM);
int ConvertAmount(char *EnteredAmount ,unsigned long * ulAmount);
void PropmptForFallBack(void);
void bcd2a(char *dest ,char *src ,unsigned short bcdlen);
static int iCorrect_track2_buffer(char * track2,int track2Length);
//int createScriptBuffers(byte *scriptBuf, int iLen, short iType);
short getNextRawTLVData(unsigned short *tag, byte *data, const byte *buffer);
int createScriptFiles(byte *buffer);

void displayAt(int x,int y,const char* m,int o) ;
void dispStrAtRight(char* pchInStr,int iYCord,int iClearDp) ;
/// unsigned short debitAccountSelection(char *transTypeBuff);



