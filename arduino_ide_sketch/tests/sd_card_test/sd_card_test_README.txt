SD CARD TEST — Teensy 4.1
==========================
PARcast Project | Status: PASSED (04/01/2026)

WHAT THIS TEST DOES:
--------------------
Initializes the microSD card and writes a small test CSV file called
test.csv to confirm the card is correctly formatted and the Teensy
can read and write to it successfully.

This test applies to both PARcast units. Each unit has its own
microSD card with a unique volume label so they can be distinguished
on a host computer:

  E_s(PAR) reference station   ->  PARCAST_Es
  E_d(z, PAR) profiler         ->  PARCAST_Ed

(FAT32 displays volume labels in uppercase, so the cards will mount
as PARCAST_ES and PARCAST_ED on a Mac.)

SD CARD REQUIREMENTS:
---------------------
The card MUST be formatted as FAT32 with MBR partition scheme.
See SD_CARD_FORMAT_INSTRUCTIONS.txt for full formatting guide.

Format command (replace /dev/diskN with your card's actual disk
identifier from `diskutil list`):

  E_s(PAR) reference card:
    diskutil eraseDisk FAT32 PARCAST_Es MBR /dev/diskN

  E_d(z, PAR) profiler card:
    diskutil eraseDisk FAT32 PARCAST_Ed MBR /dev/diskN

WARNING: Double-check the disk identifier before running eraseDisk.
Targeting the wrong disk will erase your hard drive.

HOW TO RUN:
-----------
  1. Insert the appropriate card (PARCAST_Es or PARCAST_Ed) into the
     Teensy microSD slot on the underside of the board.
     Push firmly until you feel a click.
  2. Open Arduino IDE
  3. Go to File > Open
  4. Navigate to:
     arduino_ide_sketch/tests/sd_card_test/sd_card_test.ino
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
  3. Open Terminal and run (replace card name as appropriate):
       cat /Volumes/PARCAST_ES/test.csv
     or
       cat /Volumes/PARCAST_ED/test.csv
  4. You should see:
       timestamp,message
       2026-04-01,SD card is working!
       2026-04-01,PARcast unit ready!

TROUBLESHOOTING:
----------------
If SD initialization fails:
  1. Card may not be seated — push firmly until it clicks
  2. Card may not be formatted correctly — see SD_CARD_FORMAT_INSTRUCTIONS.txt
  3. Run `diskutil info /dev/diskNs1` to verify FAT32 + MBR format

RESULT (04/01/2026):
--------------------
PASSED — test.csv written and verified on Mac.
Key fix: card must use MBR partition scheme, not GUID.
Command used (E_s(PAR) reference card during initial test):
  diskutil eraseDisk FAT32 PARCAST_Es MBR /dev/disk4
