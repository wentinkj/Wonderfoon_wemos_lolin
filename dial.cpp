#include "dial.h"
#include "wonderfoon.h"
#include <Wire.h>

DebouncePin hookpin(HOOKPIN, 20);
DebouncePin dialpin(DIALPIN, 20);
DebouncePin pulsepin(PULSEPIN, 20);

Dial::Dial() {
  pinMode(HOOKPIN, INPUT_PULLUP);

  #ifdef DIALPIN
  pinMode(DIALPIN, INPUT_PULLUP);
  #endif
  
  #ifdef PULSEPIN
  pinMode(PULSEPIN, INPUT_PULLUP);
  #endif

  #ifdef EXPANDER

  #endif
}

boolean Dial::numberChanged() {
  boolean changed = this->numberChange;
  this->numberChange = false;
  return changed;
}

boolean Dial::pulseStateChanged() {
  boolean changed = (pulseCurrent != pulseLast);
  pulseLast = pulseCurrent;
  return changed;
}

boolean Dial::dialStateChanged() {
  boolean changed = (dialCurrent != dialLast);
  dialLast = dialCurrent;
  return changed;
}

boolean Dial::hookStateChanged() {
  boolean changed = (hookCurrent != hookLast);
  hookLast = hookCurrent;
  return changed;
}

#ifdef PULSEPIN
void Dial::readState() {
  this->hookCurrent = hookpin.read();
  this->dialCurrent = dialpin.read();
  this->pulseCurrent = pulsepin.read();

  if (this->isHookPickedUp()) {
    if (this->dialStateChanged()) {
      if (this->isDialing()) {
        // dial start
        this->pulseCount = 0; 
      } else {
        // dial end
        this->addDigitToPhoneNumber();
      }
    }
  }

  // count pulses when dialing 
  if (   this->isDialing()
      && this->pulseStateChanged()) {
        this->pulseCount++;
  }
}
#else
void Dial::readState() {
  
}
#endif


bool Dial::isHookPickedUp() {
  return (this->hookCurrent == LOW);
}

bool Dial::isDialing() {
  return (this->dialCurrent == LOW);
}


/*******************************************************************
*  addDigitToPhoneNumber
*
* combines the new digit with the existing (if dialed within 10 seconds)
*******************************************************************/
void Dial::addDigitToPhoneNumber() {
  this->numberChange = true; 

  // allow for 10 seconds to dial a number
  if (millis() - this->firstDial > 10000) {
    this->firstDial = millis();
    this->dialedPhoneNumber = this->pulseCount;
  } else { 
    this->dialedPhoneNumber = (this->dialedPhoneNumber * 10) + this->pulseCount;
  }

  // only the last 3 digits are relevant
  this->dialedPhoneNumber = (this->dialedPhoneNumber % 1000);
}


DebouncePin::DebouncePin(byte pin, int debounce) {
  this->pin = pin;
  this->debounce = debounce;
}

byte DebouncePin::read() {
  if (   this->debounceTime > millis()
      || this->debounceState != this->currentState) {
      this->debounceTime = millis();
  }

  if (millis() - this->debounceTime > this->debounce) {
    this->currentState = digitalRead(HOOKPIN);
    if (this->currentState != this->debounceState) {
      this->debounceTime = millis();
    }
  }
  this->debounceState = this->currentState;
  return this->currentState;
}
