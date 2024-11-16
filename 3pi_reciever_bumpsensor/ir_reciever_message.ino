#include <Pololu3piPlus32U4.h>


using namespace Pololu3piPlus32U4;

BumpSensors bumpSensors;
Buzzer buzzer;

// Structure to hold predefined messages and their binary strings
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

const int numMessages = sizeof(messages) / sizeof(messages[0]);
const char* delimiter = "1111";

// OLED display instance
OLED display;

// Buffer to store received binary data
char receivedMessage[100];
int messageIndex = 0;
bool receivingMessage = false;
unsigned long lastSampleTime = 0;
int delimiterCount = 0;

// Function to decode the received binary message
const char* decodeMessage(const char* binaryMessage) {
  for (int i = 0; i < numMessages; i++) {
    if (strcmp(messages[i].binary, binaryMessage) == 0) {
      //buzzer.play("a32");
      return messages[i].text;
    }
  }
  return "";
}

// Function to display the decoded message on the OLED
void displayMessage(const char* status) {
  display.clear();
  display.gotoXY(0, 0);
  display.print("Message:");
  display.gotoXY(0, 1);
  display.print(status);
}

void setup() {
  Serial.begin(9600);
  bumpSensors.calibrate(1);
  display.clear();
  display.gotoXY(0, 1);
  display.print("Waiting...");
  

}

void loop() {
  // Read the bump sensors (IR signal detectors)
  bumpSensors.read();
  bool leftSignal = bumpSensors.leftIsPressed();
  bool rightSignal = bumpSensors.rightIsPressed();

  // Ensure sampling occurs every 10ms
  unsigned long currentTime = millis();
  if (currentTime - lastSampleTime < 50) {
    return; // Skip processing if 10ms hasn't passed
  }
  lastSampleTime = currentTime;

  // Detect signal from either sensor
  bool signal = leftSignal || rightSignal;

  // Detect the delimiter (4 consecutive '1's)
  if (!receivingMessage) {
    if (signal) {
      delimiterCount++;
    } else {
      delimiterCount = 0; // Reset delimiter count if interrupted
    }

    if (delimiterCount >= 4) {
      receivingMessage = true; // Start receiving the message
      messageIndex = 0;        // Reset message buffer index
    }
    return; // Continue looping until the delimiter is detected
  }
  else{
    if (receivingMessage && messageIndex < 12 ){
      receivedMessage[messageIndex++] = signal ? '1' : '0';
    }
    else{
      //buzzer.play("a32");
      messageIndex++;
      receivedMessage[messageIndex] = '\0'; // Null-terminate the message
      messageIndex=0;
    // Process the received message
      const char* binaryData = receivedMessage;
      const char* decodedText = decodeMessage(binaryData);
      Serial.println(binaryData);
      displayMessage(decodedText); // Display decoded message
      if(signal)
        delimiterCount = 1;         // Reset delimiter count for future messages
      receivingMessage = false;
      
      return;
    }
  }
}


  

