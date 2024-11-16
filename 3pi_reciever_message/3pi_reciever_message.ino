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

//Message messages[] = {
//  {"hello", "010010010010"},
//  {"bye", "000101010100"},
//  {"yes", "001010101010"},
//  {"no", "010001101100"},
//  {"ok", "011000101000"},
//  {"thanks", "001110110110"},
//  {"please", "010101101101"},
//  {"help", "000110011011"},
//  {"sorry", "011101010101"},
//  {"stop", "001001001001"},
//  {"start", "011010110100"},
//  {"wait", "010100101110"},
//  {"go", "000111000111"},
//  {"come", "001011101011"},
//  {"left", "010011010010"},
//  {"right", "011100101001"},
//  {"up", "001110101001"},
//  {"down", "010001110100"},
//  {"forward", "001100101100"},
//  {"backward", "011011001001"}
//};

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
const unsigned long readInterval = 100; // Same interval as the transmitter

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
      // Extract the message between the delimiters
      char* start = strstr(receivedMessage, delimiter) + strlen(delimiter);
      char* end = strstr(start, delimiter);
      Serial.print("receivedMessage:");
      Serial.println(receivedMessage);
      Serial.print("\n");
      if (end) {
        *end = '\0';
        // Decode the message
        decodeMessage(start);
        // Shift the remaining part of the message to the start
        messageIndex = strlen(end + strlen(delimiter));
        memmove(receivedMessage, end + strlen(delimiter), messageIndex + 1);
      }
    }

    //delay(10); // Adjust the delay as needed
  }
}
