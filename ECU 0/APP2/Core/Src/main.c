#include "RCC.h"
#include "GPIO.h"
#include "updates.h"


void delay(u32 time_ms);
void ledInit(void);
void ledToggle(u32 delayTime);
void jumpToBoot(void);


int main(void)
{
	RCC_InitClock();

	updatesInit();
	updatesSetState(READY_FOR_UPDATES, jumpToBoot);
	ledInit();
	NVIC_UnMaskIRQs();

	while (1)
	{
		ledToggle(5000);
	}

	return 0;
}





void delay(u32 time_ms)
{
	for(volatile u32 i = 0; i < 1130UL * (time_ms<<2); i++);
}


void ledInit(void)
{
	GPIO_Pin_t  ledPin = {
		.port       =  GPIO_PORTC,
		.pinNumber  =  GPIO_PIN13
	};

	GPIO_PinConfg_t  ledPinConfig = {
		.pinMode      =  GENERAL_PURPOSE_OUTPUT_PUSH_PULL,
		.outputSpeed  =  OUTPUT_SPEED_10MHz
	};

	GPIO_InitPins(&ledPin, &ledPinConfig);
}


void ledToggle(u32 delayTime)
{
	GPIO_Pin_t  ledPin = {
		.port       =  GPIO_PORTC,
		.pinNumber  =  GPIO_PIN13
	};

	GPIO_SetPinsOutputVoltage(&ledPin, GPIO_HIGH);
	delay(delayTime);
	GPIO_SetPinsOutputVoltage(&ledPin, GPIO_LOW);
	delay(delayTime);
}



void jumpToBoot(void)
{
	ECU_ConfigActivateBoot();
	NVIC_MaskIRQs(0);
	NVIC_ResetSystem();
}

