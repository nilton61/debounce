#include "Debouncer.h"

// Initiera statiska variabler
volatile uint8_t* Debouncer::port = nullptr;
uint8_t Debouncer::mask = 0;
uint8_t Debouncer::lastReading = 0;
uint8_t Debouncer::bounceCounter = 0;
uint8_t Debouncer::uThresh = 10;
uint8_t Debouncer::reading = 0;
Debouncer::stancePointer Debouncer::currentStance = &Debouncer::stable;
void (*Debouncer::onStateChangeCallback)(uint8_t) = &Debouncer::dummyCallback;

Debouncer::Debouncer(volatile uint8_t* p, uint8_t m, uint8_t threshold, void (*callback)(uint8_t)) {
  port = p;
  mask = m;
  uThresh = threshold;
  onStateChangeCallback = callback;
  
  TCCR2A = 0;
  TCCR2B = 0;
  TCCR2A |= (1 << WGM21);   // CTC-läge
  OCR2A = 159;              // För 10µs interval
  TCCR2B |= (1 << CS20);    // Ingen prescaler
  TIMSK2 |= (1 << OCIE2A);  // Aktivera interrupt
  
  lastReading = *port & mask;
  reading = lastReading;
  currentStance = &Debouncer::stable;
  
  sei();
} // Debouncer konstruktor

void Debouncer::interrupt() {
  (*currentStance)();
} // interrupt

Debouncer::stance Debouncer::stable() {
  reading = *port & mask;
  
  if (reading != lastReading) {
    bounceCounter = 0;
    onStateChangeCallback(reading);
    currentStance = &Debouncer::transient;
  } // if reading changed
  
  lastReading = reading;  // Uppdatera alltid lastReading
  return;
} // stable

Debouncer::stance Debouncer::transient() {
  bounceCounter++;
  reading = *port & mask;
  
  if (reading != lastReading) {
    bounceCounter = 0;
  } // if reading changed
  else if (bounceCounter > uThresh) {
    currentStance = &Debouncer::stable;
  } // else if stable
  
  lastReading = reading;  // Uppdatera alltid lastReading
  return;
} // transient

// Timer2 interrupt-hanterare - denna behöver vara i .cpp-filen
// men utanför klassen
ISR(TIMER2_COMPA_vect) {
  Debouncer::interrupt();
} // Timer2 interrupt