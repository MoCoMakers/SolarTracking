#ifndef _TIMEOUT__H_
#define _TIMEOUT__H_

#include <StateMachine.h>
#include <OneShot.h>

class Timeout : public StateMachine
{
private:
  enum {
    eIdle, eTiming, eTimeout
  } m_state;
  OneShot m_os;
  
public:
  Timeout(const unsigned long msec) : 
  StateMachine(1, true), m_state(eIdle)
  {
    m_os.setDelay(msec);
  }

  virtual bool update()
  {
    if (m_os.update()) {
      if (m_state == eTiming) {
        if (!m_os.isTriggered()) {
          m_state = eTimeout;
        }
      }
      return true;
    }
    return false;
  }

  void clear()
  {
    m_os.clear();
    m_state = eIdle;
  }

  void trigger()
  {
    m_os.trigger();
    m_state = eTiming;
  }

  bool isTimeout()
  {
    if (m_state == eTimeout) {
      m_state = eIdle;
      return true;
    } else {
      return false;
    }
  }
};

#endif

