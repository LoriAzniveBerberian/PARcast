SD CARD TEST — Teensy 4.1
==========================
PARcast Project | Status: PASSED (04/01/2026)

WHAT THIS TEST DOES:
--------------------
Initializes the microSD card and writes a small test CSV file called
test.csv to confirm the card is correctly formatted and the Teensy
can read and write to it successfully.

SD CARD REQUIREMENTS:
---------------------
The card MUST be formatted as FAT32 with MBR partition scheme.
See SD_CARD_FORMAT_INSTRUCTIONS.txt for full formatting guide.

Card name: MEMORY_B (land station) — formatted April 2026
Format command used:
  diskutil eraseDisk FAT32 MEMORY_B MBR /dev/disk4

HOW TO RUN:
-----------
  1. Make sure MEMORY_B card is inserted in the Teensy slot (underside)
     Push firmly until you feel a click
  2. Open Arduino IDE
  3. Go to File > Open
  4. Navigate to:
     arduino_ide_sketches/PAR_Profiler/Land_Station/tests/sd_card_test/sd_card_test.ino
  5. Click Open
  6. Make sure Tools > Board is set to: Teensy 4.1
  7. Make sure Tools > Port shows your Teensy (listed under "teensy ports")
  8. Click Upload
  9. Open Serial Monitor (Tools > Serial Monitor or Cmd+Shift+M)

WHAT TO EXPECT IN SERIAL MONITOR:
----------------------------------
  SD Card Write Test
  SD initialized!
  File written successfully!
  Eject card and check for test.csv on your Mac!

VERIFY THE FILE WAS WRITTEN:
-----------------------------
  1. Eject card from Teensy
  2. Insert into Mac
  3. Open Terminal and run:
       cat /Volumes/MEMORY_B/test.csv
  4. You should see:
       timestamp,message
       2026-04-01,SD card is working!
       2026-04-01,PARcast land station ready!

TROUBLESHOOTING:
----------------
If SD initialization fails:
  1. Card may not be seated — push firmly until it clicks
  2. Card may not be formatted correctly — see SD_CARD_FORMAT_INSTRUCTIONS.txt
  3. Run diskutil info /dev/disk4s1 to verify FAT32 + MBR format

RESULT (04/01/2026):
--------------------
PASSED — test.csv written and verified on Mac.
Key fix: card must use MBR partition scheme, not GUID.
Command: diskutil eraseDisk FAT32 MEMORY_B MBR /dev/disk4
