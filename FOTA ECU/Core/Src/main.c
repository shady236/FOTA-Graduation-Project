#include "serverComm.h"
#include "ECU_Config.h"
#include "ECU_Comm.h"
#include "RCC.h"
#include "srec.h"


extern ECU_Config_t  ecusConfig[ECUS_NUM];


void delay(u32 time);
void ledInit(void);
void ledToggle(u32 delayTime);
void handleUpdate(u8 ecuNum, ImgType_t imgType);



int main(void)
{
	RCC_InitClock();

	serverCommInit();
	ECU_CommInit();
	ledInit();

	while(1)
	{
		for (u8 ecuNum = 0; ecuNum < ECUS_NUM; ecuNum++)
		{
			handleUpdate(ecuNum, APP);
			handleUpdate(ecuNum, BOOTLOADER);
			ledToggle(1000000);
		}
		delay(0xFFFFFFF);
	}

	return 0;
}



void delay(u32 time)
{
	for(volatile u32 i = 0; i < time; i++);
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


void handleUpdate(u8 ecuNum, ImgType_t imgType)
{
	u8 latestServerVersion;
	u8 latestLOcalVersion;

	switch(imgType)
	{
		case APP:
			latestLOcalVersion = ecusConfig[ecuNum].appVersion;
		break;

		case BOOTLOADER:
			latestLOcalVersion = ecusConfig[ecuNum].bootVersion;
		break;

		default:
			return;
		break;
	}


	Boolean_t  state  =  getEcuLatestVersion(ecuNum, imgType, &latestServerVersion);

	if(state == FALSE)
		return;

	u8 data[100];
	u8 dataLen;
	Boolean_t  isFileEnded = FALSE;

	if(latestServerVersion <= latestLOcalVersion)
	{
		serverTerminateConn();
		return;
	}
	else
	{
//		serverTerminateConn();
		if(ECU_SendUpdateReq(ecuNum, imgType) == FALSE)
		{
			serverTerminateConn();
			return;
		}
//		getNextLineOfUpdate(data, &dataLen);
//		return;
	}

//	ledToggle(1000000);


//	state = ECU_SendUpdateReq(ecuNum, imgType);
//	if(state == FALSE)
//	{
//		serverTerminateConn();
//		return;
//	}

//	state = ECU_SendUpdateReq(ecuNum, imgType);
//	if(state == FALSE)
//	{
//		serverTerminateConn();
//		return;
//	}




	while(isFileEnded == FALSE)
	{
		getNextLineOfUpdate(data, &dataLen);

		SRecord_t record;
		sRecordParse(data, &record);
//		if(sRecordParse(data, &record) != RECORD_NO_ERRORS)
//		{
//			// error
//
//			return;
//		}
//		else
//		{
			switch(record.recordType)
			{
				case HEADER_RECORD:
				case COUNT_RECORD_16_BIT:
				case COUNT_RECORD_24_BIT:
				break;

				case DATA_RECORD_16_BIT_ADDRESS:
				case DATA_RECORD_24_BIT_ADDRESS:
				case DATA_RECORD_32_BIT_ADDRESS:
//					state = ECU_TxRec(ecuNum, &record);

//					if(state == FALSE)
//					{
//						serverTerminateConn();
//					}
				break;

				case END_RECORD_16_BIT_ADDRESS:
				case END_RECORD_24_BIT_ADDRESS:
				case END_RECORD_32_BIT_ADDRESS:
//					state = ECU_TxRec(ecuNum, &record);
//					if(state == FALSE)
//					{
//						serverTerminateConn();
//					}
//					else
//					{
						isFileEnded = TRUE;
//					}
				break;
//			}
		}
	}

	getNextLineOfUpdate(data, &dataLen);
	getNextLineOfUpdate(data, &dataLen);

//	if(state == TRUE)
//	{
		switch(imgType)
		{
			case APP:
				ecusConfig[ecuNum].appVersion = latestServerVersion;
				if(ecusConfig[ecuNum].activeAppRegion == 1)
					ecusConfig[ecuNum].activeAppRegion = 2;
				else
					ecusConfig[ecuNum].activeAppRegion = 1;
			break;

			case BOOTLOADER:
				ecusConfig[ecuNum].bootVersion = latestServerVersion;
				if(ecusConfig[ecuNum].activeBootRegion == 1)
					ecusConfig[ecuNum].activeBootRegion = 2;
				else
					ecusConfig[ecuNum].activeBootRegion = 1;
			break;
		}
		ECU_UpdateConfig(ecuNum);
//	}
}

