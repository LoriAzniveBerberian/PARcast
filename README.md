# PARcast

A bespoke instrument for profiling photosynthetically active radiation (PAR) via water column casts.

## Instruments

**E_d(z, PAR) profiler — in-water unit**
Descent-assisted instrument that profiles downwelling PAR through the water column inside a Blue Robotics 4" enclosure. Components: Teensy 4.1, ADS1115 16-bit ADC, DS3231 RTC, SQ-500-SS PAR sensor, MS5837 pressure/temperature sensor, microSD card, Orbtronic 18650 batteries.

**E_s(PAR) reference station — surface unit**
Surface reference PAR sensor that logs incident sunlight above water during in-water deployments. Used for profile normalization and derivation of apparent optical properties. Components: Teensy 4.1, ADS1115 16-bit ADC, DS3231 RTC, SQ-500-SS PAR sensor, microSD card.

## Naming Convention

PARcast uses parenthesized scientific notation in human-readable text (comments, READMEs, plot labels) and underscored versions in filenames, paths, and code identifiers.

| Context | Surface unit | In-water unit |
|---|---|---|
| Prose, comments, plot labels | `E_s(PAR)` | `E_d(z, PAR)` |
| File names, folder names, code | `E_sPAR` | `E_dzPAR` |

### File naming on the SD cards
- Surface reference:  `E_sPAR_YYYYMMDD_0001.CSV`
- In-water profiler:  `E_dzPAR_YYYYMMDD_0001.CSV`

### SD card volume labels
- Surface reference:  `PARCAST_Es`
- In-water profiler:  `PARCAST_Ed`

## Repository Structure
```
PARcast/
├── arduino_ide_sketch/
│   ├── E_sPAR/
│   │   └── final/
│   │       └── E_sPAR_reference_final/
│   │           └── E_sPAR_reference_final.ino
│   ├── E_dzPAR/
│   │   └── final/
│   │       └── E_dzPAR_profiler_final/
│   │           └── E_dzPAR_profiler_final.ino
│   └── tests/                          shared diagnostic sketches
│       ├── ads1115_test/
│       ├── blink_test/
│       ├── i2c_scanner/
│       ├── par_sensor_test/
│       ├── rtc_test/
│       ├── sd_card_test/
│       └── ms5837_test/                in-water unit only
├── data_processing/
│   ├── data/
│   │   ├── raw/
│   │   │   ├── E_sPAR/                 surface CSVs (local only)
│   │   │   └── E_dzPAR/                profiler CSVs (local only)
│   │   └── processed/                  output from Python scripts
│   └── scripts/                        Jupyter notebooks and Python scripts
├── field_notes/
│   └── field_notes_template.txt
├── docs/
│   ├── calibration_and_corrections.md  PPFD calibration, immersion correction, error budget
│   └── sensor_log.md                   serial numbers and recalibration history
├── requirements.txt
└── README.md
```

**Note:** Raw data files are excluded from this repository and stored locally only.

## Getting Started
```bash
conda create -n parcast python=3.11
conda activate parcast
pip install -r requirements.txt
```

## Community

We welcome suggestions, questions, and contributions from the community. If you're using PARcast or thinking about building one, head to the discussion forum to introduce yourself and share ideas:

[**PARcast Discussions**](https://github.com/LoriAzniveBerberian/PARcast/discussions/1#discussion-9950905)
