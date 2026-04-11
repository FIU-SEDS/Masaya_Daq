#include "servoValve.h"

servoValve::servoValve(int servoPin) {
    _servoPin = servoPin;
    _currentDegree = 0;
    _lastMoveTime = 0;
    _isMoving = false;
    _stepDelay = 0;
}

void servoValve::begin() {
    _servo.attach(_servoPin);
    close();
}

void servoValve::close() {
    _isMoving = false;
    _currentDegree = 0;
    _servo.write(0);
}

void servoValve::open() {
    _isMoving = false;
    _currentDegree = 90;
    _servo.write(90);
}

void servoValve::closeModerate() {
    _currentDegree = 90;
    _stepDelay = 3;
    _isMoving = true;
    _lastMoveTime = millis();
}

void servoValve::closeSlow() {
    _currentDegree = 90;
    _stepDelay = 6;
    _isMoving = true;
    _lastMoveTime = millis();
}

void servoValve::update() {
    if (!_isMoving) return;

    if (millis() - _lastMoveTime >= (unsigned long)_stepDelay) {
        _lastMoveTime = millis();
        _servo.write(_currentDegree);
        _currentDegree--;

        if (_currentDegree <= 0) {
            _currentDegree = 0;
            _isMoving = false;
        } else {
            _servo.write(_currentDegree--);
        }
    }
}

bool servoValve::isMoving() {
    return _isMoving;
}

int servoValve::getPosition() {
    return _currentDegree;
}
// Include valve.update() in main.cpp