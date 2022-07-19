#include "updates.h"


#define  ECU_UPDATE_PAYLOAD_ID		(((ECU_NUM)<<1UL) + 0x50UL)
#define  ECU_UPDATE_CONTROL_ID		(((ECU_NUM)<<1UL) + 0x51UL)



/* private variable to store received data from server */



/* private constants */
static const u8  ECU_ACK   =  'A';
static const u8  ECU_NAK   =  'N';


/* private variable to store received data from FOTA ECU */
static CAN_Frame_t rxFrame;
static Boolean_t   newRxFrame = FALSE;


static void CAN_CallBackFun(void)
{
	CAN_Rx(&rxFrame, 0);
	newRxFrame = TRUE;
}


static Boolean_t  validateRes(void)
{
	u32 timeut = 0xFFFFFF;
	while(newRxFrame == FALSE && timeut)
		timeut--;

	if(newRxFrame == FALSE)
		return FALSE;

	newRxFrame = FALSE;

	if(rxFrame.idType != STANDARD_ID)
		return FALSE;

	if(rxFrame.id != ECU_UPDATE_CONTROL_ID && rxFrame.id != ECU_UPDATE_PAYLOAD_ID)
		return FALSE;

	if(rxFrame.frameType != DATA_FRAME)
		return FALSE;

	return TRUE;
}


void  updatesInit(void)
{
	RCC_EnablePeripheralClock(CAN_PERIPHERAL);
	CAN_Init(CAN_CallBackFun);

	/* config filters */
	CAN_RxFilterBankConfig_t filterConfig = {
		.filterBankNum      =      0,
		.desFifo            =      FIFO_0,
		.mode               =      ID_LIST,
		.scale              =      SCALE_16BIT,
		.accepted           =      {
			{
				.id               =  ECU_UPDATE_PAYLOAD_ID,
				.idType           =  STANDARD_ID,
				.frameType        =  DATA_FRAME
			},
			{
				.id               =  ECU_UPDATE_CONTROL_ID,
				.idType           =  STANDARD_ID,
				.frameType        =  DATA_FRAME
			},
			{
				.id               =  ECU_UPDATE_PAYLOAD_ID,
				.idType           =  STANDARD_ID,
				.frameType        =  DATA_FRAME
			},
			{
				.id               =  ECU_UPDATE_CONTROL_ID,
				.idType           =  STANDARD_ID,
				.frameType        =  DATA_FRAME
			}
		}
	};

	CAN_InitFilterBank(&filterConfig);
}


Boolean_t  updateImg(ImgType_t*  imgType)
{
	CAN_Frame_t txFrame = {
		.id               =    ECU_UPDATE_CONTROL_ID,
		.DLC              =    1,
		.frameType        =    DATA_FRAME,
		.idType           =    STANDARD_ID,
		.data             =    {ECU_ACK}
	};

	CAN_Tx(&txFrame);

	Boolean_t res  =  validateRes();
	if(res == FALSE)
		return FALSE;

	if(rxFrame.DLC != 1)
		return FALSE;

	*imgType = rxFrame.data[0];
	if((*imgType) != APP && (*imgType) != BOOTLOADER)
		return FALSE;


	Boolean_t  isFileEnded = FALSE;
	while(isFileEnded == FALSE)
	{
		CAN_Tx(&txFrame);
		res  =  validateRes();
		if(res == FALSE)
			return FALSE;

		if(rxFrame.id == ECU_UPDATE_CONTROL_ID)
			return rxFrame.data[0];

		if(rxFrame.DLC <= 4)
			return FALSE;


		u32 address = 0;
		for(u8 i = 0; i < 4; i++)
		{
			address |= rxFrame.data[i]<<(i<<3);
		}


		u16 startByteOffset  =  (address - FLASH_BASE_ADDRESS) & 0x3FF;
		u16 startBytePageNum =  (address - FLASH_BASE_ADDRESS) >> 10;

		FPEC_Error_t  flashState;

		if(startByteOffset == 0)
		{
			flashState = FPEC_ErasePage(startBytePageNum);

			if(flashState != FPEC_NO_ERRORS)
			{
				txFrame.data[0] = ECU_NAK;
				return FALSE;
			}
		}


		for(u8 i = 4; i < rxFrame.DLC; i++)
		{
			flashState = FPEC_DirectWriteByte(startBytePageNum, startByteOffset, rxFrame.data[i]);

			if(flashState != FPEC_NO_ERRORS)
			{
				txFrame.data[0] = ECU_NAK;
				return FALSE;
			}
		}
	}


	return TRUE;
}





