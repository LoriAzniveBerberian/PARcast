// PARcast RTC Set Tool (Interactive)
// Upload to Teensy 4.1. Open Serial Monitor at 9600 baud with line ending = "Newline".
// Follow the prompts to set the DS3231 to any date and time you want.
// After the time is confirmed, the sketch continues printing the current time every
// second so you can verify the RTC is keeping time. Upload a different sketch when done.

#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

// Reads a line of input from the user via Serial Monitor.
// Returns when the user presses Enter (sends a newline).
String readSerialLine() {
  while (Serial.available() == 0) {
    delay(50);  // wait for user input
  }
  String input = Serial.readStringUntil('\n');
  input.trim();  // remove any stray whitespace or carriage returns
  return input;
}

// Asks a question, waits for input, and returns the user's response as an integer.
int askForNumber(const char* prompt) {
  Serial.print(prompt);
  String response = readSerialLine();
  Serial.println(response);  // echo back so they can see what they typed
  return response.toInt();
}

void setup() {
  Serial.begin(9600);
  while (!Serial) { delay(10); }
  delay(500);

  Serial.println();
  Serial.println("=== PARcast RTC Set Tool ===");
  Serial.println();

  // Connect to the DS3231
  if (!rtc.begin()) {
    Serial.println("ERROR: DS3231 not found! Check I2C wiring (SDA, SCL, VDD, GND).");
    while (1) { delay(1000); }
  }
  Serial.println("DS3231 connected.");

  // Show what the RTC currently thinks the time is
  DateTime current = rtc.now();
  Serial.print("Current RTC time: ");
  Serial.print(current.year());      Serial.print("-");
  if (current.month()  < 10) Serial.print("0"); Serial.print(current.month());  Serial.print("-");
  if (current.day()    < 10) Serial.print("0"); Serial.print(current.day());    Serial.print(" ");
  if (current.hour()   < 10) Serial.print("0"); Serial.print(current.hour());   Serial.print(":");
  if (current.minute() < 10) Serial.print("0"); Serial.print(current.minute()); Serial.print(":");
  if (current.second() < 10) Serial.print("0"); Serial.println(current.second());
  Serial.println();

  Serial.println("Make sure Serial Monitor's line ending is set to 'Newline' (bottom right).");
  Serial.println("Enter the new date and time. Use 24-hour clock for hours (0-23).");
  Serial.println();

  // Gather each field from the user
  int year   = askForNumber("Year (e.g. 2026): ");
  int month  = askForNumber("Month (1-12): ");
  int day    = askForNumber("Day (1-31): ");
  int hour   = askForNumber("Hour (0-23): ");
  int minute = askForNumber("Minute (0-59): ");
  int second = askForNumber("Second (0-59): ");

  // Basic sanity check
  if (year < 2024 || year > 2099 ||
      month < 1 || month > 12 ||
      day < 1 || day > 31 ||
      hour < 0 || hour > 23 ||
      minute < 0 || minute > 59 ||
      second < 0 || second > 59) {
    Serial.println();
    Serial.println("ERROR: One of those values looked wrong. Press the Teensy reset button");
    Serial.println("(or re-upload) to try again. RTC was NOT changed.");
    while (1) { delay(1000); }
  }

  // Set the RTC
  rtc.adjust(DateTime(year, month, day, hour, minute, second));

  Serial.println();
  Serial.println("RTC time set!");
  Serial.println();
  Serial.println("Verifying — printing current RTC time every second.");
  Serial.println("When done, upload a different sketch so this one doesn't run again.");
  Serial.println();
}

void loop() {
  DateTime now = rtc.now();
  Serial.print(now.year());      Serial.print("-");
  if (now.month()  < 10) Serial.print("0"); Serial.print(now.month());  Serial.print("-");
  if (now.day()    < 10) Serial.print("0"); Serial.print(now.day());    Serial.print(" ");
  if (now.hour()   < 10) Serial.print("0"); Serial.print(now.hour());   Serial.print(":");
  if (now.minute() < 10) Serial.print("0"); Serial.print(now.minute()); Serial.print(":");
  if (now.second() < 10) Serial.print("0"); Serial.println(now.second());
  delay(1000);
}