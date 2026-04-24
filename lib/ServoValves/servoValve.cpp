#include "servoValve.h"

// Adjusted for 270 Degree Servos, Each With Different Values.

servoValve::servoValve(int servoPin) {
    _servoPin = servoPin;
    _lastMoveTime = 0;
    _isMoving = false;
    _stepDelay = 0;

    if (_servoPin == PA1) {
        _currentDegree = 9;
    } else if (_servoPin == PA4) {
        _currentDegree = 2.5;
    } else if (_servoPin == PA5) {
        _currentDegree = 3;
    } else if (_servoPin == PA3) {
        _currentDegree = 7;
    } else {
        _currentDegree = 0;
    }
}

void servoValve::begin() {
    _servo.attach(_servoPin);
    close();
}

void servoValve::close() {
    _isMoving = false;
    if (_servoPin == PA1) {
        _currentDegree = 9;
        _servo.write(9);
    } else if (_servoPin == PA4) {
        _currentDegree = 2.5;
        _servo.write(2.5);
    } else if (_servoPin == PA5) {
        _currentDegree = 3;
        _servo.write(3);
    } else if (_servoPin == PA3) {
        _currentDegree = 7;
        _servo.write(7);
    } else {
        _currentDegree = 0;
        _servo.write(0);
    }
}

void servoValve::open() {
    _isMoving = false;
    if (_servoPin == PA1) {
        _currentDegree = 74;
        _servo.write(74);
    } else if (_servoPin == PA4) {
        _currentDegree = 67.5;
        _servo.write(67.5);
    } else if (_servoPin == PA5) {
        _currentDegree = 98;
        _servo.write(98);
    } else if (_servoPin == PA3) {
        _currentDegree = 100;
        _servo.write(100);
    } else {
        _currentDegree = 90;
        _servo.write(90);
    }
}

void servoValve::closeModerate() {
    _stepDelay = 3;
    _isMoving = true;
    _lastMoveTime = millis();
    // _currentDegree should already be at open position;
    // call open() first if unsure of state
}

void servoValve::closeSlow() {
    _stepDelay = 6;
    _isMoving = true;
    _lastMoveTime = millis();
    // _currentDegree should already be at open position;
    // call open() first if unsure of state
}

void servoValve::update() {
    if (!_isMoving) return;

    if (millis() - _lastMoveTime >= (unsigned long)_stepDelay) {
        _lastMoveTime = millis();

        // Determine the closed target for this servo
        float closeTarget;
        if (_servoPin == PA1) {
            closeTarget = 9;
        } else if (_servoPin == PA4) {
            closeTarget = 2.5;
        } else if (_servoPin == PA5) {
            closeTarget = 3;
        } else if (_servoPin == PA3) {
            closeTarget = 7;
        } else {
            closeTarget = 0;
        }

        if (_currentDegree > closeTarget) {
            _currentDegree--;
            _servo.write(_currentDegree);
        }

        if (_currentDegree <= closeTarget) {
            _currentDegree = closeTarget;
            _servo.write(_currentDegree);
            _isMoving = false;
        }
    }
}

bool servoValve::isMoving() {
    return _isMoving;
}

float servoValve::getPosition() {
    return _currentDegree;
}