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
  TCCR2A |= (1 << WGM21);   //CTC
  OCR2A = 159;              //16 MHz * 159 = 10 us
  TCCR2B |= (1 << CS20);    //Prescaler = 1 
  TIMSK2 |= (1 << OCIE2A);  //Aktivate interrupt
  
  lastReading = *port & mask;
  reading = lastReading;
  currentStance = &Debouncer::stable;
  
  sei();
} // Debouncer konstruktor

void Debouncer::interrupt() {
  (*currentStance)();
} // interrupt

Debouncer::stance Debouncer::stable() {
  reading = *port & mask;                       //check switces
  if (reading != lastReading) {                 //enter transient state
    bounceCounter = 0;                          //reset counter
    onStateChangeCallback(reading);             //do callback
    currentStance = &Debouncer::transient;      //change state
  }//if new reading
  lastReading = reading;                        //Always update
  return;
} // stable

Debouncer::stance Debouncer::transient() {
  bounceCounter++;                              //keep tally
  reading = *port & mask;                       //check swtiches
  if (reading != lastReading) bounceCounter = 0;//new value, reset counter
  //check count treshold for stable state:
  else if (bounceCounter > uThresh) currentStance = &Debouncer::stable;
  lastReading = reading;                        //always update
  return;
} // transient

//TIMER2 interrupt handler, in file but outside class
ISR(TIMER2_COMPA_vect) {
  Debouncer::interrupt();
} // Timer2 interrupt