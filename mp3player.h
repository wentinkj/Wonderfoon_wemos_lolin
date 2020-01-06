#ifndef MP3PLAYER_H
#define MP3PLAYER_H

#include <Arduino.h>

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
#define FEEDBACK_EASTER      40
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

class MP3Player {
  public:
    MP3Player();
    void wake();
    void sleep();
    void stop();
    void setFolder(int fol = 1);
    void setVolume(int vol = 1);
    void playTrack(int track = 1);
    boolean isPlaying();
    void setRandomPlay(boolean rand = false);
    boolean isRandomPlay();
    void playRandom();
    void setContinuousPlay();
    boolean isContinuousPlay();

    void playPhoneStart();
    void dialtone();
    void alarm();
    void reset();
    void easter();

  private:
    boolean randomPlay;
    boolean continuousPlay;
    int busyPin;
    int folder;
    int track;
    int volume;
    void playFeedback(int num);
    void execute(byte command, byte param1, byte param2, int delayafter);
};

#endif
