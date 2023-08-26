#include <Arduino.h>

// For Arduino UNO
// #define BLINK_LED 13

// For ESP8266
#define BLINK_LED D4

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(BLINK_LED, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(BLINK_LED, HIGH);
  Serial.println("LED is ON");
  delay(1000);
  digitalWrite(BLINK_LED, LOW);
  Serial.println("LED is OFF");
  delay(1000);
}