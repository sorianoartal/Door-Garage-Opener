#pragma once

#include <Arduino.h>

class Delay{
  private:
    unsigned long delayTime;          // us
    unsigned long previousTime;      // us
  public:
    Delay(){}                         // Empty Constructor. Do not used
    Delay(unsigned long delayTime);   // Constructor
    
    void init();     
    void init(unsigned long delayTime);                                     // Configure the internal varialbles of the class to keep track the time
    bool isDelayTimeElapsed();                                                // returns true when the time set for the Delay has elapsed
    
    void updateDelayTime(unsigned long newDelayTime);          // set a new time for the Delay
    void restartTimer();                                                            // returns true when the delay time has elapse and reset the counter time
};
