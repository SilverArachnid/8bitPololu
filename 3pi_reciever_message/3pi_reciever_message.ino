#include "LineSensors.h"
#include <string.h>

#define BUZZER_PIN 6

// Create an instance of the CustomLineSensors class
CustomLineSensors lineSensors;
float midwayThreshold = 0.0;  // Midway threshold initialized to 0
unsigned long calibrationStartTime;  // Track the calibration start time
bool flagstart=false;
long starttime;
long transmissionrate=100;
String str="";

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
  midwayThreshold =  0.95;                      //(minTotal + maxTotal) / (2 * NUM_LINE_SENSORS);  // Midpoint of calibrated min and max
  Serial.print("Midway Threshold: ");
  Serial.println(midwayThreshold);  // Print the calculated threshold for verification
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
  //Serial.println(combinedValue);

  // Generate a binary output based on the combined value compared to midway threshold
  int binaryOutput = combinedValue > midwayThreshold ? 1 : 0;
  if(binaryOutput==1 && !flagstart){
    flagstart=true;
    starttime=millis();
  }
  //Serial.println(binaryOutput);
  if(str.length()<60 && flagstart){
    if((millis()-starttime)>50){
      if(binaryOutput==1){
        str=str+"1";
        //analogWrite(BUZZER_PIN, 50);
      }
      else{
        str=str+"0";
        //analogWrite(BUZZER_PIN, 0);
      }
      starttime+=transmissionrate;
    }
  }
  else{
    //Serial.println(str);
  }
  
  // Print the final binary output for Serial Plotter
  //Serial.print("Binary Output: ");
  Serial.println(combinedValue);

  // Optional: IR signal detection with buzzer
  //if (binaryOutput == 1) {
    //Serial.println("IR signal detected!");
    //analogWrite(BUZZER_PIN, 50);  // Buzzer alert if needed
  //} else {
    //analogWrite(BUZZER_PIN, 0);
  //}
  
  //delay(10); // Adjust the delay as needed
}
