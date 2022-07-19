#include "CAN.h"

static void (*callBack)(void) = NULL;


void  USB_LP_CAN1_RX0_IRQHandler(void)
{
	if(callBack != NULL)
		callBack();
}



void CAN_Init(void (*ptr)(void))
{
	callBack = ptr;

	RCC_EnablePeripheralClock(CAN_PERIPHERAL);
//	RCC_EnablePeripheralClock(GPIOA_PERIPHERAL);
	RCC_EnablePeripheralClock(GPIOB_PERIPHERAL);
	RCC_EnablePeripheralClock(AFIO_PERIPHERAL);

	NVIC_EnableInterrupt(USB_LP_CAN1_RX0_IRQn);


	CLR_BIT(CAN1->MCR, 1);		// exit sleep mode
	while(GET_BIT(CAN1->MSR, 0) == 1);		// wait until sleep ack be 0


	SET_BIT(CAN1->MCR, 0);		// enter intialization mode
	while(GET_BIT(CAN1->MSR, 0) == 0);		// wait until intialization ack be 1

	SET_BIT(CAN1->IER, 1);		// FIFI 0 Rx IRQ enable

//	SET_BIT(CAN1->BTR, 31);		// silent mode
	CLR_BIT(CAN1->BTR, 31);		// not silent mode
//	SET_BIT(CAN1->BTR, 30);		// loopback mode
	CLR_BIT(CAN1->BTR, 30);		// not loopback mode

	// lower data rate to be 250 Kbps
	CAN1->BTR &= 0xF0000000;		// clear all register except high ones
	SET_BIT(CAN1->BTR, 0);		// BRP bits (9:0) = 1 --> prescaller = 2
	SET_BIT(CAN1->BTR, 1);		// BRP bits (9:0) = 3 --> prescaller = 4
	SET_BIT(CAN1->BTR, 2);		// BRP bits (9:0) = 7 --> prescaller = 8

	SET_BIT(CAN1->BTR, 17);		// TS1 bits (19:16) = 2  --> t_BS1 = 3  tq
	SET_BIT(CAN1->BTR, 18);		// TS1 bits (19:16) = 6  --> t_BS1 = 7  tq
	SET_BIT(CAN1->BTR, 19);		// TS1 bits (19:16) = 14 --> t_BS1 = 15 tq

	SET_BIT(CAN1->BTR, 20);		// TS2 bits (22:20) = 1 --> t_BS2 = 2 tq

	SET_BIT(CAN1->BTR, 24);		// SJW bits (25:24) = 1 --> SJW = tq


//	CLR_BIT(CAN1->MCR, 16);	// no debug freeze
//	SET_BIT(CAN1->MCR, 4);	// no automatic retransmission


	CAN1->FMR &= ~(0x3FUL<<8);	// all filters for CAN1
	CAN1->FMR |=  (14UL<<8);		// all filters for CAN1

	SET_BIT(CAN1->MCR, 6);		// Bus-Off is left automatically by hardware

	SET_BIT(CAN1->MCR, 2);		// Tx mailbox priority by first requested

	CLR_BIT(CAN1->MCR, 0);		// exit intialization mode
	while(GET_BIT(CAN1->MSR, 0) == 1);		// wait until intialization mode ack


	// remap CAN pins, CAN_RX mapped to PB8, CAN_TX mapped to PB9
	SET_BIT(AFIO->MAPR, 14);


	// CAN TX --> PA12 --> Alternate function push-pull
	GPIO_Pin_t  CAN_TxPin = {
		.port       =  GPIO_PORTB,
		.pinNumber  =  GPIO_PIN9
	};

	GPIO_PinConfg_t  CAN_TxPinConfig = {
		.pinMode      =  ALTERNATE_FUNCTION_OUTPUT_PUSH_PULL,
		.outputSpeed  =  OUTPUT_SPEED_50MHz
	};

	GPIO_InitPins(&CAN_TxPin, &CAN_TxPinConfig);



	// CAN RX --> PA11 --> Input floating / Input pull-up
	GPIO_Pin_t  CAN_RxPin = {
		.port       =  GPIO_PORTB,
		.pinNumber  =  GPIO_PIN8
	};

	GPIO_PinConfg_t  CAN_RxPinConfig = {
		.pinMode      =  INPUT_PULL_UP,
		.outputSpeed  =  OUTPUT_SPEED_50MHz
	};

	GPIO_InitPins(&CAN_RxPin, &CAN_RxPinConfig);
}



void CAN_InitFilterBank(CAN_RxFilterBankConfig_t*  filterConfig)
{
	if((filterConfig->filterBankNum) > 13)
		return;


	SET_BIT(CAN1->FMR, 0);	// enter filter initialization mode
	CLR_BIT(CAN1->FA1R, filterConfig->filterBankNum);		// deactivate filter


	switch(filterConfig->desFifo)
	{
		case FIFO_0:
			CLR_BIT(CAN1->FFA1R, filterConfig->filterBankNum);
		break;

		case FIFO_1:
			SET_BIT(CAN1->FFA1R, filterConfig->filterBankNum);
		break;
	}


	switch(filterConfig->mode)
	{
		case ID_MASK:
			CLR_BIT(CAN1->FM1R, filterConfig->filterBankNum);	// id mask mode

			switch(filterConfig->scale)
			{
				case SCALE_16BIT:
					CLR_BIT(CAN1->FS1R, filterConfig->filterBankNum);	// 16bit scale

					/*
					 * Configure acceptance filter parameters
					 * for 16-bit id mask mode, 2 standard IDs are used with 2 masks
					 */
					CAN1->sFilterRegister[filterConfig->filterBankNum].FR1 = (
						((0UL)<<3)  |  // standard ID
						(((u32)filterConfig->accepted[0].frameType)<<4)   |  // RTR
						(((u32)filterConfig->accepted[0].id)<<5)          |  // ID
						(((u32)(!filterConfig->accepted[0].maskIdType))<<19) |  // standard ID
						(((u32)(!filterConfig->accepted[0].maskFrameType))<<20)  |  // RTR
						(((u32)filterConfig->accepted[0].mask)<<21)       // ID mask
					);


					CAN1->sFilterRegister[filterConfig->filterBankNum].FR2 = (
						((0UL)<<3)  |  // standard ID
						(((u32)filterConfig->accepted[1].frameType)<<4)  |  // RTR
						(((u32)filterConfig->accepted[1].id)<<5)    |   // ID
						(((u32)(!filterConfig->accepted[1].maskIdType))<<19)  |  // standard ID
						(((u32)(!filterConfig->accepted[1].maskFrameType))<<20)  |  // RTR
						(((u32)filterConfig->accepted[1].mask)<<21)       // ID mask
					);
				break;

				case SCALE_32BIT:
					SET_BIT(CAN1->FS1R, filterConfig->filterBankNum);	// 32bit scale

					/*
					 * Configure acceptance filter parameters
					 * for 32-bit id mask mode, 1 ID is used with its mask
					 */
					CAN1->sFilterRegister[filterConfig->filterBankNum].FR1 = (
						(((u32)filterConfig->accepted[0].frameType)<<1)  |  // RTR
						(((u32)filterConfig->accepted[0].idType)<<2)     |  // IDE
						((0xFFFFFFF8ul)<<3)		// set all ID bits
					);

					CAN1->sFilterRegister[filterConfig->filterBankNum].FR2 = (
						(((u32)(!filterConfig->accepted[0].maskFrameType))<<1)  |  // RTR
						(((u32)(!filterConfig->accepted[0].maskIdType))<<2)     |  // IDE
						((0xFFFFFFF8ul)<<3)		// set all ID bits
					);

					// clear 0 bits from ID & mask
					switch(filterConfig->accepted[0].idType)
					{
						case STANDARD_ID:
							CAN1->sFilterRegister[filterConfig->filterBankNum].FR1 &= (
								((filterConfig->accepted[0].id)<<21)    |  // standard  ID
								(0x1FFFFFUL)		// keep lower 21 bits as default
							);

							CAN1->sFilterRegister[filterConfig->filterBankNum].FR2 &= (
								((filterConfig->accepted[0].mask)<<21)    |  // standard  ID
								(0x1FFFFFUL)		// keep lower 21 bits as default
							);
						break;

						case EXTENDED_ID:
							CAN1->sFilterRegister[filterConfig->filterBankNum].FR1 &= (
								(((u32)filterConfig->accepted[0].id)<<3)   |   // extended ID
								(0x7UL)		// keep lower 3 bits as default
							);

							CAN1->sFilterRegister[filterConfig->filterBankNum].FR2 &= (
								(((u32)filterConfig->accepted[0].mask)<<3)     |  // extended ID
								(0x7UL)		// keep lower 3 bits as default
							);
						break;
					}
				break;
			}
		break;

		case ID_LIST:
			SET_BIT(CAN1->FM1R, filterConfig->filterBankNum);

			switch(filterConfig->scale)
			{
				case SCALE_16BIT:
					CLR_BIT(CAN1->FS1R, filterConfig->filterBankNum);	// 16bit scale

					/*
					 * Configure acceptance filter parameters
					 * for 16-bit id list mode, 4 standard IDs are used without masks
					 */
					CAN1->sFilterRegister[filterConfig->filterBankNum].FR1 = (
						((filterConfig->accepted[0].idType)<<3)  |  // standard ID
						((filterConfig->accepted[0].frameType)<<4)  |  // RTR
						((filterConfig->accepted[0].id)<<5)    // ID
					);

					CAN1->sFilterRegister[filterConfig->filterBankNum].FR1 |= (
						((filterConfig->accepted[1].idType)<<19)  |  // standard ID
						((filterConfig->accepted[1].frameType)<<20)  |  // RTR
						((filterConfig->accepted[1].id)<<21)    // ID
					);

					CAN1->sFilterRegister[filterConfig->filterBankNum].FR2 = (
						((filterConfig->accepted[2].idType)<<3)  |  // standard ID
						((filterConfig->accepted[2].frameType)<<4)  |  // RTR
						((filterConfig->accepted[2].id)<<5)    // ID
					);

					CAN1->sFilterRegister[filterConfig->filterBankNum].FR2 |= (
						((filterConfig->accepted[3].idType)<<19)  |  // standard ID
						((filterConfig->accepted[3].frameType)<<20)  |  // RTR
						((filterConfig->accepted[3].id)<<21)    // ID
					);
				break;

				case SCALE_32BIT:
					SET_BIT(CAN1->FS1R, filterConfig->filterBankNum);	// 32bit scale

					/*
					 * Configure acceptance filter parameters
					 * for 32-bit id list mode, 2 IDs are used without mask
					 */
					CAN1->sFilterRegister[filterConfig->filterBankNum].FR1 = (
						((u32)(filterConfig->accepted[0].frameType)<<1)  |  // RTR
						((u32)(filterConfig->accepted[0].idType)<<2)    // IDE
					);

					switch(filterConfig->accepted[0].idType)
					{
						case STANDARD_ID:
							CAN1->sFilterRegister[filterConfig->filterBankNum].FR1 |= (
								((u32)(filterConfig->accepted[0].id)<<21)    // ID
							);
						break;

						case EXTENDED_ID:
							CAN1->sFilterRegister[filterConfig->filterBankNum].FR1 |= (
								((u32)(filterConfig->accepted[0].id)<<3)    // ID
							);
						break;
					}


					CAN1->sFilterRegister[filterConfig->filterBankNum].FR2 = (
						((u32)(filterConfig->accepted[1].frameType)<<1)  |  // RTR
						((u32)(filterConfig->accepted[1].idType)<<2)    // IDE
					);

					switch(filterConfig->accepted[1].idType)
					{
						case STANDARD_ID:
							CAN1->sFilterRegister[filterConfig->filterBankNum].FR2 |= (
								((u32)(filterConfig->accepted[1].id)<<21)    // ID
							);
						break;

						case EXTENDED_ID:
							CAN1->sFilterRegister[filterConfig->filterBankNum].FR2 |= (
								((u32)(filterConfig->accepted[1].id)<<3)    // ID
							);
						break;
					}
				break;
			}
		break;
	}



	SET_BIT(CAN1->FA1R, filterConfig->filterBankNum);		// activate filter

	CLR_BIT(CAN1->FMR, 0);	// exit filter initialization mode
}




void CAN_Tx(CAN_Frame_t* frame)
{
	/* wait while all three mailboxes are busy (bits 26, 27, 28 are 0) */
	while((((CAN1->TSR)>>26) & 0x7) == 0);

	u8 freeMailBox = 0;
	for(u8 i = 0; i < 3; i++)
	{
		if(GET_BIT(CAN1->TSR, i + 26) == 1)
		{
			freeMailBox = i;
			break;
		}
	}

	/* fill lowest 4 bytes of data in TDLR register */
	CAN1->sTxMailBox[freeMailBox].TDLR = 0;
	for(u8 i = 0; i < 4; i++)
	{
		CAN1->sTxMailBox[freeMailBox].TDLR |= (frame->data[i])<<(i<<3);
	}

	/* fill highest 4 bytes of data in TDHR register */
	CAN1->sTxMailBox[freeMailBox].TDHR = 0;
	for(u8 i = 0; i < 4; i++)
	{
		CAN1->sTxMailBox[freeMailBox].TDHR |= (frame->data[i + 4])<<(i<<3);
	}

	CAN1->sTxMailBox[freeMailBox].TDTR = frame->DLC;			// Data Lenght Code (DLC)


	/* configure IDE & RTR */
	CAN1->sTxMailBox[freeMailBox].TIR = (
		((frame->frameType)<<1)  |   // RTR
		((frame->idType)<<2)         // IDE
	);

	/* configure ID */
	switch(frame->idType)
	{
		case STANDARD_ID:
			CAN1->sTxMailBox[freeMailBox].TIR |= (frame->id)<<21;
		break;

		case EXTENDED_ID:
			CAN1->sTxMailBox[freeMailBox].TIR |= (frame->id)<<3;
		break;
	}


	switch(frame->frameType)
	{
		case DATA_FRAME:
			CAN1->sTxMailBox[freeMailBox].TIR |= (frame->id)<<21;
		break;

		case REMOTE_FRAME:
			CAN1->sTxMailBox[freeMailBox].TIR |= (frame->id)<<3;
		break;
	}


	SET_BIT(CAN1->sTxMailBox[freeMailBox].TIR, 0);	// Tx request
}


void CAN_Rx(CAN_Frame_t* frame, CAN_RxFifo_t fifo)
{
	// wait until there's received data in FIFO
	switch(fifo)
	{
		case FIFO_0:
			while((CAN1->RF0R & 0x3) == 0);
		break;

		case FIFO_1:
			while((CAN1->RF1R & 0x3) == 0);
		break;
	}


	/* read frame type (RTR) */
	switch(GET_BIT(CAN1->sFIFOMailBox[fifo].RIR, 1))
	{
		case 0:
			frame->frameType = DATA_FRAME;
		break;

		case 1:
			frame->frameType = REMOTE_FRAME;
		break;
	}


	/* read ID type (IDE) */
	switch(GET_BIT(CAN1->sFIFOMailBox[fifo].RIR, 2))
	{
		case 0:
			frame->idType = STANDARD_ID;
		break;

		case 1:
			frame->idType = EXTENDED_ID;
		break;
	}


	/* read ID */
	switch(frame->idType)
	{
		case STANDARD_ID:
			frame->id = (CAN1->sFIFOMailBox[fifo].RIR)>>21;
		break;

		case 1:
			frame->id = (CAN1->sFIFOMailBox[fifo].RIR)>>3;
		break;
	}


	/* read DLC */
	frame->DLC = (CAN1->sFIFOMailBox[fifo].RDTR) & 0xF;



	/* read lower 4 bytes of data from RDLR register */
	for(u8 i = 0; i < 4; i++)
	{
		frame->data[i] = (CAN1->sFIFOMailBox[fifo].RDLR)>>(i<<3);
	}

	/* read higher 4 bytes of data from RDHR register */
	for(u8 i = 0; i < 4; i++)
	{
		frame->data[i + 4] = (CAN1->sFIFOMailBox[fifo].RDHR)>>(i<<3);
	}


	/* release FIFO (pop first message) */
	switch(fifo)
	{
		case FIFO_0:
			SET_BIT(CAN1->RF0R, 5);
		break;

		case FIFO_1:
			SET_BIT(CAN1->RF1R, 5);
		break;
	}
}

