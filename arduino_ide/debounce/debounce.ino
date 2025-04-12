#include "debouncer.h"

// Färgkonstanter
enum Color { RED, YELLOW, GREEN, BLUE };

// Korrekt mappning mellan bitar och LEDs
const uint8_t switchPin[] = {2, 3, 4, 5};
const uint8_t ledPin[] = {A0, A1, A2, A3};

void buttonChanged(uint8_t state) {
  static uint8_t prevState = 0;
  uint8_t changed = state ^ prevState;
  prevState = state;
  
  // Gå igenom alla färger
  for (int color = RED; color <= BLUE; color++) {
    int bitMask = (1 << switchPin[color]);
    
    // Om biten har ändrats och blivit låg (aktiv)
    if ((changed & bitMask) && !(state & bitMask)) {
      int led = ledPin[color];
      digitalWrite(led, !digitalRead(led)); 
    }//bit high -> low
  }//for color
}//button changed

// Skapa debouncer-instans för knappar på PIND (digital pins 0-7)
// som lyssnar på pins 2, 3, 4 och 5 (mask 0x3C = 0b00111100)
Debouncer bounce(&PIND, 0x3C, 40, buttonChanged);

void setup() {
  //Serial.begin(9600);
    for(int color = RED; color <= BLUE; color++){
    pinMode(switchPin[color], INPUT);
    pinMode(ledPin[color], OUTPUT);
  }//for color
} // setup

void loop() {
  // Inget att göra här - interrupt sköter allt
} // loop