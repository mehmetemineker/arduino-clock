#include "DS3231.h"
#include <Wire.h>
#include "SevSeg.h"
#include <JC_Button.h>

RTClib rtc;
DS3231 Clock;
SevSeg sevseg;
Button button(13);

enum states_t {VIEW, D0EDIT, D1EDIT, D2EDIT, D3EDIT};

void setup() {
  Serial.begin(9600);
  Wire.begin();

  byte numDigits = 4;
  byte digitPins[] = {9, 10, 11, 12};
  byte segmentPins[] = {2, 3, 4, 5, 6, 7, 8};
  bool resistorsOnSegments = false;
  byte hardwareConfig = COMMON_ANODE;
  bool updateWithDelays = false;
  bool leadingZeros = false;

  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments, updateWithDelays, leadingZeros);
  sevseg.setBrightness(90);

  button.begin();
}

void loop() {
  static unsigned long timer = millis();
  static int deciSeconds = 0;
  static states_t STATE;
  static char sevsegChars[4] = "----";

  button.read();

  if (millis() - timer >= 1000) {
    timer += 1000;
    deciSeconds++;

    if (deciSeconds == 10) {
      deciSeconds = 0;
    }

  }

  switch (STATE)
  {
    case VIEW: {
        DateTime t = rtc.now();
        int hour = t.hour() * 100;
        int minute = t.minute();
        int hoMi = hour + minute;
        sevseg.setNumber(hoMi, 4);

        sprintf(sevsegChars, "%04d", hoMi);

        if (button.wasPressed() )
        {
          STATE = D0EDIT;
          deciSeconds = 0;
        }
      }
      break;
    case D0EDIT: {
        if (deciSeconds == 3) {
          deciSeconds = 0;
        }

        sprintf(sevsegChars, "%i---", deciSeconds);
        sevseg.setChars(sevsegChars);

        if (button.wasPressed() )
        {
          STATE = D1EDIT;
          deciSeconds = 0;
        }
      } break;
    case D1EDIT: {
        if (String(sevsegChars[0]).toInt() == 2 && deciSeconds == 4) {
          deciSeconds = 0;
        }

        sevsegChars[1] = String(deciSeconds)[0];
        sevseg.setChars(sevsegChars);

        if (button.wasPressed() )
        {
          STATE = D2EDIT;
          deciSeconds = 0;
        }
      } break;
    case D2EDIT: {
        if (deciSeconds == 6) {
          deciSeconds = 0;
        }

        sevsegChars[2] = String(deciSeconds)[0];
        sevseg.setChars(sevsegChars);

        if (button.wasPressed() )
        {
          STATE = D3EDIT;
          deciSeconds = 0;
        }
      } break;
    case D3EDIT: {
        sevsegChars[3] = String(deciSeconds)[0];
        sevseg.setChars(sevsegChars);

        if (button.wasPressed() )
        {
          STATE = VIEW;
          deciSeconds = 0;
          
          int h0 = String(sevsegChars[0]).toInt();
          int h1 = String(sevsegChars[1]).toInt();
          int m0 = String(sevsegChars[2]).toInt();
          int m1 = String(sevsegChars[3]).toInt();

          if (h0 == 0) {
            Clock.setHour(h1);
          } else {
            Clock.setHour(h0 * 10 + h1);
          }

          if (m0 == 0) {
            Clock.setMinute(m1);
          } else {
            Clock.setMinute(m0 * 10 + m1);
          }

          Clock.setSecond(0);
        }
      } break;
  }

  sevseg.refreshDisplay();
}
