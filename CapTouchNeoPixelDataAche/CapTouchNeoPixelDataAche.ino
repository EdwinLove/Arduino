#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>

#define PRESSURE_THRESHOLD 40
#define NUM_PIXELS         8

bool previous[2] = {false, false};

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, 2, NEO_GRB + NEO_KHZ800);
SoftwareSerial BTserial(10, 11);
uint32_t colours[10] = {
  pixels.Color(255,255,255),
  pixels.Color(0,127,0),
  pixels.Color(0,255,0),
  pixels.Color(127,0,0),
  pixels.Color(127,127,0),
  pixels.Color(127,255,0),
  pixels.Color(255,0,0),
  pixels.Color(255,127,0),
  pixels.Color(255,255,0),
  pixels.Color(0,0,0) 
};
bool debug = true;

void setup() {
  Serial.begin(9600);
  BTserial.begin(38400);
  pixels.begin();
  pixels.show();
}

bool readTouch() {
  bool current = readCapacitivePin(8) > PRESSURE_THRESHOLD;
  bool averaged = current && previous[0] && previous[1];
  previous[1] = previous[0];
  previous[0] = current;

  return averaged;
}

void sendTouch(bool isTouched) {
  if (isTouched) {
    Serial.print(" [Send touch 1] ");
    BTserial.write("1");
  } else {
    Serial.print(" [Send touch 0] ");
    BTserial.write("0");
  }
}



uint8_t readCapacitivePin(int pinToMeasure){
  volatile uint8_t* port;
  volatile uint8_t* ddr;
  volatile uint8_t* pin;

    
  byte bitmask;
  if ((pinToMeasure >= 0) && (pinToMeasure <= 7)){
    port = &PORTD;
    ddr = &DDRD;
    bitmask = 1 << pinToMeasure;
    pin = &PIND;
  }
  if ((pinToMeasure > 7) && (pinToMeasure <= 13)){
    port = &PORTB;
    ddr = &DDRB;
    bitmask = 1 << (pinToMeasure - 8);
    pin = &PINB;
  }
  if ((pinToMeasure > 13) && (pinToMeasure <= 19)){
    port = &PORTC;
    ddr = &DDRC;
    bitmask = 1 << (pinToMeasure - 13);
    pin = &PINC;
  }
  // Discharge the pin first by setting it low and output
  *port &= ~(bitmask);
  *ddr  |= bitmask;
  delay(1);
  // Make the pin an input WITHOUT the internal pull-up on
  *ddr &= ~(bitmask);
  // Now see how long the pin to get pulled up
  int cycles = 16000;
  for(int i = 0; i < cycles; i++){
    if (*pin & bitmask){
      cycles = i;
      break;
    }
  }
  *port &= ~(bitmask);
  *ddr  |= bitmask;
  
  if (debug) {
    Serial.print("Touch value: ");
    Serial.print(cycles);
    Serial.print(", ");
    if (cycles > PRESSURE_THRESHOLD) {
      Serial.print("ABOVE");
    } else {
      Serial.print("BELOW");
    } 
    Serial.print(" threshold");
  }
  
  return cycles;
}

int inputValueFromBT() {
  if (BTserial.available() > 0) {
    char rawValue = BTserial.read();
    int asciiValue = (int)rawValue;
    if (debug) {
      Serial.print(" [In from BT: ");
      Serial.print(rawValue);
      Serial.print(">");
      Serial.print(asciiValue);
      Serial.print(">");
      Serial.print(asciiValue - 48);
      if (asciiValue > 57 || asciiValue < 48) {
        Serial.print("##INVALID##");
      }
      Serial.print("] ");

      return asciiValue - 48;
    }
  } else {
    Serial.print(" [No input from BT] ");
  }
  
  return -1;
}

uint32_t getColour(bool isTouched) {
  int inputValue = inputValueFromBT();
  if (-1 == inputValue) {
    if (isTouched) {
      if (debug) {
        Serial.print("Colour: White");
      }
      return colours[0];
    } else {
      if (debug) {
        Serial.print("Colour: Black");
      }
      return colours[9];
    }
  }

  if (debug) {
    Serial.print("Colour: ");
    Serial.print(inputValue);
  }

  return colours[inputValue];
}

void showColour(uint32_t colour) {
  if (debug) {
    Serial.print(" > uint: ");
    Serial.print(colour);
  }
  for (int i = 0; i < NUM_PIXELS; i++) {
    pixels.setPixelColor(i, colour);
  }
  pixels.show();
}

void loop() {
  bool isTouched = readTouch();
  sendTouch(isTouched);
  showColour(getColour(isTouched));
  if (debug) {
    Serial.print(" actual: ");
    Serial.print(pixels.getPixelColor(0));
    Serial.println(".");
  }
}
