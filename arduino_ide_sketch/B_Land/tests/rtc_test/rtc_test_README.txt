RTC TEST — DS3231 — Teensy 4.1
================================
PARcast Project | Status: PASSED (03/19/2026, 04/01/2026)

WHAT THIS TEST DOES:
--------------------
Reads the DS3231 Real Time Clock and prints the date, time, and
chip temperature to the Serial Monitor every 3 seconds.
Confirms the RTC is wired correctly and the coin cell battery
is keeping time accurately.

WHY WE TEST THE RTC:
--------------------
The DS3231 gives every data row a timestamp. Without it there is
no way to know when each PAR reading was taken or to synchronize
the land station and underwater instrument in post-processing.
The CR1220 coin cell keeps the clock running between deployments.

HOW TO RUN:
-----------
Option A — Use your locally saved sketch (recommended):
  1. Open Arduino IDE
  2. Go to File > Open
  3. Navigate to:
     arduino_ide_sketches/PAR_Profiler/Land_Station/tests/rtc_test/rtc_test.ino
  4. Click Open
     NOTE: This local version is modified for Teensy compatibility.
     It removes the Serial hang line that causes issues on Teensy.

Option B — Use the RTClib library example:
  1. Open Arduino IDE
  2. Go to File > Examples > RTClib > ds3231
     NOTE: This version may hang on Teensy — use Option A if it does.

Then:
  3. Make sure Tools > Board is set to: Teensy 4.1
  4. Make sure Tools > Port shows your Teensy (listed under "teensy ports")
  5. Click Upload
  6. Open Serial Monitor (Tools > Serial Monitor or Cmd+Shift+M)

WHAT TO EXPECT:
---------------
Date, time, and temperature print every 3 seconds. Example:
  2026/4/1 (Wednesday) 16:29:12
  Temperature: 23.00 C

IMPORTANT NOTES:
----------------
- First upload sets the clock to compile time automatically.
  After that the CR1220 battery keeps it running when unpowered.
- If the time is wrong, re-upload this sketch to reset it.
- I2C address: 0x68 — connected to Teensy Pin 18 (SDA) and Pin 19 (SCL)
- Replace coin cell with CR1220 (+ side up) if clock loses time.

RECHECK THE RTC WHEN:
---------------------
- You swap out the Teensy for a different one
- The coin cell battery dies
- The time appears wrong in your CSV data
- After long storage without power

RESULTS:
--------
PASSED (03/19/2026) — 2026/3/19 (Thursday) 15:40:56 | Temp: 22.50 C
PASSED (04/01/2026) — 2026/4/1 (Wednesday) 16:29:12 | Temp: 23.00 C
