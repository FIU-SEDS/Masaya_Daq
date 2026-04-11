#ifndef TC_SENSORS_H
#define TC_SENSORS_H

#include <Wire.h>
#include <Adafruit_ADS1X15.h>

class tcSensors {
  private:
    int _SCL;
    int _SDA;
    Adafruit_ADS1115 _tcSensors;

    int16_t  _results[3];
    uint8_t  _currentCh;
    bool     _converting;

  public:
    tcSensors(int SCL, int SDA);
    void begin(uint8_t addr);
    void startNext();
    void poll();
    float ch_read(uint8_t chNum);
};

#endif