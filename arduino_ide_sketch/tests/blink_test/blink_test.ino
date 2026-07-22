// Blink Test — Teensy 4.1
// PARcast Project
// Status: PASSED 03/19/2026
//
// Built-in Arduino example.
// Also accessible via File > Examples > 01.Basics > Blink
//
// Turns the onboard LED on for one second, then off for one second.
// Confirms solder joints and USB communication are working.

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}