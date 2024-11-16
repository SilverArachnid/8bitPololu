#include "LineSensors.h"
#include <PololuOLED.h>     // Pololu OLED library

#define BUZZER_PIN 6
#define EMIT_PIN 11

// Define the message map
struct Message {
  const char* text;
  const char* binary;
};

Message messages[] = {
  {"hello", "010010010010"},
  {"bye", "000101010100"},
  {"yes", "001010101010"},
  {"no", "010001101100"},
  {"ok", "011000101000"}
};

PololuSH1106 display(1, 30, 0, 17, 13);

const int numMessages = sizeof(messages) / sizeof(messages[0]);

// Create an instance of the CustomLineSensors class
CustomLineSensors lineSensors;
float midwayThreshold = 0.0;  // Midway threshold initialized to 0
unsigned long calibrationStartTime;  // Track the calibration start time

const char* delimiter = "1111";
char receivedMessage[100] = "";
int messageIndex = 0;
unsigned long lastReadTime = 0;
const unsigned long readInterval = 25; // Same interval as the transmitter

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
  midwayThreshold = 0.90; //(minTotal + maxTotal) / (2 * NUM_LINE_SENSORS);  // Midpoint of calibrated min and max
  Serial.print("Midway Threshold: ");
  Serial.println(midwayThreshold);  // Print the calculated threshold for verification
}

void setup() {
  // Initialize the line sensors
  lineSensors.initializeSensors();
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(EMIT_PIN, INPUT);

  // Initialize the Serial port for debugging and plotting
  Serial.begin(9600);

  // Run calibration for 5 seconds
  Serial.println("Calibrating...");
  calibrateSensors();
  Serial.println("Calibration complete!");
}

void decodeMessage(const char* binaryMessage) {
  for (int i = 0; i < numMessages; i++) {
    if (strcmp(binaryMessage, messages[i].binary) == 0) {
      Serial.print("Decoded Message: ");
      Serial.println(messages[i].text);
      displayMessage(messages[i].text);
      analogWrite(BUZZER_PIN, 120);  // Buzzer alert
      delay(500);  // Buzzer on duration
      analogWrite(BUZZER_PIN, 0);  // Buzzer off
      return;
    }
  }
  Serial.println("Unknown message.");
}

void displayMessage(const char* status) {
  display.clear();
  display.gotoXY(2, 0);
  display.print(status);
  display.print("       ");
  display.gotoXY(0, 1);
  display.print("RECIEVED");
  display.print("       ");
}

void loop() {
  if (millis() - lastReadTime >= readInterval) {
    lastReadTime = millis();

    // Get the latest calibrated readings from the line sensors
    lineSensors.calculateCalibratedValues();

    // Combine all calibrated values to produce a single averaged value
    float combinedValue = 0.0;
    for (int i = 0; i < NUM_LINE_SENSORS; i++) {
      combinedValue += lineSensors.calibratedReadings[i];
    }
    combinedValue /= NUM_LINE_SENSORS;  // Get average calibrated value

    // Generate a binary output based on the combined value compared to midway threshold
    int binaryOutput = combinedValue > midwayThreshold ? 1 : 0;

    // Print the combined value for troubleshooting
    Serial.print("Combined Calibrated Value: ");
    Serial.println(combinedValue);

    // Print the binary output for debugging
    Serial.print("Binary Output: ");
    Serial.println(binaryOutput);

    // Append the binary output to the receivedMessage buffer
    receivedMessage[messageIndex++] = '0' + binaryOutput;
    receivedMessage[messageIndex] = '\0';

    // Check for the delimiter
    if (strstr(receivedMessage, delimiter)) {
      // Find the first occurrence of the delimiter
      char* delimiterPos = strstr(receivedMessage, delimiter);
      
      // Calculate the positions of potential start and end of the message
      char* startBefore = delimiterPos - 12;
      char* startAfter = delimiterPos + strlen(delimiter);
      char* endAfter = startAfter + 12;
      
      // Debugging output
      Serial.print("receivedMessage:");
      Serial.println(receivedMessage);
      Serial.print("\n");

      // Check if the message exists before the delimiter
      if (startBefore >= receivedMessage && delimiterPos >= receivedMessage + 12) {
        // Null terminate the string before the delimiter
        *(delimiterPos) = '\0';
        // Decode the message before the delimiter
        decodeMessage(startBefore);
        // Shift the remaining part of the message to the start
        messageIndex = strlen(delimiterPos + strlen(delimiter));
        memmove(receivedMessage, delimiterPos + strlen(delimiter), messageIndex + 1);
      }
      // Check if the message exists after the delimiter
      else if (strlen(startAfter) >= 12) {
        // Null terminate the string after the message
        *(endAfter) = '\0';
        // Decode the message after the delimiter
        decodeMessage(startAfter);
        // Shift the remaining part of the message to the start
        messageIndex = strlen(endAfter + strlen(delimiter));
        memmove(receivedMessage, endAfter + strlen(delimiter), messageIndex + 1);
      }
    }

    //delay(10); // Adjust the delay as needed
  }
}
