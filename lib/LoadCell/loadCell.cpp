#include "loadCell.h"

loadCell::loadCell(int DOUT, int SCK) {
    _DOUT = DOUT;
    _SCK = SCK;
    _lastReading = 0.0f;
}

void loadCell::begin() {
    _loadCell.begin(_DOUT, _SCK);
    if (_DOUT == PC4) {
        _loadCell.set_scale(-45228.2617f);
    } else {
        _loadCell.set_scale(-45160.8671f);
    }
    _loadCell.tare();
}

float loadCell::lc_read() {
    // Only read if a new sample is ready, otherwise return cached value.
    // HX711 produces a sample ~every 12.5ms (80 SPS), so most calls skip.
    if (_loadCell.is_ready()) {
        long raw = _loadCell.read();
        _lastReading = (raw - _loadCell.get_offset()) / _loadCell.get_scale();
    }
    return _lastReading;
}