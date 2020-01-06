#include "mp3player.h"

MP3Player::MP3Player(int busy) {
   this->busyPin = busy;
   this->randomPlay = false;
   this->continuousPlay = false;
}

boolean MP3Player::isPlaying() {
  return (digitalRead(this->busyPin) == 0);
}

boolean MP3Player::isContinuousPlay() {
  return this->continuousPlay;
}

void MP3Player::playPhoneStart() {
  this->playFeedback(FEEDBACK_PHONE_START);  
}

void MP3Player::alarm() {
  this->execute(MP3CMD_VOLUME, 0, 19, MP3CMD_DEFAULTDELAY); // set volume to max
  this->execute(MP3CMD_PLAYTRACK, FEEDBACK_FOLDER, FEEDBACK_ALARM, MP3CMD_DEFAULTDELAY);
}

void MP3Player::playRandom() {
  this->folder = random(1, 4);
  this->playTrack(random(1, 11));
}

void MP3Player::reset() {
  this->execute(MP3CMD_PLAYTRACK, FEEDBACK_FOLDER, FEEDBACK_RESET, MP3CMD_DEFAULTDELAY);
}

void MP3Player::setContinuousPlay() {
  this->continuousPlay = true;
  this->execute(MP3CMD_PLAYTRACK, FEEDBACK_FOLDER, FEEDBACK_CONTINUOUSPLAY, MP3CMD_DEFAULTDELAY);
}

void MP3Player::dialtone()  {
  this->execute(MP3CMD_VOLUME, 0, this->volume, MP3CMD_DEFAULTDELAY);
  this->execute(MP3CMD_PLAYTRACK, FEEDBACK_FOLDER, FEEDBACK_DIALTONE, MP3CMD_DEFAULTDELAY);
}

void MP3Player::wake() {
  this->execute(MP3CMD_PLAYMODUS, 0, MP3CMD_PLAYSD, 500 + MP3CMD_DEFAULTDELAY); 
}

void MP3Player::stop() {
  this->continuousPlay = false;
  this->execute(MP3CMD_STOPPLAY, 0, 0, MP3CMD_DEFAULTDELAY);
}

/*******************************************************************
*  sleep
*
* Let the MP3 module sleep if not used
*******************************************************************/
void MP3Player::sleep() {
  this->execute(MP3CMD_PLAYMODUS, 0, MP3CMD_SLEEP, MP3CMD_DEFAULTDELAY); // Set MP3 player in sleep mode
  this->execute(0x0A, 0, 0, MP3CMD_DEFAULTDELAY); // Set MP3 player in power loss
}

/*******************************************************************
*  setFolder
*
* set the current folder to play from ( max 3 )
*******************************************************************/
void MP3Player::setFolder(int fol) {
  this->folder = fol % 4;
  this->playFeedback(10 + this->folder);
}

void MP3Player::setVolume(int vol) {
  this->volume = vol;
  this->execute(MP3CMD_VOLUME, 0, this->volume, MP3CMD_DEFAULTDELAY);
}

/*******************************************************************
* playFeedback
*
* wrapper or MP3_execute to play a track in a folder on the SD-card
*******************************************************************/
void MP3Player::playFeedback(int track) {
  this->execute(MP3CMD_PLAYTRACK, FEEDBACK_FOLDER, track, MP3CMD_DEFAULTDELAY + 2000);
}

/*******************************************************************
* playTrack
*
* wrapper or MP3_execute to play a track in a folder on the SD-card
*******************************************************************/
void MP3Player::playTrack(int track) {
  this->execute(MP3CMD_PLAYTRACK, this->folder, track, MP3CMD_DEFAULTDELAY);
}

void MP3Player::setRandomPlay(boolean rand) {
  this->randomPlay = rand;
  if (this->randomPlay) {
    this->playFeedback(FEEDBACK_RANDOM_ON);
  } else {
    this->playFeedback(FEEDBACK_RANDOM_OFF);
  }
}

boolean MP3Player::isRandomPlay() {
  return this->randomPlay;
}

/*******************************************************************
*  execute
*
* Send serial command to the MP3 module
*******************************************************************/
// Excecute the command and parameters
void MP3Player::execute(byte command, byte param1, byte param2, int delayafter) {
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
