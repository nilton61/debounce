#include <arduino.h>

class Debouncer {
private:
  // Tillståndsmaskindefinitioner för att hålla en enkel, läsbar struktur
  typedef void stance;
  typedef stance (*stancePointer)();
  
  // Variabler för att hantera portar och tillstånd
  volatile uint8_t* port;
  uint8_t mask;
  uint8_t currentState;
  uint8_t lastReading;
  uint8_t bounceCounter;
  uint8_t uThresh;
  
  // Callback-funktion som anropas vid tillståndsändringar
  void (*onStateChangeCallback)(uint8_t);
  
  // Tillståndsfunktioner enligt diagrammet
  stance stableInactive();
  stance transientActive();
  stance stableActive();
  stance transientInactive();
  
public:
  // Konstruktorer och interrupthanterare
  Debouncer();
  Debouncer(volatile uint8_t* port, uint8_t mask, uint8_t threshold, void (*callback)(uint8_t));
  static void interrupt();
}; // Debouncer

// Konstruktor som konfigurerar Timer 2 för avStudsningshantering
Debouncer::Debouncer() {
  // Nollställ timer-register för ren konfiguration
  TCCR2A = 0;
  TCCR2B = 0;
  
  // Sätt CTC-läge för precis timing
  TCCR2A |= (1 << WGM21);
  
  // Konfigurera för ca 10 µs intervall (16MHz CPU)
  OCR2A = 159;
  
  // Använd prescaler 1 för maximal precision
  TCCR2B |= (1 << CS20);
  
  // Aktivera interrupt vid Compare Match
  TIMSK2 |= (1 << OCIE2A);
  
  // Sätt på globala interrupts
  sei();
} // Debouncer()

// Koppla Timer 2's interrupt till vår hanterare
ISR(TIMER2_COMPA_vect) {
  Debouncer::interrupt();
}

// Statisk metod som anropas vid varje Timer 2 interrupt
void Debouncer::interrupt() {
  digitalWrite(14, 1);
}


Debouncer bounce;

void setup(){
   pinMode(14, OUTPUT);
};
void loop(){};