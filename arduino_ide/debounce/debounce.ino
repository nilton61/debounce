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
  
  // Endast en konstruktor behövs nu
  Debouncer(volatile uint8_t* port, uint8_t mask, uint8_t threshold = 10, 
           void (*callback)(uint8_t) = &Debouncer::dummyCallback);
  static void interrupt();
}; // Debouncer

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

ISR(TIMER2_COMPA_vect) {
  Debouncer::interrupt();
} // Timer2 interrupt

// Färgkonstanter
enum Color { RED = 0, YELLOW, GREEN, BLUE };

// Korrekt mappning mellan bitar och LEDs
const uint8_t switchBits[] = {2, 3, 4, 5};
const uint8_t ledPins[] = {A0, A1, A2, A3};

void buttonChanged(uint8_t state) {
  static uint8_t prevState = 0;
  uint8_t changed = state ^ prevState;
  prevState = state;
  
  static unsigned long toggleCounts[4] = {0, 0, 0, 0}; // Räknare för varje knapp
  
  // Gå igenom alla färger
  for (int color = RED; color <= BLUE; color++) {
    int bitPos = switchBits[color];
    
    // Om biten har ändrats och blivit låg (aktiv)
    if ((changed & (1 << bitPos)) && !(state & (1 << bitPos))) {
      int led = ledPins[color];
      digitalWrite(led, !digitalRead(led));
      
      // Öka räknaren och skriv ut
      toggleCounts[color]++;
      Serial.print("Knapp ");
      Serial.print(color);
      Serial.print(" togglerad ");
      Serial.print(toggleCounts[color]);
      Serial.println(" gånger");
    }
  }
}
Debouncer bounce(&PIND, 0x3C, 50, buttonChanged);

void setup() {
  Serial.begin(9600);
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  
  digitalWrite(A0, LOW);
  digitalWrite(A1, LOW);
  digitalWrite(A2, LOW);
  digitalWrite(A3, LOW);
} // setup

void loop() {
} // loop