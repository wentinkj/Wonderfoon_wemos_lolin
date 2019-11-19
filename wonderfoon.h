#ifdef DEBUGON
#define debug(text) Serial.println(text)
#define debug1(text) Serial.print(text)
#else
#define debug(text)
#define debug1(text)
#endif

#define Start_Byte 0x7E
#define Version_Byte 0xFF
#define Command_Length 0x06
#define End_Byte 0xEF
#define Acknowledge 0x00 //Returns info with command 0x41 [0x01: info, 0x00: no info]
#define ACTIVATED LOW

