#ifndef SUPERVISOR_H
#define SUPERVISOR_h

#define SUPER_PWD_LEN 6
#define MERCHANT_PWD_LEN 4

#define HOST_IP_LEN 25
#define PORT_NO_LEN 6
#define MERCHANT_ID_LEN 13
#define MERCHANT_NAME_LEN 30
#define TID_LEN 11
#define MASTER_KEY             "A1D0D31016A40BD649E52AD09EC716AB"

#define USER_AUDIT_FILE	"F:15/UserAudit.txt"

typedef struct
{
	char operID[9];
	char operPWD[13];
} Operator;

typedef struct
{
	char operID[9];
	char code;
	char Date[9];				// format date
	char Time[9];				// format time

}UserAudit;

//char pwd_map[] = {"", };

short checkpassword(char *temppassword);
short changePassword(void);
short dldConfiguration(void);
short Download(void);
short KeyDownload(void);
short validateSupervisorPassword(void);
short validateMerchantPassword(void);
short AddOperator();
short DeleteOperator();
short ChangeOperatorPassword();
short ChangeSupervisorPassword();
short ResetOperatorPassword();
short ResetSupervisorPassword();
short ListOperators();
short validateOperatorPassword();
short promptpin(int pin_set);
short saveUserdtls(char opt, char* oper);

#endif 
