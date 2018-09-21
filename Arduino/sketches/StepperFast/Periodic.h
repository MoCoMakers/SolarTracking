#ifndef _PERIODIC__H_
#define _PERIODIC__H_

#include <StateMachine.h>
#include <PeriodicTimer.h>

#include "StepperFast.h"

class Periodic : public StateMachine
{
private:
  enum E_STATE {
    eIdle, eMove, eWait, eStartPending, eStopPending
  } m_state;

  StepperFast& m_stepper;
  PeriodicTimer m_timer;

  float m_current;        // degrees
  float m_start;          // degrees
  float m_stop;           // degrees
  float m_delta;          // degrees
  unsigned int m_count;   // cycles
  
  unsigned int m_periodPend;
  float m_startPend;
  float m_stopPend;
  float m_deltaPend;
  unsigned int m_countPend;
  
public:
  Periodic(StepperFast& stepper) : StateMachine(100, true),
  m_state(eIdle), m_stepper(stepper), m_timer(), 
  m_current(0.0), m_start(0.0), m_stop(0.0), m_delta(0.0), m_count(0),
  m_periodPend(0), m_startPend(0.0), m_stopPend(0.0), m_deltaPend(0.0), m_countPend(0)
  {
  }

  virtual bool update()
  {
    m_timer.update();
    if (StateMachine::update()) {
      if (m_state == eStartPending) {
        if (m_stepper.isComplete()) {
          m_current = m_start = m_startPend;
          m_stop = m_stopPend;
          m_delta = m_deltaPend;
          m_count = m_countPend;
          m_timer.start(m_periodPend);
          m_stepper.step(m_start);
          m_state = eMove;
        }
      } else if (m_state == eStopPending) {
        if (m_stepper.isComplete()) {
          m_state = eIdle;
        }
      } else if (m_state == eMove) {
        if (m_stepper.isComplete()) {
          m_state = eWait;
        }
      } else if (m_state == eWait) {
        if (m_timer.isAlarm()) {
          // Period elapsed, time for action.
          // Adjust position by delta.
          m_current += m_delta;
          // Is new position past stop?
          // (Take sign of delta into account.)
          if ((m_delta > 0.0 && m_current > m_stop)
            || (m_delta < 0.0 && m_current < m_stop)) {
            // Sweep complete, do we recycle?
            // If count is already zero, we are cycling
            // indefinitely.  If count doesn't decrement
            // to zero, recycle.  Otherwise, we are done.
            if (m_count == 0 || --m_count > 0) {
              // Recycle to start position.
              m_current = m_start;
              m_stepper.step(m_current);
              m_state = eMove;
            } else {
              // Done.
              m_timer.stop();
              m_state = eIdle;
            }
          } else {
            // Sweep not complete, move to next position.
            m_stepper.step(m_current);
            m_state = eMove;
          }
        }
      }
      return true;
    }
    return false;
  }

  void start(
    unsigned int period,  // seconds
    float start,          // degrees
    float stop,           // degrees
    float delta,          // degrees
    unsigned int count    // cycles, 0 = indefinite
  )
  {
    if (m_state == eMove) {
      m_periodPend = period;
      m_startPend = start;
      m_stopPend = stop;
      m_deltaPend = delta;
      m_countPend = count;
      m_state = eStartPending;
    } else {
      m_current = m_start = start;
      m_stop = stop;
      m_delta = delta;
      m_count = count;
      m_timer.start(period);
      m_stepper.step(m_start);
      m_state = eMove;
    }
  }

  void stop()
  {
    m_timer.stop();
    if (m_state == eMove) {
      m_state = eStopPending;
    } else {
      m_state = eIdle;
    }
  }

  unsigned int getTime() const
  {
    return m_timer.getTime();
  }
};

#endif
