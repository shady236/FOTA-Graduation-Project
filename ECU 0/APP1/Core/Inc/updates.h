
#ifndef INC_UPDATES_H_
#define INC_UPDATES_H_


#include "STD_types.h"
#include "CAN.h"
#include "ECU_Config.h"


typedef enum
{
	IGNORE_UPDATES,
	READY_FOR_UPDATES
}ECU_UpdatesState_t;


void  updatesInit(void);
void  updatesSetState(ECU_UpdatesState_t  state, void (*updatesAvailableCallBackFun)(void));



#endif /* INC_UPDATES_H_ */
