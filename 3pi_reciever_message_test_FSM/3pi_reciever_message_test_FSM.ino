//Team 8BitBot - University of Bristol
//refer to 3pi_reciever_message code for detailed comments
#include "LineSensors.h"
#include <PololuOLED.h>  // Pololu OLED library
#include <Wire.h>
#include <LSM6.h>
LSM6 imu;

#define BUZZER_PIN 6
#define EMIT_PIN 11
#define BUTTON_A_PIN 14

// Define the message map
struct Message {
  const char* text;
  const char* binary;
};

//Message messages[] = {
//  {"hello", "010010010010"},
//  {"bye", "000101010100"},
//  {"yes", "001010101010"},
//  {"no", "010001101100"},
//  {"ok", "011000101000"}
//};

Message messages[] = {
  {"hello", "010010010010"},
  {"bye", "000101010100"},
  {"yes", "001010101010"},
  {"no", "010001101100"},
  {"ok", "011000101000"},
  {"thanks", "001110110110"},
  {"please", "010101101101"},
  {"help", "000110011011"},
  {"sorry", "011101010101"},
  {"stop", "001001001001"},
  {"start", "011010110100"},
  {"wait", "010100101110"},
  {"go", "000111000111"},
  {"come", "001011101011"},
  {"left", "010011010010"},
  {"right", "011100101001"},
  {"up", "001110101001"},
  {"down", "010001110100"},
  {"forward", "001100101100"},
  {"backward", "011011001001"}
};
PololuSH1106 display(1, 30, 0, 17, 13);

const int numMessages = sizeof(messages) / sizeof(messages[0]);

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

const char* delimiter = "1111";
char receivedMessage[100] = "";
int messageIndex = 0;
bool receivingMessage = false;
unsigned long lastSampleTime = 0;
int delimiterCount = 0;

int messageCountRead = 0;

unsigned long testDuration = 32000; // Test duration in milliseconds (32 seconds)
unsigned long testStartTime;
int decodedMessageCount = 0;
int expectedMessageCount;

enum State {CALIBRATION, TESTING, IDLE};
State currentState = TESTING;


//caliberate sensors before 
void calibrateSensors() {
  calibrationStartTime = millis();
  while (millis() - calibrationStartTime < 2000) {
    lineSensors.readRawValues();
    delay(20);
  }

  float minTotal = 0.0, maxTotal = 0.0;
  for (int i = 0; i < NUM_LINE_SENSORS; i++) {
    minTotal += lineSensors.minValues[i];
    maxTotal += lineSensors.maxValues[i];
  }
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
    midwayThreshold = (combinedValueMax + combinedValueMin) / 2;
  }
}

void setup() {
  lineSensors.initializeSensors();
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(EMIT_PIN, INPUT);
  Serial.begin(9600);
  Wire.begin();

  pinMode(BUTTON_A_PIN, INPUT_PULLUP);

  // Initialize the accelerometer - not used in code
  if (!imu.init()) {
    while (1) {
      Serial.println("Failed to detect and initialize IMU!");
      delay(1000);
    }
  }
  imu.enableDefault();

  calibrateSensors();
  calibrateThreshold();

  testStartTime = millis();
  expectedMessageCount = (testDuration / (16 * 100));
}


//check decoded message in Message map
const char* decodeMessage(const char* binaryMessage) {
  for (int i = 0; i < numMessages; i++) {
    if (strcmp(messages[i].binary, binaryMessage) == 0) {
      decodedMessageCount++;
      return messages[i].text;
    }
  }
  return "Unknown";
}

void displayMessage(const char* status) {
  display.clear();
  display.gotoXY(2, 0);
  display.print(status);
  display.print("       ");
  display.gotoXY(0, 1);
  display.print("REC: ");
  display.print(messageCountRead);
  display.print("       ");
}

void testRoutine() {
  if (millis() - lastSampleTime >= 100) {
    lastSampleTime = millis();

    lineSensors.calculateCalibratedValues();
    float combinedValue = 0.0;
    float combinedValueRaw = 0.0;
    for (int i = 0; i < NUM_LINE_SENSORS; i++) {
      combinedValue += lineSensors.calibratedReadings[i];
      combinedValueRaw += lineSensors.rawReadings[i];

    }
    combinedValue /= NUM_LINE_SENSORS;
    combinedValueRaw /= NUM_LINE_SENSORS;

    float signalStrength = 1000.0 - combinedValueRaw;


    if (combinedValueRawMax < signalStrength) {
      combinedValueRawMax = signalStrength;
    }
    if (combinedValueRawMin > signalStrength) {
      combinedValueRawMin = signalStrength;
    }

    combinedValueRawSum += signalStrength;
    combinedValueCount++;
    if (combinedValue > combinedValueMax) combinedValueMax = combinedValue;
    if (combinedValue < combinedValueMin) combinedValueMin = combinedValue;

    int signal = combinedValue > midwayThreshold ? 1 : 0;


    // the decoding algorithm starts here
    if (!receivingMessage) {
      if (signal) {
        delimiterCount++;
      } else {
        delimiterCount = 0;
      }

     //checl if we get the delimeter
      if (delimiterCount >= 4) {
        receivingMessage = true;
        messageIndex = 0;
        delimiterCount = 0;
      }
    } else {
      if (messageIndex < 12) {
        receivedMessage[messageIndex++] = '0' + signal;
      }

      //if we get 12 bits then decode
      if (messageIndex == 12) {
        receivingMessage = false;
        receivedMessage[messageIndex] = '\0';
        messageIndex = 0;

        const char* decodedText = decodeMessage(receivedMessage);
        messageCountRead=messageCountRead + 1;
        //Serial.println(receivedMessage);
        displayMessage(decodedText);
      }
    }

    if (millis() - testStartTime >= testDuration and messageCountRead==20 ) {
      float meanSignalStrength = combinedValueRawSum / combinedValueCount;
      float accuracy = (float(decodedMessageCount) / expectedMessageCount) * 100;
      Serial.print(combinedValueRawMax); Serial.print(",");
      Serial.print(combinedValueRawMin); Serial.print(",");
      Serial.print(meanSignalStrength); Serial.print(",");
      Serial.print(decodedMessageCount); Serial.print(",");
      Serial.print(accuracy);Serial.print(",");
      Serial.println(millis() - testStartTime);

      // after the test is done, set all valyes to 0 again
      messageCountRead = 0;
      combinedValueMax = 0.0;
      combinedValueMin = 10000.0;
      combinedValueSum = 0.0;
      combinedValueCount = 0;

      messageCount = 0;
      combinedValueRawMax = 0.0;   
      combinedValueRawMin = 100000.0;
      combinedValueRawSum = 0.0;

      currentState = IDLE;
      analogWrite(BUZZER_PIN, 120);
      delay(1000);
      analogWrite(BUZZER_PIN, 0);
    }
  }
}

void loop() {
  switch (currentState) {
    case CALIBRATION:   //recaliberation state
      displayMessage("Re-Calib");
      delay(3000);
      combinedValueMax = 0.0;
      combinedValueMin = 10000.0;
      combinedValueSum = 0.0;
      combinedValueCount = 0;

      calibrateSensors();
      calibrateThreshold();
      currentState = TESTING;
      testStartTime = millis();
      decodedMessageCount = 0;
      combinedValueRawSum = 0;
      combinedValueCount = 0;
      break;
    case TESTING:     //testing state that runs for a fixed time
      testRoutine();
      break;
    case IDLE:        //press button to test again
      if (digitalRead(BUTTON_A_PIN) == LOW) {
        currentState = CALIBRATION;
        displayMessage("Done");
      }
      break;
  }
}
