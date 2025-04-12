#ifndef DEBOUNCER_H
#define DEBOUNCER_H

#include <Arduino.h>

class Debouncer {
private:
  typedef void stance;
  typedef stance (*stancePointer)();
  
  static volatile uint8_t* port;    // Registret vi läser från
  static uint8_t mask;              // Isolerar relevanta bitar
  static uint8_t lastReading;       // För att detektera ändringar
  static uint8_t bounceCounter;     // Tidmätning för stabilitet
  static uint8_t uThresh;           // Stabilitetsgräns
  
  static void (*onStateChangeCallback)(uint8_t);
  static stancePointer currentStance;
  
  static stance stable();
  static stance transient();
  
  // Dummy callback för att undvika nullpointer-kontroll
  static void dummyCallback(uint8_t state) { }
  
public:
  static uint8_t reading;  // Publik senaste läsning för användare
  
  // Konstruktor
  Debouncer(volatile uint8_t* port, uint8_t mask, uint8_t threshold = 10, 
           void (*callback)(uint8_t) = &Debouncer::dummyCallback);
  
  // Timer-interrupt-hanterare
  static void interrupt();
};

#endif // DEBOUNCER_H