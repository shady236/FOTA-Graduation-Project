#include "ECU_Config.h"


void  ECU_ConfigActivateBoot(void)
{
	FPEC_InitFlash();
	FPEC_WriteByte(BRANCHING_PAGE_NUMBER, BRANCHING_BYTE_OFFSET, BOOTLOADER);
}
