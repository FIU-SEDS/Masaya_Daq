#include "ptSensors.h"

ptSensors::ptSensors(int SCL, int SDA) {
    _SCL = SCL;
    _SDA = SDA;
    _currentCh = 0;
    _converting = false;
    memset(_results, 0, sizeof(_results));
}

void ptSensors::begin(uint8_t addr) {
    _ptSensors.begin(addr, &Wire);
    _ptSensors.setDataRate(RATE_ADS1115_860SPS);
    startNext();
}

void ptSensors::startNext() {
    _ptSensors.startADCReading(MUX_BY_CHANNEL[_currentCh], /*continuous=*/false);
    _converting = true;
}

void ptSensors::poll() {
    if (!_converting) return;
    if (_ptSensors.conversionComplete()) {
        _results[_currentCh] = _ptSensors.getLastConversionResults();
        _currentCh = (_currentCh + 1) % 4;
        startNext();
    }
}

float ptSensors::ch_read(uint8_t chNum) {
    return ((_ptSensors.computeVolts(_results[chNum]) - 0.5) / 4.0) * 1500.0;
}