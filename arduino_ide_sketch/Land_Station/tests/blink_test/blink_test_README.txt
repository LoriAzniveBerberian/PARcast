BLINK TEST — Teensy 4.1
=======================
PARcast Project | Status: PASSED (03/19/2026)

WHAT THIS TEST DOES:
--------------------
Blinks the onboard LED on and off every second.
Confirms the Teensy is working, solder joints are good,
and USB communication with your Mac is working.

HOW TO RUN:
-----------
Option A — Use your locally saved sketch (recommended):
  1. Open Arduino IDE
  2. Go to File > Open
  3. Navigate to:
     arduino_ide_sketches/PAR_Profiler/Land_Station/tests/blink_test/blink_test.ino
  4. Click Open

Option B — Use the built-in Arduino example:
  1. Open Arduino IDE
  2. Go to File > Examples > 01.Basics > Blink

Then:
  3. Make sure Tools > Board is set to: Teensy 4.1
  4. Make sure Tools > Port shows your Teensy (listed under "teensy ports")
  5. Click Upload (right arrow button)

WHAT TO EXPECT:
---------------
The orange LED on the Teensy blinks on and off every 1 second.
No Serial Monitor needed for this test.

RESULT (03/19/2026):
--------------------
PASSED — LED blinked confirming solder joints and USB communication good.
