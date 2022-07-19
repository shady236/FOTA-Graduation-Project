#include "updates.h"
#include "RCC.h"


extern ECU_Config_t  ecuConfig;


void jumpToApp(void);


int main(void)
{
	RCC_InitClock();
	updatesInit();
	NVIC_UnMaskIRQs();

	ImgType_t  imgType;
	Boolean_t  state = updateImg(&imgType);
	if(state == TRUE)
	{
		switch(imgType)
		{
			case APP:
				if (ecuConfig.activeAppRegion == 1)
					ecuConfig.activeAppRegion = 2;
				else
					ecuConfig.activeAppRegion = 1;
			break;

			case BOOTLOADER:
				if (ecuConfig.activeBootRegion == 1)
					ecuConfig.activeBootRegion = 2;
				else
					ecuConfig.activeBootRegion = 1;
			break;
		}

		ECU_UpdateConfig();
	}

	jumpToApp();

	while (1)
	{

	}

	return 0;
}


void jumpToApp(void)
{
	ECU_ConfigActivateApp();
	NVIC_MaskIRQs(0);
	NVIC_ResetSystem();
}

