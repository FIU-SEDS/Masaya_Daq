#include "loadCell.h"

loadCell::loadCell(int DOUT, int SCK) {
    _DOUT = DOUT;
    _SCK = SCK;
    _lastReading = 0.0f;
}

void loadCell::begin() {
    _loadCell.begin(_DOUT, _SCK);
    _loadCell.set_scale(-925.74);
    _loadCell.tare();
}

void loadCell::poll() {
    if (_loadCell.is_ready()) {
        _lastReading = _loadCell.get_units(1);
    }
}

float loadCell::lc_read() {
    return _lastReading;
}