#include "OneShot.h"

OneShot::OneShot(const unsigned long dly) :
  StateMachine(1, true), m_delay(dly), m_timeout(0L), m_triggered(false)
{
}

bool OneShot::update()
{
  if (StateMachine::update()) {
    // If in triggered state and timeout period has elapsed,
    // turn off triggered state.
    if (m_triggered && ((long) (millis() - m_timeout) >= 0)) {
      m_triggered = false;
    }
    return true;
  }
  return false;
}
  
  void OneShot::setDelay(const unsigned long dly)
  {
    m_delay = dly;
  }
  
  unsigned long OneShot::getDelay() const
  {
    return m_delay;
  }

void OneShot::trigger()
{
  // Set time of timeout.
  m_timeout = millis() + m_delay;
  // Set triggered state.
  m_triggered = true;
}

bool OneShot::isTriggered() const
{
  // Return triggered state.
  return m_triggered;
}

void OneShot::clear()
{
  m_triggered = false;
}