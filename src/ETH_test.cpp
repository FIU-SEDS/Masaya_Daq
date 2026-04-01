#include <Arduino.h>

HardwareSerial TestSerial(PA10, PA9); 

void setup() {
    TestSerial.begin(115200);
}

void loop() {
    TestSerial.write((byte*)"Hello", sizeof(float));     
    delay(50);
}