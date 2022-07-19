
#ifndef INC_ECU_CONFIG_H_
#define INC_ECU_CONFIG_H_

#include "STD_types.h"
#include "FPEC.h"


#define  ECU_CONFIG_PAGE_NUMBER    5

#define  ECU_NUM				0


#define  BRANCHING_BYTE_OFFSET      0
#define  BRANCHING_PAGE_NUMBER      ECU_CONFIG_PAGE_NUMBER

#define  ACTIVE_BOOT_BYTE_OFFSET    1
#define  ACTIVE_BOOT_PAGE_NUMBER    ECU_CONFIG_PAGE_NUMBER

#define  ACTIVE_APP_BYTE_OFFSET     2
#define  ACTIVE_APP_PAGE_NUMBER     ECU_CONFIG_PAGE_NUMBER


#define  BOOT1_STRT_PAGE            6
#define  BOOT1_PAGES_SIZE           11
#define  BOOT1_END_PAGE             (BOOT1_STRT_PAGE + BOOT1_PAGES_SIZE - 1)

#define  BOOT2_STRT_PAGE            17
#define  BOOT2_PAGES_SIZE           11
#define  BOOT2_END_PAGE             (BOOT2_STRT_PAGE + BOOT2_PAGES_SIZE - 1)

#define  APP1_STRT_PAGE             28
#define  APP1_PAGES_SIZE            50
#define  APP1_END_PAGE              (APP1_STRT_PAGE + APP1_PAGES_SIZE - 1)

#define  APP2_STRT_PAGE             78
#define  APP2_PAGES_SIZE            50
#define  APP2_END_PAGE              (APP2_STRT_PAGE + APP2_PAGES_SIZE - 1)



typedef enum
{
	APP        = 'A',
	BOOTLOADER = 'B'
}ImgType_t;



typedef struct
{
	u8  activeAppRegion;
	u8  activeBootRegion;
}ECU_Config_t;


void  ECU_LoadConfig(void);
void  ECU_UpdateConfig(void);
void  ECU_ConfigActivateApp(void);



#endif /* INC_ECU_CONFIG_H_ */
