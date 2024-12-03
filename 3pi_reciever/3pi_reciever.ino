#include "LineSensors.h"

#define BUZZER_PIN 6

// Create an instance of the CustomLineSensors class
CustomLineSensors lineSensors;
float midwayThreshold = 0.0;  // Midway threshold initialized to 0
unsigned long calibrationStartTime;  // Track the calibration start time

float combinedValueMax = 0.0;
float combinedValueMin = 10000.0;
float combinedValueSum = 0.0;
int combinedValueCount = 0;

int messageCount = 0;
float combinedValueRawMax = 0.0;
float combinedValueRawMin = 100000.0;
float combinedValueRawSum = 0.0;

void calibrateSensors() {
  // Calibrate for 5 seconds to find min and max values
  calibrationStartTime = millis();
  while (millis() - calibrationStartTime < 5000) {
    lineSensors.readRawValues();
    delay(20); // Slight delay for each reading during calibration
  }

  // Calculate the midway threshold after calibration
  float minTotal = 0.0, maxTotal = 0.0;
  for (int i = 0; i < NUM_LINE_SENSORS; i++) {
    minTotal += lineSensors.minValues[i];
    maxTotal += lineSensors.maxValues[i];
  }
 // midwayThreshold =  0.90;                      //(minTotal + maxTotal) / (2 * NUM_LINE_SENSORS);  // Midpoint of calibrated min and max
//  Serial.print("Midway Threshold: ");
//  Serial.println(midwayThreshold);  // Print the calculated threshold for verification
}

void calibrateThreshold() {
  calibrationStartTime = millis();
  while (millis() - calibrationStartTime < 2000) {
    lineSensors.calculateCalibratedValues();
    float combinedValue = 0.0;
    for (int i = 0; i < NUM_LINE_SENSORS; i++) {
      combinedValue += lineSensors.calibratedReadings[i];
    }
    combinedValue /= NUM_LINE_SENSORS;
    if (combinedValueMax < combinedValue) {
      combinedValueMax = combinedValue;
    }
    if (combinedValueMin > combinedValue) {
      combinedValueMin = combinedValue;
    }
    midwayThreshold = 0.94; //(combinedValueMax + combinedValueMin) / 2;
  }
}

void setup() {
  // Initialize the line sensors
  lineSensors.initializeSensors();
  pinMode(BUZZER_PIN, OUTPUT);

  // Initialize the Serial port for debugging and plotting
  Serial.begin(9600);

  // Run calibration for 5 seconds
  Serial.println("Calibrating...");
  calibrateSensors();
  calibrateThreshold();
  Serial.println("Calibration complete!");
}

void loop() {
  // Get the latest calibrated readings from the line sensors
  lineSensors.calculateCalibratedValues();

  // Combine all calibrated values to produce a single averaged value
  float combinedValue = 0.0;
  for (int i = 0; i < NUM_LINE_SENSORS; i++) {
    combinedValue += lineSensors.calibratedReadings[i];
  }
  combinedValue /= NUM_LINE_SENSORS;  // Get average calibrated value

  // Print the combined value for troubleshooting
  //Serial.print("Combined Calibrated Value: ");
  Serial.print(combinedValue);
  Serial.print(",");

  // Generate a binary output based on the combined value compared to midway threshold
  int binaryOutput = combinedValue > midwayThreshold ? 1 : 0;

  // Print the final binary output for Serial Plotter
  //Serial.print("Binary Output: ");
  Serial.println(binaryOutput);

  // Optional: IR signal detection with buzzer
  if (binaryOutput == 1) {
    //Serial.println("IR signal detected!");
    //analogWrite(BUZZER_PIN, 50);  // Buzzer alert if needed
  } else {
    //analogWrite(BUZZER_PIN, 0);
  }

  delay(100); // Adjust the delay as needed
}
