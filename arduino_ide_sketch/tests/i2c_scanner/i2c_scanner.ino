// I2C Scanner — PARcast
// Scans I2C bus and prints address of every device found
// Upload to Teensy 4.1. Open Serial Monitor at 9600 baud.
// Expected devices for PARcast:
//   0x48 = ADS1115 ADC
//   0x68 = DS3231 RTC
//   0x76 = MS5837 Pressure Sensor (E_d(z, PAR) profiler only)

#include <Wire.h>

void setup() {
  Serial.begin(9600);
  delay(5000);
  Wire.begin();
}

void loop() {
  int devicesFound = 0;
  
  Serial.println("I2C Scanner — PARcast");
  Serial.println("Scanning...");
  
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found device at 0x");
      Serial.print(addr, HEX);
      if (addr == 0x48) Serial.print("  --> ADS1115 ADC");
      if (addr == 0x68) Serial.print("  --> DS3231 RTC");
      if (addr == 0x76) Serial.print("  --> MS5837 Pressure Sensor");
      Serial.println();
      devicesFound++;
    }
  }
  
  Serial.print("Scan complete. ");
  Serial.print(devicesFound);
  Serial.println(" device(s) found.");
  Serial.println("---------------------------");
  delay(3000);
}