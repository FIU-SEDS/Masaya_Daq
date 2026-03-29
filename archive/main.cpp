#include <Wire.h>
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 adc;

unsigned long previousMicros = 0;
const unsigned long sampleInterval = 1162; // 1,000,000 micros / 860 Hz = ~1162us

void setup() {
  // Bumped up the baud rate to handle the high data throughput
  Serial.begin(921600); 
  
  delay(5000); // Shortened to 5s so you don't have to wait so long
  
  Serial.println("STM32 Booting up...");

  // EXPLICITLY DEFINE THE I2C PINS HERE
  Wire.setSCL(PB6);
  Wire.setSDA(PB7);
  
  // Speed up I2C to 400kHz to reduce communication delay
  Wire.setClock(400000); 
  Wire.begin(); 

  Serial.println("I2C Pins Set. Looking for ADS1115...");

  if (!adc.begin(0x48, &Wire)) {
    Serial.println("ERROR: Failed to find ADS1115. Check wiring!");
    while (1); 
  }

  Serial.println("ADS1115 Found! Starting reads...");

  adc.setDataRate(RATE_ADS1115_860SPS);
}

void loop() {
  unsigned long currentMicros = micros();

  // Check if 1162 microseconds have passed
  if (currentMicros - previousMicros >= sampleInterval) {
    // Increment by interval to prevent drift over time
    previousMicros += sampleInterval; 

    // Read only ONE channel to maintain 860 SPS. 
    int16_t ch0_raw_value = adc.readADC_SingleEnded(0); 
    float ch0_volts = adc.computeVolts(ch0_raw_value); 

    int16_t ch1_raw_value = adc.readADC_SingleEnded(1); 
    float ch1_volts = adc.computeVolts(ch1_raw_value);

    /* --- UNCOMMENTING THESE WILL DROP YOUR SAMPLE RATE TO ~215 Hz ---


    int16_t ch2_raw_value = adc.readADC_SingleEnded(2); 
    float ch2_volts = adc.computeVolts(ch2_raw_value); // Fixed bug here

    int16_t ch3_raw_value = adc.readADC_SingleEnded(3); 
    float ch3_volts = adc.computeVolts(ch3_raw_value); // Fixed bug here
    */

    // Print Channel 0
    Serial.print(ch0_raw_value); 
    Serial.print(",");
    Serial.print(ch0_volts, 3); 

    Serial.print(",");

    Serial.print(ch1_raw_value); 
    Serial.print(",");
    Serial.print(ch1_volts, 3); 

    Serial.print(",");
    
    // Convert micros back to a seconds timestamp and print
    Serial.println((currentMicros - 10) / 1000000.0, 6);
  } 
}