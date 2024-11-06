//#include <Pololu3piPlus32U4.h>

// Define the pin used to activate the IR LEDs and buzzer
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

const int numMessages = sizeof(messages) / sizeof(messages[0]);
int currentMessageIndex = 3; // Change this index to select a different message
const char* currentBinaryMessage;
int binaryIndex = 0;

// Concatenate '1111' delimiters to the message
const char* delimiter = "1111";
char fullMessage[100];

void setup() {
  // Configure the EMIT pin as output and set it to high
  pinMode(EMIT_PIN, OUTPUT);
  pinMode(BUZZ_PIN, OUTPUT);
  digitalWrite(EMIT_PIN, HIGH);
  timetaken = millis();
  // Initialize the serial port
  Serial.begin(9600);

  // Choose the message to transmit
  currentBinaryMessage = messages[currentMessageIndex].binary;

  // Create the full message with delimiters
  strcpy(fullMessage, delimiter);
  strcat(fullMessage, currentBinaryMessage);
  //strcat(fullMessage, delimiter);
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
    }

    timetaken = millis();
  }
}
