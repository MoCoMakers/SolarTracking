#ifndef _MOTION_CONTROL__H_
#define _MOTION_CONTROL__H_

#include <TimeStep.h>
#include <RateLimiter.h>
#include <Integrator.h>
#include <PID.h>

class MotionControl
{
private:
  enum E_STATE {
    eStop, eRun, eStep
  } m_state;
  
  RateLimiter m_limitA;
  Integrator m_integA;
  Integrator m_integV;
  PID m_pidV;
  PID m_pidX;
  
  const double m_maxA, m_maxV;
  
  double m_cmdA, m_actA;
  double m_cmdV, m_actV;
  double m_cmdX, m_actX;
  
public:
  MotionControl(
    const TimeStep& dt, 
    const double maxJ, 
    const double maxA,
    const double maxV
  ) :
    m_state(eStop),
    m_limitA(dt, maxJ),
    m_integA(dt),
    m_integV(dt),
    m_pidV(dt, 1.0, 0.0, 0.0),
    m_pidX(dt, 4.0, 0.0, 2.0),
    m_maxA(maxA), m_maxV(maxV),
    m_cmdA(0.0), m_actA(0.0),
    m_cmdV(0.0), m_actV(0.0),
    m_cmdX(0.0), m_actX(0.0)
  {
  }

  void step()
  {
    if (m_state == eStop) {
      m_cmdA = m_actA = 0.0;
      m_cmdV = m_actV = 0.0;
    } else {
      if (m_state == eStep) {
        m_cmdV = m_pidX.step(m_cmdX - m_actX);
      }
      m_cmdV = constrain(m_cmdV, -m_maxV, m_maxV);
      m_cmdA = m_pidV.step(m_cmdV - m_actV);
      m_actA = m_limitA.step(m_cmdA);
      m_actV = m_integA.step(m_actA);
      m_actX = m_integV.step(m_actV);
    }
  }

  void setCmdSpeed(const double v)
  {
    m_state = eRun;
    m_cmdV = v;    
  }

  double getActSpeed() const
  {
    return m_actV;
  }

  void setCmdPosition(const double x)
  {
    m_state = eStep;
    m_cmdX = x;
  }

  double getActPosition() const
  {
    return m_actX;
  }

  void brake()
  {
    m_state = eStop;
    m_integA.reset();
  }

  void zero()
  {
    brake();
    m_cmdX = m_actX = 0.0;
  }
};

#endif
