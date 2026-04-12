# Masaya-DAQ

STM32 firmware for the Masaya liquid rocket engine test stand data acquisition and valve control system. Runs on an STM32F4 series board using the Arduino/PlatformIO framework, communicating with the ground station over UDP via a CH9121 Ethernet module.

---

## Overview

The DAQ firmware handles all real-time hardware I/O on the test stand:
- Non-blocking ADC polling of 8 pressure transducers and 3 thermocouples via ADS1115s (I²C, 860 SPS)
- Non-blocking load cell polling via 2× HX711s (80Hz)
- Servo valve control with configurable closing speeds (instant, moderate, slow)
- Solenoid valve control with automated burping for leak testing
- 40-byte telemetry frames transmitted at 100Hz over UART → CH9121 → UDP
- 3-byte command frame reception for valve actuation from the ground station

---

## Hardware

| Component | Part | Interface | Address / Pin |
|-----------|------|-----------|---------------|
| Microcontroller | STM32F4 | — | — |
| Ethernet Module | CH9121 | UART | PA9 (TX), PA10 (RX) |
| Pressure Transducers (PT0–PT3) | ADS1115 | I²C | 0x48, PB6/PB7 |
| Pressure Transducers (PT4–PT7) | ADS1115 | I²C | 0x4A, PB6/PB7 |
| Thermocouples (TC0–TC2) | ADS1115 | I²C | 0x4B, PB6/PB7 |
| Load Cell — IPA (LC0) | HX711 | GPIO | DOUT: PC4, SCK: PC5 |
| Load Cell — N2O (LC1) | HX711 | GPIO | DOUT: PB0, SCK: PC5 |
| Servo Valve — SEV01F | Servo | PWM | PA5 |
| Servo Valve — SEV02F | Servo | PWM | PA1 |
| Servo Valve — SEV03OX | Servo | PWM | PA4 |
| Servo Valve — SEV04OX | Servo | PWM | PA3 |
| Solenoid — SOV01F | Solenoid | GPIO | PC6 |
| Solenoid — SOV02OX | Solenoid | GPIO | PD7 |

**Pressure transducer conversion:** `(V - 0.5) / 4.0 × 1500.0` PSI (0.5–4.5V, 0–1500 PSI range)

**Thermocouple conversion:** `(V - 1.25) / 0.005` °C

**Load cell scale factor:** `-925.74` (calibrated, set in firmware)

---

## Pin Summary

```
PA1   — SEV02F servo PWM
PA3   — SEV04OX servo PWM
PA4   — SEV03OX servo PWM
PA5   — SEV01F servo PWM
PA9   — UART TX → CH9121
PA10  — UART RX ← CH9121
PB0   — HX711 LC1 DOUT
PB6   — I²C SCL (all ADS1115s)
PB7   — I²C SDA (all ADS1115s)
PC4   — HX711 LC0 DOUT
PC5   — HX711 shared SCK
PC6   — SOV01F solenoid
PD7   — SOV02OX solenoid
```

---

## Communication Protocol

### Telemetry Frame (STM32 → Ground Station)
Transmitted every **10ms** over UART at 115200 baud. 40 bytes total.

| Bytes | Field | Encoding |
|-------|-------|----------|
| 0 | Header `0xFF` | — |
| 1–2 | PT0 | uint16 = value × 10 |
| 3–4 | PT1 | uint16 = value × 10 |
| 5–6 | PT2 | uint16 = value × 10 |
| 7–8 | PT3 | uint16 = value × 10 |
| 9–10 | PT4 | uint16 = value × 10 |
| 11–12 | PT5 | uint16 = value × 10 |
| 13–14 | PT6 | uint16 = value × 10 |
| 15–16 | PT7 | uint16 = value × 10 |
| 17–18 | TC0 | uint16 = value × 10 |
| 19–20 | TC1 | uint16 = value × 10 |
| 21–22 | TC2 | uint16 = value × 10 |
| 23–24 | LC0 | uint16 = value × 10 |
| 25–26 | LC1 | uint16 = value × 10 |
| 27–28 | SEV01F position | uint16 = degrees × 10 |
| 29–30 | SEV02F position | uint16 = degrees × 10 |
| 31–32 | SEV03OX position | uint16 = degrees × 10 |
| 33–34 | SEV04OX position | uint16 = degrees × 10 |
| 35–36 | SOV01F position | uint16 = degrees × 10 |
| 37–38 | SOV02OX position | uint16 = degrees × 10 |
| 39 | XOR checksum | XOR of bytes 0–38 |

All sensors encoded as `uint16_t = value * 10` for 1 decimal place precision. Range 0–6553.5. **Note: negative values are clipped to 0.**

### Command Frame (Ground Station → STM32)
3 bytes received over UART from CH9121.

| Byte | Field |
|------|-------|
| 0 | Device ID |
| 1 | Command |
| 2 | XOR checksum (byte 0 XOR byte 1) |

**Device IDs:**

| ID | Device |
|----|--------|
| 0 | SEV01F |
| 1 | SEV02F |
| 2 | SEV03OX |
| 3 | SEV04OX |
| 4 | SOV01F |
| 5 | SOV02OX |

**Commands:**

| Command | Value | Valid For | Description |
|---------|-------|-----------|-------------|
| CMD_OPEN | 0x01 | Servos, Solenoids | Open immediately |
| CMD_CLOSE_MOD | 0x02 | Servos only | Close at ~0.3°/3ms (≈0.6s) |
| CMD_CLOSE_SLOW | 0x03 | Servos only | Close at ~0.3°/6ms (≈1s) |
| CMD_CLOSE | 0x04 | Servos, Solenoids | Close immediately, stops burping |
| CMD_LLT | 0x06 | Solenoids | Start localized leak test (10 PSI threshold) |
| CMD_TLT | 0x07 | Solenoids | Start total leak test (900 PSI threshold) |

---

## Servo Valve Behavior

Servos move between 0° (closed) and 90° (open).

- **Open:** Jumps directly to 90°
- **Close (immediate):** Jumps directly to 0°
- **Close Moderate:** Steps from 90° to 0° at 1°/3ms (~0.6 seconds total)
- **Close Slow:** Steps from 90° to 0° at 1°/6ms (~1 second total)

`servoValve.update()` must be called every loop iteration to drive the step-close motion.

---

## Leak Test / Burping

When a LLT or TLT command is received by a solenoid, the firmware enters burping mode. Each solenoid operates independently:

- If its associated PT reads **above** the pressure threshold → toggles open/close every 100ms
- If its associated PT reads **below** the threshold → holds closed

**Solenoid → PT mapping:**

| Solenoid | Watches | Physical Sensor |
|----------|---------|-----------------|
| SOV01F (ID 4) | pts_a CH1 | PT-02-F |
| SOV02OX (ID 5) | pts_b CH2 | PT-08-OX |

Both solenoids share a single `pressureThreshold` set by the last LLT/TLT command received. Send `CMD_CLOSE` to either solenoid to stop burping globally.

---

## Library Structure

```
Masaya-DAQ/
├── src/
│   └── main.cpp          # Main loop, frame parsing, telemetry TX, burp logic
├── lib/
│   ├── servoValve/
│   │   ├── servoValve.h
│   │   └── servoValve.cpp  # PWM servo with stepped close modes
│   ├── solenoid/
│   │   ├── solenoid.h
│   │   └── solenoid.cpp    # Digital solenoid open/close
│   ├── ptSensors/
│   │   ├── ptSensors.h
│   │   └── ptSensors.cpp   # ADS1115 non-blocking poller, PT conversion
│   ├── tcSensors/
│   │   ├── tcSensors.h
│   │   └── tcSensors.cpp   # ADS1115 non-blocking poller, TC conversion
│   ├── loadCell/
│   │   ├── loadCell.h
│   │   └── loadCell.cpp    # HX711 non-blocking poller
│   └── CD4051BE/
│       ├── CD4051BE.h
│       └── CD4051BE.cpp    # 8-ch analog mux driver (reserved, unused)
└── platformio.ini
```

---

## Build & Flash

This project uses [PlatformIO](https://platformio.org/).

**Dependencies (auto-resolved by PlatformIO):**
- `Adafruit ADS1X15`
- `HX711` (bogde)
- `Servo` (STM32 compatible)

**Build:**
```bash
pio run
```

**Flash:**
```bash
pio run --target upload
```

**Monitor serial (debug):**
```bash
pio device monitor --baud 115200
```

---

## Loop Design

The main loop is fully non-blocking. Nothing in `loop()` waits or delays:

| Task | Mechanism |
|------|-----------|
| PT/TC sampling | ADS1115 conversion-complete polling, result cached per channel |
| Load cell sampling | HX711 `is_ready()` check, result cached |
| Servo stepping | `millis()` delta per step |
| Serial RX | `available()` check, byte-by-byte into fixed buffer |
| Telemetry TX | `millis()` delta, 10ms interval |
| Burp toggling | `millis()` delta, 100ms interval per solenoid |

---

## Network Configuration (CH9121)

The CH9121 must be pre-configured (via its own configuration utility) to match the ground station:

| Parameter | Value |
|-----------|-------|
| CH9121 IP | `192.168.1.200` |
| CH9121 UDP TX port | `5005` (sends telemetry to ground station) |
| CH9121 UDP RX port | `2000` (receives commands from ground station) |
| UART baud | `115200` |
| Mode | UDP |

---

## Known Limitations

- Negative sensor values (load cell tare drift, sub-ambient thermocouple readings) are clipped to 0 in the uint16 fixed-point encoding and will appear as 0.0 on the ground station
- `burping = false` on `CMD_CLOSE` is global — closing either solenoid stops burping on both
- The serial RX parser has no sync byte; a partial frame received on startup or reconnect will be misaligned until the checksum rejects it and the next valid frame re-syncs
- `tare()` in `loadCell::begin()` blocks for approximately 10 samples at 80Hz (~125ms per cell) during `setup()` — this is acceptable as it only runs once at boot
