#include "tcSensors.h"

tcSensors::tcSensors(int SCL, int SDA) {
    _SCL = SCL;
    _SDA = SDA;
}

void tcSensors::begin() {
    Wire.setSCL(_SCL);
    Wire.setSDA(_SDA);
    Wire.setClock(400000);
    Wire.begin();
    _tcSensors.begin(0x48, &Wire);
    _tcSensors.setDataRate(RATE_ADS1115_860SPS);
}

float tcSensors::ch_read(uint8_t chNum) {
    int16_t ch = _tcSensors.readADC_SingleEnded(chNum);
    return ((_tcSensors.computeVolts(ch) - 1.25)) / 0.005;
}
