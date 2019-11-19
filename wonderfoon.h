#ifdef DEBUGON
#define debug(text) Serial.println(text)
#define debug1(text) Serial.print(text)
#else
#define debug(text)
#define debug1(text)
#endif

// EEPROM defaults
#define VOLUMEDEFAULT 15
#define FOLDERDEFAULT 1
#define RANDOMDEFAULT false

// (too) small functions
#define playRandom() playTrackInFolder(random(1, 11), random(1, 4))

// EEPROM
#define ADDRVOLUME      0
#define ADDRFOLDER      1
#define ADDRRANDOM      2
#define ADDRINITIALISED 100

#define EEPROMINITIALISED 77

// feedback ID's -> mp3 song & folder numbers etc
#define FEEDBACK_FOLDER       4

#define FEEDBACK_CONTINUOUSPLAY 5
#define FEEDBACK_RESET        6
#define FEEDBACK_RANDOM_ON    4
#define FEEDBACK_RANDOM_OFF   3

#define FEEDBACK_PHONE_START 10
#define FEEDBACK_FOLDER1     11
#define FEEDBACK_FOLDER2     12
#define FEEDBACK_FOLDER3     13
#define FEEDBACK_VOLUME_BASE 11
#define FEEDBACK_DIALTONE    99
#define FEEDBACK_ALARM       112

// MP3 commands
#define MP3CMD_STARTBYTE   0x7E
#define MP3CMD_ENDBYTE     0xEF
#define MP3CMD_VERSIONBYTE 0xFF
#define MP3CMD_LENGTH      0x06
#define MP3CMD_ACKNOWLEDGE 0x00 //Returns info with command 0x41 [0x01: info, 0x00: no info]

#define MP3CMD_PLAYMODUS   0x09
#define MP3CMD_PLAYSD      0x02
#define MP3CMD_SLEEP       0x03
#define MP3CMD_PLAYTRACK   0x0F
#define MP3CMD_STOPPLAY    0x16
#define MP3CMD_VOLUME      0x06

#define MP3CMD_DEFAULTDELAY 500
