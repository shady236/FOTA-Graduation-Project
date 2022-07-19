
#ifndef INC_ECU_COMM_H_
#define INC_ECU_COMM_H_


#include "STD_types.h"
#include "srec.h"
#include "CAN.h"
#include "ECU_Config.h"




void  ECU_CommInit(void);
Boolean_t  ECU_SendUpdateReq(u8 ecuNum, ImgType_t imgType);
Boolean_t  ECU_TxRec(u8 ecuNum, SRecord_t* record);
void  ECU_TerminateConn(u8 ecuNum, Boolean_t state);


#endif /* INC_ECU_COMM_H_ */
