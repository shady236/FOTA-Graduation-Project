#include "updates.h"


#define  ECU_UPDATE_PAYLOAD_ID		(((ECU_NUM)<<1UL) + 0x50UL)
#define  ECU_UPDATE_CONTROL_ID		(((ECU_NUM)<<1UL) + 0x51UL)



/* private variable to store received data from server */
static ECU_UpdatesState_t  updatesState;
static void (*updatesAvailableCallBack)(void);


/* private constants */
static const u8  ECU_NAK   =  'N';


static void CAN_CallBackFun(void)
{
	CAN_Frame_t  rxFrame;
	CAN_Rx(&rxFrame, 0);


	CAN_Frame_t  txFrame = {
		.id           =   ECU_UPDATE_CONTROL_ID,
		.idType       =   STANDARD_ID,
		.frameType    =   DATA_FRAME,
		.DLC          =   1,
		.data         =   {ECU_NAK}
	};

	if(updatesState != READY_FOR_UPDATES)
	{
		CAN_Tx(&txFrame);
		return;
	}

	if(rxFrame.id != ECU_UPDATE_CONTROL_ID)
	{
		CAN_Tx(&txFrame);
		return;
	}

	if(rxFrame.DLC != 1)
	{
		CAN_Tx(&txFrame);
		return;
	}

	if(rxFrame.data[0] != APP && rxFrame.data[0] != BOOTLOADER)
	{
		CAN_Tx(&txFrame);
		return;
	}

	if(updatesAvailableCallBack == NULL)
	{
		CAN_Tx(&txFrame);
		return;
	}

	updatesAvailableCallBack();
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
				.id               =  ECU_UPDATE_CONTROL_ID,
				.idType           =  STANDARD_ID,
				.frameType        =  DATA_FRAME
			},
			{
				.id               =  ECU_UPDATE_CONTROL_ID,
				.idType           =  STANDARD_ID,
				.frameType        =  DATA_FRAME
			},
			{
				.id               =  ECU_UPDATE_CONTROL_ID,
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


void  updatesSetState(ECU_UpdatesState_t  state, void (*updatesAvailableCallBackFun)(void))
{
	updatesState = state;
	updatesAvailableCallBack = updatesAvailableCallBackFun;
}
