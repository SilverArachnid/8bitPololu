/***************************************
 ,        .       .           .     ,-.  
 |        |       |           |        ) 
 |    ,-: |-. ,-. |-. ,-. ,-. |-      /  
 |    | | | | `-. | | |-' |-' |      /   
 `--' `-` `-' `-' ' ' `-' `-' `-'   '--' 
****************************************/

#ifndef _LINESENSORS_H
#define _LINESENSORS_H

#define NUM_LINE_SENSORS 5

const int sensorPins[NUM_LINE_SENSORS] = {A11, A0, A2, A3, A4};
#define IR_EMIT_PIN 11

class CustomLineSensors {
public:
  float rawReadings[NUM_LINE_SENSORS];
  float minValues[NUM_LINE_SENSORS] = {10000.0, 10000.0, 10000.0, 10000.0, 10000.0};
  float maxValues[NUM_LINE_SENSORS] = {0.0, 0.0, 0.0, 0.0, 0.0};
  float scaleFactors[NUM_LINE_SENSORS];
  float calibratedReadings[NUM_LINE_SENSORS];

  CustomLineSensors() {
    // Constructor
  }

  void initializeSensors() {
    pinMode(IR_EMIT_PIN, OUTPUT);
    digitalWrite(IR_EMIT_PIN, HIGH);
    for (int sensor = 0; sensor < NUM_LINE_SENSORS; sensor++) {
      pinMode(sensorPins[sensor], INPUT_PULLUP);
    }
  }

  void readRawValues() {
    initializeSensors();
    for (int sensor = 0; sensor < NUM_LINE_SENSORS; sensor++) {
      rawReadings[sensor] = analogRead(sensorPins[sensor]);
      if (rawReadings[sensor] > maxValues[sensor]) {
        maxValues[sensor] = rawReadings[sensor];
      }
      if (rawReadings[sensor] < minValues[sensor]) {
        minValues[sensor] = rawReadings[sensor];
      }
      scaleFactors[sensor] = maxValues[sensor] - minValues[sensor];
    }
  }

  void calculateCalibratedValues() {
    readRawValues();
    for (int sensor = 0; sensor < NUM_LINE_SENSORS; sensor++) {
      calibratedReadings[sensor] = (float)((rawReadings[sensor] - minValues[sensor]) / scaleFactors[sensor]);
    }
  }
};

#endif
