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

const bool dialDisc = true;  // DialDisk
const int numPadType = 1;

// Variables for Dialdisc Hook and Amplifier

//const int hookPin   = 12;     // the in for the telephone hook                 D6 12
 //const int dialPin   = 14;     //  the in for the telephone dialpulse (yellow)  D5 14
//const int pulsePin  = 13;     // the in for the telephone dialbusy (green)      D7 13
//const int ampPin    = 15;     // to turn amplifier on / off                   D2 15
//const int busyPin   = 16;     // when ready playing                           D0 16

const int pulsePin  = 2;     // the in for the telephone dialbusy (green)      D4 2
const int busyPin   = 4;     // when ready playing                             D3 0
const int hookPin   = 12;     // the in for the telephone hook                  D2 4
const int dialPin   = 5;     //  the in for the telephone dialpulse (yellow)   D1 5
//const int ampPin    = 15;     // to turn amplifier on / off                   D2 15

//dialpad config.b
const int numPad[4][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {0, 10, 0}};
const int rowPin[4] = {13, 15, 2, 14};  //   D7, D8, D4, D5
const int colPin[3] = {5, 0, 14 };        //  D1, D?, D5


//const int rowPin[4] = {D7, D8, D9, D13};  //  D7 D8 D9 D13
//const int colPin[3] = {D3, D4, D5};         //  D3 D4 D5

int row[4];   //Array dialpad rows
int col[3];   //Array dialpad cols
int num;      // dialpad number
int lastNum;  //
//dialpad config.e


//boolean isPlaying = false;  // mp3

int countedPulses;          // then number of pulses counted at the end of dialbusy
int pulseCount;             // number of pulses during counting
int audioVolume;            // Audio Volume
int hookState = HIGH;       // the current state of the hook
int dialState = HIGH;       // the current state of the dailbusy
int pulseState = HIGH;      // the current state of the pulse
int lastHookState = HIGH;   // the previous reading from the hook
int lastDialState = HIGH;   // the previous reading from dailbusy
int lastPulseState = HIGH;  // the previous reading from the pulse
//bool apmlifierState = false;

int lastDialed[4] = {20, 21, 22, 23};   // last 4 dialed digits for PIN
int folderNumber;                       // current folderNumber
int playMode;                           // current playmode

bool PlayingContinuesly = false ;             // random playing mode from app or keypad  *

unsigned long lastTime[4];                // time last for digits were dialed to determine pin

// the following variables are unsigned long's because the time, measured in miliseconds,

unsigned long lastHookDebounceTime = 0;   // the last time the hook pin was toggled
unsigned long lastDialDebounceTime = 0;   // the last time the dial pin was
unsigned long lastPulseDebounceTime = 0;  // the last time the pulse pin was
unsigned long hookDebounceDelay = 50;     // the debounce time; increase if the output flickers
unsigned long dialDebounceDelay = 50;     // the debounce time;
unsigned long pulseDebounceDelay = 20;    // the debounce time;
unsigned long waitNextDial = 0;           // wait 1 second for next number  // not in use or depreciated


void setup() {
  delay(100);
  EEPROM.begin(256);                      // using 0-20 max
  Serial.begin(9600);                             // start serial for debug and mp3
  EEPROM_init(0);                          // initialize to check if this is the first time the Wonderfoon is started addess 100 = 77

  folderNumber = EEPROM_getFolder();
  audioVolume = EEPROM_getVolume();
  //setMP3Volume(audioVolume);
  playMode = EEPROM_getPlayMode();

  Serial.println("Booting");                      // 2 debuglines that will always be displayed in logging.
  pinMode(hookPin, INPUT_PULLUP);                 //Set pins to input and add pullup resitor
  pinMode(dialPin, INPUT_PULLUP);
  pinMode(pulsePin, INPUT_PULLUP);
//  pinMode(ampPin, OUTPUT);                        //Set Aplifier pin to output

  // Setup for Numpad.b
  for (int thisPin = 0; thisPin < (sizeof(rowPin) / sizeof(rowPin[0])); thisPin++) {  // Set pins to input and add pullup resitor
    pinMode(rowPin[thisPin], INPUT_PULLUP);
  }
  //
  for (int thisPin = 0; thisPin < (sizeof(colPin) / sizeof(colPin[0])) ; thisPin++) { // Set pins to input and add pullup resitor
    pinMode(colPin[thisPin], INPUT_PULLUP);
  }

  // Setup for Numpad.b
  // switch amplifier on to read boot feedback
  mp3Wake();
  setMP3Volume(audioVolume);
  playTrackInFolder(10, 4);                               // Wonderfoon has started
  delay(2000);
  //put mp3 in sleep mode to save battery
  debug("Setup ready");
  debug("Start");
  debug("");
  playVolume();                                           // play vulume status
  playWillekeurig(playMode);                              // play random status
  playFolder(folderNumber);                               // play folder number status
  mp3Sleep();                                             // set mp3 to battry save mode
  Serial.println("");
  Serial.println("Ready....");
}

// wait for someone to pick up the hook
void loop() {
  // read the state hook switch into a local variable:
  int hookReading = digitalRead(hookPin);

   // If the hook changed, due to noise or pressing:
  if (hookReading != lastHookState) {
    lastHookDebounceTime = millis();
  }

  if ((millis() - lastHookDebounceTime) > hookDebounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the hook state has changed:
    if (hookReading != hookState) {             // (one time action)
      hookState = hookReading;                  // put the value read to the current hookstate.
      if (hookState == LOW) {                   // check state is LOW
        debug("The hook is picked up");         // only when hookstate changes and goes to low
        mp3Wake();
        setMP3Volume(audioVolume);
        playTrackInFolder(99, 4);               // Play dailtone
      }

      else {                                    // if not low but high (still one time action)
        debug("The hook is down on the phone");
        countedPulses = 0;                     //reset counting parameters
        pulseCount = 0;
        MP3stop();                             // Stop MP3Player
        mp3Sleep();                            // Put MP3 is sleep mode since hook is down
        PlayingContinuesly = false;
      }
      debug("");
    }
  }

  if (hookState == LOW) {                       // if Hook is LOW waitforDial or checkNumPad dependig on the version
    if (dialDisc)
    {
      waitForDial();  // every loop we check if hookstate is low and wait for dial.
    }
    else {
      checkNumPad();
    }
    checkPlaying();
  }
  if (hookState == HIGH) {
  }
  lastHookState = hookReading;    //store current status in lastHookState for bounceTime check  
}

/*--------------------------------------------------------------------------------------------------
 * function_general
 * 
 *--------------------------------------------------------------------------------------------------
 */

void clearLastDialed()
{
  lastDialed[0] = 20;
  lastDialed[1] = 21;
  lastDialed[2] = 22;
  lastDialed[3] = 23;
}
//other common functions

int playRand() {
  return random(1, 11);   // Randomizer for track
}
int foldRand() {          // Randomizer for folder
  return random(1, 4);  // (3)
}

void addLastNumber(int newNumber) {
  debug("add " + String(newNumber));
  lastDialed[0] = lastDialed[1];
  lastDialed[1] = lastDialed[2];
  lastDialed[2] = lastDialed[3];
  lastDialed[3] = newNumber;
}

void addLastTime(long newTime) {
  lastTime[0] = lastTime[1];
  lastTime[1] = lastTime[2];
  lastTime[2] = lastTime[3];
  lastTime[3] = newTime;
}

void checkChangeVolume()
{
  if (lastDialed[1] == 2 && lastDialed[2] == 1 && checkChangeTime())
  {
    debug("volume change");
    audioVolume = lastDialed[3] + 9;
    setMP3Volume(audioVolume);
    storeMP3Volume(audioVolume);
    playVolume();
    clearLastDialed();
  }
}

void checkChangeFolder()
{
  if (lastDialed[1] == 3 && lastDialed[2] == 1 && checkChangeTime())
  {
    debug("folder change");
    if ( lastDialed[3] < 4 )
    {
      EEPROM_storeFolder(lastDialed[3]);
      playFolder(lastDialed[3]);
      clearLastDialed();
    }
  }
}

void checkChangeWillekeurig()
{
  if (lastDialed[1] == 4 && lastDialed[2] == 1 && checkChangeTime())
  {
    debug("Speel willekeurig");
    if ( lastDialed[3] < 3 )
    {
      EEPROM_storePlayMode(lastDialed[3] - 1);
      playWillekeurig(lastDialed[3] - 1);
      clearLastDialed();
    }

  }
}

void checkReset()
{
  if (lastDialed[1] == 9 && lastDialed[2] == 9 && checkChangeTime())
  {
    //willekeurig
    if ( lastDialed[3] == 9 )
    {
      playReset();
      EEPROM_init(true);
      audioVolume = EEPROM_getVolume();
      setMP3Volume(audioVolume);
      folderNumber = EEPROM_getFolder();
      playMode = EEPROM_getPlayMode();
      mp3Wake();
      playVolume();                                           // play vulume status
      playFolder(folderNumber);                               // play folder number status
      playWillekeurig(playMode);
      clearLastDialed();
    }
  }
}

void checkChangeContinues() {
  if (lastDialed[1] == 5 && lastDialed[2] == 1 && checkChangeTime())
  {
    debug("continu change");
    if ( lastDialed[3] == 1 )
    {
      debug("PlayRandom");
      PlayingContinuesly = true ;
      playContinu();
      // playRandom();
      clearLastDialed();
    }
  }
}

bool check112() {
  if (lastDialed[1] == 1 && lastDialed[2] == 1 && checkChangeTime())
  {
    if ( lastDialed[3] == 2 )
    {
      playAlarm();
      clearLastDialed();
    }
  }
}

bool checkChangeTime() {
  if (lastTime[3] - lastTime[1] < 10000)
    return true;
  else
    return false;
}

void checkAll() {

  checkChangeVolume();
  checkChangeFolder();
  checkChangeWillekeurig();
  checkChangeContinues();
  check112();
  checkReset();
}



/*--------------------------------------------------------------------------------------------------
 * function_mp3
 * 
 *--------------------------------------------------------------------------------------------------
 */
void checkPlaying()
{
  //debug("Check Playing:");
  if (digitalRead(busyPin) == 1) {
    if (PlayingContinuesly) 
    { debug("Check Playing: start next random number");
      delay(1000);
      playRandom();
    }
  }
}

void setMP3Volume(int volume)
{
  execute_CMD(0x06, 0, volume); // Set the volume (0x00~0x30)
}

void storeMP3Volume(int volume)
{
  EEPROM_storeVolume(volume);
  }

void playRandom() {
  playTrackInFolder(playRand(), foldRand());
}

void mp3Wake()
{
  execute_CMD(0x09, 0, 2); // Set Playmode to SD
  delay(500);
}

void mp3Sleep()
{
  execute_CMD(0x09, 0, 3); // Set MP3 player in sleep mode

  execute_CMD(0x0A, 0, 0); // Set MP3 player in power loss
}

void playTrackInFolder(int track, int folder)
{
  debug( "playTrackInFolder" + String(track) + "-" + String(folder));
  //amplifier(1);
  if (!playMode && !PlayingContinuesly || folder == 4) // if not Playmode = random or playinRandom is true
    execute_CMD(0x0F, folder, track);
  else
    execute_CMD(0x0F, foldRand(), playRand());
}

void MP3stop()
{
  execute_CMD(0x16, 0, 0);
}

void execute_CMD(byte CMD, byte Par1, byte Par2)
// Excecute the command and parameters
{
  //debug(millis());
  debug("CMD " + String(CMD) + " - " + String (Par1) + " - " + String(Par2));
  // Calculate the checksum (2 bytes)
  word checksum = -(Version_Byte + Command_Length + CMD + Acknowledge + Par1 + Par2);
  // Build the command line
  byte Command_line[10] = { Start_Byte, Version_Byte, Command_Length, CMD, Acknowledge,
                            Par1, Par2, highByte(checksum), lowByte(checksum), End_Byte
                          };
  //Send the command line to the module
  for (byte k = 0; k < 10; k++)
  {
    Serial.write( Command_line[k]);
  }
  delay(500);
  //debug(millis());
}

void playFolder(int folder) {
  playTrackInFolder(folder + 10, 4);
  delay(2000);
}

void playAlarm() {
  setMP3Volume(19);
  playTrackInFolder(112, 4);
  delay(14000);
  setMP3Volume(EEPROM_getVolume());
  countedPulses = 99;
  folderNumber = 4;
}

void playVolume() {
  playTrackInFolder(audioVolume + 11, 4);
  delay(2000);
}

void playWillekeurig(int pm) {
  playTrackInFolder(pm + 3, 4);
  delay(2000);
}

void playContinu(){
playTrackInFolder(5,4);
  delay(2000);
}
void playReset(){
playTrackInFolder(6,4);
  delay(2000);
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
  
  byte val = EEPROM.read(100);          // initialize first boot, and write 77 in adress 100 to make sure we know next time the phone starts it has already be started before.
  int initRead = val;
  if (initRead != 77|| force)
  {                                     // if started the first time

    debug ("init 14 - 1- 0 - 77");
    EEPROM.write(0, 14);                // volume set to 5
    delay(500);
    EEPROM.write(1, 1);                 //  folder set to 1
    delay(500);
    EEPROM.write(2, 0);                 //  willekeurig set to 1
    delay(500);
    EEPROM.write(100, 77);              // set initialized to 77    
    delay(500);
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

void EEPROM_storeVolume(int volume) {  // store volume setting is EEPROM address 0
  int EEADRESS = 0;
  if (volume > 19) {                      
    volume = 19;
  }

  EEPROM.write(EEADRESS, volume);
  debug("Volume store =" + String(volume));
    delay(500);  // delay to prevent crashes during storing data

  EEPROM.commit();
}

unsigned int EEPROM_getVolume() {
  int EEADRESS = 0;                               //volume adress = 0
  byte val = EEPROM.read(EEADRESS);               //read volume from EEPROM
  int volumeRead = val;
  debug("GetVolume =" + String(volumeRead));
  return (volumeRead);

}

void EEPROM_storeFolder(int folder) {   //store music folder selection setting is EEPROM address 1
  int EEADRESS = 1;
  EEPROM.write(EEADRESS, folder);
  debug("Store Folder =" + String(folder));
  EEPROM.commit();
  folderNumber = folder;
  delay(500);
}

unsigned int EEPROM_getFolder() {       //retrieve music folder selection setting is EEPROM address 1
  int EEADRESS = 1;
  byte val = EEPROM.read(EEADRESS);
  // delay(200);
  int folder = val;
  debug("Getfolder =" + String(folder));
  return (folder);
}


void EEPROM_storePlayMode(int pmode) {     //store random mode setting is EEPROM address 2
  int EEADRESS = 2;
  EEPROM.write(EEADRESS, pmode);
  debug("Store Playmode =" + String(pmode));
  EEPROM.commit();
  playMode = pmode;
  delay(500);
}

unsigned int EEPROM_getPlayMode() {       //retrieve random mode setting is EEPROM address 2
  int EEADRESS = 2;
  byte val = EEPROM.read(EEADRESS);
  //delay(200);
  int pmode = val;
  debug("GetPlaymode =" + String(pmode));
  return (pmode);
}



/*--------------------------------------------------------------------------------------------------
 * function_dial
 * 
 *--------------------------------------------------------------------------------------------------
 */

void waitForDial()
{
  int dialReading = digitalRead(dialPin);
  if (dialReading != lastDialState) {
    // reset the debouncing timer
    lastDialDebounceTime = millis();
  }

  if ((millis() - lastDialDebounceTime) > dialDebounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (dialReading != dialState) {
      dialState = dialReading;

      // If dialstate goes from High to Low  , Dial starts
      if (dialState == LOW) {
        debug("");
        Serial.write(" dial start");
        PlayingContinuesly = false ;  //reset play continu
        debug("" );
        // resetMP3();
      }

      // if dialState goes from Low to High . Dial Ends
      if (dialState == HIGH)
      
      {
        countedPulses = pulseCount;
        if (countedPulses <= 1 )
        {
          countedPulses = 1;
        }
        if (countedPulses >= 10 )
        {
          countedPulses = 10;
        }
        pulseCount = 0;
        debug("");
        Serial.write(" dial stop ");
        debug("");
        Serial.write(" dialed ");
        debug1(String(countedPulses));
        debug("");
        //ledState = HIGH;
        //digitalWrite(ledPin, ledState);
        addLastNumber(countedPulses);  // add last dailed number for pin array
        addLastTime(millis());         // add last time for pin
        checkAll();
        playTrackInFolder(countedPulses, folderNumber);
        folderNumber = EEPROM_getFolder();
      }
    }

    if (dialState == LOW) {
      countPulse();

    }
  }
  lastDialState = dialReading;
}


void countPulse()
{
  int pulseReading = digitalRead(pulsePin);
  // debug(pulseReading);
  if (pulseReading != lastPulseState) {
    // reset the debouncing timer
    lastPulseDebounceTime = millis();
  }
  if ((millis() - lastPulseDebounceTime) > pulseDebounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (pulseReading != pulseState) {
      pulseState = pulseReading;


      // only toggle the LED if the new button state is LOW
      if (pulseState == HIGH) {
        //ledState = LOW;

        pulseCount =  pulseCount + 1;
        Serial.write("write pulse ");
        debug1(String(pulseCount));
        debug("");
      }
    }
  }

  // set the LED:
  // digitalWrite(ledPin, ledState);
  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastPulseState = pulseReading;
}


/*--------------------------------------------------------------------------------------------------
 * function_numpad
 * 
 *--------------------------------------------------------------------------------------------------
 */

///  Numpad type 1  4x3 numpad 7 wire and ground
void checkNumPad() {
  delay(20);                                     // check every 20 ms
  int czer = 0;                                  // reset column
  int rzer = 0;                                  // reset row

  for (int thisRow = 0; thisRow < (sizeof(row) / sizeof(row[0])); thisRow++) {
    row[thisRow] = digitalRead(rowPin[thisRow]);                                  // read if a row is pressed
    if (row[thisRow] == 0 && rzer == 0)
    {
      rzer = 1;
      break;
    }
  }
  //debug("");
  //debug1(" col ");
  for (int thisCol = 0; thisCol < (sizeof(col) / sizeof(col[0])) ; thisCol++) {
    col[thisCol] = digitalRead(colPin[thisCol]);                                   // read if a col is pressed
    if (col[thisCol] == 0 && czer == 0)
    {
      czer = 1;
      break;
    }
  }

  if (czer && rzer) {                                                             // if both are 1 ( to prevent the reading of the row and col half way between row and col a change was happening.
    PlayingContinuesly = false ;
    num = numPad[arrayFind("row")][arrayFind("col")] ;                            // find the number that belongs to this combination of row and col in the numPad array

    if (num > 10 ) {                                                              // 11 and 12 are * and #
      num = 0;
    }
    //debug(String (num));
    debug("Number = " + String(num));
    if (num != 0 && num != lastNum) {       // 0=no button pressed lastNum to check if number changed
      mp3Wake();                            // Wake up MP3 since hook is picked up
      playTrackInFolder(num, folderNumber);                      // Play song
      addLastNumber(num);                   // add last dailed number for pin array
      addLastTime(millis());                // add last time for pin
      lastNum = num;
    }
  }
  else lastNum = 0;                         // if button is
  // debug("");
  debug(String(lastNum));
}

int arrayFind(String type) {

  int wantedVal = 0;
  int wantedPos = 5 ;
  if ( type == "row") {

    for (int i = 0; i < 4; i++) {

      if (wantedVal == row[i]) {
        wantedPos = i;
        break;
      }
    }
  }
  else {
    for (int i = 0; i < 3; i++) {

      if (wantedVal == col[i]) {
        wantedPos = i;
        break;
      }
    }
  }

  return wantedPos;
}
