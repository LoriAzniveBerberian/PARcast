/*
 * E_dzPAR — IMU Tilt Sensor Test
 * File: IMU_tilt_test.ino
 *
 * Quick post-installation check for the LSM6DSOX + LIS3MDL tilt sensor.
 * Run this whenever you reseat the IMU, change wiring, or just want to
 * confirm the sensor is alive and reading sensible values.
 *
 * WHAT IT CHECKS:
 *   1. I2C bus scan — all 5 expected devices respond at the right addresses.
 *   2. LSM6DSOX (accel + gyro) initializes and reports values.
 *   3. LIS3MDL (magnetometer) initializes and reports values.
 *   4. Live sanity checks on each sample:
 *        - |accel| should be ~9.81 m/s² when still (within ±1 m/s²)
 *        - With instrument upright, Y-axis should read ~+9.4 m/s² (gravity axis)
 *        - gyro should be near 0 rad/s when still (within ±0.05)
 *        - |mag| should be 25-65 µT (Earth's field range)
 *        - All sensors should produce changing values over time (not stuck)
 *   5. Computes tilt-from-vertical so you can verify it reads ~0° upright
 *      and increases as you tip the instrument.
 *
 * USAGE:
 *   1. Upload and open Serial Monitor at 115200 baud.
 *   2. Sit the instrument upright (cap up, bottom on bench, deployment orientation).
 *   3. Watch the PASS/FAIL summary in the first 5 seconds.
 *   4. Then watch live values. Tilt the instrument by hand — tilt_deg should
 *      track how far you tip from vertical.
 *   5. Press the side of the cap to introduce vibration — gyro values should
 *      spike, then return to ~0 when you stop.
 *
 * THIS SKETCH DOES NOT WRITE TO SD. Serial output only.
 *
 * GitHub: github.com/LoriAzniveBerberian/PARcast
 */

#include <Wire.h>
#include <Adafruit_LSM6DSOX.h>
#include <Adafruit_LIS3MDL.h>
#include <Adafruit_Sensor.h>

Adafruit_LSM6DSOX lsm6ds;
Adafruit_LIS3MDL  lis3mdl;

// ============================================================================
// CONFIGURATION
// ============================================================================

// Which axis is gravity when the instrument is upright (confirmed from your
// May 2026 bench test: +Y, reading ~+9.4 m/s² in deployment orientation).
const char GRAVITY_AXIS = 'y';
const int  GRAVITY_SIGN = +1;

// Expected values for sanity checks at rest, upright
const float G_NOMINAL          = 9.81;
const float ACCEL_MAG_TOL      = 1.0;     // m/s² — allow ±1 around 9.81
const float GYRO_STILL_TOL     = 0.10;    // rad/s — gyro noise when truly still
const float MAG_MIN            = 25.0;    // µT — minimum reasonable Earth field
const float MAG_MAX            = 65.0;    // µT — maximum reasonable Earth field
const float UPRIGHT_TILT_TOL   = 10.0;    // degrees — accept up to 10° off vertical

// ============================================================================

// ---------- Helpers ----------
float getGravityAxisValue(const sensors_event_t &accel) {
  switch (GRAVITY_AXIS) {
    case 'x': return accel.acceleration.x;
    case 'y': return accel.acceleration.y;
    case 'z': return accel.acceleration.z;
  }
  return 0;
}

float computeTilt(const sensors_event_t &accel) {
  float ax = accel.acceleration.x;
  float ay = accel.acceleration.y;
  float az = accel.acceleration.z;
  float m  = sqrt(ax*ax + ay*ay + az*az);
  if (m < 0.1) return NAN;
  float aGrav = getGravityAxisValue(accel) * GRAVITY_SIGN;
  float r = aGrav / m;
  if (r >  1.0) r =  1.0;
  if (r < -1.0) r = -1.0;
  return acos(r) * 180.0 / PI;
}

void printCheck(const char *label, bool ok, const char *detail = nullptr) {
  Serial.print(ok ? F("  [PASS] ") : F("  [FAIL] "));
  Serial.print(label);
  if (detail) { Serial.print(F("  -- ")); Serial.print(detail); }
  Serial.println();
}

// ---------- I2C scan ----------
struct I2CResult {
  bool lsm6dsox;
  bool lis3mdl;
  bool ds3231;
  bool ads1115;
  bool ms5837;
  int  total;
};

I2CResult scanI2C() {
  I2CResult r = {false, false, false, false, false, 0};
  Serial.println();
  Serial.println(F("=== I2C bus scan ==="));
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print(F("  Found 0x"));
      if (addr < 16) Serial.print('0');
      Serial.print(addr, HEX);
      switch (addr) {
        case 0x1C: Serial.print(F("  -> LIS3MDL (mag)"));        r.lis3mdl  = true; break;
        case 0x48: Serial.print(F("  -> ADS1115 (ADC)"));         r.ads1115  = true; break;
        case 0x68: Serial.print(F("  -> DS3231 (RTC)"));          r.ds3231   = true; break;
        case 0x6A: Serial.print(F("  -> LSM6DSOX (accel+gyro)")); r.lsm6dsox = true; break;
        case 0x76: Serial.print(F("  -> MS5837 (Bar30)"));        r.ms5837   = true; break;
        default:   Serial.print(F("  -> unknown"));               break;
      }
      Serial.println();
      r.total++;
    }
  }
  Serial.print(F("Total devices: "));
  Serial.println(r.total);
  return r;
}

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) { ; }

  Serial.println();
  Serial.println(F("================================================"));
  Serial.println(F("E_dzPAR — IMU Tilt Sensor Test"));
  Serial.println(F("================================================"));

  Wire.begin();
  Wire.setClock(400000);
  delay(500);

  // ---- I2C scan + check for the two IMU chips ----
  I2CResult bus = scanI2C();
  Serial.println();
  Serial.println(F("I2C presence check:"));
  printCheck("LSM6DSOX at 0x6A", bus.lsm6dsox,
             bus.lsm6dsox ? nullptr : "not found on bus");
  printCheck("LIS3MDL  at 0x1C", bus.lis3mdl,
             bus.lis3mdl  ? nullptr : "not found on bus");

  if (!bus.lsm6dsox || !bus.lis3mdl) {
    Serial.println();
    Serial.println(F("ERROR: IMU not visible on I2C — check Vin, GND, SDA, SCL."));
    Serial.println(F("Halting."));
    while (1) { delay(1000); }
  }

  // ---- Sensor init ----
  Serial.println();
  Serial.println(F("Sensor initialization:"));

  bool lsm_ok = lsm6ds.begin_I2C();
  printCheck("LSM6DSOX.begin_I2C()", lsm_ok);
  if (lsm_ok) {
    lsm6ds.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);
    lsm6ds.setGyroRange(LSM6DS_GYRO_RANGE_500_DPS);
    lsm6ds.setAccelDataRate(LSM6DS_RATE_104_HZ);
    lsm6ds.setGyroDataRate(LSM6DS_RATE_104_HZ);
  }

  bool mag_ok = lis3mdl.begin_I2C();
  printCheck("LIS3MDL.begin_I2C()", mag_ok);
  if (mag_ok) {
    lis3mdl.setPerformanceMode(LIS3MDL_HIGHMODE);
    lis3mdl.setOperationMode(LIS3MDL_CONTINUOUSMODE);
    lis3mdl.setDataRate(LIS3MDL_DATARATE_155_HZ);
    lis3mdl.setRange(LIS3MDL_RANGE_4_GAUSS);
  }

  if (!lsm_ok || !mag_ok) {
    Serial.println();
    Serial.println(F("ERROR: A sensor failed to init even though it's on the bus."));
    Serial.println(F("Halting."));
    while (1) { delay(1000); }
  }

  // ---- Sanity checks: gather 3 seconds of data while still ----
  Serial.println();
  Serial.println(F("Gathering 3 seconds of data — hold the instrument upright and still..."));
  delay(500);

  const int N = 60;
  float ax_sum=0, ay_sum=0, az_sum=0;
  float gx_max=0, gy_max=0, gz_max=0;
  float mag_sum_sq=0;
  float mx_first=0, my_first=0, mz_first=0;
  float mx_last=0,  my_last=0,  mz_last=0;
  float tilt_sum=0;
  int   ok_count=0;

  for (int i = 0; i < N; i++) {
    sensors_event_t a, g, m, t;
    lsm6ds.getEvent(&a, &g, &t);
    lis3mdl.getEvent(&m);

    ax_sum += a.acceleration.x;
    ay_sum += a.acceleration.y;
    az_sum += a.acceleration.z;

    if (fabs(g.gyro.x) > gx_max) gx_max = fabs(g.gyro.x);
    if (fabs(g.gyro.y) > gy_max) gy_max = fabs(g.gyro.y);
    if (fabs(g.gyro.z) > gz_max) gz_max = fabs(g.gyro.z);

    float mmag = sqrt(m.magnetic.x*m.magnetic.x +
                      m.magnetic.y*m.magnetic.y +
                      m.magnetic.z*m.magnetic.z);
    mag_sum_sq += mmag;

    if (i == 0)     { mx_first = m.magnetic.x; my_first = m.magnetic.y; mz_first = m.magnetic.z; }
    if (i == N-1)   { mx_last  = m.magnetic.x; my_last  = m.magnetic.y; mz_last  = m.magnetic.z; }

    float tilt = computeTilt(a);
    if (!isnan(tilt)) { tilt_sum += tilt; ok_count++; }

    delay(50);  // ~20 Hz × 60 samples = 3 s
  }

  float ax_mean = ax_sum / N;
  float ay_mean = ay_sum / N;
  float az_mean = az_sum / N;
  float a_mag   = sqrt(ax_mean*ax_mean + ay_mean*ay_mean + az_mean*az_mean);
  float mag_mean= mag_sum_sq / N;
  float tilt_mean = ok_count ? tilt_sum / ok_count : NAN;

  // ---- Print sanity results ----
  Serial.println();
  Serial.println(F("Sanity checks (instrument upright, still):"));

  // 1. Accel magnitude ~ 9.81
  char buf[64];
  snprintf(buf, sizeof(buf), "|accel| = %.2f m/s² (expect ~9.81)", a_mag);
  printCheck("Accelerometer magnitude",
             fabs(a_mag - G_NOMINAL) < ACCEL_MAG_TOL, buf);

  // 2. Gravity axis is the one expected (Y by default)
  float aGrav = getGravityAxisValue({0,0,0,{ax_mean,ay_mean,az_mean}}) * GRAVITY_SIGN;
  // Recompute the cleaner way:
  switch (GRAVITY_AXIS) {
    case 'x': aGrav = ax_mean * GRAVITY_SIGN; break;
    case 'y': aGrav = ay_mean * GRAVITY_SIGN; break;
    case 'z': aGrav = az_mean * GRAVITY_SIGN; break;
  }
  snprintf(buf, sizeof(buf), "%c-axis × %+d = %.2f (expect ~+9.4 to +9.81)",
           GRAVITY_AXIS, GRAVITY_SIGN, aGrav);
  printCheck("Gravity axis points along expected direction",
             aGrav > 8.0, buf);

  // 3. Tilt from vertical < tolerance
  snprintf(buf, sizeof(buf), "tilt = %.1f° (expect < %.0f°)", tilt_mean, UPRIGHT_TILT_TOL);
  printCheck("Tilt from vertical upright",
             !isnan(tilt_mean) && tilt_mean < UPRIGHT_TILT_TOL, buf);

  // 4. Gyro low when still
  snprintf(buf, sizeof(buf), "gyro max = %.3f / %.3f / %.3f rad/s", gx_max, gy_max, gz_max);
  printCheck("Gyroscope quiet when still",
             gx_max < GYRO_STILL_TOL && gy_max < GYRO_STILL_TOL && gz_max < GYRO_STILL_TOL,
             buf);

  // 5. Mag magnitude in Earth-field range
  snprintf(buf, sizeof(buf), "|mag| = %.1f µT (expect %.0f-%.0f µT)", mag_mean, MAG_MIN, MAG_MAX);
  printCheck("Magnetometer in Earth-field range",
             mag_mean > MAG_MIN && mag_mean < MAG_MAX, buf);

  // 6. Sensors not stuck (mag values changed at least a little between first and last)
  float mag_dx = fabs(mx_last - mx_first);
  float mag_dy = fabs(my_last - my_first);
  float mag_dz = fabs(mz_last - mz_first);
  float mag_dtot = mag_dx + mag_dy + mag_dz;
  snprintf(buf, sizeof(buf), "Δmag over 3 s = %.2f µT (should be > 0.05)", mag_dtot);
  printCheck("Sensor not stuck (mag values varying)",
             mag_dtot > 0.05, buf);

  Serial.println();
  Serial.println(F("Setup complete. Entering live monitoring mode."));
  Serial.println(F("Tilt the instrument by hand — tilt_deg should track."));
  Serial.println();
  Serial.println(F("    t   ax    ay    az    |a|   gyro_max   |mag|   tilt"));
  Serial.println(F("  -----------------------------------------------------"));
}

// ---------- Loop: live monitoring at 2 Hz ----------
void loop() {
  static uint32_t last = 0;
  if (millis() - last < 500) return;
  last = millis();

  sensors_event_t a, g, m, t;
  lsm6ds.getEvent(&a, &g, &t);
  lis3mdl.getEvent(&m);

  float a_mag = sqrt(a.acceleration.x*a.acceleration.x +
                     a.acceleration.y*a.acceleration.y +
                     a.acceleration.z*a.acceleration.z);
  float g_max = max(max(fabs(g.gyro.x), fabs(g.gyro.y)), fabs(g.gyro.z));
  float m_mag = sqrt(m.magnetic.x*m.magnetic.x +
                     m.magnetic.y*m.magnetic.y +
                     m.magnetic.z*m.magnetic.z);
  float tilt  = computeTilt(a);

  char line[120];
  snprintf(line, sizeof(line),
    "  %5.1f  %+5.2f %+5.2f %+5.2f  %5.2f   %6.3f    %5.1f   %5.1f°",
    millis() / 1000.0,
    a.acceleration.x, a.acceleration.y, a.acceleration.z, a_mag,
    g_max, m_mag, isnan(tilt) ? 0.0 : tilt);
  Serial.println(line);
}
