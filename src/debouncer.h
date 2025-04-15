#ifndef DEBOUNCER_H
#define DEBOUNCER_H

#include <Arduino.h>

class Debouncer {
private:
  typedef void stance;
  typedef stance (*stancePointer)();
  
  static volatile uint8_t* port;    //port pins for switches
  static uint8_t mask;              //mask bits coresponding to switches

  static uint8_t bounceCounter;     //timekeeping
  static uint8_t uThresh;           //treshold for stable state
  static uint8_t lastReading;       //compare reading to last reading
  static uint8_t reading;           //in order to detect cahnges
 
  
  static void (*onStateChangeCallback)(uint8_t);
  static stancePointer currentStance;
  
  static stance stable();
  static stance transient();
  
  //dummy callback avoids nulptr check
  static void dummyCallback(uint8_t state) { };
  
public:
  static uint8_t getReading(){return lastReading;}; 
  //constructor declararion: default callback = dummyCallback()
  Debouncer(volatile uint8_t* port, uint8_t mask, uint8_t threshold = 10, 
           void (*callback)(uint8_t) = &Debouncer::dummyCallback);
  static void interrupt();        //interrupt handler
};//Debouncer class

#endif // DEBOUNCER_H