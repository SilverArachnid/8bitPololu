#define EMIT_PIN 11
#define BUZZ_PIN 6

// Define a list of messages and their binary encodings
struct Message {
  String text;
  String binary;
};

Message messages[] = {
  {"hello", "010010010010"},
  {"bye", "000101010100"},
  {"yes", "001010101010"},
  {"no", "010001101100"},
  {"ok", "011000101000"}
};

int selectedMessageIndex = 0; // Index of the message to be transmitted

const unsigned long bitDuration = 100; // Duration of each bit in milliseconds

void setup() {
  pinMode(EMIT_PIN, OUTPUT);
  pinMode(BUZZ_PIN, OUTPUT);
  Serial.begin(9600);
  
  // Print available messages
  Serial.println("Available messages:");
  for (int i = 0; i < sizeof(messages)/sizeof(messages[0]); i++) {
    Serial.print(i);
    Serial.print(": ");
    Serial.println(messages[i].text);
  }

  // Select a message to transmit
  Serial.println("Enter the index of the message to transmit:");
  while (Serial.available() == 0) {} // Wait for input
  selectedMessageIndex = Serial.parseInt();
  Serial.print("Selected message: ");
  Serial.println(messages[selectedMessageIndex].text);
}

void loop() {
  // Construct the message with start and stop markers
  String message = "1111" + messages[selectedMessageIndex].binary + "0000";

  // Transmit the message
  for (int i = 0; i < message.length(); i++) {
    digitalWrite(EMIT_PIN, message[i] == '1' ? HIGH : LOW);
    analogWrite(BUZZ_PIN, message[i] == '1' ? 120 : 0);
    delay(bitDuration); // Duration for each bit
  }

  // Short delay before retransmitting the message
  delay(1000); // Adjust as needed to control the frequency of message repetition
}
