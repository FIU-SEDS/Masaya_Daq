#include "loadCell.h"

loadCell::loadCell(int DOUT, int SCK) {
    _DOUT = DOUT;
    _SCK = SCK;
    _lastReading = 0.0f;
}

void loadCell::begin() {
    _loadCell.begin(_DOUT, _SCK);
    if(_DOUT == PC4){
        _loadCell.set_scale(-45228.2617f);
    } else {
        _loadCell.set_scale(-45160.8671f);
    }
    
    _loadCell.tare();
}

void loadCell::poll() {
    if (_loadCell.is_ready()) {
        long raw = _loadCell.read();  
        _lastReading = (raw - _loadCell.get_offset()) / _loadCell.get_scale();
    }
}

float loadCell::lc_read() {
    return _lastReading;
}