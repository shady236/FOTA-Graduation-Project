#include  "ESP.h"


static u8 ESP_RxData[100];
static u8 ESP_RxDataSize = 0;


static void (*callBackFun)(void);


/* when nnn characters (xxxxxxxxxx) are received from server, ESP_RxData should contain --> +IPD,nnn:xxxxxxxxxx */
#define  SERVER_DATA_MIN_HEADER_SIZE       9
#define  SERVER_DATA_LENGTH_STRT_IDX       7


Boolean_t isStringStartsWith(u8* str1, u8* str2)
{
	while((*str1) && (*str2))
	{
		if(*str1 != *str2)
		{
			return FALSE;
		}
		str1++;
		str2++;
	}
	if(*str2) 	return FALSE;
	return TRUE;
}


static u8 ESP_CheckErrors(u8* okRes, u8* errorRes)
{
	u8 okResLen = 0 , errorResLen = 0;
	u32 timer = 0;
	while(okRes[okResLen])
		okResLen++;
	while(errorRes[errorResLen])
		errorResLen++;

	while(timer < 0xFFFFF)
	{
		if(isStringStartsWith(&ESP_RxData[ESP_RxDataSize - okResLen], okRes))
		{
			return OK;
		}
		else if(isStringStartsWith(&ESP_RxData[ESP_RxDataSize - errorResLen], errorRes))
		{
			return NOT_OK;
		}
		timer++;
	}
	return NOT_OK;
}


void ESP_RxCallBackFun(void)
{
	if(ESP_RxDataSize < 100)
	{
		UART_RxCharUnblocking(UART1, &ESP_RxData[ESP_RxDataSize]);
		ESP_RxDataSize++;

		/* when nnn characters (xxxxxxxxxx) are received from server, ESP_RxData should contain --> +IPD,nnn:xxxxxxxxxx */
		if(isStringStartsWith(ESP_RxData, (u8*)"\r\n+IPD,"))
		{
			u16 dataFromServerSize = 0;
			u8  i = SERVER_DATA_LENGTH_STRT_IDX;
			while((i < ESP_RxDataSize) && (ESP_RxData[i] != ':'))
			{
				dataFromServerSize *= 10;
				dataFromServerSize += ESP_RxData[i] - '0';
				i++;
			}

			if(dataFromServerSize == ESP_RxDataSize - (i + 1))
			{
				callBackFun();
			}
		}
	}
}


u8 ESP_ReadRxDataFromServer(u8** data, u16* dataLen)
{
	if(isStringStartsWith(ESP_RxData, (u8*)"\r\n+IPD,"))
	{
		*dataLen = 0;
		u8 i = SERVER_DATA_LENGTH_STRT_IDX;
		while((i < ESP_RxDataSize) && (ESP_RxData[i] != ':'))
		{
			(*dataLen) *= 10;
			(*dataLen) += ESP_RxData[i] - '0';
			i++;
		}

		*data = &ESP_RxData[i + 1];
		if(*dataLen == ESP_RxDataSize - (i + 1))
		{
			return OK;
		}
	}
	return NOT_OK;
}


void ESP_ClrBuffer(void)
{
	ESP_RxDataSize = 0;
}


void ESP_Init(void (*ptrToFun)(void))
{
	callBackFun = ptrToFun;

	UART_Config_t UART_Config = {
		.UARTx                                            =   UART1,

		.UART_TxState                                     =   ENABLE,
		.UART_RxState                                     =   ENABLE,
		.UART_TxCompleteInterruptState                    =   DISABLE,
		.UART_TxBufferEmptyInterruptState                 =   DISABLE,
		.UART_RxInterruptState                            =   ENABLE,
		.UART_ParityErrorInterruptState                   =   DISABLE,
		.UART_DataBitsNum                                 =   8,
		.UART_StopBitsNum                                 =   ONE_STOP_BIT,
		.UART_Parity                                      =   NO_PARITY,
		.UART_BaudRate                                    =   115200,

		.UART_TxCompleteInterruptCallBackFunction         =   NULL,
		.UART_TxBufferEmptyInterruptCallBackFunction      =   NULL,
		.UART_RxInterruptCallBackFunction                 =   ESP_RxCallBackFun,
		.UART_ParityErrorInterruptCallBackFunction        =   NULL
	};

	UART_Init(&UART_Config);

//	UART_TxString(UART1, (u8*)"AT+UART_CUR=9600,8,1,0,0\r\n");
//	UART_Config.UART_BaudRate = 9600;
//	UART_Init(&UART_Config);
//	ESP_CheckErrors((u8*)"OK\r\n", (u8*)"ERROR\r\n");
//	ESP_RxDataSize = 0;
}


u8 ESP_ConnectToRouter(const u8* ssid, const u8* password)
{
	ESP_RxDataSize = 0;
	UART_TxString(UART1, (u8*)"AT+CWJAP=\"");
	UART_TxString(UART1, ssid);
	UART_TxString(UART1, (u8*)"\",\"");
	UART_TxString(UART1, password);
	UART_TxString(UART1, (u8*)"\"\r\n");

	u8 status = ESP_CheckErrors((u8*)"OK\r\n", (u8*)"ERROR\r\n");
	ESP_RxDataSize = 0;
	return status;
}


u8 ESP_SetWiFiMode(void)
{
	ESP_RxDataSize = 0;
	UART_TxString(UART1, (u8*)"AT+CWMODE=3\r\n");

	u8 status = ESP_CheckErrors((u8*)"OK\r\n", (u8*)"ERROR\r\n");
	ESP_RxDataSize = 0;
	return status;
}


u8 ESP_SetAsTCPClient(const u8* serverIP, const u8* portnum)
{
	ESP_RxDataSize = 0;
	UART_TxString(UART1, (u8*)"AT+CIPSTART=\"TCP\",\"");
	UART_TxString(UART1, serverIP);
	UART_TxString(UART1, (u8*)"\",");
	UART_TxString(UART1, portnum);
	UART_TxString(UART1, (u8*)"\r\n");

	u8 status = ESP_CheckErrors((u8*)"OK\r\n", (u8*)"ERROR\r\n");
	ESP_RxDataSize = 0;
	return status;
}


u8 ESP_EndTCPConnection(void)
{
	ESP_RxDataSize = 0;
	UART_TxString(UART1, (u8*)"AT+CIPCLOSE\r\n");

	u8 status = ESP_CheckErrors((u8*)"OK\r\n", (u8*)"ERROR\r\n");
	ESP_RxDataSize = 0;
	return status;
}


u8 ESP_EnableUARTWiFiPassthroughMode(void)
{
	ESP_RxDataSize = 0;
	UART_TxString(UART1, (u8*)"AT+CIPMODE=1\r\n");

	u8 status = ESP_CheckErrors((u8*)"OK\r\n", (u8*)"ERROR\r\n");
	ESP_RxDataSize = 0;
	return status;
}


u8 ESP_DisableUARTWiFiPassthroughMode(void)
{
	ESP_RxDataSize = 0;
	UART_TxString(UART1, (u8*)"AT+CIPMODE=0\r\n");

	u8 status = ESP_CheckErrors((u8*)"OK\r\n", (u8*)"ERROR\r\n");
	ESP_RxDataSize = 0;
	return status;
}


u8 ESP_StartSendingData(const u8* data,  u16 dataLen)
{
	ESP_RxDataSize = 0;
	u8 dataLenStr[5] = {0};
	s8 i;
	for(i = 3; i >= 0 && dataLen; i--)
	{
		dataLenStr[i] = dataLen % 10 + '0';
		dataLen /= 10;
	}

	UART_TxString(UART1, (u8*)"AT+CIPSEND=");
	UART_TxString(UART1, dataLenStr + 1 + i);
	UART_TxString(UART1, (u8*)"\r\n");

	u8 status = ESP_CheckErrors((u8*)"OK\r\n> ", (u8*)"ERROR\r\n");
	ESP_RxDataSize = 0;
	if(status == OK)
	{
		UART_TxString(UART1, data);
		status = ESP_CheckErrors((u8*)"OK\r\n", (u8*)"FAIL\r\n");
	}
	ESP_RxDataSize = 0;
	return status;
}

