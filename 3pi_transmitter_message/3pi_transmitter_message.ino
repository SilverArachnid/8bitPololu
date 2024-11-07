#include <stdlib.h>  // Include the standard library for random functions
#include <PololuOLED.h>     // Pololu OLED library

#define EMIT_PIN 11
#define BUZZ_PIN 6

unsigned long timetaken;
bool emittion = HIGH;

// Structure to hold messages and their corresponding binary strings
struct Message {
  const char* text;
  const char* binary;
};

// Predefined dictionary of messages
Message messages[] = {
  {"hello", "010010010010"},
  {"bye", "000101010100"},
  {"yes", "001010101010"},
  {"no", "010001101100"},
  {"ok", "011000101000"}
};

PololuSH1106 display(1, 30, 0, 17, 13);

const int numMessages = sizeof(messages) / sizeof(messages[0]);
const char* currentBinaryMessage;
int binaryIndex = 0;
int messageRepeatCount = 0;  // Counter to keep track of how many times the current message has been sent

// Concatenate '1111' delimiters to the message
const char* delimiter = "1111";
char fullMessage[100];

void displayMessage(const char* status) {
  display.clear();
  display.gotoXY(2, 0);
  display.print(status);
  display.print("       ");
  display.gotoXY(0, 1);
  display.print("  SENT");
  display.print("       ");
}

void setup() {
  // Configure the EMIT pin as output and set it to high
  pinMode(EMIT_PIN, OUTPUT);
  pinMode(BUZZ_PIN, OUTPUT);
  digitalWrite(EMIT_PIN, HIGH);
  timetaken = millis();
  // Initialize the serial port
  Serial.begin(9600);

  // Seed the random number generator
  randomSeed(analogRead(0));

  // Select and prepare the first message to transmit
  selectRandomMessage();
}

void loop() {
  // Continuously emit IR light
  if (millis() - timetaken > 100) {
    emittion = (fullMessage[binaryIndex] == '1');
    digitalWrite(EMIT_PIN, emittion);
    if (emittion) {
      analogWrite(BUZZ_PIN, 0);
    } else {
      analogWrite(BUZZ_PIN, 0);
    }
    Serial.print(emittion);

    binaryIndex++;
    if (fullMessage[binaryIndex] == '\0') {
      binaryIndex = 0; // Reset to the start of the full message
      messageRepeatCount++;  // Increment the counter

      // Check if the message has been sent 10 times
      if (messageRepeatCount >= 10) {
        messageRepeatCount = 0;  // Reset the counter
        selectRandomMessage();   // Select a new random message
      }
    }

    timetaken = millis();
  }
}

void selectRandomMessage() {
  int randomIndex = random(numMessages);
  currentBinaryMessage = messages[randomIndex].binary;

  // Create the full message with delimiters
  strcpy(fullMessage, delimiter);
  strcat(fullMessage, currentBinaryMessage);
  // strcat(fullMessage, delimiter); // Optional: add delimiter at the end if needed

  Serial.print("New message selected: ");
  Serial.println(messages[randomIndex].text);
  displayMessage(messages[randomIndex].text);
}
