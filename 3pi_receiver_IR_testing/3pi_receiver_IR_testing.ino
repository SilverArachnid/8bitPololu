//Team 8BitBot - University of Bristol
#include "LineSensors.h"
#include <PololuOLED.h>  // Pololu OLED library
#include <Wire.h>
#include <Pololu3piPlus32U4.h>

#define BUZZER_PIN 6
#define EMIT_PIN 11
#define BUTTON_A_PIN 14

using namespace Pololu3piPlus32U4;
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

uint32_t time_per_bit=100;
const int numMessages = sizeof(messages) / sizeof(messages[0]);
BumpSensors bumpSensors;
Buzzer buzzer;
uint32_t minimum, maximum;

float midwayThreshold = 0.0;  // Midway threshold initialized to 0
unsigned long calibrationStartTime;  // Track the calibration start time


int messageCount = 0;

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

void calibrateSensors() {
  //this should be modified to adjust your threshold (write a code for the same)
  minimum=100000;
  maximum=0;
  int avg=0;
  uint32_t sum=0;
  bumpSensors.timeout=50000;
  /*for(int i=0;i<20;i++){ 
    bumpSensors.calibrate(1);
    
    Serial.println(avg);
    sum+=avg;
  }
  
  */
  decodedMessageCount = 0;
  for(int i=0;i<100;i++){
    bumpSensors.read();
    avg=(bumpSensors.sensorValues[0]+bumpSensors.sensorValues[1])/2;
    sum+=avg;
  }
  bumpSensors.threshold[0]=bumpSensors.threshold[1]=sum/100;
  Serial.println(bumpSensors.threshold[0]);
}

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(EMIT_PIN, INPUT);
  Serial.begin(9600);
  Wire.begin();

  pinMode(BUTTON_A_PIN, INPUT_PULLUP);


  calibrateSensors();

  testStartTime = millis();
  expectedMessageCount = (testDuration / (16 * time_per_bit));
}

const char* decodeMessage(const char* binaryMessage) {
  Serial.println(binaryMessage);
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
  bumpSensors.read();
  bool leftSignal = bumpSensors.leftIsPressed();
  bool rightSignal = bumpSensors.rightIsPressed();
  unsigned long currentTime = millis();
  if (currentTime - lastSampleTime > time_per_bit) {
    bool signal = leftSignal || rightSignal ;
    lastSampleTime +=time_per_bit;
    Serial.print(signal);
    Serial.print(",");
    Serial.print(bumpSensors.threshold[0]);
    Serial.print(",");
    Serial.print(bumpSensors.threshold[1]);
    Serial.print(",");
    Serial.print(minimum);
    Serial.print(",");
    Serial.print(bumpSensors.sensorValues[0]);
    Serial.print(",");
    Serial.print(bumpSensors.sensorValues[1]);
    Serial.print(",");
    Serial.println(maximum);

    

    int avg=(bumpSensors.sensorValues[0]+bumpSensors.sensorValues[1])/2;
    

    minimum=min(minimum,avg);
    maximum = max(maximum,avg);


    if (!receivingMessage) {
      if (signal) {
        delimiterCount++;
      } else {
        delimiterCount = 0;
      }

      if (delimiterCount >= 4) {
        receivingMessage = true;
        messageIndex = 0;
        delimiterCount = 0;
      }
    } else {
      if (messageIndex < 12) {
        if(messageIndex==0 && signal==1){
          while(signal==1){
            bumpSensors.read();
            leftSignal = bumpSensors.leftIsPressed();
            rightSignal = bumpSensors.rightIsPressed();  
            signal = leftSignal || rightSignal;
          }
          lastSampleTime=millis();
        }
        receivedMessage[messageIndex++] = '0' + signal;
      }

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

    if (millis() - testStartTime >= testDuration and messageCountRead==expectedMessageCount) {
      float accuracy = (float(decodedMessageCount) / expectedMessageCount) * 100;
      Serial.print(maximum); Serial.print(",");
      Serial.print(minimum); Serial.print(",");
      Serial.print(25000-bumpSensors.threshold[0]); Serial.print(",");
      Serial.print(decodedMessageCount); Serial.print(",");
      Serial.print(accuracy);Serial.print(",");
      Serial.println(millis() - testStartTime);


      currentState = IDLE;
      analogWrite(BUZZER_PIN, 120);
      delay(1000);
      analogWrite(BUZZER_PIN, 0);
    }
  }
}

void loop() {
  switch (currentState) {
    case CALIBRATION:
      displayMessage("Re-Calib");
      delay(3000);

      calibrateSensors();
      currentState = TESTING;
      testStartTime = millis();

      break;
    case TESTING:
      testRoutine();
      break;
    case IDLE:
      if (digitalRead(BUTTON_A_PIN) == LOW) {
        currentState = CALIBRATION;
        decodedMessageCount=0;
        messageCountRead=0;
         

        displayMessage("Done");
      }
      break;
  }
}