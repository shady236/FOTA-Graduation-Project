
#ifndef INC_SERVERCOMM_H_
#define INC_SERVERCOMM_H_

#include "STD_types.h"
#include "ESP.h"
#include "ECU_Config.h"











void  serverCommInit(void);
void  serverTerminateConn(void);
Boolean_t  getEcuLatestVersion(u8 ecuNum, ImgType_t imgType, u8* version);
Boolean_t  getNextLineOfUpdate(u8* data, u8* dataLen);



#endif /* INC_SERVERCOMM_H_ */
