#include <Wire.h>
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 adc;

unsigned long previousMicros = 0;
const unsigned long sampleInterval = 1162; // ~860 SPS

void setup() {
  Serial.begin(921600);
  delay(5000);
  Serial.println("STM32 Booting up...");

  Wire.setSCL(PB6);
  Wire.setSDA(PB7);
  Wire.setClock(400000);
  Wire.begin();

  if (!adc.begin(0x48, &Wire)) {
    Serial.println("ERROR: ADS1115 not found!");
    while (1);
  }

  adc.setGain(GAIN_TWOTHIRDS);        // ±6.144V, 0.1875mV/bit — explicit
  adc.setDataRate(RATE_ADS1115_860SPS);

  // Continuous mode on channel 0 — non-blocking reads
  adc.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_0, /*continuous=*/true);

  Serial.println("raw,volts,tempC,timestamp_s");  // CSV header
}

void loop() {
  unsigned long currentMicros = micros();

  if (currentMicros - previousMicros >= sampleInterval) {
    previousMicros += sampleInterval;

    int16_t raw  = adc.getLastConversionResults();  // Non-blocking
    float volts  = adc.computeVolts(raw);
    float tempC  = (volts - 1.25) / 0.005;

    // Clean CSV output
    Serial.print(raw);                              Serial.print(",");
    Serial.print(volts, 4);                         Serial.print(",");
    Serial.print(tempC, 2);                         Serial.print(",");
    Serial.println(currentMicros / 1000000.0, 6);
  }
}