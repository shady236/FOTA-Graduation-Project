
#ifndef INC_CAN_H_
#define INC_CAN_H_


#include <stm32f103xb.h>
#include "RCC.h"
#include "GPIO.h"
#include "STD_types.h"
#include "bitMainuplation.h"
#include "NVIC.h"


typedef enum
{
	STANDARD_ID = 0,
	EXTENDED_ID
}CAN_IdType_t;


typedef enum
{
	DATA_FRAME = 0,
	REMOTE_FRAME
}CAN_FrameType_t;



typedef struct
{
	/*
	 * id is the message ID:
	 *   - for STANDARD_ID, first 11 bit represent id
	 *   - for EXTENDED_ID, first 29 bit represent id
	 */
	u32 id;

	/*
	 * idType represents the message ID type; standard (STANDARD_ID) or extended (EXTENDED_ID)
	 */
	CAN_IdType_t idType;

	/*
	 * frameType represents frame type; data frame (DATA_FRAME) or remote frame (REMOTE_FRAME)
	 */
	CAN_FrameType_t frameType;

	/*
	 * DLC represents Data Length Code; from 0 to 8
	 */
	u8 DLC;

	/*
	 * data represents actual data to be transmitted
	 */
	u8 data[8];
}CAN_Frame_t;



typedef enum
{
	SCALE_16BIT,
	SCALE_32BIT
}CAN_RxFilterScale_t;


typedef enum
{
	ID_LIST,
	ID_MASK
}CAN_RxFilterMode_t;


typedef enum
{
	FIFO_0 = 0,
	FIFO_1
}CAN_RxFifo_t;



typedef struct
{
	/*
	 * filterNum represents filter bank number, from 0 to 13
	 */
	u8 filterBankNum;

	/*
	 * each filter bank has 2 32-bit filtering registers
	 * scale represents number of bits used to filter the message ID
	 *    - SCALE_16BIT uses 16 bits only for filtering IDs, it is used with standard IDs.
	 *    - SCALE_32BIT uses 32 bits for filtering IDs, it is used with extended IDs.
	 *
	 * for extended IDs, 32-bit scale are used as it needs 29-bit
	 *
	 * for standard IDs, 32-bit or 16-bit scale can be used
	 * but 16-bit is recommended as it divides the register, so that
	 * it can filter twice of what 32-bit scale can do.
	 */
	CAN_RxFilterScale_t  scale;

	/*
	 * mode represents type of filtering
	 *     - ID_LIST is used to select single   ID to pass through the filter
	 *       is uses both of the 32-bit registers for filtering different messages
	 *     - ID_MASK is used to select multiple ID to pass through the filter
	 *       it uses both of the 32-bit registers for filtering the same message
	 */
	CAN_RxFilterMode_t  mode;

	/*
	 * desFifo represents which FIFO to be used when Rx message matches this filter
	 */
	CAN_RxFifo_t  desFifo;


	struct
	{
		/*
		 * id is the message ID to be accepted:
		 *   - for STANDARD_ID, first 11 bit represent id
		 *   - for EXTENDED_ID, first 29 bit represent id
		 */
		u32 id;

		/*
		 * mask represents the discarded (son't care) bits of ID,
		 * i.e. bits that can differ in Rx ID rather than the filter ID and still been accepted:
		 *   - for STANDARD_ID, first 11 bit represent mask
		 *   - for EXTENDED_ID, first 29 bit represent mask
		 *
		 * mask is represented at the level of bits:
		 *   - 0 means the corresponding ID bit is a don't care bit, can differ in Rx message
		 *   - 1 means the corresponding ID bit must match the Rx bit
		 *
		 * this field is active only for ID mask mode
		 */
		u32 mask;


		/*
		 * idType represents the message ID type; standard (STANDARD_ID) or extended (EXTENDED_ID)
		 */
		CAN_IdType_t idType;

		/*
		 * frameType represents frame type; data frame (DATA_FRAME) or remote frame (REMOTE_FRAME)
		 */
		CAN_FrameType_t frameType;

		/*
		 * maskIdType represents if the ID type to be masked or not:
		 *    - TRUE  means ID type is    masked, i.e. any ID type (standard or extended) is accepted
		 *    - FALSE means ID type isn't masked, i.e. the ID type specified by idType is only accepted
		 *
		 * this field is active only for ID mask mode
		 */
		Boolean_t  maskIdType;

		/*
		 * maskFrameType represents if the frame type to be masked or not:
		 *    - TRUE  means frame type is    masked, i.e. any frame (data or remote) is accepted
		 *    - FALSE means frame type isn't masked, i.e. the frame specified by frameType is only accepted
		 *
		 * this field is active only for ID mask mode
		 */
		Boolean_t  maskFrameType;
	}accepted[4];
}CAN_RxFilterBankConfig_t;


void CAN_Init(void (*ptr)(void));
void CAN_InitFilterBank(CAN_RxFilterBankConfig_t*  filterConfig);

void CAN_Tx(CAN_Frame_t* frame);
void CAN_Rx(CAN_Frame_t* frame, CAN_RxFifo_t fifo);


#endif /* INC_CAN_H_ */
