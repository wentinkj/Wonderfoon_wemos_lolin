/*--------------------------------------------------------------------------------------------------
 * Wonderfoon D1Mini 1001
 * http://wonderfoon.eu
 * WEMOS version
 * 
 * 
 *--------------------------------------------------------------------------------------------------
 */
#include <EEPROM.h>
#include "ESP8266WiFi.h"
#include "wonderfoon.h"

#define DEBUGON

#define VOLUMEDEFAULT 15
#define FOLDERDEFAULT 1
#define RANDOMDEFAULT false

// STATE
byte buttons[] = {hookPin, dialPin, pulsePin};
#define NUMBUTTONS sizeof(buttons)
volatile byte pressed[NUMBUTTONS], justreleased[NUMBUTTONS], justpressed[NUMBUTTONS];

bool continuousPlay = false;
bool randomPlay = RANDOMDEFAULT;
byte folderNumber = FOLDERDEFAULT;
byte audioVolume = VOLUMEDEFAULT;



void setup() {
  delay(100);
  EEPROM.begin(256);                     // using 0-20 max
  Serial.begin(9600);                    // start serial for debug and mp3
  copyStateFromEEPROM(false);
  debug("Booting");                      // 2 debuglines that will always be displayed in logging.

  // button setup
  pinMode(hookPin, INPUT_PULLUP);                 //Set pins to input and add pullup resitor
  pinMode(dialPin, INPUT_PULLUP);
  pinMode(pulsePin, INPUT_PULLUP);

//  // Setup for Numpad.b
//  for (int thisPin = 0; thisPin < (sizeof(rowPin) / sizeof(rowPin[0])); thisPin++) {  // Set pins to input and add pullup resitor
//    pinMode(rowPin[thisPin], INPUT_PULLUP);
//  }
//  //
//  for (int thisPin = 0; thisPin < (sizeof(colPin) / sizeof(colPin[0])) ; thisPin++) { // Set pins to input and add pullup resitor
//    pinMode(colPin[thisPin], INPUT_PULLUP);
//  }

  MP3Wake();
  MP3Volume(audioVolume);
  playFeedback(FEEDBACK_PHONE_START);
  debug("Setup ready");
  debug("Start");
  playFeedback(FEEDBACK_VOLUME_BASE + audioVolume);
  playFeedback(randomPlay ? FEEDBACK_RANDOM_ON : FEEDBACK_RANDOM_OFF);
  playFeedback(FEEDBACK_FOLDER1 + folderNumber - 1);      // play folder number status
  MP3Sleep();                                             // set mp3 to battry save mode
  debug("Ready....");
}

// wait for someone to pick up the hook
void loop() {
  static byte pulseCount;
  check_inputs();

  // hook is picked up
  if (justpressed[0]) {
    debug("The hook is picked up");         // only when hookstate changes and goes to low
    continuousPlay = false;
    MP3Wake();
    MP3Volume(audioVolume);
    playTrackInFolder(FEEDBACK_DIALTONE, FEEDBACK_FOLDER);
  }
  
  // the hook is down
  if (justreleased[0]) {
    debug("The hook is down on the phone");
    MP3Stop();
    MP3Sleep();
  }

  // dial start
  if (justpressed[1]) {
    debug("dial start")
    pulseCount = 0;
  }

  // dial end
  if (justreleased[1]) {
    debug("dial end");
    // check hook state to see if the number should be processed
    if (!pressed[0])
      return;
      
    // check pulseCount
    int dialed = addDigitToPhoneNumber(pulseCount);
    debug("dialed "  + String(dialed));
    if (!checkDialCommands(dialed)) {
      // if random play
      if (randomPlay) {
        playRandom();
      } else {
        debug("playing song " + dialed);
      }
    }
  }

  // pulse count
  if (justpressed[2]) {
    pulseCount++;
  }

  // if continuous play is enabled and the hook is picked up
  if (continuousPlay && pressed[0]) {
    // and not playing, get the next song
    if (digitalRead(busyPin) == 1) {
      debug("Check Playing: start next random number");
      delay(1000);
      playRandom();
    }
  }  
}


// initialize to check if this is the first time the Wonderfoon is started addess 100 = 77
void copyStateFromEEPROM(bool forceDefaults) {
  EEPROM_init(forceDefaults); 
  folderNumber = EEPROM_getValue(ADDRFOLDER);
  audioVolume = EEPROM_getValue(ADDRVOLUME);
  randomPlay = EEPROM_getValue(ADDRRANDOM);
}

void check_inputs() {
  static byte previousstate[NUMBUTTONS];
  static byte currentstate[NUMBUTTONS];
  static long lasttime = 0;

  if (millis() < lasttime) {  // in case of rollover
    lasttime = millis();
    return;
  }
  if (millis() - lasttime < 30) {
    return;
  }
  lasttime = millis();

  for (int idx = 0; idx < NUMBUTTONS; idx++) {
    justreleased[idx] = 0;
    currentstate[idx] = digitalRead(buttons[idx]);

    if (currentstate[idx] == previousstate[idx]) {
      if ((pressed[idx] == LOW) && (currentstate[idx] == LOW)) {
          justpressed[idx] = 1;
      }
      else if ((pressed[idx] == HIGH) && (currentstate[idx] == HIGH)) {
          justreleased[idx] = 1;
      }
      pressed[idx] = !currentstate[idx];  // remember, digital HIGH means NOT pressed
    }
    previousstate[idx] = currentstate[idx];   // keep a running tally of the buttons

  }
}

int addDigitToPhoneNumber(int digit) {
  static int number = 0;
  static long firstDial = 0;

  // allow for 10 seconds to dial a number
  if (millis() - firstDial > 10000) {
    firstDial = millis();
    number = digit;
  } else { 
    number = (number * 10) + digit;
  }

  // only the last 3 digits are relevant
  return (number % 1000);
}

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
      return true;
    case 311:
      debug("folder 1");
      playFeedback(FEEDBACK_FOLDER1);
      folderNumber = 1;
      return true;      
    case 312:
      debug("folder 2");
      playFeedback(FEEDBACK_FOLDER2);      
      folderNumber = 2;
      return true;
    case 313:
      debug("folder 3");
      playFeedback(FEEDBACK_FOLDER3);
      folderNumber = 3;
      return true;
    case 411:
      debug("play random off");
      randomPlay = false;
      playFeedback(FEEDBACK_RANDOM_OFF);      
      return true;
    case 412:
      debug("play random on");
      randomPlay = true;
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

/*--------------------------------------------------------------------------------------------------
 * function_mp3
 * 
 *--------------------------------------------------------------------------------------------------
 */

void MP3Sleep() {
  MP3_execute(MP3CMD_PLAYMODUS, 0, MP3CMD_SLEEP, MP3CMD_DEFAULTDELAY); // Set MP3 player in sleep mode
  MP3_execute(0x0A, 0, 0, MP3CMD_DEFAULTDELAY); // Set MP3 player in power loss
}

void playTrackInFolder(int track, int folder) {
  debug( "playTrackInFolder" + String(track) + "-" + String(folder));
  MP3_execute(MP3CMD_PLAYTRACK, folder, track, MP3CMD_DEFAULTDELAY);
}

// Excecute the command and parameters
void MP3_execute(byte command, byte param1, byte param2, int delayafter) {
//  debug("CMD " + String(CMD) + " - " + String (param1) + " - " + String(param2));
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

/*--------------------------------------------------------------------------------------------------
 * function_EEprom
 * 
 *--------------------------------------------------------------------------------------------------
 */
//EEPROM

//   Stores values read from analog input 0 into the EEPROM.
//   These values will stay in the EEPROM when the board is
//   turned off and may be retrieved later by another sketch.

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

void EEPROM_clear(){
 for (int L = 0; L < 7; ++L) {
    EEPROM.write(0 + L, 254);
  }
  EEPROM.write(100, 254);
  EEPROM.commit();
  }

int EEPROM_storeValue(int address, int value) {
  EEPROM.write(address, value);
  delay(500);  // delay to prevent crashes during storing data
  EEPROM.commit();
  
  return value;
}

int EEPROM_getValue(int address) {
  byte val = EEPROM.read(address);
  return (int) val;
}

/*--------------------------------------------------------------------------------------------------
 * function_numpad
 * 
 *--------------------------------------------------------------------------------------------------
 */

///  Numpad type 1  4x3 numpad 7 wire and ground
//void checkNumPad() {
//  delay(20);                                     // check every 20 ms
//  int czer = 0;                                  // reset column
//  int rzer = 0;                                  // reset row
//
//  for (int thisRow = 0; thisRow < (sizeof(row) / sizeof(row[0])); thisRow++) {
//    row[thisRow] = digitalRead(rowPin[thisRow]);                                  // read if a row is pressed
//    if (row[thisRow] == 0 && rzer == 0)
//    {
//      rzer = 1;
//      break;
//    }
//  }
//  //debug("");
//  //debug1(" col ");
//  for (int thisCol = 0; thisCol < (sizeof(col) / sizeof(col[0])) ; thisCol++) {
//    col[thisCol] = digitalRead(colPin[thisCol]);                                   // read if a col is pressed
//    if (col[thisCol] == 0 && czer == 0)
//    {
//      czer = 1;
//      break;
//    }
//  }
//
//  if (czer && rzer) {                                                             // if both are 1 ( to prevent the reading of the row and col half way between row and col a change was happening.
//    PlayingContinuesly = false ;
//    num = numPad[arrayFind("row")][arrayFind("col")] ;                            // find the number that belongs to this combination of row and col in the numPad array
//
//    if (num > 10 ) {                                                              // 11 and 12 are * and #
//      num = 0;
//    }
//    //debug(String (num));
//    debug("Number = " + String(num));
//    if (num != 0 && num != lastNum) {       // 0=no button pressed lastNum to check if number changed
//      mp3Wake();                            // Wake up MP3 since hook is picked up
//      playTrackInFolder(num, folderNumber);                      // Play song
//      addLastNumber(num);                   // add last dailed number for pin array
//      addLastTime(millis());                // add last time for pin
//      lastNum = num;
//    }
//  }
//  else lastNum = 0;                         // if button is
//  // debug("");
//  debug(String(lastNum));
//}
//
//int arrayFind(String type) {
//
//  int wantedVal = 0;
//  int wantedPos = 5 ;
//  if ( type == "row") {
//
//    for (int i = 0; i < 4; i++) {
//
//      if (wantedVal == row[i]) {
//        wantedPos = i;
//        break;
//      }
//    }
//  }
//  else {
//    for (int i = 0; i < 3; i++) {
//
//      if (wantedVal == col[i]) {
//        wantedPos = i;
//        break;
//      }
//    }
//  }
//
//  return wantedPos;
//}
