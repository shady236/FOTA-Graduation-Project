#include "ECU_Comm.h"


#define  ECU_UPDATE_PAYLOAD_ID(ecuNum)		(((ecuNum)<<1UL) + 0x50UL)
#define  ECU_UPDATE_CONTROL_ID(ecuNum)		(((ecuNum)<<1UL) + 0x51UL)



/* private variable to store received data from server */
static CAN_Frame_t rxFrame;
static Boolean_t  newRxFrame = FALSE;


/* private constants */
static const u8  ECU_ACK   =  'A';


static void CAN_CallBackFun(void)
{
	CAN_Rx(&rxFrame, 0);
	newRxFrame = TRUE;
}


static Boolean_t  validateRes(u8 ecuNum)
{
	u32 timeut = 0xFFFFFF;
	while(newRxFrame == FALSE && timeut)
		timeut--;

	if(newRxFrame == FALSE)
		return FALSE;

	newRxFrame = FALSE;

	if(rxFrame.idType != STANDARD_ID)
		return FALSE;

	if(rxFrame.id != ECU_UPDATE_CONTROL_ID(ecuNum))
		return FALSE;

	if(rxFrame.DLC != 1)
		return FALSE;

	if(rxFrame.frameType != DATA_FRAME)
		return FALSE;

	if(rxFrame.data[0] != ECU_ACK)
		return FALSE;

	return TRUE;
}


/*
 * send the data in a CAN frame of 8 data bytes:
 *    - first  4 bytes for start address
 *    - second 4 bytes for the record data
 *
 * Segmentation is useful for:
 *    - reducing traffic at CAN bus by making sure that all frames but one has data with maximum size
 *    - simplify the receiving ECU task, it is sure that all data comes in the same format
 */
static Boolean_t  ECU_TxSeg(u8 ecuNum, u8* address, u8* data, u8 dataLen)
{
	CAN_Frame_t frame = {
		.idType      =   STANDARD_ID,
		.frameType   =   DATA_FRAME,
		.id          =   ECU_UPDATE_PAYLOAD_ID(ecuNum),
		.DLC         =   4 + dataLen
	};

	for (u8 i = 0; i < 4; i++)
	{
		frame.data[i] = address[i];
	}

	for (u8 i = 0; i < dataLen; i++)
	{
		frame.data[i + 4] = data[i];
	}

	CAN_Tx(&frame);
	return validateRes(ecuNum);
}



void  ECU_TerminateConn(u8 ecuNum, Boolean_t state)
{
	CAN_Frame_t frame = {
		.idType      =   STANDARD_ID,
		.frameType   =   DATA_FRAME,
		.id          =   ECU_UPDATE_CONTROL_ID(ecuNum),
		.DLC         =   1,
		.data        =   {state}
	};

	CAN_Tx(&frame);
}




void  ECU_CommInit(void)
{
	RCC_EnablePeripheralClock(CAN_PERIPHERAL);
	CAN_Init(CAN_CallBackFun);

	/* config filters for recieving from all ECUs */
	for(u8 ecuNum = 0; ecuNum < ECUS_NUM; ecuNum += 2)
	{
		CAN_RxFilterBankConfig_t filterConfig = {
			.filterBankNum      =      ecuNum>>1,
			.desFifo            =      FIFO_0,
			.mode               =      ID_LIST,
			.scale              =      SCALE_16BIT,
			.accepted           =      {
				{
					.id               =  ECU_UPDATE_PAYLOAD_ID(ecuNum),
					.idType           =  STANDARD_ID,
					.frameType        =  DATA_FRAME
				},
				{
					.id               =  ECU_UPDATE_CONTROL_ID(ecuNum),
					.idType           =  STANDARD_ID,
					.frameType        =  DATA_FRAME
				},
				{
					.id               =  ECU_UPDATE_PAYLOAD_ID(ecuNum + 1),
					.idType           =  STANDARD_ID,
					.frameType        =  DATA_FRAME
				},
				{
					.id               =  ECU_UPDATE_CONTROL_ID(ecuNum + 1),
					.idType           =  STANDARD_ID,
					.frameType        =  DATA_FRAME
				}
			}
		};

		CAN_InitFilterBank(&filterConfig);
	}
}



Boolean_t  ECU_SendUpdateReq(u8 ecuNum, ImgType_t imgType)
{
	CAN_Frame_t frame = {
		.idType      =   STANDARD_ID,
		.frameType   =   DATA_FRAME,
		.id          =   ECU_UPDATE_CONTROL_ID(ecuNum),
		.DLC         =   1,
		.data        =   {imgType}
	};

	CAN_Tx(&frame);
	return validateRes(ecuNum);
}



Boolean_t  ECU_TxRec(u8 ecuNum, SRecord_t* record)
{
	/*
	 * send the data in a CAN frame of 8 data bytes:
	 *    - first  4 bytes for start address
	 *    - second 4 bytes for the record data
	 *
	 * To handle that, the first record data is divided into segments of 4 bytes, each segment is transmitted.
	 *
	 * If last segment of the first record data isn't of 4 bytes, don't transmit it, instead we do that:
	 *     - save the last segment data in a static variable in order to keep it after function terminates
	 *     - when next record to be sent, get its first data bytes to complete the previous segment, then transmit it
	 *
	 * if there's no next data records, i.e. the current record is termination record and there were previous
	 * uncompleted segment, then transmit it
	 *
	 * Segmentation is useful for:
	 *    - reducing traffic at CAN bus by making sure that all frames but one has data with maximum size
	 *    - simplify the receiving ECU task, it is sure that all data comes in the same format
	 */


	static u8  prevSeg[4] = {0};

	/* initially, there is no prev segment */
	static u8   prevSegIdx      =  0;
	static u32  prevSegAddress  =  0;


	u8  address[4]  =  {0};
	u8  recordDataIdx = 0;

	switch(record->recordType)
	{
		case END_RECORD_16_BIT_ADDRESS:
		case END_RECORD_24_BIT_ADDRESS:
		case END_RECORD_32_BIT_ADDRESS:
			/* send the previous record uncompleted */

			if(prevSegIdx > 0)
			{
				for(u8 i = 0; i < 4; i++)
				{
					address[i] = prevSegAddress>>(i<<3);
				}

				Boolean_t res = ECU_TxSeg(ecuNum, address, prevSeg, prevSegIdx);

				if(res == FALSE)
				{
					return FALSE;
				}
				else
				{
					ECU_TerminateConn(ecuNum, TRUE);
					prevSegIdx = 0;
					return TRUE;
				}
			}
			else
			{
				ECU_TerminateConn(ecuNum, TRUE);
				return TRUE;
			}
		break;


		case DATA_RECORD_16_BIT_ADDRESS:
		case DATA_RECORD_24_BIT_ADDRESS:
		case DATA_RECORD_32_BIT_ADDRESS:
			/* complete the previous segment data */

			while(recordDataIdx < record->dataBytesCount)
			{
				while(prevSegIdx < 4 && recordDataIdx < record->dataBytesCount)
				{
					prevSeg[prevSegIdx] = record->data[recordDataIdx];

					prevSegIdx++;
					recordDataIdx++;
				}

				/* If segment is completed, transmit it */
				if(prevSegIdx == 4)
				{
					if(prevSegAddress == 0)
					{
						prevSegAddress = record->startAddress;
					}

					for(u8 i = 0; i < 4; i++)
					{
						address[i] = prevSegAddress>>(i<<3);
					}


					Boolean_t res = ECU_TxSeg(ecuNum, address, prevSeg, 4);

					if(res == FALSE)
					{
						return FALSE;
					}

					prevSegIdx = 0;
					prevSegAddress += 4;
				}
			}

			return TRUE;
		break;
	}

	return  TRUE;
}



