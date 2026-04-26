# Calibration and Corrections

This document explains the calibration and correction factors needed to convert raw PARcast measurements into accurate scientific values. It applies to both the E_s(PAR) reference station and the E_d(z, PAR) profiler, both of which use the Apogee SQ-500-SS quantum sensor.

For sensor-specific information (serial numbers, certificate dates, immersion factors), see [sensor_log.md](sensor_log.md).

## Quick Reference

```
PPFD_air   = millivolts × 100.0
PPFD_water = PPFD_air × immersion_factor
```

| Quantity | Value | Notes |
|---|---|---|
| ADS1115 LSB at GAIN_SIXTEEN | 0.0078125 mV per count | 256 mV / 32768 counts |
| Apogee SQ-500-SS calibration factor | 100.0 µmol m⁻² s⁻¹ per mV | Manufacturer-specified, exact |
| Calibration uncertainty | ± 5% | NIST-traceable |
| Immersion correction (S/N ≥ 2876) | 1.25 | Underwater only |
| Immersion correction (S/N ≤ 2875) | 1.32 | Underwater only |

## Calibration Factor

The Apogee SQ-500 owner's manual states that all SQ-500 sensors are calibrated to a standard PPFD calibration factor of **exactly 100.0 µmol m⁻² s⁻¹ per mV** (sensitivity of 0.01 mV per µmol m⁻² s⁻¹). Each sensor is calibrated by side-by-side comparison to four transfer standard quantum sensors traceable to the National Institute of Standards and Technology (NIST), with a calibration uncertainty of ± 5%.

This means every PARcast SQ-500-SS uses the same multiplier in code:

```cpp
PPFD_air = millivolts * 100.0;   // µmol m⁻² s⁻¹
```

## ADC Conversion

Raw counts from the ADS1115 are converted to millivolts using the LSB at GAIN_SIXTEEN:

```
millivolts = raw_counts × 0.0078125
```

Where 0.0078125 mV is the resolution at GAIN_SIXTEEN (±256 mV full-scale ÷ 32768 counts). This is set in firmware via `ads.setGain(GAIN_SIXTEEN)`. Always use GAIN_SIXTEEN with the SQ-500 because its 0–40 mV output is too small to resolve at the default gain.

## Immersion Correction (in-water only)

When a quantum sensor calibrated in air is submerged, more light is backscattered out of the diffuser due to the refractive index difference between air (1.00) and water (1.33). Without correction, underwater readings are 25–32% too low.

Apogee specifies the correction as a single multiplicative factor for the SQ-500 series:

| Sensor S/N | Immersion factor |
|---|---|
| 2876 and above | 1.25 |
| 0 – 2875 | 1.32 |

Apply this **only** to in-water E_d(z, PAR) profiler data. The E_s(PAR) reference station is in air and does not need correction.

```cpp
// In-water profiler firmware:
float ppfd_air   = millivolts * 100.0;
float ppfd_water = ppfd_air * IMMERSION_FACTOR;  // 1.25 or 1.32
```

The factor for each PARcast sensor is recorded in [sensor_log.md](sensor_log.md).

## Error Budget

Per the Apogee SQ-500 manual (page 7), the manufacturer-specified error sources are:

| Source | Magnitude | Notes |
|---|---|---|
| Calibration uncertainty | ± 5% | Absolute accuracy vs NIST reference |
| Cosine response at 45° zenith | ± 2% | Small at midday |
| Cosine response at 75° zenith | ± 5% | Significant at low sun angles |
| Temperature response | -0.11 ± 0.04 % per °C | Referenced to ~20 °C |
| Long-term drift | < 2% per year | Recommend recalibration every 2 years |
| Spectral error (clear sky sun) | < 1% | Negligible for PARcast deployments |
| Measurement repeatability | < 0.5% | Within a single deployment |
| Azimuth error | < 0.5% | Mount with cable pointing toward true north |
| Tilt error | < 0.5% | Use AL-100 leveling plate |
| Non-linearity | < 1% (up to 4000 µmol m⁻² s⁻¹) | Negligible for typical sun |

For a typical clear-sky midday deployment with a properly leveled sensor, the dominant error source is the ± 5% calibration uncertainty. Other terms add a few percent in quadrature.

## Deployment Guidance

### Solar geometry
Cosine error is the largest practical concern after calibration uncertainty. Deploy within ± 2 hours of solar noon to keep the solar zenith angle below ~ 45°, where cosine error is ± 2%. Outside this window the error grows toward ± 5% at 75° zenith, and the increasing diffuse-to-direct ratio further complicates K_d derivation.

### Leveling
The sensor must be horizontal to accurately measure PPFD on a horizontal surface. Use the Apogee AL-100 leveling plate or equivalent on the surface unit. For the in-water profiler, the descending sensor should be oriented upward (cosine collector pointing at the sky).

### Cable orientation
To minimize azimuth error, mount with the sensor cable pointing toward true north (in the northern hemisphere). Azimuth error is small (< 0.5%) but free to eliminate.

### Cleaning
Salt deposits and biofouling reduce diffuser transmittance and bias readings low. Clean with vinegar (for salt) or water and a soft cloth (for organic matter). Never use abrasives, alcohol, or acetone on the diffuser.

### Recalibration
Apogee recommends recalibration every 2 years. Track recalibration history in [sensor_log.md](sensor_log.md). To check whether your sensor has drifted, use the Clear Sky Calculator (www.clearskycalculator.com) and compare measured to predicted PPFD on a clear day near solar noon. Consistent disagreement greater than ~ 6% indicates the sensor should be cleaned, re-leveled, or sent in for recalibration.

## References

- Apogee Instruments. SQ-500 Owner's Manual, Rev. 11-July-2025.
- Smith, R.C., 1969. An underwater spectral irradiance collector. *Journal of Marine Research* 27:341–351.
- Tyler, J.E., and R.C. Smith, 1970. *Measurements of Spectral Irradiance Underwater.* Gordon and Breach, New York.
- Apogee underwater PAR measurements page: http://www.apogeeinstruments.com/underwater-par-measurements/
