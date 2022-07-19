#include "serverComm.h"

/* ECU_Config_t --> private variable to store configurations of ECUs */
extern ECU_Config_t  ecusConfig[ECUS_NUM];

/* private variable to received data from server */
static u8 rxDataFromSrvr[100];
static u8 rxDataSize = 0;



/* private constants */
static const u8* ACCESS_POINT_NAME    =  (u8*) "MANSOUR";		// MANSOUR   , Xperia L1_1f2b
static const u8* ACCESS_POINT_PASS    =  (u8*) "xXxMANxXx";			// xXxMANxXx , 111141111

static const u8* SRVR_IP              =  (u8*) "192.168.1.6";              // 192.168.1.10 , 192.168.43.118
static const u8* SRVR_PORT_NUM        =  (u8*) "800";

static const u8* SRVR_ACK             =  (u8*) "A";
static const u8  SRVR_ACK_LEN         =   1 ;
static const u8* SRVR_NAK             =  (u8*) "N";
static const u8  SRVR_NAK_LEN         =   1 ;

static const u8* SRVR_APP_DOWNLOAD_REQ        =  (u8*) "A";
static const u8  SRVR_APP_DOWNLOAD_REQ_LEN    =   1 ;
static const u8* SRVR_BOOT_DOWNLOAD_REQ       =  (u8*) "B";
static const u8  SRVR_BOOT_DOWNLOAD_REQ_LEN   =   1 ;

static const u8* SRVR_DOWNLOAD_REQ        =  (u8*) "D";
static const u8  SRVR_DOWNLOAD_REQ_LEN    =   1 ;





static u8 u8ToStr(u8 n, u8* str)
{
	u8 len = 0;
	for(u8 i = 1; i <= 100 && n / i > 0; i *= 10)
		len++;

	if(len == 0)
		len = 1;

	str[len] = 0;

	for(s8 i = len - 1; i >= 0; i--)
	{
		str[i] = n % 10 + '0';
		n /= 10;
	}

	return len;
}


static u8 strToU8(u8* str, u8 strLen)
{
	u8 res = 0;
	u8 digitWeight = 1;

	for(s8 i = strLen - 1; i >= 0; i--)
	{
		res += (str[i] - '0') * digitWeight;
		digitWeight *= 10;
	}

	return res;
}


static void ESP_SrvrRxDataCallBack(void)
{
	u8* data;
	u16 dataLen;
	if(ESP_ReadRxDataFromServer(&data, &dataLen) == OK)
	{
		/* move received data into safe area */
		for(u16 i = 0; i < dataLen; i++)
		{
			rxDataFromSrvr[i] = data[i];
		}
		rxDataSize = dataLen;
		rxDataFromSrvr[rxDataSize] = 0;
		ESP_ClrBuffer();
	}
}



void  serverCommInit(void)
{
	/* load configurations of ECUs */
	ECU_LoadConfig();

	/* initialize ESP with the callback function */
	ESP_Init(ESP_SrvrRxDataCallBack);
	ESP_SetWiFiMode();
	ESP_ConnectToRouter(ACCESS_POINT_NAME, ACCESS_POINT_PASS);
}



void  serverTerminateConn(void)
{
	ESP_StartSendingData(SRVR_NAK, SRVR_NAK_LEN);	// no need to download now
}





Boolean_t  getEcuLatestVersion(u8 ecuNum, ImgType_t imgType, u8* version)
{
	rxDataSize = 0;    // Clear Rx Buffer

	u8 ecuNumStr[4];
	u8 ecuNumLen = u8ToStr(ecuNum, ecuNumStr);


	/* loop twice to check for app & bootloader updates checking */
	u8* imgDownloadReq;
	u8  imgDownloadReqLen;
	u8  imgVersion;
	u8  activeImgRegion;

	switch(imgType)
	{
		case APP:
			imgDownloadReq = (u8*) SRVR_APP_DOWNLOAD_REQ;
			imgDownloadReqLen = SRVR_APP_DOWNLOAD_REQ_LEN;
			imgVersion = ecusConfig[ecuNum].appVersion;
			activeImgRegion = ecusConfig[ecuNum].activeAppRegion;
		break;

		case BOOTLOADER:
			imgDownloadReq = (u8*) SRVR_BOOT_DOWNLOAD_REQ;
			imgDownloadReqLen = SRVR_BOOT_DOWNLOAD_REQ_LEN;
			imgVersion = ecusConfig[ecuNum].bootVersion;
			activeImgRegion = ecusConfig[ecuNum].activeBootRegion;

		break;

		default:
			return FALSE;
		break;
	}


	u8 requiredRegionStr[2] = "1";
	if(activeImgRegion == 1)
		requiredRegionStr[0] = '2';




	/* Connect to server, if Connection fails return FALSE */
	if(ESP_SetAsTCPClient(SRVR_IP, SRVR_PORT_NUM) != OK)
	{
		return FALSE;
	}

	/* Send download request, if request fails return FALSE */
	if(ESP_StartSendingData(SRVR_DOWNLOAD_REQ, SRVR_DOWNLOAD_REQ_LEN) != OK)
	{
		return FALSE;
	}

	/* Send ECU number, if sending fails return FALSE */
	if(ESP_StartSendingData(ecuNumStr, ecuNumLen) != OK)
	{
		return FALSE;
	}

	/* Send download request for app or boot, if request fails return FALSE */
	if(ESP_StartSendingData(imgDownloadReq, imgDownloadReqLen) != OK)
	{
		return FALSE;
	}

	/* Send required region, if sending fails return FALSE */
	if(ESP_StartSendingData(requiredRegionStr, 1) != OK)
	{
		return FALSE;
	}


	// wait some time until server replies
	u32 timeout = 0xFFFFF;
	while(rxDataSize == 0 && timeout)
		timeout--;

	/* if time is out without server replies, return FALSE */
	if(rxDataSize == 0)
		return FALSE;


	/* Read latest image version on the server */
	*version = strToU8(rxDataFromSrvr, rxDataSize);
	rxDataSize = 0;		// clear buffer
	return TRUE;
}



Boolean_t  getNextLineOfUpdate(u8* data, u8* dataLen)
{
	rxDataSize = 0;		// clear buffer

	/* Send ACK, if sending fails return FALSE */
	if(ESP_StartSendingData(SRVR_ACK, SRVR_ACK_LEN) != OK)
	{
		return FALSE;
	}

	/* wait until total line is received */
	while(rxDataSize == 0 || rxDataFromSrvr[rxDataSize - 1] != '\n');

	/* copy rx data */
	for(u8 i = 0; i < rxDataSize; i++)
	{
		data[i] = rxDataFromSrvr[i];
	}
	*dataLen = rxDataSize;


	return TRUE;
}



