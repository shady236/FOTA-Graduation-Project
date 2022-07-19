#ifndef ESP_H_
#define ESP_H_

#include "STD_types.h"
#include <UART.h>


#include  "ESP.h"


u8 ESP_ReadRxDataFromServer(u8** data, u16* dataLen);
void ESP_ClrBuffer(void);
void ESP_Init(void (*ptrToFun)(void));
u8 ESP_ConnectToRouter(const u8* ssid, const u8* password);
u8 ESP_SetWiFiMode(void);
u8 ESP_SetAsTCPClient(const u8* serverIP, const u8* portnum);
u8 ESP_EndTCPConnection(void);
u8 ESP_EnableUARTWiFiPassthroughMode(void);
u8 ESP_DisableUARTWiFiPassthroughMode(void);
u8 ESP_StartSendingData(const u8* data, u16 dataLen);


#endif
