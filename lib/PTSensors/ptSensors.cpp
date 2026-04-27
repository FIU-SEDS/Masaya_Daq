#include "ptSensors.h"

ptSensors::ptSensors(int SCL, int SDA) {
    _SCL = SCL;
    _SDA = SDA;
}

void ptSensors::begin(uint8_t addr) {
    _addr = addr;
    _ptSensors.begin(addr, &Wire);
    _ptSensors.setDataRate(RATE_ADS1115_860SPS);
}

float ptSensors::ch_read(uint8_t chNum) {
    // Blocking single-shot read — takes ~1.2ms at 860 SPS
    int16_t raw = _ptSensors.readADC_SingleEnded(chNum);
    float volts = _ptSensors.computeVolts(raw);

    // pts_a (0x4A): ch0 and ch1 → 3000 PSI range
    // pts_b (0x48): ch2 and ch3 → 3000 PSI range
    if ((_addr == 0x4A && chNum <= 1) || (_addr == 0x48 && chNum >= 2)) {
        return ((volts - 0.5f) / 4.0f) * 3000.0f;
    }

    return ((volts - 0.5f) / 4.0f) * 1500.0f;
}