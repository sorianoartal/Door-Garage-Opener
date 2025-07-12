#include "HardwareSerial.h"
#include "Delay/Delay.h"
#include "Arduino.h"


/**Constructor: we pass the  target time delay*/
Delay::Delay(unsigned long delayTime) : delayTime(delayTime),previousTime(0){}

/** set the target time delay and start counting */
void Delay::init(){
  this->delayTime = delayTime;
  this->previousTime = micros();
}

void Delay::init(unsigned long delayTime){
  this->delayTime = delayTime;
  previousTime = micros();
}


/** Calculate if the delay time has elapsed*/
bool Delay::isDelayTimeElapsed(){ 
  unsigned long now = micros();
  if(now - previousTime >= delayTime) {
    restartTimer() ; 
    return true;
  } else {
    return false;
  }
 
}

/** when the time delay has elapse we update the time counter*/
void Delay::restartTimer(){
  this->previousTime = micros();
}

/** Set new Delay Value for the Class*/
void Delay::updateDelayTime(unsigned long newDelayTime){
  this->delayTime = newDelayTime;
}