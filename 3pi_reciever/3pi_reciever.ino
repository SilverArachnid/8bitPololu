#include "LineSensors.h"

#define BUZZER_PIN 6

// Create an instance of the CustomLineSensors class
CustomLineSensors lineSensors;
float calibrationThreshold = 0.8;

void setup() {
  // Initialize the line sensors
  lineSensors.initializeSensors();
  pinMode(BUZZER_PIN, OUTPUT);

  // Initialize the Serial port for debugging
  Serial.begin(9600);
}

void loop() {
  // Get the latest calibrated readings from the line sensors
  lineSensors.calculateCalibratedValues();

  // Access and print the updated calibrated readings
  for (int i = 0; i < NUM_LINE_SENSORS; i++) {
    Serial.print(lineSensors.calibratedReadings[i]);
    if (i < NUM_LINE_SENSORS - 1) {
      Serial.print(", ");
    }
  }
  Serial.println();

  // Detect the IR signal by checking the calibrated sensor readings
  for (int i = 0; i < NUM_LINE_SENSORS; i++) {
    if (lineSensors.calibratedReadings[i] < calibrationThreshold) {
      // Implement your logic here when IR signal is detected
      Serial.println("IR signal detected!");
      // analogWrite(BUZZER_PIN, 50);
      // Add your follower bot movement logic here
    } else {
      // analogWrite(BUZZER_PIN, 0);
    }
  }

  delay(100); // Adjust the delay as needed
}
