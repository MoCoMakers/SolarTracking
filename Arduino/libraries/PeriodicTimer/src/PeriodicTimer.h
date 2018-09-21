#ifndef _PERIODIC_TIMER__H_
#define _PERIODIC_TIMER__H_

#include <StateMachine.h>

class PeriodicTimer : public StateMachine
{
private:
  unsigned int m_period;
  unsigned int m_remaining;
  unsigned int m_substep;
  bool m_alarm;

public:
  PeriodicTimer() : StateMachine(100, true),
  m_period(0), m_remaining(0), m_substep(0),
  m_alarm(false)
  {
  }
  
  virtual bool update()
  {
    if (StateMachine::update()) {
      if (m_period > 0) {
        if (--m_substep == 0) {
          m_substep = 10;
          if (--m_remaining == 0) {
            m_alarm = true;
            m_remaining = m_period;
          }
        }
      }
      return true;
    }
    return false;
  }
  
  void start(const unsigned int sec)
  {
    m_substep = 10;
    m_remaining = m_period = sec;
  }
  
  void stop()
  {
    m_period = m_remaining = m_substep = 0;
  }
  
  unsigned int getTime() const
  {
    return m_remaining;
  }
  
  bool isAlarm()
  {
    bool alarm = m_alarm;
    m_alarm = false;
    return alarm;
  }
};

#endif
