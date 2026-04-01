#ifndef LOAD_CELL_H
#define LOAD_CELL_H

#include "HX711.h"

class loadCell {
  private:
    int _DOUT;
    int _SCK;
    HX711 _loadCell;

  public:
    loadCell(int DOUT, int SCK);
    void begin();
    float lc_read();
};

#endif