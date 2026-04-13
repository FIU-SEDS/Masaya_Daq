#include <Arduino.h>
#include "HX711.h"
#include "Adafruit_ADS1X15.h"

#define DOUT_PIN  PC4 
#define SCK_PIN   PC5

HX711 scale;

void setup() {
    Serial.begin(115200);
    while (!Serial);

    scale.begin(DOUT_PIN, SCK_PIN);
    scale.set_scale(-45160.8671f); // lc0 -45228.2617f lc1 -45160.8671
    scale.tare();

    Serial.println("Tared. Reading...");
}

void loop() {
    Serial.print("Weight: ");
    Serial.print(scale.get_units(5), 3);
    Serial.println(" kg");
    delay(500);
}