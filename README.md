# PARcast
A bespoke instrument for profiling photosynthetically active radiation (PAR) via water column casts.

## Instruments

**Instrument A — Aquatic Profiler (WPAR)**
Descent-assisted instrument that profiles PAR through the water column inside a Blue Robotics 4" enclosure. Components: Teensy 4.1, ADS1115 16-bit ADC, DS3231 RTC, SQ-500-SS PAR sensor, MS5837 pressure/temperature sensor, microSD card, Orbtronic 18650 batteries.

**Instrument B — Land Station (LPAR)**
Surface reference PAR sensor that logs sunlight above water during aquatic deployments. Components: Teensy 4.1, ADS1115 16-bit ADC, DS3231 RTC, SQ-500-SS PAR sensor, microSD card.

## How It Works
The land station records surface PAR (E0) while the aquatic profiler records PAR and depth simultaneously. These datasets are combined in post-processing to calculate the light attenuation coefficient Kd.

## File Naming Convention
Land station:      `B_LPAR_YYYYMMDD_0001.CSV`
Aquatic profiler:  `A_WPAR_YYYYMMDD_0001.CSV`

## Repository Structure
```
PARcast/
├── arduino_ide_sketch/
│   ├── Land_Station/
│   │   ├── tests/
│   │   │   ├── ads1115_test/
│   │   │   ├── blink_test/
│   │   │   ├── i2c_scanner/
│   │   │   ├── par_sensor_test/
│   │   │   ├── rtc_test/
│   │   │   └── sd_card_test/
│   │   └── final/
│   │       └── land_station_final/
│   └── InWater_Instrument/
│       ├── tests/
│       │   └── ms5837_test/
│       └── final/
├── data_processing/
│   ├── data/
│   │   ├── raw/
│   │   │   ├── LPAR/     ← B_LPAR CSV files go here (local only)
│   │   │   └── WPAR/     ← A_WPAR CSV files go here (local only)
│   │   └── processed/    ← output from Python scripts
│   ├── scripts/          ← Jupyter notebooks and Python scripts
│   └── notebooks/
├── field_notes/
│   └── field_notes_template.txt
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

## Contributors
Lori Berberian & Nareg — UCLA Geography / PhsiQuantum
