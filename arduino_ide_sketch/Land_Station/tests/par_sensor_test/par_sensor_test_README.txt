PAR SENSOR TEST — SQ-500-SS — Teensy 4.1
==========================================
PARcast Project | Status: PASSED (03/20/2026)

WHAT THIS TEST DOES:
--------------------
Reads the SQ-500-SS PAR sensor through the ADS1115 ADC and prints
raw counts, millivolts, and PPFD to the Serial Monitor every second.
No SD card or RTC needed — PAR sensor only.

WIRING REMINDER:
----------------
  PAR sensor white wire  → ADS1115 A0
  PAR sensor black wire  → ADS1115 A1
  PAR sensor clear wire  → GND rail (one end only)
  DO NOT connect PAR sensor to power — it is self-powered by light!

HOW TO RUN:
-----------
  1. Open Arduino IDE
  2. Go to File > Open
  3. Navigate to:
     arduino_ide_sketches/PAR_Profiler/Land_Station/tests/par_sensor_test/par_sensor_test.ino
  4. Click Open
  5. Make sure Tools > Board is set to: Teensy 4.1
  6. Make sure Tools > Port shows your Teensy (listed under "teensy ports")
  7. Click Upload
  8. Open Serial Monitor (Tools > Serial Monitor or Cmd+Shift+M)

WHAT TO EXPECT:
---------------
  PAR Sensor Test — SQ-500-SS
  ADS1115 initialized with GAIN_SIXTEEN
  -----------------------------------------------------------
  Raw counts:   797
  Millivolts:   6.2266 mV
  PPFD:         622.66 umol/m2/s

Cover sensor with cap — PPFD should drop near zero.
Point at bright light outdoors — PPFD should read 600-900+ umol/m2/s.

CALIBRATION:
------------
  millivolts = raw_counts x 0.0078125
  PPFD (umol/m2/s) = millivolts x 100
  (Apogee SQ-500-SS calibration factor)

RESULT (03/20/2026):
--------------------
PASSED — Cap on: ~3 umol/m2/s | Outdoors: 614-622 umol/m2/s
