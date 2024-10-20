#include "LineSensors.h"

#define BUZZER_PIN 6
// Create an instance of the LineSensors_c class
LineSensors_c line_sensors;
float some_threshold = 500;

void setup() {
  // Initialize the line sensors
  line_sensors.initialiseForADC();
  pinMode(BUZZER_PIN, OUTPUT);


  // Initialize the Serial port for debugging
  Serial.begin(9600);
}

void loop() {
  // Get latest readings from the line sensors
  line_sensors.readSensorsADC();

  // Access and print the updated readings variable within the line_sensors class instance
  for (int i = 0; i < NUM_SENSORS; i++) {
    Serial.print(line_sensors.readings[i]);
    if (i < NUM_SENSORS - 1) {
      Serial.print(", ");
    }
  }
  Serial.println();

  // Detect the IR signal by checking the line sensor readings
  for (int i = 0; i < NUM_SENSORS; i++) {
    if (line_sensors.readings[i] < some_threshold) {
      // Implement your logic here when IR signal is detected
      Serial.println("IR signal detected!");
     // analogWrite( BUZZER_PIN, 50 );
      // Add your follower bot movement logic here
    }
    else{
      //analogWrite( BUZZER_PIN, 0 );
    }
  }

  delay(100); // Adjust the delay as needed
}
