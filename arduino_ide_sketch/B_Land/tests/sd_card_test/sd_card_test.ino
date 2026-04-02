#include <SD.h>

void setup() {
  Serial.begin(9600);
  delay(3000);
  Serial.println("SD Card Write Test");

  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("SD initialization failed!");
    while (1);
  }
  Serial.println("SD initialized!");

  File dataFile = SD.open("test.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.println("timestamp,message");
    dataFile.println("2026-04-01,SD card is working!");
    dataFile.println("2026-04-01,PARcast land station ready!");
    dataFile.close();
    Serial.println("File written successfully!");
    Serial.println("Eject card and check for test.csv on your Mac!");
  } else {
    Serial.println("Error opening file!");
  }
}

void loop() {}