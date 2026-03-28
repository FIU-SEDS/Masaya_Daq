#include <Wire.h>
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 adc;

void setup() {
  Serial.begin(115200);
  
  delay(2000); 
  
  Serial.println("STM32 Booting up...");

  // EXPLICITLY DEFINE THE I2C PINS HERE
  Wire.setSCL(PB6);
  Wire.setSDA(PB7);
  Wire.begin(); // Start the I2C bus on those specific pins

  Serial.println("I2C Pins Set. Looking for ADS1115...");

  // Initialize the ADC and pass the explicit Wire object just to be safe
  if (!adc.begin(0x48, &Wire)) {
    Serial.println("ERROR: Failed to find ADS1115. Check wiring!");
    while (1); // Freeze here if board isn't found
  }

  Serial.println("ADS1115 Found! Starting reads...");
}

void loop() {
  int16_t ch0_raw_value = adc.readADC_SingleEnded(0); 
  float ch0_volts = adc.computeVolts(ch0_raw_value); // Converts the raw number to Volts

  int16_t ch1_raw_value = adc.readADC_SingleEnded(1); 
  float ch1_volts = adc.computeVolts(ch1_raw_value);
  
  // Serial.print("Channel 0 Raw: ");
  Serial.print(ch0_raw_value); 
  Serial.print(",");
  Serial.print(ch0_volts, 3); // Tells it to print 3 decimal places

  Serial.print(",");

  // Serial.print("  |  Channel 1 Raw: ");
  Serial.print(ch1_raw_value); 
  Serial.print(",");
  Serial.println(ch1_volts, 3); 
  
  delay(2);
}