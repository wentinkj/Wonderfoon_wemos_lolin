/*--------------------------------------------------------------------------------------------------
   Wonderfoon
   http://wonderfoon.eu
   WEMOS version

    REWRITE, derived from https://github.com/hvtil/Wonderfoon_wemos_lolin
    - supports 8574 expander for TDK phones
    - supports rotary discs through pins on the d1 mini
  --------------------------------------------------------------------------------------------------
*/
#include "ESP8266WiFi.h"
#include "wonderfoon.h"
#include "mp3player.h"
#include "settings.h"
#include "dial.h"

Settings config;
MP3Player mp3;
Dial dialer; // rotary disc or tdk dialer and pins are determined in wonderfoon.h

void setup() {
  delay(100);
  WiFi.disconnect();
  delay(1);
  WiFi.forceSleepBegin();
  delay(1);
  
  Serial.begin(9600); // start serial for debug and mp3
  Serial.println("Wonderfoon " VERSION );

  mp3.wake();
  mp3.playPhoneStart();
  mp3.setVolume(config.getVolume());
  mp3.setFolder(config.getFolder());
  mp3.setRandomPlay(config.isRandom());;
  mp3.sleep();
  debug("Ready....");
}

/*******************************************************************
   loop

  main program loop which checks the different situations which could occur
*******************************************************************/
void loop() {
  dialer.readState();

  /* wait for someone to pick up the hook
   */
  if (dialer.hookStateChanged()) {
    if (dialer.isHookPickedUp()) {
      debug("The hook is picked up");
      mp3.wake();
      mp3.dialtone();
    } else {
      debug("The hook is down on the phone");
      mp3.stop();
      mp3.sleep();
    }
  }

  /* if a new number is dialed check what to do
   */
  if (dialer.numberChanged()) {
    debug(dialer.dialed());
    /* check if special (command) number and handle that, otherwise play the requested track
     */
    if ( !checkDialCommands(dialer.dialed()) ) {
      int track = dialer.dialed() % 10;
      /* for compatibility with existing wonderfoon mp3 mapping */
      if (track == 0) track = 10;
      debug1("playing track");
      debug(track);
      mp3.playTrack(track);
    }
  }

  /* if continuous play is enabled and the hook is picked up
   */
  if (   mp3.isContinuousPlay()
         && dialer.isHookPickedUp()) {
    /* and not playing, get the next song
     */
    if (!mp3.isPlaying()) {
      debug("Check Playing: start next random number");
      delay(1000);
      mp3.playRandom();
    }
  }
}


/*******************************************************************
   checkDialCommands

  check (and execute) commands given by dialing a number
*******************************************************************/
bool checkDialCommands(int dialed) {
  int audioVolume = 0;
  int folderNumber = 1;
  switch (dialed) {
    case 112:
      mp3.alarm();
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
      mp3.setVolume(audioVolume);
      config.setVolume(audioVolume);
      return true;
    case 311:
    case 312:
    case 313:
      debug("folder change");
      folderNumber = dialed % 10;
      config.setFolder(folderNumber);
      mp3.setFolder(folderNumber);
      return true;
    case 411:
      debug("play random off");
      mp3.setRandomPlay(false);
      config.setRandom(false);
      return true;
    case 412:
      debug("play random on");
      mp3.setRandomPlay(true);
      config.setRandom(true);
      return true;
    case 511:
      debug("continuous random play");
      mp3.setContinuousPlay();
      return true;
    case 998:
      mp3.easter();
      return true;
    case 999:
      debug("reset");
      config.defaults();
      mp3.setVolume(config.getVolume());
      mp3.setFolder(config.getFolder());
      mp3.setRandomPlay(config.isRandom());
      mp3.reset();
      return true;
  }
  return false;
}
