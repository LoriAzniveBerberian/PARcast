# PARcast
PARcast is a DIY oceanography instrument supporting cost-effective PAR profiling of shallow (0–30 m) environments where shipborne profilers and autonomous floats are generally not applicable. PARcast is manually deployed by an in-water swimmer or diver at the water’s surface and falls via tethered descent. It comprises two units with cosine-corrected quantum sensors (Apogee SQ-500).

## Instruments
**E_d(z, PAR) aquatic profiler**
Descent-assisted instrument that profiles downwelling PAR through the water column inside a Blue Robotics 4" enclosure. Components: Teensy 4.1, ADS1115 16-bit ADC, DS3231 RTC, SQ-500-SS PAR sensor, MS5837 pressure/temperature sensor, microSD card, Orbtronic 18650 batteries.

**E_s(PAR) surface reference**
Surface reference PAR sensor that logs incident sunlight above water during in-water deployments. Used for profile normalization and derivation of apparent optical properties. Components: Teensy 4.1, ADS1115 16-bit ADC, DS3231 RTC, SQ-500-SS PAR sensor, microSD card.

## Naming Convention
Notes and comments use `E_s(PAR)` and `E_d(z, PAR)`.
File names, folder names, and code use `E_sPAR` and `E_dzPAR`.

CSV file names:
- surface reference: `E_sPAR_YYYYMMDD_0001.CSV`
- aquatic profiler: `E_dzPAR_YYYYMMDD_0001.CSV`

SD card labels:
- surface reference: `E_SPAR`
- aquatic profiler: `E_DZPAR`
  
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
│       └── ms5837_test/                aquatic profiler only
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

## For Students and Newcomers
PARcast was developed as a learning project as well as an instrument. The resources below were most useful during the build:

The [`literature/`](literature/) folder contains relevant published work on sensor electronics, instrumentation design, embedded systems radiometry, calibration and validation, and ocean optics.

**Ocean optics:** [Ocean Optics Web Book](https://www.oceanopticsbook.info/) — Curtis Mobley's free online textbook, the best place to start if PAR and irradiance are new to you. Also [IOCCG Reports](https://ioccg.org/what-we-do/ioccg-publications/ioccg-reports/) for protocols and methods.

**Embedded systems:** [Making Embedded Systems](https://www.oreilly.com/library/view/making-embedded-systems/9781098151539/) by Elecia White and the [Adafruit Learning System](https://learn.adafruit.com/) for hands-on tutorials. The [Arduino reference](https://www.arduino.cc/reference/) and [Teensy documentation](https://www.pjrc.com/teensy/) were used throughout firmware development.

## Acknowledgments
PARcast's firmware and electronics design were developed iteratively with assistance from Anthropic's [Claude](https://claude.ai/) to explain concepts and debug circuits. The final firmware and test sketches were adapted from example sketches provided by Arduino, Teensy/PJRC, and the maker communities behind each sensor library used in this project.
