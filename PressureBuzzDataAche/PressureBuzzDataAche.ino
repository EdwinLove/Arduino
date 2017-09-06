#include <Adafruit_DRV2605.h>
#include <SoftwareSerial.h>

#define BUZZ_NONE            0
#define BUZZ_SHORT_PULSE     15
#define BUZZ_LONG_PULSE      2

#define NUM_PRESSURE_SENSORS 4

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
  int totalPressure = 0;
  for (int i = 0; i < NUM_PRESSURE_SENSORS; i++) {
    if (debug && i > 0) {
      Serial.print(", ");
    }
    totalPressure = totalPressure + (pressureValue(i) * max(1, pow(3, i)));
  }
  if (debug) {
    Serial.print("]");
  }

  return totalPressure;
}

void sendPressure(int pressure) {
  if (debug) {
    Serial.print(" send: ");
    Serial.print(pressure);
  }
  BTserial.println(pressure);
}

int pressureValue(int sensor) {
  int sensorValue = analogRead(sensor);
  if (debug) {
    Serial.print(sensor);
    Serial.print(": ");
    Serial.print(sensorValue);
  }

  if (sensorValue > 500) {
    return 2;
  }

  if (sensorValue > 10) {
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
    Serial.print(" [No input from BT] ");
  }

  return -1;
}

int getBuzz(int pressure) {
  int inputValue = inputValueFromBT();

//  if (0 != inputValue) {
//    if (pressure > 0) {
//      return BUZZ_SHORT_PULSE;
//    }
//  
//    if (1 == inputValue) {
//      return BUZZ_LONG_PULSE;
//    }
//  }

  return BUZZ_NONE;
}


void loop() {
  int pressure = readPressure();
  sendPressure(pressure);
  playBuzz(getBuzz(pressure));
}

