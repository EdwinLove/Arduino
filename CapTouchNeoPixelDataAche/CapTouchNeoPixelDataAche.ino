#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>

#define PRESSURE_THRESHOLD 80
#define NUM_PIXELS         8
#define NUM_REMOTE_SENSORS 4

bool averaged[3] = {false, false, false};
bool currentTouchState = false;
int remoteSensors[NUM_REMOTE_SENSORS];

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
  for (int i = 0; i < NUM_REMOTE_SENSORS; i++) {
    remoteSensors[i] = 0;
  }
}

bool readTouch() {
  averaged[2] = averaged[1];
  averaged[1] = averaged[0];
  averaged[0] = readCapacitivePin(8) > PRESSURE_THRESHOLD;

  if (currentTouchState) {
    return averaged[0] || averaged[1] || averaged[2];
  }

  return averaged[0] && averaged[1] && averaged[2];
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

String readFromBtSerial() {
  String rawValue = "";
  while (BTserial.available() > 0) {
//    Serial.print("#");
//    Serial.print(BTserial.read());
//    Serial.print("#");
    char charRead = BTserial.read();
    if (charRead == '\n' || charRead == '\r') {
      return rawValue;
    }
    rawValue += charRead;
  }

  rawValue.trim();
  return rawValue;
}

void updateRemoteValuesFromBT() {
  Serial.print(" [In from BT: ");
  String rawValue = readFromBtSerial();
  Serial.print(rawValue);
  Serial.print("]");

  int index = 0;
  for (int i = 0; i< rawValue.length(); i++) {
    char character = rawValue[i];
    if (',' == character) {
      continue; 
    }
    remoteSensors[index] = int(character) - 48;
    index ++;
    if (NUM_REMOTE_SENSORS >= index) {
      break;
    }
  }

  if (debug) {
    Serial.print("[Remote - ");
    for (int i = 0; i < NUM_REMOTE_SENSORS; i++) {
      if (i > 0) {
              Serial.print(", ");
      }
      Serial.print(i);
      Serial.print(":");
      Serial.print(remoteSensors[i]);
    }
    Serial.print("]");
  }
//  if ("" <> rawValue) {
 //   int intValue = (int)rawValue;
//    int splitValue[NUM_REMOTE_SENSORS];
//
//    for (int i = 0; i < NUM_REMOTE_SENSORS; i++) {
//      splitValue[i] = 
//    }
//    
//    if (debug) {
//      Serial.print(rawValue);
//      Serial.print(">");
//      Serial.print(asciiValue);
//      Serial.print(">");
//      Serial.print(asciiValue - 48);
//      if (asciiValue > 57 || asciiValue < 48) {
//        Serial.print("##INVALID##");
//      }
//      Serial.print("] ");
//
//      return asciiValue - 48;
//    }
//  } else {
//    Serial.print(" [No input from BT] ");
//  }
  
//  return -1;
}

uint32_t getColour(bool isTouched) {
  int inputValue = 1;//inputValueFromBT();
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
  currentTouchState = readTouch();
  sendTouch(currentTouchState);
  updateRemoteValuesFromBT();
  showColour(getColour(currentTouchState));
  if (debug) {
    Serial.print(" actual: ");
    Serial.print(pixels.getPixelColor(0));
    Serial.println(".");
  }
}
