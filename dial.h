#ifndef DIAL_H
#define DIAL_H

#include <Arduino.h>

class Dial {
  public:
    Dial();
    void readState();
    boolean hookStateChanged(); // can be read but will reset to false after reading
    boolean isHookPickedUp();
    boolean dialStateChanged(); // can be read but will reset to false after reading
    boolean isDialing();
    boolean dialedNumberChanged(); // can be read but will reset to false after reading
    boolean pulseStateChanged(); // can be read but will reset to false after reading
    boolean numberChanged(); // can be read but will reset to false after reading

  private:
    long firstDial = 0;
    int dialedPhoneNumber = 0;
    boolean numberChange = false;
    byte hookCurrent = HIGH;
    byte hookLast = HIGH;
    byte dialCurrent = HIGH;
    byte dialLast = HIGH;
    byte pulseCurrent = HIGH;
    byte pulseLast = HIGH;
    byte pulseCount;

    void addDigitToPhoneNumber();
};


class DebouncePin {
  public:
    DebouncePin(byte pin, int debounce);
    byte read();
    
  private:
    byte pin;
    int debounce; // debounce (wait)time
    byte debounceState;
    byte currentState;
    long debounceTime; // last time state changed
};


#endif
