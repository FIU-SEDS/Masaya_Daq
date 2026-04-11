#include "solenoid.h"

solenoid::solenoid(int pin) {
    _pin = pin;
}

void solenoid::begin() {
    pinMode(_pin, OUTPUT);

    digitalWrite(_pin, LOW); //Start Closed
    _currentPosition = 0;
}

void solenoid::close(){
    digitalWrite(_pin, LOW);
    _currentPosition = 0;
}

void solenoid::open(){
    digitalWrite(_pin, HIGH);
    _currentPosition = 90;
}

int solenoid::getPosition() {
    return _currentPosition;
}