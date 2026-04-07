#include <Arduino.h>
#include <Wire.h>
#include "servoValve.h"
#include "solenoid.h"
#include "ptSensors.h"
#include "tcSensors.h"
#include "loadCell.h"

HardwareSerial CommSerial(PA10, PA9);

// --- Frame format (RX): [DEVICE_ID] [CMD] [CHECKSUM] ---
#define FRAME_LEN       3

// --- Device IDs ---
#define NUM_SERVOS      4
#define NUM_SOLENOIDS   2
// Servo IDs:   0–4
// Solenoid IDs: 5–6

// --- Commands ---
#define CMD_OPEN_FAST   0x01
#define CMD_OPEN_MOD    0x02
#define CMD_OPEN_SLOW   0x03
#define CMD_CLOSE       0x04

// --- Telemetry ---
#define TELEM_INTERVAL_MS  50
#define NUM_PT             8    // 2x ADS1115, 4ch each
#define NUM_TC             3    // 1x ADS1115, 3ch used
#define NUM_LC             2    // 2x HX711
// TX frame: [0xFF] [13x floats as 2-byte fixed point] [CHECKSUM] = 28 bytes
// Each sensor encoded as uint16_t (value * 10 for 1 decimal place)
#define TELEM_HEADER    0xFF
#define NUM_SENSORS     (NUM_PT + NUM_TC + NUM_LC)  // 13
#define TELEM_LEN       (1 + NUM_SENSORS * 2 + 1)  // 28 bytes

// --- Servos ---
servoValve servos[NUM_SERVOS] = {
    servoValve(PA5),
    servoValve(PA1),
    servoValve(PA2),
    servoValve(PA3)
};

// --- Solenoids ---
solenoid solenoids[NUM_SOLENOIDS] = {
    solenoid(PC6),
    solenoid(PC8),
};

// --- Sensors ---
// PT: ADS1115 at 0x48 (4ch) and 0x49 (4ch)
// TC: ADS1115 at 0x4A (3ch used)
// Note: Wire.begin() called once in setup(); libs must not call it again
ptSensors pts_a(PC10, PC11);  // 0x48 - PTs 0-3
ptSensors pts_b(PC10, PC11);  // 0x49 - PTs 4-7
tcSensors tcs(PC10, PC11);    // 0x4A - TCs 0-2
loadCell lc0(PC4, PB1);
loadCell lc1(PB0, PB1);

// --- RX Buffer ---
uint8_t rxBuf[FRAME_LEN];
uint8_t rxIdx = 0;

// --- Telemetry timer ---
uint32_t lastTelemTime = 0;

// -------------------------------------------------------

uint8_t calcChecksum(uint8_t *buf, uint8_t len) {
    uint8_t sum = 0;
    for (uint8_t i = 0; i < len; i++) sum ^= buf[i];
    return sum;
}

bool processFrame(uint8_t *frame) {
    if (calcChecksum(frame, FRAME_LEN - 1) != frame[FRAME_LEN - 1])
        return false;

    uint8_t id  = frame[0];
    uint8_t cmd = frame[1];

    // Servo valve (IDs 0–4)
    if (id < NUM_SERVOS) {
        switch (cmd) {
            case CMD_OPEN_FAST: servos[id].open();         break;
            case CMD_OPEN_MOD:  servos[id].openModerate(); break;
            case CMD_OPEN_SLOW: servos[id].openSlow();     break;
            case CMD_CLOSE:     servos[id].close();        break;
            default: return false;
        }
        return true;
    }

    // Solenoid (IDs 5–6)
    uint8_t solId = id - NUM_SERVOS;
    if (solId < NUM_SOLENOIDS) {
        switch (cmd) {
            case CMD_OPEN_FAST: solenoids[solId].open();  break;
            case CMD_CLOSE:     solenoids[solId].close(); break;
            default: return false;  // MOD/SLOW invalid for solenoids
        }
        return true;
    }

    return false;  // Unknown ID
}

void sendTelemetry() {
    uint8_t frame[TELEM_LEN];
    uint8_t idx = 0;

    frame[idx++] = TELEM_HEADER;

    // Helper: encode float as uint16_t (×10, 1 decimal place, 0–6553.5 range)
    auto encodeFloat = [&](float val) {
        uint16_t encoded = (uint16_t)constrain(val * 10.0f, 0, 65535);
        frame[idx++] = (encoded >> 8) & 0xFF;
        frame[idx++] = encoded & 0xFF;
    };

    // PTs (8 channels)
    for (uint8_t ch = 0; ch < 4; ch++) encodeFloat(pts_a.ch_read(ch));
    for (uint8_t ch = 0; ch < 4; ch++) encodeFloat(pts_b.ch_read(ch));

    // TCs (3 channels)
    for (uint8_t ch = 0; ch < 3; ch++) encodeFloat(tcs.ch_read(ch));

    // Load cells (2)
    encodeFloat(lc0.lc_read());
    encodeFloat(lc1.lc_read());

    // Checksum over everything except the last byte slot
    frame[idx] = calcChecksum(frame, idx);

    CommSerial.write(frame, TELEM_LEN);
}

void setup() {
    CommSerial.begin(921600);

    // I2C bus — init once for all ADS1115 devices
    Wire.setSCL(PC10);
    Wire.setSDA(PC11);
    Wire.setClock(400000);
    Wire.begin();

    // Sensors — pass address explicitly (your libs will need a minor tweak
    // to accept address in begin() rather than calling Wire.begin() themselves)
    pts_a.begin(0x48);
    pts_b.begin(0x49);
    tcs.begin(0x4A);

    lc0.begin();
    lc1.begin();

    // Servos & solenoids
    for (auto &s : servos)    s.begin();
    for (auto &s : solenoids) s.begin();
}

void loop() {
    // Update servo positions (required for openModerate/openSlow stepping)
    for (auto &s : servos) s.update();

    // Non-blocking serial receive
    while (CommSerial.available()) {
        rxBuf[rxIdx++] = CommSerial.read();
        if (rxIdx >= FRAME_LEN) {
            processFrame(rxBuf);
            rxIdx = 0;
        }
    }

    // Periodic telemetry
    if (millis() - lastTelemTime >= TELEM_INTERVAL_MS) {
        lastTelemTime = millis();
        sendTelemetry();
    }
}