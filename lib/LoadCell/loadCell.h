#ifndef LOAD_CELL_H
#define LOAD_CELL_H

#include "HX711.h"

class loadCell {
  private:
    int _DOUT;
    int _SCK;
    HX711 _loadCell;
    float _lastReading;

  public:
    loadCell(int DOUT, int SCK);
    void begin();
    void poll();        // Call every loop() iteration
    float lc_read();    // Returns last cached value, non-blocking
};

#endif