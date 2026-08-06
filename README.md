# Proof of Feeling

**Affective Core AC-7** — a speculative mechatronic artefact: an artificial
robotic heart that makes a humanoid AI's internal state visible through
pulse, motion, and light.

See [`docs/design-reference.md`](docs/design-reference.md) for the full
project concept and design brief.

## Repository Layout

```
.
├── docs/               Project-wide documentation
│   ├── design-reference.md          Project context/story + physical prototype
│   └── pcb-design-reference.md      KiCad 10 PCB design reference
├── Firmware/           ESP32-C6-Supermini embedded firmware
│   ├── src/             Servo sweep, LED chase, proximity speed, eye animation
│   └── include/          Pin assignments and tunables
├── dashboard/          BLE debug dashboard (ASCII face/sonar/heartbeat, Web Bluetooth)
├── Electronics/        KiCad PCB project (single-layer milled board)
└── Mechanical/         3D models (not yet included in this repo)
```

## Hardware Summary

* ESP32-C6-Supermini
* 2x MG995 servos, 5V (continuous 0–180° sweep, mirrored)
* HC-SR04 ultrasonic sensor (proximity → continuous speed control)
* 1x momentary button, wired but currently unused by firmware
* 8x auxiliary LEDs, top-to-bottom chase ("pulse point" glow array)
* I2C OLED (SSD1306-class), 4-pin header — always-on blinking eye
* Custom single-layer milled PCB (KiCad, `Electronics/`)
* Powered via 5V DC barrel plug

## Getting Started

* Project context, story, and what a viewer experiences: [`docs/design-reference.md`](docs/design-reference.md)
* Firmware build/flash instructions: [`Firmware/README.md`](Firmware/README.md)
* PCB design reference: [`docs/pcb-design-reference.md`](docs/pcb-design-reference.md)
* KiCad project: [`Electronics/EPD 3D G6.kicad_pro`](Electronics/EPD%203D%20G6.kicad_pro)
* BLE debug dashboard: [`dashboard/index.html`](dashboard/index.html) — open in Chrome/Edge, click Connect
