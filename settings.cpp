#include "settings.h"
#include "wonderfoon.h"

Settings::Settings() {
  EEPROM.begin(256); // using 0-20 max
  this->EEPROM_init(false); 
  this->folderNumber = this->EEPROM_getValue(ADDRFOLDER);
  this->audioVolume = this->EEPROM_getValue(ADDRVOLUME);
  this->randomPlay = this->EEPROM_getValue(ADDRRANDOM);
}

void Settings::defaults() {
  this->EEPROM_init(true);
}

void Settings::setVolume(int vol) {
  this->audioVolume = vol;
  this->EEPROM_storeValue(ADDRVOLUME, vol);
}

int Settings::getVolume() {
  return this->audioVolume;
}

void Settings::setFolder(int fol) {
  this->folderNumber = fol;
  this->EEPROM_storeValue(ADDRFOLDER, fol);
}

int Settings::getFolder() {
  return this->folderNumber;
}

void Settings::setRandom(bool rand) {
  this->randomPlay = rand;
  this->EEPROM_storeValue(ADDRRANDOM, rand);
}

boolean Settings::isRandom() {
  return this->randomPlay;
}


/*******************************************************************
*  EEPROM_init
*
* check if not initialised or forced initalisation then store default values
*******************************************************************/
void Settings::EEPROM_init(bool force) {
  int initRead = EEPROM_getValue(ADDRINITIALISED);
  if (initRead != EEPROMINITIALISED || force) {
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
void Settings::EEPROM_clear() {
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
int Settings::EEPROM_storeValue(int address, int value) {
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
int Settings::EEPROM_getValue(int address) {
  byte val = EEPROM.read(address);
  return (int) val;
}
