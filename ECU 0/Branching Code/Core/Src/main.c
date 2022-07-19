#include "ECU_Config.h"
#include "RCC.h"
#include "NVIC.h"
#include "FPEC.h"


extern ECU_Config_t  ecuConfig;


void  jumpToApp(void);
void  jumpToBoot(void);


int main(void)
{
	NVIC_MaskIRQs(1);
	RCC_InitClock();

	ECU_LoadConfig();

	/* Jump to destination */
	switch(ecuConfig.activeImg)
	{
		case APP:
			jumpToApp();
		break;

		case BOOTLOADER:
		default:
			jumpToBoot();
		break;
	}

	ecuConfig.activeBootRegion = 1;
	jumpToBoot();

	ecuConfig.activeBootRegion = 2;
	jumpToBoot();

	while(1)
	{

	}

}



void  jumpToApp(void)
{
	ecuConfig.activeImg = APP;

	u8 distAppStrtPage;
	switch(ecuConfig.activeAppRegion)
	{
		case 1:
			distAppStrtPage = APP1_STRT_PAGE;
		break;

		case 2:
			distAppStrtPage = APP2_STRT_PAGE;
		break;

		default:
			jumpToBoot();
		break;
	}

	void (*disResetHandler)(void) = (void*) FPEC_ReadWord(distAppStrtPage, 4);
	if((u32)disResetHandler == 0xFFFFFFFF)   // If there's NO App in this destination, jump to active Bootloader
	{
		jumpToBoot();
	}
	else    // If there's App in this destination
	{
		ECU_UpdateConfig();
		NVIC_RealocateVectorTable(FLASH_REGION, FLASH_BASE + distAppStrtPage * PAGE_SIZE_BYTES);
		__set_CONTROL(0);     // Privilege mode + use MSP as Stack Pointer
		__set_MSP(FPEC_ReadWord(distAppStrtPage, 0));    // Read initial Stack Pointer value, load it in MSP
		disResetHandler();   // direct jump to destination App
	}
}


void  jumpToBoot(void)
{
	ecuConfig.activeImg = BOOTLOADER;

	u8 distBootStrtPage;
	switch(ecuConfig.activeBootRegion)
	{
		case 1:
			distBootStrtPage = BOOT1_STRT_PAGE;
		break;

		case 2:
			distBootStrtPage = BOOT2_STRT_PAGE;
		break;

		default:
			ecuConfig.activeBootRegion = 1;
			distBootStrtPage = BOOT1_STRT_PAGE;
		break;
	}

	void (*disResetHandler)(void) = (void*) FPEC_ReadWord(distBootStrtPage, 4);
	if((u32)disResetHandler != 0xFFFFFFFF)   // If there's Bootloader in this destination
	{
		ECU_UpdateConfig();
		NVIC_RealocateVectorTable(FLASH_REGION, FLASH_BASE + distBootStrtPage * PAGE_SIZE_BYTES);
		__set_CONTROL(0);     // Privilege mode + use MSP as Stack Pointer
		__set_MSP(FPEC_ReadWord(distBootStrtPage, 0));    // Read initial Stack Pointer value, load it in MSP
		disResetHandler();   // direct jump to destination App
	}
}

