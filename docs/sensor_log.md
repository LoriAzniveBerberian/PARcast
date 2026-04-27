# Sensor Log
Inventory of physical PARcast hardware: serial numbers, sensor-to-unit assignments, calibration history, and field notes about individual sensors. Update this file whenever a sensor is swapped, recalibrated, repaired, or noted as anomalous.

For the information on the calibration values listed here, see [calibration_and_corrections.md](calibration_and_corrections.md).

---

## SQ-500-SS Quantum Sensors
PARcast uses two Apogee SQ-500-SS full-spectrum quantum sensors, one per unit. Both were purchased together in a single order.

### Sensor 1 — E_s(PAR) reference station (surface unit)

| Field | Value |
|---|---|
| Model | Apogee SQ-500-SS |
| Serial number |  |
| Date received |  |
| First calibration date (Apogee certificate) |  |
| Calibration factor | 100.0 µmol m⁻² s⁻¹ per mV (manufacturer standard) |
| Wiring scheme | ☐ White/Black/Clear (S/N ≥ 1559)  ☐ Red/Black/Clear (S/N ≤ 1558) |
| Cable length | 5 m |
| Notes |  |

### Sensor 2 — E_d(z, PAR) profiler (in-water unit)

| Field | Value |
|---|---|
| Model | Apogee SQ-500-SS |
| Serial number |  |
| Date received |  |
| First calibration date (Apogee certificate) |  |
| Calibration factor | 100.0 µmol m⁻² s⁻¹ per mV (manufacturer standard) |
| Immersion factor | ☐ 1.25 (S/N ≥ 2876)   ☐ 1.32 (S/N ≤ 2875) |
| Wiring scheme | ☐ White/Black/Clear (S/N ≥ 1559)  ☐ Red/Black/Clear (S/N ≤ 1558) |
| Cable length | 5 m |
| Notes |  |

---

## How to Find Your Serial Numbers
The model number and serial number are printed on a small label on the sensor cable, near the pigtail leads (see Apogee SQ-500 manual page 6). 

## How to Pick the Right Immersion Factor
The serial number determines which immersion correction applies to the in-water profiler:
- **S/N 2876 and above:** factor = 1.25
- **S/N 0 through 2875:** factor = 1.32

Once you know the in-water sensor's serial number, update the firmware constant `IMMERSION_FACTOR` in `E_dzPAR_profiler_final.ino` and check the matching box in this log.

## How to Pick the Right Wiring Scheme
Apogee changed wiring colors in March 2018:
- **S/N 1559 and above (or any sensor with the M8 connector):** White = positive, Black = negative, Clear = shield
- **S/N 0 through 1558:** Red = positive, Black = negative, Clear = shield

The PARcast firmware assumes the newer scheme (white wire on A0, black on A1). If either of your sensors has the older wiring, swap red for white in your wiring and update the test READMEs accordingly.

---

## Recalibration History
Apogee recommends recalibration every 2 years. Log each recalibration here.

| Sensor S/N | Date sent | Date received | Pre-cal drift | Notes |
|---|---|---|---|---|
|  |  |  |  |  |
|  |  |  |  |  |

---

## Field Anomalies and Repairs
Note any deployments where a sensor behaved unexpectedly, or any physical damage, repairs, or parts swapped. Include the date and the file/deployment reference if applicable.

| Date | Sensor S/N | Description | Action taken |
|---|---|---|---|
|  |  |  |  |
|  |  |  |  |

---
