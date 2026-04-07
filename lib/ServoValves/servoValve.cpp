#include "servoValve.h"

servoValve::servoValve(int servoPin) {
    _servoPin = servoPin;
    _currentDegree = 0;
    _lastMoveTime = 0;
    _isOpening = false;
    _stepDelay = 0;
}

void servoValve::begin() {
    _servo.attach(_servoPin);
    close();
}

void servoValve::close() {
    _isOpening = false;
    _servo.write(0);
}

void servoValve::open() {
    _isOpening = false;
    _servo.write(180);
}

void servoValve::openModerate() {
    _currentDegree = 0;
    _stepDelay = 3;
    _isOpening = true;
    _lastMoveTime = millis();
}

void servoValve::openSlow() {
    _currentDegree = 0;
    _stepDelay = 6;
    _isOpening = true;
    _lastMoveTime = millis();
}

void servoValve::update() {
    if (!_isOpening) return;

    if (millis() - _lastMoveTime >= (unsigned long)_stepDelay) {
        _lastMoveTime = millis();
        _servo.write(_currentDegree);
        _currentDegree++;

        if (_currentDegree > 180) {
            _isOpening = false;
        }
    }
}

bool servoValve::isMoving() {
    return _isOpening;
}

// Include valve.update() in main.cpp