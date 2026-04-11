#ifndef SERVO_VALVE_H
#define SERVO_VALVE_H

#include <Arduino.h>
#include <Servo.h>

class servoValve {
  private:
    int _servoPin;
    Servo _servo;

    int _currentDegree;
    unsigned long _lastMoveTime;
    bool _isMoving;
    int _stepDelay;

  public:
    servoValve(int servoPin);
    void begin();
    void close();
    void open();
    void closeSlow();  
    void closeModerate();
    void update();                       
    bool isMoving();
    int getPosition();                     
};

#endif