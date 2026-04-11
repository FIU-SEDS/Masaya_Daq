#include "tcSensors.h"

tcSensors::tcSensors(int SCL, int SDA) {
    _SCL = SCL;
    _SDA = SDA;
    _currentCh = 0;
    _converting = false;
    memset(_results, 0, sizeof(_results));
}

void tcSensors::begin(uint8_t addr) {
    _tcSensors.begin(addr, &Wire);
    _tcSensors.setDataRate(RATE_ADS1115_860SPS);
    startNext();
}

void tcSensors::startNext() {
    _tcSensors.startADCReading(MUX_BY_CHANNEL[_currentCh], /*continuous=*/false);
    _converting = true;
}

void tcSensors::poll() {
    if (!_converting) return;
    if (_tcSensors.conversionComplete()) {
        _results[_currentCh] = _tcSensors.getLastConversionResults();
        _currentCh = (_currentCh + 1) % 3;  // Only 3 TC channels
        startNext();
    }
}

float tcSensors::ch_read(uint8_t chNum) {
    return ((_tcSensors.computeVolts(_results[chNum]) - 1.25)) / 0.005;
}