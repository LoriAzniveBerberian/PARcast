#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads;

void setup(void)
{
  Serial.begin(9600);
  Serial.println("PAR Sensor Test — SQ-500-SS");

  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS1115. Check wiring!");
    while (1);
  }
  
  ads.setGain(GAIN_SIXTEEN);
  Serial.println("ADS1115 initialized with GAIN_SIXTEEN");
}

void loop(void)
{
  int16_t results;
  float millivolts;
  float ppfd;

  results = ads.readADC_Differential_0_1();
  millivolts = ads.computeVolts(results) * 1000.0;
  ppfd = millivolts * 100.0;

  Serial.println("-----------------------------------------------------------");
  Serial.print("Raw counts:   "); Serial.println(results);
  Serial.print("Millivolts:   "); Serial.print(millivolts); Serial.println(" mV");
  Serial.print("PPFD:         "); Serial.print(ppfd); Serial.println(" umol/m2/s");

  delay(1000);
}