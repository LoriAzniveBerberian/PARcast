I2C SCANNER — Teensy 4.1
=========================
PARcast Project | Status: PASSED (03/19/2026)

WHAT THIS TEST DOES:
--------------------
Scans the I2C bus and prints the address of every device it finds.
Run this any time you rewire components or swap the Teensy to confirm
all devices are still connected and communicating correctly.

EXPECTED DEVICES:
-----------------
  0x48 = ADS1115 ADC        (always present on land station)
  0x68 = DS3231 RTC         (always present on land station)
  0x76 = MS5837 Pressure    (underwater instrument only)

HOW TO RUN:
-----------
  1. Open Arduino IDE
  2. Go to File > Open
  3. Navigate to:
     arduino_ide_sketches/PAR_Profiler/Land_Station/tests/i2c_scanner/i2c_scanner.ino
  4. Click Open
  5. Make sure Tools > Board is set to: Teensy 4.1
  6. Make sure Tools > Port shows your Teensy (listed under "teensy ports")
  7. Click Upload
  8. Open Serial Monitor (Tools > Serial Monitor or Cmd+Shift+M)
  9. Wait 5 seconds for the first scan to appear

WHAT TO EXPECT:
---------------
  I2C Scanner — PARcast
  Scanning...
  Found device at 0x48  --> ADS1115 ADC
  Found device at 0x68  --> DS3231 RTC
  Scan complete. 2 device(s) found.
  ---------------------------

Repeats every 3 seconds. If a device is missing check its wiring.

RESULT (03/19/2026):
--------------------
PASSED — ADS1115 at 0x48 and DS3231 at 0x68 both found.
