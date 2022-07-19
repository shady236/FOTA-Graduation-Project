#include "ECU_Config.h"

ECU_Config_t  ecuConfig;


void  ECU_LoadConfig(void)
{
	FPEC_InitFlash();

	ecuConfig.activeImg         =  FPEC_ReadByte(BRANCHING_PAGE_NUMBER  , BRANCHING_BYTE_OFFSET  );
	ecuConfig.activeAppRegion   =  FPEC_ReadByte(ACTIVE_APP_PAGE_NUMBER , ACTIVE_APP_BYTE_OFFSET );
	ecuConfig.activeBootRegion  =  FPEC_ReadByte(ACTIVE_BOOT_PAGE_NUMBER, ACTIVE_BOOT_BYTE_OFFSET);
}


void  ECU_UpdateConfig(void)
{
	FPEC_WriteByte(BRANCHING_PAGE_NUMBER  , BRANCHING_BYTE_OFFSET  , ecuConfig.activeImg);
	FPEC_WriteByte(ACTIVE_APP_PAGE_NUMBER , ACTIVE_APP_BYTE_OFFSET , ecuConfig.activeAppRegion);
	FPEC_WriteByte(ACTIVE_BOOT_PAGE_NUMBER, ACTIVE_BOOT_BYTE_OFFSET, ecuConfig.activeBootRegion);
}

