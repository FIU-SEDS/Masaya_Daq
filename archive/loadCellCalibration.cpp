#include "HX711.h"
#include "Adafruit_ADS1X15.h"

const int LOADCELL_DOUT_PIN = PC4;
const int LOADCELL_SCK_PIN = PC5;

HX711 scale;

// Tell the code what weight you are using to calibrate (e.g., 100 grams)
float known_weight = 100.0; 

void setup() {
  Serial.begin(115200);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  delay(5000);

  Serial.println("Please empty the scale completely.");
  delay(3000); // Give you 3 seconds to clear it

  // --- STEP 1 ---
  // Call set_scale() with no parameter.
  scale.set_scale(); 

  // --- STEP 2 ---
  // Call tare() with no parameter.
  scale.tare();      

  Serial.println("Place your known weight on the scale now.");
  Serial.println("Reading in 5 seconds...");
  delay(5000); // Give you 5 seconds to place the weight on the scale

  // --- STEP 3 ---
  // Place a known weight on the scale and call get_units(10).
  float result = scale.get_units(10); 

  // --- STEP 4 ---
  // Divide the result in step 3 to your known weight.
  float calculated_factor = result / known_weight; 

  // --- THE OUTPUT ---
  Serial.print("Raw reading (Step 3): ");
  Serial.println(result);
  
  Serial.print("Your exact Calibration Factor is: ");
  Serial.println(calculated_factor);
  
  Serial.println("Write this number down and put it in your main code!");
}

void loop() {
  // We leave this empty because we only needed to do the math once!
}