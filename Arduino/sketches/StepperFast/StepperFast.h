#ifndef _STEPPER_FAST__H_
#define _STEPPER_FAST__H_

#include <StateMachine.h>
#include <TimeStep.h>

#include "MotionControl.h"

class StepperFast : public StateMachine
{
  private:
    // bit patterns for stepper drivers
    const byte m_outs[4] = {0b0101, 0b1001, 0b1010, 0b0110};

    // state machine states
    enum E_STATE {
      eStop, eRun, eStep
    } m_state;

    TimeStep        m_timeStep;
    MotionControl   m_motionCtrl;

    const int       m_numSteps; // steps/revolution of stepper motor
    const int*      m_enables;  // list of enable pins (order insensitive)
    const int*      m_drivers;  // list of driver pins (order sensitive)

    double          m_speedCmd; // cmd speed in deg/sec
    double          m_posCmd;   // cmd position in deg
    double          m_speedAct; // actual speed in deg/sec
    double          m_posAct;   // actual position in deg (floating)
    long            m_pos;      // actual position in steps (whole)
    byte            m_out;      // driver output select

    void x_SetEnable(const bool ena);
    void x_SetDrivers(const byte pos);
    
  public:
    // Constructor
    StepperFast(
      const unsigned long time_step, // microseconds
      const int           num_steps, // per full rotation
      const int*          enables,   // enable pins
      const int*          drivers    // driver pins
    );

    // Update state
    virtual bool update();

    // Continuous motion at speed
    void run(const double speed);

    // Move to angle
    void step(const double angle);

    // Brake (reset motion control)
    void brake();

    // Emergency stop, disable stepper
    void estop();

    // Zero position, enable stepper
    void zero();

    // Has stop completed?
    bool isStopped() const;

    // Has move completed?
    bool isComplete() const;

    // Return actual position
    double getPosition() const;

    // Return actual speed
    double getSpeed() const;
};

#endif

