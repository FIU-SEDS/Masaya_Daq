#include <Arduino.h>
#include "servoValve.h"

HardwareSerial TestSerial(PA10, PA9);

// Servo
servoValve testServo(PA5);

void setup() {
    TestSerial.begin(115200);
    testServo.begin();
}

void loop() {
    if (TestSerial.available() > 0) {
        char incoming = TestSerial.read();
        
        if (incoming == 'O') {
            testServo.open();
            TestSerial.print("Opened"); // Send back a simple string
        }
        else if (incoming == 'C') {
            testServo.close();
            TestSerial.print("Closed");
        }

        testServo.close();
        delay(100);
        testServo.open();
        delay(100);
    }
}