#include <Pololu3piPlus32U4.h>

bool emittion = HIGH;
// Define the pin used to activate the IR LEDs
#define EMIT_PIN 11
#define BUZZ_PIN 6
unsigned long timetaken;

void setup() {
  // Configure the EMIT pin as output and set it to high
  pinMode(EMIT_PIN, OUTPUT);
  pinMode(BUZZ_PIN,OUTPUT);
  digitalWrite(EMIT_PIN, HIGH);
  timetaken=millis();
  // Initialize the serial port
  Serial.begin(9600);
}

void loop() {
  // Continuously emit IR light
  while(Serial.available()){
    
  }

  String strval = Serial.readString();
  for(int i=0;i<strval.length();){
    digitalWrite(EMIT_PIN, emittion);
    
    if(millis()-timetaken>1000){
      emittion = (strval.charAt(i)=='0')?0:1;
      timetaken=millis();
      if(emittion){
        analogWrite(BUZZ_PIN,120);
      }
      else{
        analogWrite(BUZZ_PIN,0);
      }
      i++;
    }
  }
  digitalWrite(EMIT_PIN, 0);

  Serial.println(emittion);
}
