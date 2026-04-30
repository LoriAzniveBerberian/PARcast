# PARcast: DIY-oceanography system for profiling PAR in kelp forests

Nearshore environments support taxonomically-rich biological assemblages that rely on sufficient solar (and lunar) illumination for critical functions including primary production, diel vertical migration, and the timing of reproduction. Photosynthetically Available Radiation, or PAR, describes the visible light (400–700 nm) that autotrophs use for photosynthesis. Quantifying PAR and its attenuation in aquatic environments is therefore essential for understanding ecological and physiological processes governing productivity and habitat suitability—relevant to species distribution and community structure. We present a prototype DIY-oceanography system, PARcast, supporting PAR profiling of complex nearshore and shallow coastal environments where shipborne profilers and autonomous floats are generally not applicable. Although PARcast is designed for manual deployment by a swimmer or diver in kelp forests, its scalable design may extend its utility to a variety of aquatic environments (e.g., coral reefs, seagrass meadows, and inland water bodies) and deployment platforms (e.g., small crafts, kayaks, or docks). The prototype comprises two cosine-corrected quantum sensors (Apogee SQ-500), one of which is mounted on an acrylic enclosure where E_d(z, PAR), pressure, temperature, and tilt are logged onboard a microSD card; complemented by an accessory sensor capable of measuring E_s(PAR). Both units are supported by a Teensy 4.1 microcontroller, analog-to-digital converter, and real-time clock. We evaluate the suitability of PARcast for research and teaching applications and present initial data from cruise deployments associated with NASA’s Student Airborne Research Program and Plumes and Blooms. Instrument specifications, validation datasets, and results will be shared via GitHub for community use and feedback.

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
├── prototyping/                        schematics and hardware design files
│   ├── E_sPAR_schematic.pdf
│   ├── E_sPAR_schematic.pptx
│   ├── E_dzPAR_schematic.pdf
│   └── E_dzPAR_schematic.pptx
├── literature/                         relevant published work
│   ├── radiometry/
│   │   ├── atmospheric/                cosine response (Michalsky, Harrison)
│   │   └── aquatic/                    immersion factors (Hooker, Zibordi, Mueller)
│   ├── sensor_electronics/             photodiode and TIA design
│   └── embedded_systems/               firmware and microcontroller references
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
