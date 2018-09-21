#include <Arduino.h>
#include <math.h>
#include <Streaming.h>

#include "StepperFast.h"

// private methods

void StepperFast::x_SetEnable(const bool ena)
{
  int out = ena ? HIGH : LOW;
  digitalWrite(m_enables[0], out);
  digitalWrite(m_enables[1], out);
}

void StepperFast::x_SetDrivers(const byte pos)
{
  byte curr = pos & 0x03;
  if (curr != m_out) {
    byte b = m_outs[curr];
    for (byte i = 0; i < 4; ++i) {
      digitalWrite(m_drivers[i], (b & 0x01) ? HIGH : LOW);
      b >>= 1;
    }
    m_out = curr;
  }
}

// public methods

StepperFast::StepperFast(
  const unsigned long time_step,
  const int num_steps,
  const int* enables,
  const int* drivers
) : StateMachine(time_step, true, true),
    m_timeStep((double) time_step / 1.0e6),
    m_motionCtrl(m_timeStep, 90.0, 180.0, 30.0),
    m_numSteps(num_steps), 
    m_enables(enables), m_drivers(drivers),
    m_state(eStop), 
    m_speedCmd(0.0), m_posCmd(0.0),
    m_speedAct(0.0), m_posAct(0.0),
    m_pos(0L), m_out(0x00)
{
  pinMode(m_enables[0], OUTPUT);
  pinMode(m_enables[1], OUTPUT);
  x_SetEnable(false);
  pinMode(m_drivers[0], OUTPUT);
  pinMode(m_drivers[1], OUTPUT);
  pinMode(m_drivers[2], OUTPUT);
  pinMode(m_drivers[3], OUTPUT);
  x_SetDrivers(m_pos);
}

bool StepperFast::update()
{
  if (StateMachine::update()) {
    m_motionCtrl.step();
    m_speedAct = m_motionCtrl.getActSpeed();    // deg/sec
    m_posAct = m_motionCtrl.getActPosition();   // deg
    m_pos = (long) round(m_posAct * m_numSteps / 360.0);
    x_SetDrivers(m_pos);
    return true;
  }
  return false;
}

void StepperFast::run(const double speed)
{
  m_state = eRun;
  m_speedCmd = speed;
  m_motionCtrl.setCmdSpeed(m_speedCmd);
}

void StepperFast::step(const double angle)
{
  m_state = eStep;
  m_posCmd = angle;
  m_motionCtrl.setCmdPosition(m_posCmd);
}

void StepperFast::estop()
{
  m_state = eStop;
  m_speedCmd = 0.0;
  m_motionCtrl.zero();
  x_SetEnable(false);
}

void StepperFast::brake()
{
  m_motionCtrl.brake();
}

void StepperFast::zero()
{
  m_motionCtrl.zero();
  x_SetEnable(true);
}

bool StepperFast::isStopped() const
{
  return (abs(m_speedCmd - m_speedAct) < 0.025);
}

bool StepperFast::isComplete() const
{
  if (m_state == eStep) {
    return isStopped() && (abs(m_posCmd - m_posAct) < 0.01);
  } else {
    return false;
  }
}

double StepperFast::getPosition() const
{
  return m_pos * 360.0 / (double) m_numSteps;
}

double StepperFast::getSpeed() const
{
  return m_speedAct;
}

