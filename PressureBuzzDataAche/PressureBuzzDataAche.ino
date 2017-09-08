#include <Adafruit_DRV2605.h>
#include <SoftwareSerial.h>

#define BUZZ_NONE            0
#define BUZZ_SHORT_PULSE     15
#define BUZZ_LONG_PULSE      2

#define NUM_PRESSURE_SENSORS 4
#define PRESSURE_THRESHOLD   100

SoftwareSerial BTserial(10, 11);
Adafruit_DRV2605 drv;
bool debug = true;

void setup() {
  Serial.begin(9600);
  BTserial.begin(38400);
  drv.begin();
  drv.selectLibrary(1);
  drv.setMode(DRV2605_MODE_INTTRIG);
}

void playBuzz(uint8_t buzz) {
  if (buzz != BUZZ_NONE) {
    drv.setWaveform(0, buzz);
    drv.setWaveform(1, 0);
    drv.go();
    if (debug) {
      Serial.print("Buzz: ");
      Serial.println(buzz);
    }
  } else if (debug) {
    Serial.println("No Buzz");
  }
}

int readPressure() {
  if (debug) {
    Serial.print("Pressure [");
  }

  int pressure = 0;
  for (int i = 0; i < NUM_PRESSURE_SENSORS; i++) {
    if (debug && i > 0) {
      Serial.print(", ");
    }
    int sensorPressure = pressureValueForSensor(i);
    int power = bit(i);
    int poweredPressure = sensorPressure * power;
    pressure += poweredPressure;
    if (debug) {
      Serial.print("(");
      Serial.print(sensorPressure);
      Serial.print(",");
      Serial.print(i);
      Serial.print(",");
      Serial.print(power);
      Serial.print(",");
      Serial.print(poweredPressure);
      Serial.print(")");
    }
  }
  if (debug) {
    Serial.print("]");
  }

  return pressure;
}

void sendPressure(int pressure) {
  char asciiPressure = pressure + 48;
  if (debug) {
    Serial.print(" send: ");
    Serial.print(pressure);
    Serial.print(" as ");
    Serial.print(asciiPressure);
  }
  BTserial.println(asciiPressure);
}

int pressureValueForSensor(int sensor) {
  int sensorValue = analogRead(sensor);
  if (debug) {
    Serial.print(sensor);
    Serial.print(": ");
    Serial.print(sensorValue);
  }

//  if (sensorValue > 500) {
//    return 2;
//  }

  if (sensorValue > PRESSURE_THRESHOLD) {
    return 1;
  }

  return 0;
}

int inputValueFromBT() {
  if (BTserial.available() > 0) { // Checks whether data is comming from the serial port
    char rawValue = BTserial.read();
    if (debug) {
      Serial.print(" [In from BT: ");
      Serial.print(rawValue);
      Serial.print("] ");
    }
    if ('0' == rawValue) {
      return 0;
    }
    if ('1' == rawValue) {
      return 1;
    }
  } else if (debug) {
    Serial.print(" [In from BT:-1] ");
  }

  return -1;
}

int getBuzz(int pressure) {
  int inputValue = inputValueFromBT();

  if (pressure == 0) {
    return 1 == inputValue ? 94 : 0;
  }
  if (pressure == 1 || pressure == 2 || pressure == 4 || pressure == 8) {
    return 1 == inputValue ? 56 : 58;
  }
  return 1 == inputValue ? 83 : 15;
}


void loop() {
  int pressure = readPressure();
  sendPressure(pressure);
  playBuzz(getBuzz(pressure));
}

