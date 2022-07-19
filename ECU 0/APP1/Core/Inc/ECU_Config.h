
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



typedef enum
{
	APP        = 'A',
	BOOTLOADER = 'B'
}ImgType_t;



void  ECU_ConfigActivateBoot(void);


#endif /* INC_ECU_CONFIG_H_ */
