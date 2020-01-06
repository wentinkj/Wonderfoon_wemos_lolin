#include "dial.h"
#include <Wire.h>
#include "wonderfoon.h"

DebouncePin hookpin(HOOKPIN, 20);

#ifdef ROTARY
DebouncePin dialpin(DIALPIN, 20);
DebouncePin pulsepin(PULSEPIN, 20);
#endif

#ifdef TDK
DebounceExpander expander(EXPANDER, 50);
#endif

Dial::Dial() {

}

int Dial::dialed() {
  return this->dialedPhoneNumber;
}

bool Dial::isHookPickedUp() {
  return (this->hookCurrent == LOW);
}

boolean Dial::hookStateChanged() {
  boolean changed = (hookCurrent != hookLast);
  hookLast = hookCurrent;
  return changed;
}

boolean Dial::numberChanged() {
  boolean changed = this->numberChange;
  this->numberChange = false;
  return changed;
}

#ifdef ROTARY
boolean Dial::pulseStateChanged() {
  boolean changed = (pulseCurrent != pulseLast);
  pulseLast = pulseCurrent;
  return changed;
}

bool Dial::isDialing() {
  return (this->dialCurrent == LOW);
}

boolean Dial::dialStateChanged() {
  boolean changed = (dialCurrent != dialLast);
  dialLast = dialCurrent;
  return changed;
}

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
        this->addDigitToPhoneNumber(this->pulseCount);
      }
    }
  }

  // count pulses when dialing
  if (   this->isDialing()
         && this->pulseStateChanged()) {
    this->pulseCount++;
  }
}
#endif

#ifdef TDK
boolean Dial::expanderChanged() {
  boolean changed = (expanderCurrent != expanderLast);
  expanderLast = expanderCurrent;
  return changed;
}

void Dial::readState() {
  this->hookCurrent = hookpin.read();
  this->expanderCurrent = expander.read();
  if (this->isHookPickedUp()) {
    if (this->expanderChanged()) {
#ifdef EASTERPORT
      if ((this->expanderCurrent & (0x01 << EASTERPORT)) > 0) {
        this->addDigitToPhoneNumber(998);
      }
#endif
      // translate row & column bits to digit
      if ((this->expanderCurrent & maskA1) == maskA1) {
        if ((this->expanderCurrent & maskV1) == maskV1) {
          this->addDigitToPhoneNumber(1);
        }
        if ((this->expanderCurrent & maskV2) == maskV2) {
          this->addDigitToPhoneNumber(2);
        }
        if ((this->expanderCurrent & maskV3) == maskV3) {
          this->addDigitToPhoneNumber(3);
        }
        return;
      }
      if ((this->expanderCurrent & maskA2) == maskA2) {
        if ((this->expanderCurrent & maskV1) == maskV1) {
          this->addDigitToPhoneNumber(4);
        }
        if ((this->expanderCurrent & maskV2) == maskV2) {
          this->addDigitToPhoneNumber(5);
        }
        if ((this->expanderCurrent & maskV3) == maskV3) {
          this->addDigitToPhoneNumber(6);
        }
        return;
      }
      if ((this->expanderCurrent & maskA3) == maskA3) {
        if ((this->expanderCurrent & maskV1) == maskV1) {
          this->addDigitToPhoneNumber(7);
        }
        if ((this->expanderCurrent & maskV2) == maskV2) {
          this->addDigitToPhoneNumber(8);
        }
        if ((this->expanderCurrent & maskV3) == maskV3) {
          this->addDigitToPhoneNumber(9);
        }
        return;
      }
      if ((this->expanderCurrent & maskA4) == maskA4) {
        /* * and # are not used and are at A4 V1 and V3 */
        if ((this->expanderCurrent & maskV2) == maskV2) {
          this->addDigitToPhoneNumber(0);
        }
      }
    }
  }
}
#endif

/*******************************************************************
   addDigitToPhoneNumber

  combines the new digit with the existing (if dialed within 10 seconds)
*******************************************************************/
void Dial::addDigitToPhoneNumber(int digit) {
  this->numberChange = true;

  // allow for 10 seconds to dial a number
  if (millis() - this->firstDial > 10000) {
    this->firstDial = millis();
    this->dialedPhoneNumber = digit;
  } else {
    this->dialedPhoneNumber = (this->dialedPhoneNumber * 10) + digit;
  }

  // only the last 3 digits are relevant
  this->dialedPhoneNumber = (this->dialedPhoneNumber % 1000);
}

/*******************************************************************
   class: DebouncePin

  simple wrapper to debouce the input from a button/hook
*******************************************************************/
DebouncePin::DebouncePin(byte pin, int debounce) {
  this->pin = pin;
  this->debounce = debounce;
  pinMode(pin, INPUT_PULLUP);
}

byte DebouncePin::read() {
  if (   this->debounceTime > millis()
      || this->debounceState != this->currentState) {
    this->debounceTime = millis();
  }

  if (millis() - this->debounceTime > this->debounce) {
    this->currentState = digitalRead(pin);
    if (this->currentState != this->debounceState) {
      this->debounceTime = millis();
    }
  }
  this->debounceState = this->currentState;
  return this->currentState;
}

#ifdef TDK
/*******************************************************************
   class: DebounceExpander

  simple wrapper to debouce the input from the expander reading all ports as 1 byte
*******************************************************************/
DebounceExpander::DebounceExpander(int addr, int debounce) {
  this->addr = addr;
  this->debounce = debounce;
  Wire.begin();

  Wire.beginTransmission(this->addr);
  Wire.write(0xFF);
  Wire.endTransmission();
}

byte DebounceExpander::read() {
  if (   this->debounceTime > millis()
      || this->debounceState != this->currentState) {
    this->debounceTime = millis();
  }

  if (millis() - this->debounceTime > this->debounce) {
    Wire.beginTransmission(this->addr);
    Wire.endTransmission();
    Wire.requestFrom(this->addr, 1);
    if (Wire.available() == 1) {
      this->currentState = Wire.read() ^ 0xff;
    }
    if (this->currentState != this->debounceState) {
      this->debounceTime = millis();
    }
  }
  this->debounceState = this->currentState;
  return this->currentState;
}
#endif
