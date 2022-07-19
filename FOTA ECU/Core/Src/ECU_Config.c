#include "ECU_Config.h"


ECU_Config_t  ecusConfig[ECUS_NUM];


void ECU_LoadConfig(void)
{
	FPEC_InitFlash();

	for(u8 ecuNum = 0; ecuNum < ECUS_NUM; ecuNum++)
	{
		ecusConfig[ecuNum].activeAppRegion  =  FPEC_ReadByte(
			ACTIVE_APP_PAGE_NUMBER(ecuNum), ACTIVE_APP_BYTE_OFFSET(ecuNum)
		);

		ecusConfig[ecuNum].activeBootRegion  =  FPEC_ReadByte(
			ACTIVE_BOOT_PAGE_NUMBER(ecuNum), ACTIVE_BOOT_BYTE_OFFSET(ecuNum)
		);

		ecusConfig[ecuNum].appVersion  =  FPEC_ReadByte(
			APP_VERSION_PAGE_NUMBER(ecuNum), APP_VERSION_BYTE_OFFSET(ecuNum)
		);

		ecusConfig[ecuNum].bootVersion =  FPEC_ReadByte(
			BOOT_VERSION_PAGE_NUMBER(ecuNum), BOOT_VERSION_BYTE_OFFSET(ecuNum)
		);
	}
}



void  ECU_UpdateConfig(u8 ecuNum)
{
	FPEC_WriteByte(
		ACTIVE_APP_PAGE_NUMBER(ecuNum), ACTIVE_APP_BYTE_OFFSET(ecuNum), ecusConfig[ecuNum].activeAppRegion
	);

	FPEC_WriteByte(
		ACTIVE_BOOT_PAGE_NUMBER(ecuNum), ACTIVE_BOOT_BYTE_OFFSET(ecuNum), ecusConfig[ecuNum].activeBootRegion
	);

	FPEC_WriteByte(
		APP_VERSION_PAGE_NUMBER(ecuNum), APP_VERSION_BYTE_OFFSET(ecuNum), ecusConfig[ecuNum].appVersion
	);

	FPEC_WriteByte(
		BOOT_VERSION_PAGE_NUMBER(ecuNum), BOOT_VERSION_BYTE_OFFSET(ecuNum), ecusConfig[ecuNum].bootVersion
	);
}
