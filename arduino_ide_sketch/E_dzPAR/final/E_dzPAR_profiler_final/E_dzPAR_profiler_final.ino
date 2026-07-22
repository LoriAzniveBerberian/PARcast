/*
 * E_dzPAR — PARcast underwater profiler data logger
 * File: E_dzPAR.ino
 *
 * Measures downwelling PAR irradiance E_d(z, PAR) through the water column,
 * along with orientation, depth, and supporting sensor data.
 *
 * WHAT THIS DOES:
 *   Reads all 5 sensors and writes a CSV row to microSD on every sample.
 *   Full 21-column CSV format — every raw measurement is preserved so the
 *   data can be re-analyzed later (e.g. re-running sensor fusion with
 *   different parameters). Serial Monitor shows a curated subset for live
 *   monitoring. See PARcast_Data_Reference.docx for column definitions.
 *
 * SENSORS:
 *   - DS3231 RTC                  — timestamp for each row
 *   - SQ-500 PAR sensor + ADS1115 — PAR (mV + PPFD)
 *   - MS5837 (Bar30)              — depth, pressure, water temperature
 *   - LSM6DSOX                    — accelerometer + gyroscope (raw)
 *   - LIS3MDL                     — magnetometer (calibrated)
 *   - Madgwick filter             — roll / pitch / yaw (derived)
 *
 * OUTPUT:
 *   CSV file: /E_dzPAR_YYYYMMDD_NNNN.CSV on the Teensy's microSD slot.
 *   NNNN is an auto-incrementing deployment counter for the day, starting
 *   at 0001. Matches the E_s(PAR) reference station's naming convention so
 *   surface and profiler files can be paired by date + deploy number.
 *   A new file is created each Teensy boot.
 *
 * BEFORE RUNNING:
 *   1. Set DS3231 time first using PARcast_RTC_set (interactive sketch).
 *   2. Run IMU_calibration and paste mag offsets into the section below.
 *   3. Verify SQ500_CAL_FACTOR matches your sensor's calibration certificate.
 *   4. Insert a FAT32-formatted microSD card into the Teensy's built-in slot.
 *
 * NOTE: This sketch is for bench/dry-land testing only. PAR values are NOT
 * immersion-corrected. For underwater deployments, switch to the deployment
 * sketch (PARcast_logger_deploy.ino) which applies Apogee's 1.32 immersion
 * factor for the SQ-500 series.
 *
 * REQUIRED LIBRARIES:
 *   - Adafruit LSM6DSOX
 *   - Adafruit LIS3MDL
 *   - Adafruit ADS1X15
 *   - Adafruit Unified Sensor
 *   - Adafruit BusIO
 *   - RTClib (by Adafruit)
 *   - MS5837 (BlueRobotics: github.com/bluerobotics/BlueRobotics_MS5837_Library)
 *   - Madgwick (by Arduino)
 *   - SD (built-in)
 *
 * GitHub: github.com/LoriAzniveBerberian/PARcast
 */

#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_LSM6DSOX.h>
#include <Adafruit_LIS3MDL.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_Sensor.h>
#include <RTClib.h>
#include "MS5837.h"
#include <MadgwickAHRS.h>

// ============================================================================
// CONFIGURATION — EDIT THESE TO MATCH YOUR HARDWARE / DEPLOYMENT
// ============================================================================

// Sample rate. ADS1115 limits this to ~8 Hz max in differential mode
// at the current 128 SPS data rate. For higher rates, also bump
// ads.setDataRate() in setup().
const float    SAMPLE_RATE_HZ     = 8.0;
const uint32_t SAMPLE_INTERVAL_MS = (uint32_t)(1000.0 / SAMPLE_RATE_HZ);

// IMU fusion filter rate (should be higher than sample rate for stable angles)
const float    FILTER_RATE_HZ     = 50.0;
const uint32_t FILTER_INTERVAL_US = (uint32_t)(1000000.0 / FILTER_RATE_HZ);

// SQ-500 PAR sensor calibration constant.
// Apogee provides this on the calibration cert that came with your sensor.
// Units: µmol photons / m² / s  per  mV
const float SQ500_CAL_FACTOR = 100.0;

// Water type for Bar30 depth calculation.
// 997 kg/m³ = freshwater, 1029 kg/m³ = seawater.
// On a bench in air, this value barely matters — depth will read ~0 anyway.
const float WATER_DENSITY = 1029.0;

// Magnetometer calibration (paste from IMU_calibration.ino output)
const float MAG_OFFSET_X = 0.0;
const float MAG_OFFSET_Y = 0.0;
const float MAG_OFFSET_Z = 0.0;
const float MAG_SCALE_X  = 1.0;
const float MAG_SCALE_Y  = 1.0;
const float MAG_SCALE_Z  = 1.0;

// SD card chip-select pin (Teensy 4.1 built-in slot is BUILTIN_SDCARD)
const int SD_CS_PIN = BUILTIN_SDCARD;

// ============================================================================
// END CONFIGURATION
// ============================================================================

Adafruit_LSM6DSOX lsm6ds;
Adafruit_LIS3MDL  lis3mdl;
Adafruit_ADS1115  ads;
RTC_DS3231        rtc;
MS5837            bar30;
Madgwick          filter;
File              dataFile;

uint32_t lastSample       = 0;
uint32_t lastFilterUpdate = 0;
uint32_t sampleCount      = 0;
char     filename[40];

// Sub-second timestamping. The DS3231 only resolves to whole seconds, so we
// anchor the free-running millis() clock to the RTC: each time the RTC second
// ticks over we re-capture millis(), and a row's milliseconds = millis() minus
// that anchor. Re-anchoring every second keeps millis()-drift from accumulating
// over a long deployment.
uint32_t rtcAnchorUnix   = 0;   // RTC unixtime at the last observed second rollover
uint32_t rtcAnchorMillis = 0;   // millis() captured at that rollover

const int LED_PIN = LED_BUILTIN;

void blinkError(int count) {
  for (int i = 0; i < count; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
  delay(1000);
}

void haltWithError(const char* msg, int blinkCount) {
  Serial.println(msg);
  while (1) {
    blinkError(blinkCount);
  }
}

// Helper: print a float right-padded to a fixed column width
void printPadded(float value, int decimals, int width) {
  char buf[16];
  dtostrf(value, width, decimals, buf);
  Serial.print(buf);
  Serial.print(F("  "));
}

void setup() {
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(115200);
  while (!Serial && millis() < 3000) { ; }

  Serial.println();
  Serial.println(F("================================================"));
  Serial.println(F("E_dzPAR — PARcast Data Logger"));
  Serial.println(F("================================================"));

  Wire.begin();
  Wire.setClock(400000);

  // ---- RTC ----
  Serial.print(F("DS3231 RTC... "));
  if (!rtc.begin()) {
    haltWithError("FAILED", 2);
  }
  if (rtc.lostPower()) {
    Serial.println(F("WARNING — RTC lost power, time may be incorrect."));
    Serial.println(F("         Upload PARcast_RTC_set to fix this."));
  }
  DateTime now = rtc.now();
  Serial.print(F("OK ("));
  Serial.print(now.year()); Serial.print('-');
  if (now.month()  < 10) Serial.print('0'); Serial.print(now.month());  Serial.print('-');
  if (now.day()    < 10) Serial.print('0'); Serial.print(now.day());    Serial.print(' ');
  if (now.hour()   < 10) Serial.print('0'); Serial.print(now.hour());   Serial.print(':');
  if (now.minute() < 10) Serial.print('0'); Serial.print(now.minute()); Serial.print(':');
  if (now.second() < 10) Serial.print('0'); Serial.print(now.second());
  Serial.println(F(")"));

  // ---- ADS1115 (PAR via SQ-500) ----
  Serial.print(F("ADS1115... "));
  if (!ads.begin()) {
    haltWithError("FAILED", 3);
  }
  // SQ-500 max output is ~40 mV, so use the most sensitive gain (PGA 16x)
  ads.setGain(GAIN_SIXTEEN);              // ±0.256 V full scale, ~7.8 µV per LSB
  ads.setDataRate(RATE_ADS1115_128SPS);
  Serial.println(F("OK"));

  // ---- LSM6DSOX (accel + gyro) ----
  Serial.print(F("LSM6DSOX... "));
  if (!lsm6ds.begin_I2C()) {
    haltWithError("FAILED", 4);
  }
  lsm6ds.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);
  lsm6ds.setGyroRange(LSM6DS_GYRO_RANGE_500_DPS);
  lsm6ds.setAccelDataRate(LSM6DS_RATE_104_HZ);
  lsm6ds.setGyroDataRate(LSM6DS_RATE_104_HZ);
  Serial.println(F("OK"));

  // ---- LIS3MDL (magnetometer) ----
  Serial.print(F("LIS3MDL... "));
  if (!lis3mdl.begin_I2C()) {
    haltWithError("FAILED", 5);
  }
  lis3mdl.setPerformanceMode(LIS3MDL_HIGHMODE);
  lis3mdl.setOperationMode(LIS3MDL_CONTINUOUSMODE);
  lis3mdl.setDataRate(LIS3MDL_DATARATE_155_HZ);
  lis3mdl.setRange(LIS3MDL_RANGE_4_GAUSS);
  Serial.println(F("OK"));

  // ---- MS5837 (Bar30) ----
  Serial.print(F("MS5837 Bar30... "));
  if (!bar30.init()) {
    haltWithError("FAILED", 6);
  }
  bar30.setModel(MS5837::MS5837_30BA);
  bar30.setFluidDensity(WATER_DENSITY);
  Serial.println(F("OK"));

  // ---- Madgwick filter ----
  filter.begin(FILTER_RATE_HZ);
  Serial.println(F("Madgwick filter initialized"));

  // ---- microSD ----
  Serial.print(F("microSD... "));
  if (!SD.begin(SD_CS_PIN)) {
    haltWithError("FAILED — card missing or not FAT32?", 7);
  }
  Serial.println(F("OK"));

  // Create new file with auto-incrementing deployment number for today.
  // Format: E_dzPAR_YYYYMMDD_NNNN.CSV (matches E_s(PAR) reference station).
  // Scans the SD card for the next unused NNNN under today's date.
  {
    char dateStr[12];
    snprintf(dateStr, sizeof(dateStr), "%04d%02d%02d",
             now.year(), now.month(), now.day());
    int deployNum = 1;
    while (deployNum <= 9999) {
      snprintf(filename, sizeof(filename), "E_dzPAR_%s_%04d.CSV",
               dateStr, deployNum);
      if (!SD.exists(filename)) break;
      deployNum++;
    }
  }

  dataFile = SD.open(filename, FILE_WRITE);
  if (!dataFile) {
    haltWithError("FAILED to create log file", 8);
  }

  // Write CSV header — 20 columns, full raw data archive
  dataFile.println(F(
    "iso_time,unix_time,par_mV,par_uMol_m2_s,depth_m,pressure_mbar,water_temp_C,"
    "accel_x_ms2,accel_y_ms2,accel_z_ms2,"
    "gyro_x_rads,gyro_y_rads,gyro_z_rads,"
    "mag_x_uT,mag_y_uT,mag_z_uT,"
    "roll_deg,pitch_deg,yaw_deg,"
    "imu_temp_C,millis_boot"
  ));
  dataFile.flush();

  Serial.print(F("Logging to: "));
  Serial.println(filename);
  Serial.println(F("CSV: 21 columns (full raw data). Serial preview: curated subset."));
  Serial.println(F("Bench mode — PAR values are NOT immersion-corrected."));
  Serial.println();

  // Warm up the Madgwick filter for 3 seconds before first sample
  Serial.println(F("Warming up orientation filter (3s)..."));
  uint32_t warmupStart = millis();
  while (millis() - warmupStart < 3000) {
    updateFilter();
  }

  Serial.println(F("Logging started. LED blinks on every sample."));
  Serial.println();
  // Curated Serial header — date/time, PAR, tilt, temp, pressure
  Serial.println(F(
    "iso_time             PPFD    par_mV   roll    pitch   yaw     "
    "wTemp   press     depth"
  ));
  Serial.println(F(
    "------------------------------------------------------------"
    "----------------------------"
  ));

  // Anchor the millisecond clock to the RTC just before the first sample
  {
    DateTime t0 = rtc.now();
    rtcAnchorUnix   = t0.unixtime();
    rtcAnchorMillis = millis();
  }

  lastSample = millis();
}

void updateFilter() {
  uint32_t nowUs = micros();
  if (nowUs - lastFilterUpdate < FILTER_INTERVAL_US) return;
  lastFilterUpdate = nowUs;

  sensors_event_t accel, gyro, mag, temp;
  lsm6ds.getEvent(&accel, &gyro, &temp);
  lis3mdl.getEvent(&mag);

  float mx = (mag.magnetic.x - MAG_OFFSET_X) * MAG_SCALE_X;
  float my = (mag.magnetic.y - MAG_OFFSET_Y) * MAG_SCALE_Y;
  float mz = (mag.magnetic.z - MAG_OFFSET_Z) * MAG_SCALE_Z;

  filter.update(
    gyro.gyro.x * 57.29578,
    gyro.gyro.y * 57.29578,
    gyro.gyro.z * 57.29578,
    accel.acceleration.x / 9.80665,
    accel.acceleration.y / 9.80665,
    accel.acceleration.z / 9.80665,
    mx, my, mz
  );
}

void loop() {
  // Keep filter running at FILTER_RATE_HZ between samples
  updateFilter();

  if (millis() - lastSample < SAMPLE_INTERVAL_MS) return;
  lastSample += SAMPLE_INTERVAL_MS;
  digitalWrite(LED_PIN, HIGH);

  // ---- Read PAR (ADS1115 differential A0-A1) ----
  int16_t adsRaw = ads.readADC_Differential_0_1();
  // GAIN_SIXTEEN: 1 LSB = 0.0078125 mV
  float par_mV   = adsRaw * 0.0078125;
  float par_uMol = par_mV * SQ500_CAL_FACTOR;

  // ---- Read Bar30 ----
  bar30.read();
  float depth_m       = bar30.depth();        // negative when above water
  float pressure_mbar = bar30.pressure();
  float water_temp    = bar30.temperature();

  // ---- Read IMU raw (for logging — filter already running on these) ----
  sensors_event_t accel, gyro, mag, imuTemp;
  lsm6ds.getEvent(&accel, &gyro, &imuTemp);
  lis3mdl.getEvent(&mag);

  float mx_cal = (mag.magnetic.x - MAG_OFFSET_X) * MAG_SCALE_X;
  float my_cal = (mag.magnetic.y - MAG_OFFSET_Y) * MAG_SCALE_Y;
  float mz_cal = (mag.magnetic.z - MAG_OFFSET_Z) * MAG_SCALE_Z;

  // ---- Get fused orientation ----
  float roll  = filter.getRoll();
  float pitch = filter.getPitch();
  float yaw   = filter.getYaw();

  // ---- Read timestamp (millisecond resolution) ----
  DateTime now     = rtc.now();
  uint32_t ms      = millis();
  uint32_t nowUnix = now.unixtime();
  if (nowUnix != rtcAnchorUnix) {   // RTC rolled into a new second -> re-anchor
    rtcAnchorUnix   = nowUnix;
    rtcAnchorMillis = ms;
  }
  uint32_t subSec = ms - rtcAnchorMillis;   // 0..999 within the current RTC second
  if (subSec > 999) subSec = 999;           // clamp (only if a full second is ever missed)
  char isoTime[30];
  snprintf(isoTime, sizeof(isoTime),
           "%04d-%02d-%02dT%02d:%02d:%02d.%03lu",
           now.year(), now.month(), now.day(),
           now.hour(), now.minute(), now.second(),
           (unsigned long)subSec);

  // ---- Write CSV row (21 columns — full raw data archive) ----
  dataFile.print(isoTime);          dataFile.print(',');
  dataFile.print(now.unixtime());   dataFile.print(',');
  dataFile.print(par_mV, 4);        dataFile.print(',');
  dataFile.print(par_uMol, 2);      dataFile.print(',');
  dataFile.print(depth_m, 3);       dataFile.print(',');
  dataFile.print(pressure_mbar, 2); dataFile.print(',');
  dataFile.print(water_temp, 2);    dataFile.print(',');
  dataFile.print(accel.acceleration.x, 3); dataFile.print(',');
  dataFile.print(accel.acceleration.y, 3); dataFile.print(',');
  dataFile.print(accel.acceleration.z, 3); dataFile.print(',');
  dataFile.print(gyro.gyro.x, 4);   dataFile.print(',');
  dataFile.print(gyro.gyro.y, 4);   dataFile.print(',');
  dataFile.print(gyro.gyro.z, 4);   dataFile.print(',');
  dataFile.print(mx_cal, 2);        dataFile.print(',');
  dataFile.print(my_cal, 2);        dataFile.print(',');
  dataFile.print(mz_cal, 2);        dataFile.print(',');
  dataFile.print(roll, 2);          dataFile.print(',');
  dataFile.print(pitch, 2);         dataFile.print(',');
  dataFile.print(yaw, 2);           dataFile.print(',');
  dataFile.print(imuTemp.temperature, 2); dataFile.print(',');
  dataFile.println((unsigned long)ms);   // millis_boot — raw clock for exact sample spacing

  // Flush every 16 samples (~2 seconds at 8 Hz) to balance throughput vs SD wear
  sampleCount++;
  if (sampleCount % 16 == 0) {
    dataFile.flush();
  }

  // ---- Curated Serial preview — what you actually want to read live ----
  // Columns: iso_time | PPFD | par_mV | roll | pitch | yaw | wTemp | press | depth
  // Print every 8 samples (~1 Hz at 8 Hz sampling) to keep Serial readable
  if (sampleCount % 8 == 0) {
    Serial.print(isoTime);            Serial.print(F("  "));
    printPadded(par_uMol,      2, 6);
    printPadded(par_mV,        4, 7);
    printPadded(roll,          2, 7);
    printPadded(pitch,         2, 7);
    printPadded(yaw,           2, 7);
    printPadded(water_temp,    2, 6);
    printPadded(pressure_mbar, 2, 8);
    Serial.println(depth_m, 3);
  }

  digitalWrite(LED_PIN, LOW);
}
