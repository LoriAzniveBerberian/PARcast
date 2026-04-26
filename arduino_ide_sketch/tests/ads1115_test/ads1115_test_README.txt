ADS1115 ADC TEST — Teensy 4.1
==============================
PARcast Project | Status: PASSED (03/20/2026)

WHAT THIS TEST DOES:
--------------------
Reads all 4 channels of the ADS1115 ADC and prints voltage values
to the Serial Monitor every second. Confirms the ADS1115 is wired
correctly and communicating with the Teensy over I2C.

This test applies to both the E_s(PAR) reference station and the
E_d(z, PAR) profiler — both units use the same ADS1115 ADC.

WHY WE CHANGE THE GAIN:
------------------------
The ADS1115 default gain covers 0-6V but the PAR sensor only outputs
0-40 mV. At default gain the PAR signal is too small to resolve.
GAIN_SIXTEEN zooms the measurement window to 0-256mV giving ~5000
steps of resolution instead of ~200. Always use GAIN_SIXTEEN.

HOW TO RUN:
-----------
Option A — Use your locally saved sketch (recommended):
  1. Open Arduino IDE
  2. Go to File > Open
  3. Navigate to:
     arduino_ide_sketch/tests/ads1115_test/ads1115_test.ino
  4. Click Open

Option B — Use the Adafruit library example:
  1. Open Arduino IDE
  2. Go to File > Examples > Adafruit ADS1X15 > singleended
  3. IMPORTANT — change this line:
       Adafruit_ADS1015 ads;
     to:
       Adafruit_ADS1115 ads;
  4. IMPORTANT — change this line:
       ads.setGain(GAIN_TWOTHIRDS);
     to:
       ads.setGain(GAIN_SIXTEEN);

Then:
  5. Make sure Tools > Board is set to: Teensy 4.1
  6. Make sure Tools > Port shows your Teensy (listed under "teensy ports")
  7. Click Upload
  8. Open Serial Monitor (Tools > Serial Monitor or Cmd+Shift+M)

WHAT TO EXPECT:
---------------
All 4 channels print every second. Example output:
  AIN0: 3132  0.59V
  AIN1: 3110  0.58V
  AIN2: 3130  0.59V
  AIN3: 3096  0.58V

RESULT (03/20/2026):
--------------------
PASSED — All 4 channels printing correctly with GAIN_SIXTEEN.
