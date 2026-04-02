# PARpedo

Bespoke open-source instrument for profiling photosynthetically active radiation (PAR) and calculating light attenuation coefficients (Kd) in coastal aquatic env

cat > "/Users/loriberberian/Desktop/bioSCape/PARcast/README.md" << 'EOF'
# PARcast
A bespoke instrument for profiling photosynthetically active radiation (PAR) via water column casts.

## Instruments

**Instrument B — Land Station (LPAR)**
Surface reference PAR sensor that logs sunlight above water during aquatic deployments. Components: Teensy 4.1, ADS1115 16-bit ADC, DS3231 RTC, SQ-500-SS PAR sensor, microSD card.

**Instrument A — Aquatic Profiler (WPAR)**
Descent-assisted instrument that profiles PAR through the water column inside a Blue Robotics 4" enclosure. Components: Teensy 4.1, ADS1115 16-bit ADC, DS3231 RTC, SQ-500-SS PAR sensor, MS5837 pressure/temperature sensor, microSD card, Orbtronic 18650 batteries.

## How It Works
The two instruments run simultaneously. The land station records surface PAR (E0) while the aquatic profiler records depth PAR (Ez). The ratio Ez/E0 gives the light attenuation coefficient Kd — a key water quality metric.

## File Naming Convention
Land station:      `B_LPAR_YYYYMMDD_0001.CSV`
Aquatic profiler:  `A_WPAR_YYYYMMDD_0001.CSV`

## Repository Structure
```
PARcast/
├── arduino_ide_sketches/     # Teensy firmware
│   └── PAR_Profiler/
│       ├── Land_Station/
│       │   ├── tests/
│       │   └── final/
│       └── InWater_Instrument/
│           ├── tests/
│           └── final/
├── data_processing/          # Python analysis
│   ├── scripts/
│   └── notebooks/
├── field_notes/              # Deployment logs
├── requirements.txt
└── README.md
```

## Getting Started
```bash
conda create -n parcast python=3.11
conda activate parcast
pip install -r requirements.txt
```

## Contributors
Lori Berberian & Nareg — UCLA Geography / PhsiQuantum
