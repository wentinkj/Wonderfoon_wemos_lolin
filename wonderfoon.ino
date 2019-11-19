/*--------------------------------------------------------------------------------------------------
 * Wonderfoon D1Mini 1001
 * http://wonderfoon.eu
 * WEMOS version
 * 
 *  TEST VERSION, derived from https://github.com/hvtil/Wonderfoon_wemos_lolin
 *--------------------------------------------------------------------------------------------------
 */
#define DEBUGON
#include <EEPROM.h>
#include "ESP8266WiFi.h"
#include "wonderfoon.h"

// IO-PIN configuration

// connected to phone hook 
const byte hookPin   = 14;
// connected to rotary disc to detect the dial start
const byte dialPin   = 13;
// connected to rotary disc to count pulses (dialed digit)
const byte pulsePin  = 12;
// connected to MP3 module busy pin to check if still playing
const byte busyPin   = 16;

// STATE
byte buttons[] = {hookPin, dialPin, pulsePin};
volatile byte pressed[sizeof(buttons)];

bool continuousPlay = false;
bool randomPlay = RANDOMDEFAULT;
byte folderNumber = FOLDERDEFAULT;
byte audioVolume = VOLUMEDEFAULT;
int dialedPhoneNumber = 0;

/*******************************************************************
*  The function/typedef name
*
* Description of the function/typedef purpose
*******************************************************************/
void setup() {
  delay(100);
  EEPROM.begin(256);                     // using 0-20 max
  Serial.begin(9600);                    // start serial for debug and mp3
  copyStateFromEEPROM(false);
  debug("Booting");

  // button setup, set the pins to input and add pullup resitor
  for (int idx=0; idx < sizeof(buttons); idx++) {
    pinMode(buttons[idx], INPUT_PULLUP);
  }

  MP3Wake();
  MP3Volume(audioVolume);
  playFeedback(FEEDBACK_PHONE_START);
  debug("Setup ready");
  debug("Start");
  playFeedback(FEEDBACK_VOLUME_BASE + audioVolume);
  playFeedback(randomPlay ? FEEDBACK_RANDOM_ON : FEEDBACK_RANDOM_OFF);
  playFeedback(FEEDBACK_FOLDER1);      // play folder number status
  MP3Sleep();                                             // set mp3 to battry save mode
  debug("Ready....");
}

/*******************************************************************
*  loop
*
* main program loop which checks the different situations which could occur
*******************************************************************/
// wait for someone to pick up the hook
void loop() {
  static byte pulseCount;
  static byte hookPrevious = false;
  static byte dialPrevious = false;
  static byte pulsePrevious = false;

  /* hook is picked up
   */
  checkinput(0, 50);
  if (pressed[0] != hookPrevious) {
    if (pressed[0]) {
      debug("The hook is picked up");
      continuousPlay = false;
      MP3Wake();
      MP3Volume(audioVolume);
      playTrackInFolder(FEEDBACK_DIALTONE, FEEDBACK_FOLDER);
    } else {
      debug("The hook is down on the phone");
      MP3Stop();
      MP3Sleep();
    }
    hookPrevious = pressed[0];
  }

  checkinput(1, 50);
  if (pressed[1] != dialPrevious) {
    if (pressed[1]) {
      debug("dial start");
      pulseCount = 0;
    } else {
      debug("dial end");
      // only process if hook is up
      if (pressed[0]) {
        addDigitToPhoneNumber(pulseCount);
        debug("dialed "  + String(dialedPhoneNumber));
        // check and execute special numbers
        if (!checkDialCommands(dialedPhoneNumber)) {
          if (randomPlay) {
            playRandom();
          } else {
            debug("playing song " + String(dialedPhoneNumber % 10));
            playTrackInFolder(dialedPhoneNumber % 10, folderNumber);
          }
        } else {
          // clear dialed
          dialedPhoneNumber = 0;
        }
      }
    }
    dialPrevious = pressed[1];
  }

  /* pulse count
   */
  checkinput(2, 20);
  if (pressed[2] != pulsePrevious) {
    pulsePrevious = pressed[2];
    if (pressed[2] && pressed[1]) {
      pulseCount++;
    }
  }

  /* if continuous play is enabled and the hook is picked up
   */
  if (continuousPlay && pressed[0]) {
    // and not playing, get the next song
    if (digitalRead(busyPin) == 1) {
      debug("Check Playing: start next random number");
      delay(1000);
      playRandom();
    }
  }  
}


/*******************************************************************
*  copyStateFromEEPROM
*
* 
*******************************************************************/
// initialize to check if this is the first time the Wonderfoon is started addess 100 = 77
void copyStateFromEEPROM(bool forceDefaults) {
  EEPROM_init(forceDefaults); 
  folderNumber = EEPROM_getValue(ADDRFOLDER);
  audioVolume = EEPROM_getValue(ADDRVOLUME);
  randomPlay = EEPROM_getValue(ADDRRANDOM);
}


/*******************************************************************
*  check_inputs
*
* reads and describes the states of the inputpins
* attempts to debounce the inputs loosely based on: 
* https://blog.adafruit.com/2009/10/20/example-code-for-multi-button-checker-with-debouncing/
*******************************************************************/
bool checkinput(int idx, int debounce) {
  static byte previousstate[sizeof(buttons)];
  static byte currentstate[sizeof(buttons)];
  static long lasttime[sizeof(buttons)];
  
  if (   lasttime[idx] > millis()
      || previousstate[idx] != currentstate[idx]) {
      lasttime[idx] = millis();
  }

  if (millis() - lasttime[idx] > debounce) {
    currentstate[idx] = digitalRead(buttons[idx]);
    if (currentstate[idx] == previousstate[idx]) {
      pressed[idx] = (currentstate[idx] == LOW);
    } else {
      lasttime[idx] = millis();
    }
  }
  previousstate[idx] = currentstate[idx];
  return pressed[idx];
}

/*******************************************************************
*  addDigitToPhoneNumber
*
* combines the new digit with the existing (if dialed within 10 seconds)
*******************************************************************/
int addDigitToPhoneNumber(int digit) {
  static long firstDial = 0;

  // allow for 10 seconds to dial a number
  if (millis() - firstDial > 10000) {
    firstDial = millis();
    dialedPhoneNumber = digit;
  } else { 
    dialedPhoneNumber = (dialedPhoneNumber * 10) + digit;
  }

  // only the last 3 digits are relevant
  return (dialedPhoneNumber % 1000);
}

/*******************************************************************
*  checkDialCommands
*
* check (and execute) commands given by dialing a number
*******************************************************************/
bool checkDialCommands(int dialed) {
  switch (dialed) {
    case 112:
      debug("play alarm");
      MP3Volume(19);
      playTrackInFolder(FEEDBACK_ALARM, FEEDBACK_FOLDER);
      delay(14000);
      MP3Volume(audioVolume);
      return true;
    case 211:
    case 212:
    case 213:
    case 214:
    case 215:
    case 216:
    case 217:
    case 218:
    case 219:
    case 220:
      audioVolume = (dialed - 201); // -210 + 9
      debug("volume " + String(audioVolume));
      MP3Volume(audioVolume);
      EEPROM_storeValue(ADDRVOLUME, audioVolume);
      playFeedback(dialed - 190); // 21 -30 for feedack
      return true;
    case 311:
    case 312:
    case 313:
      debug("folder change");
      folderNumber = dialed % 10;
      EEPROM_storeValue(ADDRFOLDER, folderNumber);
      playFeedback(FEEDBACK_FOLDER1 + folderNumber - 1);
      return true;
    case 411:
      debug("play random off");
      randomPlay = false;
      EEPROM_storeValue(ADDRRANDOM, randomPlay);
      playFeedback(FEEDBACK_RANDOM_OFF);      
      return true;
    case 412:
      debug("play random on");
      randomPlay = true;
      EEPROM_storeValue(ADDRRANDOM, randomPlay);
      playFeedback(FEEDBACK_RANDOM_ON);
      return true;
    case 511:
      debug("continuous random play");
      continuousPlay = true;
      playFeedback(FEEDBACK_CONTINUOUSPLAY);
      return true;
    case 999:
      debug("reset");
      copyStateFromEEPROM(true);
      MP3Volume(audioVolume);
      playFeedback(FEEDBACK_RESET);
      return true;
  }
  return false;
}

/*******************************************************************
*  MP3Sleep
*
* Let the MP3 module sleep if not used
*******************************************************************/
void MP3Sleep() {
  MP3_execute(MP3CMD_PLAYMODUS, 0, MP3CMD_SLEEP, MP3CMD_DEFAULTDELAY); // Set MP3 player in sleep mode
  MP3_execute(0x0A, 0, 0, MP3CMD_DEFAULTDELAY); // Set MP3 player in power loss
}

void MP3Volume(int volume) { MP3_execute(MP3CMD_VOLUME, 0, volume, MP3CMD_DEFAULTDELAY); }
void MP3Stop() { MP3_execute(MP3CMD_STOPPLAY, 0, 0, MP3CMD_DEFAULTDELAY); }
void MP3Wake() { MP3_execute(MP3CMD_PLAYMODUS, 0, MP3CMD_PLAYSD, 500 + MP3CMD_DEFAULTDELAY); }


void playFeedback (int feedbackTrack) {
  playTrackInFolder(feedbackTrack, FEEDBACK_FOLDER);
  delay(2000);
}

/*******************************************************************
*  playTrackInFolder
*
* wrapper or MP3_execute to play a track in a folder on the SD-card
*******************************************************************/
void playTrackInFolder(int track, int folder) {
  debug( "playTrackInFolder" + String(track) + "-" + String(folder));
  MP3_execute(MP3CMD_PLAYTRACK, folder, track, MP3CMD_DEFAULTDELAY);
}


/*******************************************************************
*  MP3_execute
*
* Send serial command to the MP3 module
*******************************************************************/
// Excecute the command and parameters
void MP3_execute(byte command, byte param1, byte param2, int delayafter) {
  debug("MP3CMD " + String(command) + " - " + String (param1) + " - " + String(param2));
  word checksum = -(MP3CMD_VERSIONBYTE + MP3CMD_LENGTH + command + MP3CMD_ACKNOWLEDGE + param1 + param2);   // Calculate the checksum (2 bytes)
  byte cammand_line[10] = { MP3CMD_STARTBYTE
                          , MP3CMD_VERSIONBYTE
                          , MP3CMD_LENGTH
                          , command
                          , MP3CMD_ACKNOWLEDGE
                          , param1
                          , param2
                          , highByte(checksum)
                          , lowByte(checksum)
                          , MP3CMD_ENDBYTE
                          };
  for (byte k = 0; k < 10; k++) {
    Serial.write( cammand_line[k]);
  }
  delay(delayafter);
}


/*******************************************************************
*  EEPROM_init
*
* check if not initialised or forced initalisation then store default values
*******************************************************************/
void EEPROM_init(bool force) {
  debug("EEPROM_init");
  int initRead = EEPROM_getValue(ADDRINITIALISED);
  if (initRead != EEPROMINITIALISED || force) {
    debug ("EEPROM init defaults");
    EEPROM_storeValue(ADDRVOLUME, VOLUMEDEFAULT);
    EEPROM_storeValue(ADDRFOLDER, FOLDERDEFAULT);
    EEPROM_storeValue(ADDRRANDOM, RANDOMDEFAULT);
  }
  EEPROM.commit();
}

/*******************************************************************
*  EEPROM_clear
*
* Overwrites the addresses in the EEPROM
*******************************************************************/
void EEPROM_clear() {
 for (int L = 0; L < 7; ++L) {
    EEPROM.write(0 + L, 254);
  }
  EEPROM.write(ADDRINITIALISED, 254);
  EEPROM.commit();
}

/*******************************************************************
*  EEPROM_storeValue
*
* Store an int value at the given EEPROM address
*******************************************************************/
int EEPROM_storeValue(int address, int value) {
  EEPROM.write(address, value);
  delay(500);  // delay to prevent crashes during storing data
  EEPROM.commit();
  
  return value;
}

/*******************************************************************
*  EEPROM_getValue
*
* Retrieve the value from the EEPROM for the given address
*******************************************************************/
int EEPROM_getValue(int address) {
  byte val = EEPROM.read(address);
  return (int) val;
}
