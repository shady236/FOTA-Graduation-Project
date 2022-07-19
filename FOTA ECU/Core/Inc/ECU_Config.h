
#ifndef INC_ECU_CONFIG_H_
#define INC_ECU_CONFIG_H_

#include "STD_types.h"
#include "FPEC.h"


#define  ECUS_CONFIG_BASE_PAGE_NUMBER    100

#define  ECUS_NUM		1


/*
 * Each ECU has the following configurations:
 *    - Bootloader  version
 *    - Bootloader  Active region
 *    - Application version
 *    - Application Active region
 *
 *
 * So, ECU 0 configurations are:
 *    - Bootloader  version         -->   stored in a byte with offset 0
 *    - Bootloader  Active region   -->   stored in a byte with offset 1
 *    - Application version         -->   stored in a byte with offset 2
 *    - Application Active region   -->   stored in a byte with offset 3
 *
 * ECU 1 configurations are:
 *    - Bootloader  version         -->   stored in a byte with offset 4
 *    - Bootloader  Active region   -->   stored in a byte with offset 5
 *    - Application version         -->   stored in a byte with offset 6
 *    - Application Active region   -->   stored in a byte with offset 7
 *
 * ...
 *
 * ECU i configurations are:
 *    - Bootloader  version         -->   stored in a byte with offset 4 * i     = i<<2
 *    - Bootloader  Active region   -->   stored in a byte with offset 4 * i + 1 = i<<2 + 1
 *    - Application version         -->   stored in a byte with offset 4 * i + 2 = i<<2 + 2
 *    - Application Active region   -->   stored in a byte with offset 4 * i + 3 = i<<2 + 3
 *
 *
 * Note that, ECU 255 configurations are:
 *    - Bootloader  version         -->   stored in a byte with offset 4 * 255     = 1020
 *    - Bootloader  Active region   -->   stored in a byte with offset 4 * 255 + 1 = 1021
 *    - Application version         -->   stored in a byte with offset 4 * 255 + 2 = 1022
 *    - Application Active region   -->   stored in a byte with offset 4 * 255 + 3 = 1023
 *
 * next ECU which is ECU 256 configurations can't be stored in the same page as the page
 * has only 1024 bytes, it should start from the next page
 * So, ECU 256 configurations are:
 *    - Bootloader  version         -->   stored in a byte with offset 4 * 256     = 1024
 *      byte offset --> 1024 % 1024 = 0       page number --> 1024 / 1024 = 1 + base
 *
 *    - Bootloader  Active region   -->   stored in a byte with offset 4 * 256 + 1 = 1025
 *      byte offset --> 1025 % 1024 = 1       page number --> 1025 / 1024 = 1 + base
 *
 *    - Application version         -->   stored in a byte with offset 4 * 256 + 2 = 1026
 *      byte offset --> 1026 % 1024 = 2       page number --> 1026 / 1024 = 1 + base
 *
 *    - Application Active region   -->   stored in a byte with offset 4 * 256 + 3 = 1027
 *      byte offset --> 1027 % 1024 = 3       page number --> 1027 / 1024 = 1 + base
 *
 *
 * So, ECU i configurations are:
 *    - Bootloader  version:
 *         - byte offset (4 * i) % 1024
 *         - page number (4 * i) / 1024 + base
 *
 *    - Bootloader  Active region:
 *         - byte offset (4 * i + 1) % 1024
 *         - page number (4 * i + 1) / 1024 + base
 *
 *    - Application version:
 *         - byte offset (4 * i + 2) % 1024
 *         - page number (4 * i + 2) / 1024 + base
 *
 *    - Application Active region:
 *         - byte offset (4 * i + 3) % 1024
 *         - page number (4 * i + 3) / 1024 + base
 */


/*
 * Bootloader  version for ECU i:
 *   - byte offset (4 * i) % 1024
 *   - page number (4 * i) / 1024 + base
 */
#define  BOOT_VERSION_BYTE_OFFSET(ecuNum)   	(((ecuNum)<<2UL) % 1024UL)
#define  BOOT_VERSION_PAGE_NUMBER(ecuNum)    ((((ecuNum)<<2UL) / 1024UL) + ECUS_CONFIG_BASE_PAGE_NUMBER)

/*
 * Bootloader  Active region for ECU i:
 *   - byte offset (4 * i + 1) % 1024
 *   - page number (4 * i + 1) / 1024 + base
 */
#define  ACTIVE_BOOT_BYTE_OFFSET(ecuNum)   	((((ecuNum)<<2UL) + 1UL) % 1024UL)
#define  ACTIVE_BOOT_PAGE_NUMBER(ecuNum)     (((((ecuNum)<<2UL) + 1UL) / 1024UL) + ECUS_CONFIG_BASE_PAGE_NUMBER)


/*
 * Application version for ECU i:
 *   - byte offset (4 * i + 2) % 1024
 *   - page number (4 * i + 2) / 1024 + base
 */
#define  APP_VERSION_BYTE_OFFSET(ecuNum)   	((((ecuNum)<<2UL) + 2UL) % 1024UL)
#define  APP_VERSION_PAGE_NUMBER(ecuNum)     (((((ecuNum)<<2UL) + 2UL) / 1024UL) + ECUS_CONFIG_BASE_PAGE_NUMBER)


/*
 * Application Active region for ECU i:
 *    - byte offset (4 * i + 3) % 1024
 *    - page number (4 * i + 3) / 1024 + base
 */
#define  ACTIVE_APP_BYTE_OFFSET(ecuNum)   	((((ecuNum)<<2UL) + 3UL) % 1024UL)
#define  ACTIVE_APP_PAGE_NUMBER(ecuNum)     (((((ecuNum)<<2UL) + 3UL) / 1024UL) + ECUS_CONFIG_BASE_PAGE_NUMBER)



typedef struct
{
	u8 activeAppRegion;
	u8 activeBootRegion;
	u8 appVersion;
	u8 bootVersion;
}ECU_Config_t;



typedef enum
{
	APP  = 'A',
	BOOTLOADER = 'B'
}ImgType_t;



typedef enum
{
	APP_UPDATES_AVAILABLE,
	BOOT_UPDATES_AVAILABLE,
	NO_UPDATES_AVAILABLE
}ECU_UpdatesStatus_t;


void  ECU_LoadConfig(void);
void  ECU_UpdateConfig(u8 ecuNum);


#endif /* INC_ECU_CONFIG_H_ */
