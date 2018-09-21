#ifndef _ONE_SHOT__H_
#define _ONE_SHOT__H_

#include <StateMachine.h>

class OneShot : public StateMachine
{
private:
  unsigned long m_delay;    // length in msec of on-state after trigger
  unsigned long m_timeout;  // time of next update
  bool m_triggered;         // triggered state
    
public:
  // Constructor
  // @param dly delay in msec between trigger and state returning to off
  // (default 1000 msec)
  OneShot(const unsigned long dly = 1000L);
  
  // Check if it's time to update this object's state.
  // @return true if update of state occurred, false otherwise
  virtual bool update();
  
  // Set delay.
  void setDelay(const unsigned long dly);
  
  // Get delay.
  unsigned long getDelay() const;
  
  // Trigger this object, setting it to on state and setting time when
  // it will turn off again, assuming it doesn't get retriggered first.
  void trigger();
  
  // Returns true if this object has been triggered but has not timed out yet;
  // false otherwise.
  // @return triggered state.
  bool isTriggered() const;    
  
  // Clear triggered state.
  void clear();
};

#endif
