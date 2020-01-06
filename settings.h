#ifndef SETTINGS_H
#define SETTINGS_H

#include <EEPROM.h>
#include <Arduino.h>

// EEPROM
#define ADDRVOLUME      0
#define ADDRFOLDER      1
#define ADDRRANDOM      2
#define ADDRINITIALISED 100
#define EEPROMINITIALISED 77

class Settings {
 public:
  Settings();
  void defaults();
  void setVolume(int vol);
  int getVolume();
  void setFolder(int fol);
  int getFolder();
  void setRandom(bool rand);
  boolean isRandom();

 private:
  int folderNumber;
  int audioVolume;
  boolean randomPlay;
  int EEPROM_getValue(int address);
  int EEPROM_storeValue(int address, int value);
  void EEPROM_clear();
  void EEPROM_init(bool force);
};

#endif
