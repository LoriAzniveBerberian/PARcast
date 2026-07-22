// Bar30 Pressure/Temperature Test
// Upload to Teensy 4.1. Open Serial Monitor at 115200 baud.
// Confirms the Bar30 is wired correctly and reading reasonable values.

#include <Wire.h>
#include "MS5837.h"

MS5837 sensor;

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }  // wait for Serial Monitor

  Serial.println("Bar30 Test Starting...");

  Wire.begin();

  // Initialize the sensor. If this fails, check wiring (VDD, GND, SDA, SCL).
  while (!sensor.init()) {
    Serial.println("Bar30 init failed! Check SDA/SCL wiring and that VDD is on the 3.3V rail.");
    Serial.println("Retrying in 2 seconds...");
    delay(2000);
  }

  // Tell the library which chip is inside the Bar30
  sensor.setModel(MS5837::MS5837_30BA);

  // Set fluid density. Use 997 for fresh water, 1029 for seawater.
  // For bench testing in air this value doesn't matter much — depth will just be ~0.
  sensor.setFluidDensity(1029);

  Serial.println("Bar30 initialized successfully.");
  Serial.println("Reading pressure, temperature, and depth every second:");
  Serial.println();
}

void loop() {
  sensor.read();  // read all values from the sensor

  Serial.print("Pressure:    ");
  Serial.print(sensor.pressure());
  Serial.println(" mbar");

  Serial.print("Temperature: ");
  Serial.print(sensor.temperature());
  Serial.println(" deg C");

  Serial.print("Depth:       ");
  Serial.print(sensor.depth());
  Serial.println(" m");

  Serial.print("Altitude:    ");
  Serial.print(sensor.altitude());
  Serial.println(" m (above mean sea level, in air only)");

  Serial.println();
  delay(1000);
}