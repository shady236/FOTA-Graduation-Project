
#ifndef INC_SREC_H_
#define INC_SREC_H_




#include  "STD_types.h"
#include  "FPEC.h"
#include  "NVIC.h"



typedef enum
{
	RECORD_NOT_VALID,
	RECORD_CHECK_SUM_MISMATCH,
	DATA_RECORDS_COUNT_MISMATCH,
	RECORD_FLASHING_ERROR,
	RECORD_NO_ERRORS
}SRecordHandlingError_t;


/* S record types */
typedef enum
{
	HEADER_RECORD = 0,
	DATA_RECORD_16_BIT_ADDRESS = 1,
	DATA_RECORD_24_BIT_ADDRESS = 2,
	DATA_RECORD_32_BIT_ADDRESS = 3,
	COUNT_RECORD_16_BIT = 5,
	COUNT_RECORD_24_BIT = 6,
	END_RECORD_32_BIT_ADDRESS = 7,
	END_RECORD_24_BIT_ADDRESS = 8,
	END_RECORD_16_BIT_ADDRESS = 9
}SRecordTypes_t;


/* S record fields */
typedef struct
{
	union
	{
		u32 startAddress;
		u32 dataRecordsCount;
	};
	u8 data[252];
	u8 dataBytesCount;
	u8 checkSum;
	SRecordTypes_t recordType;
}SRecord_t;



SRecordHandlingError_t  sRecordParse(u8* str, SRecord_t* record);
SRecordHandlingError_t  srecHandle(SRecord_t* record);


#endif /* INC_SREC_H_ */
