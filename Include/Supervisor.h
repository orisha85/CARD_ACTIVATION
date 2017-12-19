#ifndef SUPERVISOR_H
#define SUPERVISOR_h

#define SUPER_PWD_LEN 6
#define MERCHANT_PWD_LEN 4

#define HOST_IP_LEN 25
#define PORT_NO_LEN 6
#define MERCHANT_ID_LEN 13
#define MERCHANT_NAME_LEN 30
#define TID_LEN 11

short changePassword(void);
short dldConfiguration(void);
short Download(void);
short KeyDownload(void);
short validateSupervisorPassword(void);
short validateMerchantPassword(void);
#endif 
