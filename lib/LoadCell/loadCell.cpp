#include "loadCell.h"

loadCell::loadCell(int DOUT, int SCK) {
    _DOUT = DOUT;
    _SCK = SCK;
}

void loadCell::begin() {
    _loadCell.begin(_DOUT, _SCK);
    _loadCell.set_scale(-925.74);
    _loadCell.tare();
}

float loadCell::lc_read() {
    return _loadCell.get_units();
}
