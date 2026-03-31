#include "ptSensors.h"

ptSensors::ptSensors(int SCL, int SDA) {
    _SCL = SCL;
    _SDA = SDA;
}

void ptSensors::begin() {
    Wire.setSCL(_SCL);
    Wire.setSDA(_SDA);
    Wire.setClock(400000);
    Wire.begin();
    _ptSensors.begin(0x48, &Wire);
    _ptSensors.setDataRate(RATE_ADS1115_860SPS);
}

float ptSensors::ch_read(uint8_t chNum) {
    int16_t ch = _ptSensors.readADC_SingleEnded(chNum);
    return ((_ptSensors.computeVolts(ch) - 0.5) / 4.0) * 1500.0;
}
