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
* Custom single-layer milled PCB — see
  [`../docs/pcb-design-reference.md`](../docs/pcb-design-reference.md)

## Repository Layout

This folder holds only the firmware; project-wide docs live in
[`../docs/`](../docs/) at the repo root.

```
.
├── platformio.ini          PlatformIO project config
├── include/
│   └── config.h             Pin assignments and tunables
└── src/
    └── main.cpp              Servo sweep, LED chase, proximity speed, eye animation
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

See [`../docs/design-reference.md`](../docs/design-reference.md) §2 for
what this looks like from a viewer's side. The button (`BTN_SIG`) is
wired but currently unused — a candidate for a future interaction if one
gets designed.
