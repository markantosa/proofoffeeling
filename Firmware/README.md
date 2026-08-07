# Affective Core AC-7 — Firmware

Embedded firmware and electronics documentation for **Proof of Feeling**, a
speculative mechatronic artefact. The Affective Core is an artificial
robotic heart that makes a humanoid AI's internal state visible through
pulse, motion, and light.

See [`../docs/design-reference.md`](../docs/design-reference.md) for the
full project concept and design brief.

## Hardware

* ESP32-C6-Supermini
* 2x MG995 servos, 5V (continuous 0–180° sweep, mirrored)
* HC-SR04 ultrasonic sensor (proximity → continuous speed control)
* 5V DC barrel plug power input
* 1x momentary button — wired on the board (BTN_SIG/SW1), not read by
  the current firmware revision
* 8x auxiliary LEDs, chase top-to-bottom ("pulse point" glow array; 4
  PWM-faded, 4 on/off — see PCB doc for the LEDC channel budget)
* I2C OLED (SSD1306-class), 4-pin header — always-on eye animation,
  blinks at random intervals
* BLE telemetry (built-in radio, no extra hardware) — broadcasts
  distance + speed multiplier for the debug dashboard, see below
* Custom single-layer milled PCB — see
  [`../docs/pcb-design-reference.md`](../docs/pcb-design-reference.md)

## Repository Layout

This folder holds only the firmware; project-wide docs live in
[`../docs/`](../docs/) at the repo root, and the BLE debug dashboard
lives in [`../dashboard/`](../dashboard/).

```
.
├── platformio.ini          PlatformIO project config
├── include/
│   ├── config.h              Pin assignments and tunables
│   └── ble_telemetry.h       BLE service/characteristic declarations
└── src/
    ├── main.cpp               Servo sweep, LED chase, proximity speed, eye animation
    └── ble_telemetry.cpp      BLE GATT server — notifies distance + speed to the dashboard
```

## Build & Flash

Requires [PlatformIO](https://platformio.org/).

```
pio run              # build
pio run -t upload    # flash to ESP32-C6-Supermini
pio device monitor    # serial monitor, 115200 baud
```

## Behaviour (v3.0)

Continuous, proximity-driven — no discrete interaction-mode state
machine in this revision (an earlier v2.0 draft had one; it's gone):

| Element | Behaviour |
|---------|-----------|
| Aux LEDs | Chase top-to-bottom in a wrapping pulse, continuously |
| Servos | Sweep 0°→180°→0° continuously, mirrored between the two |
| Speed | Both the LED chase and servo sweep speed up continuously as an object gets closer to the HC-SR04 — no discrete thresholds |
| OLED | Always displays an eye; blinks at random intervals, independent of the above |
| BLE | Notifies distance + speed multiplier (~10Hz) whenever a client is connected; harmless if nothing's listening |

See [`../docs/design-reference.md`](../docs/design-reference.md) §2 for
what this looks like from a viewer's side. The button (`BTN_SIG`) is
wired but currently unused — a candidate for a future interaction if one
gets designed.

## BLE Debug Dashboard

The firmware advertises as **AC-7** and exposes one GATT service (see
`include/ble_telemetry.h` for the UUIDs) that notifies an 8-byte struct
— `distanceCm` + `speedMultiplier` — at ~10Hz whenever a client is
connected. [`../dashboard/index.html`](../dashboard/index.html) is a
self-contained Web Bluetooth client for it: open it in Chrome or Edge
(desktop or Android — Web Bluetooth isn't supported in Firefox/Safari),
click **Connect**, and it renders:

* an ASCII face (sleeping / happy / stressed, thresholded on distance)
* an ASCII sonar sweep, pinging when something's in range
* a heartbeat thump + visual pulse, tempo-matched (not phase-locked) to
  the servo/LED speed

It's a debugging/demo tool, not part of the physical artefact — nothing
in `main.cpp`'s actual behaviour depends on whether a dashboard is
connected.
