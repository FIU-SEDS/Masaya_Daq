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
// Servo IDs:   0–3
// Solenoid IDs: 4–5

// --- Commands ---
#define CMD_OPEN        0x01
#define CMD_CLOSE_MOD   0x02
#define CMD_CLOSE_SLOW  0x03
#define CMD_CLOSE       0x04
#define CMD_LLT         0x06   // Localized leak test (10 PSI threshold)
#define CMD_TLT         0x07   // Total leak test (900 PSI threshold)

// --- Telemetry ---
// At ~50ms we have plenty of headroom for blocking ADC reads.
// Each ADS1115 read at 860 SPS = ~1.2ms. 11 reads = ~14ms total.
// Plus HX711 occasional ~1ms reads. Comfortable margin.
#define TELEM_INTERVAL_MS  50
#define NUM_PT             8
#define NUM_TC             3
#define NUM_LC             2
#define NUM_VALVES_TELEM   6
#define TELEM_HEADER       0xFF
#define NUM_SENSORS        (NUM_PT + NUM_TC + NUM_LC + NUM_VALVES_TELEM)
#define TELEM_LEN          (1 + NUM_SENSORS * 2 + 1)

// --- Servos ---
servoValve servos[NUM_SERVOS] = {
    servoValve(PA5), // SEV-01F  180° servo, closed=3, open=98
    servoValve(PA1), // SEV-02F  270° servo, closed=9, open=74
    servoValve(PA4), // SEV-03OX 270° servo, closed=2.5, open=67.5
    servoValve(PA3)  // SEV-04OX 180° servo, closed=7, open=100
};

// --- Solenoids ---
solenoid solenoids[NUM_SOLENOIDS] = {
    solenoid(PC6),   // SOV-01F
    solenoid(PD7),   // SOV-02OX
};

// --- Sensors ---
ptSensors pts_a(PB6, PB7);  // PTs 0-3 @ 0x4A
ptSensors pts_b(PB6, PB7);  // PTs 4-7 @ 0x48
tcSensors tcs(PB6, PB7);    // TCs 0-2 @ 0x4B
loadCell  lc0(PC4, PC5);
loadCell  lc1(PB0, PC5);

// --- RX Buffer ---
uint8_t rxBuf[FRAME_LEN];
uint8_t rxIdx = 0;

// --- Timers ---
uint32_t lastTelemTime = 0;

// --- Burping ---
bool     burping            = false;
bool     burpState[NUM_SOLENOIDS] = {false, false};
uint32_t lastBurpTime[NUM_SOLENOIDS] = {0, 0};
float    pressureThreshold  = 0.0f;

// SOL0 → pts_a ch1, SOL1 → pts_b ch2 (PT6)
const uint8_t SOL_PT_ADC[NUM_SOLENOIDS] = {0, 1};
const uint8_t SOL_PT_CH[NUM_SOLENOIDS]  = {1, 2};

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

    if (id < NUM_SERVOS) {
        switch (cmd) {
            case CMD_OPEN:       servos[id].open();          break;
            case CMD_CLOSE_MOD:  servos[id].closeModerate(); break;
            case CMD_CLOSE_SLOW: servos[id].closeSlow();     break;
            case CMD_CLOSE:      servos[id].close();         break;
            default: return false;
        }
        return true;
    }

    uint8_t solId = id - NUM_SERVOS;
    if (solId < NUM_SOLENOIDS) {
        switch (cmd) {
            case CMD_OPEN:  solenoids[solId].open();                      break;
            case CMD_CLOSE: solenoids[solId].close();
                            burping = false;                              break;
            case CMD_LLT:   pressureThreshold = 10.0f;  burping = true;   break;
            case CMD_TLT:   pressureThreshold = 900.0f; burping = true;   break;
            default: return false;
        }
        return true;
    }

    return false;
}

void sendTelemetry() {
    uint8_t frame[TELEM_LEN];
    uint8_t idx = 0;

    frame[idx++] = TELEM_HEADER;

    auto encodeFloat = [&](float val) {
        uint16_t encoded = (uint16_t)constrain(val * 10.0f, 0, 65535);
        frame[idx++] = (encoded >> 8) & 0xFF;
        frame[idx++] = encoded & 0xFF;
    };

    // Read all PT channels (blocking, ~1.2ms each at 860 SPS)
    for (uint8_t ch = 0; ch < 4; ch++) encodeFloat(pts_a.ch_read(ch));
    // for (uint8_t ch = 0; ch < 4; ch++) encodeFloat(0);



    for (uint8_t ch = 0; ch < 4; ch++) encodeFloat(pts_b.ch_read(ch));
    // for (uint8_t ch = 0; ch < 4; ch++) encodeFloat(0);


    // Read all TC channels (blocking)
    for (uint8_t ch = 0; ch < 3; ch++) encodeFloat(tcs.ch_read(ch));
    // for (uint8_t ch = 0; ch < 3; ch++) encodeFloat(0);


    // Load cells (only blocks briefly when sample is ready)
    encodeFloat(lc0.lc_read());
    encodeFloat(lc1.lc_read());

    // Servo positions
    for (uint8_t i = 0; i < NUM_SERVOS; i++) encodeFloat(servos[i].getPosition());

    // Solenoid positions
    for (uint8_t i = 0; i < NUM_SOLENOIDS; i++) encodeFloat(solenoids[i].getPosition());

    frame[idx] = calcChecksum(frame, idx);
    CommSerial.write(frame, TELEM_LEN);
}

void setup() {
    CommSerial.begin(115200);

    Wire.setSCL(PB6);
    Wire.setSDA(PB7);
    Wire.setClock(100000);   // Drop to 100kHz — more reliable with multiple devices
    Wire.begin();

    pts_a.begin(0x4A);
    pts_b.begin(0x48);
    tcs.begin(0x4B);

    lc0.begin();
    lc1.begin();

    for (auto &s : servos)    s.begin();
    for (auto &s : solenoids) s.begin();
}

void loop() {
    // Servo position updates (for slow-close moves)
    for (auto &s : servos) s.update();

    // Non-blocking serial receive
    while (CommSerial.available()) {
        rxBuf[rxIdx++] = CommSerial.read();
        if (rxIdx >= FRAME_LEN) {
            processFrame(rxBuf);
            rxIdx = 0;
        }
    }

    // Periodic telemetry (blocking ADC reads happen here)
    if (millis() - lastTelemTime >= TELEM_INTERVAL_MS) {
        lastTelemTime = millis();
        sendTelemetry();
    }

    // Burp control: per-solenoid, sensor-gated, 100ms toggle
    if (burping) {
        // NOTE: This reads sensors AGAIN — adds another ~2.4ms blocking.
        // If this causes issues, cache the last telemetry reading and use that.
        float ptReading[NUM_SOLENOIDS];
        ptReading[0] = pts_a.ch_read(SOL_PT_CH[0]);
        ptReading[1] = pts_b.ch_read(SOL_PT_CH[1]);

        for (uint8_t i = 0; i < NUM_SOLENOIDS; i++) {
            if (ptReading[i] > pressureThreshold) {
                if (millis() - lastBurpTime[i] >= 100) {
                    lastBurpTime[i] = millis();
                    burpState[i] = !burpState[i];
                    burpState[i] ? solenoids[i].open() : solenoids[i].close();
                }
            } else {
                if (burpState[i]) {
                    burpState[i] = false;
                    solenoids[i].close();
                }
            }
        }
    } else {
        for (uint8_t i = 0; i < NUM_SOLENOIDS; i++) {
            if (burpState[i]) {
                burpState[i] = false;
                solenoids[i].close();
            }
        }
    }
}