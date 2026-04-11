#ifndef SOLENOID_H
#define SOLENOID_H

#include <Arduino.h>

class solenoid {
  private:
    int _pin;
    int _currentPosition;

  public:
    solenoid(int pin);

    void begin();
    void open();
    void close();
    int getPosition();
};

#endif