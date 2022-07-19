# FOTA Graduation Project

In this project, MCU is able to update its application code wirelessly through the internet.
 - OEM uploads its update to a server
 - FOTA ECU checks periodically if there's any updates at the server for any ECU
 - When update is available at the server, FOTA ECU will receive and forward it to the suitable ECU through CAN bus 

# Hardware Connection
Figure below shows the hardware connection:
 - **FOTA ECU**: responsible for receiving updates from server using ESP8266 Wi-Fi module
 - **Other ECUs** such as ECU 0 are application ECUs that needs to be updated

![FOTA presentation](https://user-images.githubusercontent.com/71782873/179810511-8c45d5c1-1a9d-4654-a8e8-73bcc83169e2.jpg)

ECUs are connected through CAN bus

# Bootloader Design 
Flash memory is divided to store 5 appliactions:
 - **Branching code**: redirects the system to the active destination dendeing on the shared byte and active regions info.
 - **Bootloader 1**: in case it's the active bootloader, it receives and updates application and bootloader 2 codes. 
 - **Bootloader 2**: in case it's the active bootloader, it receives and updates application and bootloader 1 codes. 
 - **Application 1**: in case it's the active application, it will run when there's no updates to be received. 
 - **Application 2**: in case it's the active application, it will run when there's no updates to be received. 


# CAN Messages
For each application ECU, 2 IDs are used:
 - **Update payload ID**: to send the update content on it
 - **Control signals ID**: to send control messages on it, such as:
   - **Update request**: sent by FOTA ECU to notify application ECU that it has new update
   - **Acknowledge/Not acknowledge**: sent by appilcation ECU as a responce to the FOTA ECU
   - **Update type**: sent by FOTA ECU to inform application ECU about type of the update (Application or Bootloader)
   - **Termination signal**: sent by FOTA ECU to notify application ECU that update is sent completely
   - **Error signal**: sent by FOTA ECU to notify application ECU that an error happened, application ECU can go to application instead of waiting in bootloader


