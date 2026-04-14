#ifndef PT_SENSORS_H
#define PT_SENSORS_H

#include <Wire.h>
#include <Adafruit_ADS1X15.h>

class ptSensors {
  private:
    int _SCL;
    int _SDA;
    uint8_t _addr; 
    Adafruit_ADS1115 _ptSensors;

    int16_t  _results[4];
    uint8_t  _currentCh;
    bool     _converting;

  public:
    ptSensors(int SCL, int SDA);
    void begin(uint8_t addr);
    void startNext();
    void poll();
    float ch_read(uint8_t chNum);
};

#endif