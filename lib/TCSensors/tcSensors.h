#ifndef TC_SENSORS_H
#define TC_SENSORS_H

#include <Wire.h>
#include <Adafruit_ADS1X15.h>

class tcSensors
 {
  private:
    int _SCL;
    int _SDA;
    Adafruit_ADS1115 _tcSensors
    ;

  public:
    tcSensors
    (int SCL, int SDA);
    void begin();
    float ch_read(uint8_t chNum);
};

#endif