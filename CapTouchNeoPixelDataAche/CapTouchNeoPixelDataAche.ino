#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>

#define PRESSURE_THRESHOLD 210
#define NUM_PIXELS         8

#define BRIGHT 255
#define DIM    127

int averaged[3] = {0, 0, 0};
int averagedRemote[3] = {-1,-1, -1};

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, 2, NEO_GRB + NEO_KHZ800);
SoftwareSerial BTserial(10, 11);
uint32_t colours[16] = {
  pixels.Color(     0,     0,     0),
  pixels.Color(     0,     0,   DIM),
  pixels.Color(     0,   DIM,     0),
  pixels.Color(     0,   DIM,   DIM),
  pixels.Color(   DIM,     0,     0),
  pixels.Color(   DIM,     0,   DIM),
  pixels.Color(   DIM,   DIM,     0),
  pixels.Color(   DIM,   DIM,   DIM),
  pixels.Color(   DIM,   DIM,   DIM),
  pixels.Color(     0,     0,BRIGHT),
  pixels.Color(     0,BRIGHT,     0),
  pixels.Color(     0,BRIGHT,BRIGHT),
  pixels.Color(BRIGHT,     0,     0),
  pixels.Color(BRIGHT,     0,BRIGHT),
  pixels.Color(BRIGHT,BRIGHT,     0),
  pixels.Color(BRIGHT,BRIGHT,BRIGHT),
};
bool debug = true;

void setup() {
  Serial.begin(9600);
  BTserial.begin(38400);
  pixels.begin();
  pixels.show();
}

bool readTouch() {
  int touchValue = readCapacitivePin(8);
  averaged[2] = averaged[1];
  averaged[1] = averaged[0];
  averaged[0] = touchValue;

  int averagedTotal = averaged[0] + averaged[1] + averaged[2];
  if (debug) {
    Serial.print("Touch value: ");
    Serial.print(touchValue);
    Serial.print(" [Average: ");
    Serial.print(averagedTotal);
    Serial.print(", ");
    if (averagedTotal > PRESSURE_THRESHOLD) {
      Serial.print("ABOVE");
    } else {
      Serial.print("BELOW");
    } 
    Serial.print(" threshold");
    Serial.print("]");
  }

  return averagedTotal > PRESSURE_THRESHOLD;
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
  
  return cycles;
}

int remoteStateFromBT() {
  int returnValue = -1;

  while (BTserial.available() > 0) {
    char rawValue = BTserial.read();
    int intValue = int(rawValue) - 48;
    if (intValue >= 0 && intValue <16) {
      returnValue = intValue;
    }
  }

  averagedRemote[2] = averagedRemote[1];
  averagedRemote[1] = averagedRemote[0];
  averagedRemote[0] = returnValue;

  if (averagedRemote[0] != -1) {
    return averagedRemote[0];
  }
  if (averagedRemote[1] != -1) {
    return averagedRemote[1];
  }

  return averagedRemote[2];
}

uint32_t getColour(bool isTouched, int remoteState) {
  if (remoteState < 1) {
    if (isTouched) {
      if (debug) {
        Serial.print("Colour: Random");
      }
      return colours[random(16)];
    } else {
      if (debug) {
        Serial.print("Colour: Black");
      }
      return colours[0];
    }
  }

  if (debug) {
    Serial.print("Colour: ");
    Serial.print(remoteState);
  }

  return colours[remoteState];
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
  int remoteState = remoteStateFromBT();
  if (debug) {
    Serial.print("[In from BT: ");
    Serial.print(remoteState);
    Serial.print("] ");
  }
  bool isTouched = readTouch();
  sendTouch(isTouched);
  showColour(getColour(isTouched, remoteState));
  if (debug) {
    Serial.print(" actual: ");
    Serial.print(pixels.getPixelColor(0));
    Serial.println(".");
  }
}
