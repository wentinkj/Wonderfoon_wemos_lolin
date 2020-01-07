#ifndef DIAL_H
#define DIAL_H

#include "wonderfoon.h"
#include <Arduino.h>
#include <Wire.h>

class Dial {
  public:
    Dial();
    void readState();
    boolean hookStateChanged(); // can be read but will reset to false after reading
    boolean isHookPickedUp();
    boolean numberChanged(); // can be read but will reset to false after reading
    int dialed();

    #ifdef ROTARY
    boolean dialStateChanged(); // can be read but will reset to false after reading
    boolean isDialing();
    boolean dialedNumberChanged(); // can be read but will reset to false after reading
    boolean pulseStateChanged(); // can be read but will reset to false after reading
    #endif
    #ifdef TDK
    boolean expanderChanged(); // can be read but will reset to false after reading
    #endif
 
  private:
    long firstDial = 0;
    int dialedPhoneNumber = 0;
    boolean numberChange = false;
    byte hookCurrent = HIGH;
    byte hookLast = HIGH;
    #ifdef ROTARY
    byte dialCurrent = HIGH;
    byte dialLast = HIGH;
    byte pulseCurrent = HIGH;
    byte pulseLast = HIGH;
    byte pulseCount = 0;
    #endif
    #ifdef TDK
    byte expanderCurrent = 0;
    byte expanderLast = 0;
    byte maskV1 = 0x01 << V1PORT;
    byte maskV2 = 0x01 << V2PORT;
    byte maskV3 = 0x01 << V3PORT;
    byte maskA1 = 0x01 << A1PORT;
    byte maskA2 = 0x01 << A2PORT;
    byte maskA3 = 0x01 << A3PORT;
    byte maskA4 = 0x01 << A4PORT;
    #endif

    void addDigitToPhoneNumber(int digit);
};


class DebouncePin {
  public:
    DebouncePin(byte pin, int debounce);
    byte read();
    void reset();
    
  private:
    byte pin;
    int debounce; // debounce (wait)time
    byte debounceState;
    byte currentState;
    long debounceTime; // last time state changed
};

#ifdef TDK
class DebounceExpander {
  public:
    DebounceExpander(int addr, int debounce);
    byte read();
    
  private:
    int addr;
    int debounce; // debounce (wait)time
    byte debounceState;
    byte currentState;
    long debounceTime; // last time state changed
};
#endif
#endif
