#ifdef DEBUGON
#define debug(text) Serial.println(text)
#define debug1(text) Serial.print(text)
#else
#define debug(text)
#define debug1(text)
#endif

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
#define FEEDBACK_RANDOM_ON    3
#define FEEDBACK_RANDOM_OFF   4

#define FEEDBACK_PHONE_START 10
#define FEEDBACK_FOLDER1     11
#define FEEDBACK_FOLDER2     12
#define FEEDBACK_FOLDER3     13
#define FEEDBACK_VOLUME_BASE 20
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

// used-to-be functions which did nothing more than wrap an MP3_execute call with "readable" naming
#define MP3Volume(volume) MP3_execute(MP3CMD_VOLUME, 0, volume, MP3CMD_DEFAULTDELAY)
#define MP3Stop() MP3_execute(MP3CMD_STOPPLAY, 0, 0, MP3CMD_DEFAULTDELAY)
#define MP3Wake() MP3_execute(MP3CMD_PLAYMODUS, 0, MP3CMD_PLAYSD, 500 + MP3CMD_DEFAULTDELAY)
#define playFeedback(feedbackTrack) MP3_execute(MP3CMD_PLAYTRACK, feedbackTrack, FEEDBACK_FOLDER, 2000 + MP3CMD_DEFAULTDELAY)

// dialdisc
const bool dialDisc = true;  // DialDisk
const byte pulsePin  = 2;     // the in for the telephone dialbusy (green)      D4 2
const byte busyPin   = 4;     // when ready playing                             D3 0
const byte hookPin   = 12;     // the in for the telephone hook                  D2 4
const byte dialPin   = 5;     //  the in for the telephone dialpulse (yellow)   D1 5

//dialpad config.b
const byte numPadType = 1;

const byte numPad[4][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {0, 10, 0}};
const byte rowPin[4] = {13, 15, 2, 14};  //   D7, D8, D4, D5
const byte colPin[3] = {5, 0, 14 };        //  D1, D?, D5

int row[4];   //Array dialpad rows
int col[3];   //Array dialpad cols
int num;      // dialpad number
int lastNum;  //
//dialpad config.e
