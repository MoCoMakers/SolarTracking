#include <StateMachine.h>
#include <Streaming.h>

#include <Wire.h>
#include <Adafruit_LiquidCrystal.h>

#include "StepperFast.h"
#include "Periodic.h"
#include "Timeout.h"
#include "LcdTerminal.h"

// stepper 200 steps/rotation, gear ratio 10:1
#define STEPS 2000

// microseconds between stepper updates
#define TIME_STEP 2500L

const int enables[] = {9, 10};          // order insensitive
const int drivers[] = {8, 11, 12, 13};  // order sensitive

StepperFast stepper(TIME_STEP, STEPS, enables, drivers);
Timeout timeout(10000);     // 10 second timeout on stops
Periodic periodic(stepper);

Adafruit_LiquidCrystal lcd(3, 2, 4);

class MasterControl
{
private:
  enum E_STATE {
    eNeutral,
    eStopped, 
    eContinuous, 
    eGoto,
    ePeriodic,
    eBusy
  } m_state;
  bool m_refresh;
  
public:
  MasterControl() : m_state(eNeutral), m_refresh(false)
  {
  }

  void stopAndEcho()
  {
    stepper.brake();
    m_state = eStopped;
    Serial.write('-');
  }

  bool update()
  {
    if (m_state == eGoto) {
      if (stepper.isComplete()) {
        stopAndEcho();
      }
    } else if (m_state == eBusy) {
      if (stepper.isStopped()) {
        timeout.clear();
        stopAndEcho(); 
      } else if (timeout.isTimeout()) {
        stopAndEcho();
      }
    } else if (m_state == ePeriodic) {
      if (periodic.getTime() == 0) {
        stopAndEcho();
        m_refresh = true;
      }
    }
    if (Serial.available() > 0) {
      // Read command character.
      // Normally, this character will be echoed back
      // to the pendant.  If an invalid command is
      // received, an 'X' will be echoed instead.
      // If a command requires time for completion,
      // the command is echoed immediately, and upon
      // completion a '-' is sent.  This applies to
      // - Stop
      // - MoveBy
      // - MoveHome
      // - MoveTo
      // - Periodic
      char c = Serial.read();
      if (m_state == eNeutral) {
        if (c == 'E') {
          // no op
        } else if (c == 'Z') {
          stepper.zero();
          m_state = eStopped;
        } else {
          c = 'X';  // error
        }
      } else if (m_state == eStopped) {
        if (c == 'B') {
          float angle = Serial.parseFloat();
          double pos = stepper.getPosition();
          stepper.step(pos + angle);
          m_state = eGoto;
        } else if (c == 'C') {
          float speed = Serial.parseFloat();
          stepper.run(speed);
          m_state = eContinuous;
        } else if (c == 'E') {
          stepper.estop();
          m_state = eNeutral;
        } else if (c == 'H') {
          float angle = 360.0 * round(stepper.getPosition() / 360.0);
          stepper.step(angle);
          m_state = eGoto;
        } else if (c == 'P') {
          int period = Serial.parseInt();
          float start = Serial.parseFloat();
          float stop = Serial.parseFloat();
          float delta = Serial.parseFloat();
          int count = Serial.parseInt();
          periodic.start(period, start, stop, delta, count);
          m_refresh = true;
          m_state = ePeriodic;
        } else if (c == 'S') {
          // no op
        } else if (c == 'T') {
          float angle = Serial.parseFloat();
          stepper.step(angle);
          m_state = eGoto;
        } else if (c == 'Z') {
          stepper.zero();
        } else {
          c = 'X';
        }
      } else if (m_state == eContinuous) {
        if (c == 'C') {
          float speed = Serial.parseFloat();
          stepper.run(speed);
        } else if (c == 'E') {
          stepper.estop();
          m_state = eNeutral;
        } else if (c == 'S') {
          stepper.run(0.0);
          timeout.trigger();
          m_state = eBusy;
        } else {
          c = 'X';
        }
      } else if (m_state == eGoto) {
        if (c == 'E') {
          stepper.estop();
          m_state = eNeutral;
        } else if (c == 'S') {
          stepper.run(0.0);
          timeout.trigger();
          m_state = eBusy;
        } else {
          c = 'X';
        }
      } else if (m_state == ePeriodic) {
        if (c == 'E') {
          periodic.stop();
          stepper.estop();
          m_refresh = true;
          m_state = eNeutral;
        } else if (c == 'S') {
          periodic.stop();
          stepper.run(0.0);
          m_refresh = true;
          timeout.trigger();
          m_state = eBusy;
        } else {
          c = 'X';
        }
      } else if (m_state == eBusy) {
        if (c == 'E') {
          stepper.estop();
          m_state = eNeutral;
        } else {
          c = 'X';
        }
      }
      Serial.write(c);
      do {
        c = Serial.read();
      } while (c != '\n');
    }
  }

  bool isRefresh()
  {
    bool r = m_refresh;
    m_refresh = false;
    return r;
  }

  bool isPeriodic() const
  {
    return m_state == ePeriodic;
  }
};

MasterControl control;
StateMachine display(300L, true);   // update LCD display every 0.3 second
LcdTerminal printer(lcd, 128);      // wrap buffer around LCD display
Field field1(0, 4, 7);
Field field2(1, 4, 7);

void refresh(bool p)
{
  lcd.begin(16, 2);
  printer << _CLS << "Pos";
  printer << _CURPOS(0, 11) << "\xDF";
  printer << _CURPOS(1, 0) << (p ? "Tmr" : "Spd");
  printer << _CURPOS(1, 11) << (p ? "sec" : "\xDF/sec");
}

void setup()
{
  Serial.begin(115200);
  refresh(false);
}

void loop()
{
  timeout.update();
  control.update();
  periodic.update();
  stepper.update();
  if (display.update()) {
    if (control.isRefresh()) {
      refresh(control.isPeriodic());
    }
    printer << field1.rjust(stepper.getPosition());
    if (control.isPeriodic()) {
      printer << field2.rjust(periodic.getTime());
    } else {
      printer << field2.rjust(stepper.getSpeed());
    }
  }
  printer.update();
}
