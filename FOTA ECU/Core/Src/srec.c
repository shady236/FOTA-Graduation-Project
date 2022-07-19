
#include  "srec.h"



static u8 valueOfAsciiDigit(u8 digit);
static u8 valueOfTwoAsciiDigits(u8 d1, u8 d0);
static void strToNumArr(u8 *str, u8 *arr, u8 arrSize);






static u8 valueOfAsciiDigit(u8 digit)
{
	switch (digit)
	{
		case '0' ... '9':	return (digit - '0');
		case 'A' ... 'F':	return (digit - 'A' + 10);
		case 'a' ... 'f':	return (digit - 'a' + 10);
	}
	return 0;
}



static u8 valueOfTwoAsciiDigits(u8 d1, u8 d0)
{
	return (valueOfAsciiDigit(d1) << 4) | valueOfAsciiDigit(d0);
}



static void strToNumArr(u8 *str, u8 *arr, u8 arrSize)
{
	for (u8 i=0; i<arrSize; i++)
	{
		arr[i] = valueOfTwoAsciiDigits(str[2*i], str[2*i+1]);
	}
}




SRecordHandlingError_t sRecordParse(u8* str, SRecord_t* record)
{
	/* S record example:  S31508010020000000000000000000000000C1030108F4
	 *
	 * S record can be divided into these fields:
	 * S3    15    08010020    00 00 00 00 00 00 00 00 00 00 00 00 C1 03 01 08   F4
	 *
	 * First field is the record type, it's 1 byte described by 2 ASCII hex digits
	 * Available record types are:
	 *     - S0 --> header record
	 *     - S1 --> data record with 2 byte address
	 *     - S2 --> data record with 3 byte address
	 *     - S3 --> data record with 4 byte address
	 *     - S5 --> 16 bit count of data records
	 *     - S6 --> 24 bit count of data records
	 *     - S7 --> termination record with 4 byte address for program start
	 *     - S8 --> termination record with 3 byte address for program start
	 *     - S9 --> termination record with 2 byte address for program start
	 *
	 * Second field is the bytes count, it's 1 byte described by 2 ASCII hex digits
	 * This field describes count of the remaining bytes in the record
	 *
	 * Third field is the address or the count of data records
	 * This field can be 2,3 or 4 bytes according to record type (first field)
	 *
	 * Fourth field is the data bytes, count of these data bytes can be
	 *     - total bytes count - 3   for S1 record, where total bytes count is the second field
	 *     - total bytes count - 4   for S2 record, where total bytes count is the second field
	 *     - total bytes count - 5   for S3 record, where total bytes count is the second field
	 *
	 * Last record is the checksum, it's 1 byte described by 2 ASCII hex digits
	 * This field is the 1's complement of the record bytes starting from second field
	 * This field is used for checking errors
	 *
	 *  */

	/* Any S record must start with character S */
	if (*str != 'S')		return RECORD_NOT_VALID;

	/* The character after S (character at index 1) describes the record type
	 *     - 0 --> header record
	 *     - 1 --> data record with 2 byte address
	 *     - 2 --> data record with 3 byte address
	 *     - 3 --> data record with 4 byte address
	 *     - 5 --> 16 bit count of data records
	 *     - 6 --> 24 bit count of data records
	 *     - 7 --> termination record with 16 bit address for program start
	 *     - 8 --> termination record with 24 bit address for program start
	 *     - 9 --> termination record with 32 bit address for program start
	 */
	record->recordType = str[1] - '0';

	/* The next 2 characters (characters at index 2 & 3) describes the bytes count */
	u8 totalBytesCount = valueOfTwoAsciiDigits(str[2], str[3]);

	/* To make sure check sum is correct */
	u8 bytesSum = totalBytesCount;

	u8 addressBytes[4];

	switch(record->recordType)
	{
		case HEADER_RECORD:
			record->dataBytesCount = totalBytesCount - 3;
			record->startAddress   = 0;
			/* Next characters are data bytes */
			strToNumArr(str + 8, record->data, record->dataBytesCount);
			break;

		case DATA_RECORD_16_BIT_ADDRESS:
		case DATA_RECORD_24_BIT_ADDRESS:
		case DATA_RECORD_32_BIT_ADDRESS:

			/* for data records, next characters after bytes count are address characters
			 * Address bytes = 2 for S1 record, Address bytes = 3 for S2 record, Address bytes = 4 for S3 record
			 * Address bytes = record type + 1
			 */
			strToNumArr(str + 4, addressBytes, record->recordType + 1);

			/* Convert start address into number
			 * NOTE: Address bytes = record type + 1
			 */
			record->startAddress = 0;
			for(u8 i=0; i <= record->recordType; i++)
			{
				record->startAddress |= addressBytes[i]<<(8 * (record->recordType - i));
				bytesSum += addressBytes[i];
			}

			/* Data bytes count = totalBytesCount - x     , where x=3 for S1 record , x=4 for S2 record , x=5 for S3 record
			 * Data bytes count = totalBytesCount - x - 2 , where x=1 for S1 record , x=2 for S2 record , x=3 for S3 record
			 * Data bytes count = totalBytesCount - x - 2 , for Sx record
			 */
			record->dataBytesCount = totalBytesCount - record->recordType - 2;

			/* Next characters are data bytes */
			strToNumArr(str + 6 + 2 * record->recordType, record->data, record->dataBytesCount);
			break;


		case COUNT_RECORD_16_BIT:
		case COUNT_RECORD_24_BIT:

			/* for data records count record, next characters after bytes count are count characters
			 * Count bytes = 2 for S5 record, Count bytes = 3 for S6 record
			 * Count bytes = record type - 3
			 * As there is no address, array addressBytes can be used here for temporary storage
			 */
			strToNumArr(str + 4, addressBytes, record->recordType - 3);

			/* Convert count into number
			 * NOTE: count bytes = record type - 3
			 */
			record->dataRecordsCount = 0;
			for(u8 i=0; i < record->recordType - 3; i++)
			{
				record->dataRecordsCount |= addressBytes[i]<<(8 * (record->recordType - 4 - i));
				bytesSum += addressBytes[i];
			}

			/* Data bytes count = 0 */
			record->dataBytesCount = 0;
			break;


		case END_RECORD_16_BIT_ADDRESS:
		case END_RECORD_24_BIT_ADDRESS:
		case END_RECORD_32_BIT_ADDRESS:

			/* for end records, next characters after bytes count are address characters
			 * Address bytes = 4 for S7 record, Address bytes = 3 for S8 record, Address bytes = 2 for S9 record
			 * Address bytes = 11 - record type
			 */
			strToNumArr(str + 4, addressBytes, 11 - record->recordType);

			/* Convert address into number
			 * NOTE: Address bytes = 11 - record type
			 */
			record->startAddress = 0;
			for(u8 i=0; i < 11 - record->recordType; i++)
			{
				record->startAddress |= addressBytes[i]<<(8 * (10 - record->recordType - i));
				bytesSum += addressBytes[i];
			}

			/* Data bytes count = 0 */
			record->dataBytesCount = 0;
			break;
	}

	/* Last 2 characters are checkSum */
	record->checkSum = valueOfTwoAsciiDigits(str[2UL * totalBytesCount + 2], str[2UL * totalBytesCount + 3]);

	/* Add data bytes to the bytesSum */
	for(u8 i=0; i<record->dataBytesCount; i++)
	{
		bytesSum += record->data[i];
	}

	bytesSum += record->checkSum;

	if (bytesSum != 0xFF)		return RECORD_CHECK_SUM_MISMATCH;
	return RECORD_NO_ERRORS;
}





SRecordHandlingError_t  srecHandle(SRecord_t* record)
{
	static u32 dataRecordsCounter   = 0;

	switch(record->recordType)
	{
		case HEADER_RECORD:
			dataRecordsCounter   = 0;
		break;

		case DATA_RECORD_16_BIT_ADDRESS:
		case DATA_RECORD_24_BIT_ADDRESS:
		case DATA_RECORD_32_BIT_ADDRESS:
			dataRecordsCounter++;

			u16 halfWordValue  ;
			u16 halfWordOffset ;
			u16 halfWordPageNum;

			u16 firstByteOffset  = (record->startAddress - FLASH_BASE_ADDRESS) & 0x3FF;
			u8  firstBytePageNum = (record->startAddress - FLASH_BASE_ADDRESS) >> 10;
			if(firstByteOffset % 2 == 1)
			{
				FPEC_DirectWriteByte(firstBytePageNum, firstByteOffset, record->data[0]);
			}
			for(u8 i=firstByteOffset%2; i<record->dataBytesCount; i++)
			{
				u16 byteOffset =  (firstByteOffset + i) & 0x3FF;
				if(byteOffset % 2 == 0)
				{
					halfWordOffset  = byteOffset;
					halfWordPageNum = (record->startAddress - FLASH_BASE_ADDRESS + i) >> 10;
					halfWordValue  &= 0xFF00;
					halfWordValue  |= record->data[i];
				}
				else
				{
					halfWordValue  &= 0x00FF;
					halfWordValue  |= (record->data[i]<<8);
					if(halfWordOffset == 0)
					{
						FPEC_ErasePage(halfWordPageNum);
					}
					FPEC_DirectWriteHalfWord(halfWordPageNum, halfWordOffset, halfWordValue);
				}
			}

			u16 lastByteOffset  = (record->startAddress - FLASH_BASE_ADDRESS + record->dataBytesCount - 1) & 0x3FF;
			u8  lastBytePageNum = (record->startAddress - FLASH_BASE_ADDRESS + record->dataBytesCount - 1) >> 10;

			if(lastByteOffset % 2 == 0)
			{
				if(lastByteOffset == 0)
				{
					FPEC_ErasePage(lastBytePageNum);
				}
				FPEC_DirectWriteByte(lastBytePageNum, lastByteOffset, record->data[record->dataBytesCount - 1]);
			}
		break;

		case COUNT_RECORD_16_BIT:
		case COUNT_RECORD_24_BIT:
			if(dataRecordsCounter != record->dataRecordsCount)
			{
				return DATA_RECORDS_COUNT_MISMATCH;
			}
		break;

		case END_RECORD_16_BIT_ADDRESS:
		case END_RECORD_24_BIT_ADDRESS:
		case END_RECORD_32_BIT_ADDRESS:
			dataRecordsCounter   = 0;
//			NVIC_RealocateVectorTable(FLASH_REGION, 0x08010000);
//			void (*resetHadler)(void) = record->startAddress;
//			resetHadler();
//			void (*resetHadler)(void) = FPEC_ReadWord(64, 4);
//			resetHadler();
//			return RECORD_NO_ERRORS;
		break;

		default:
			return RECORD_NOT_VALID;
		break;
	}

	return RECORD_NO_ERRORS;
}


