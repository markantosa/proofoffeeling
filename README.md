# Proof of Feeling

**Affective Core AC-7** — a speculative mechatronic artefact: an artificial
robotic heart that makes a humanoid AI's internal state visible through
pulse, motion, and light.

<p align="center">
  <img src="assets/Proof of Feeling Render.png" alt="Proof of Feeling — rendered artificial heart with exposed PCB, mounted on its display stand" width="520">
</p>

See [`docs/design-reference.md`](docs/design-reference.md) for the full
project concept and design brief.

## Repository Layout

```
.
├── docs/               Project-wide documentation
│   ├── design-reference.md          Project context/story + physical prototype
│   └── pcb-design-reference.md      KiCad 10 PCB design reference
├── Firmware/           ESP32-C6-Supermini embedded firmware
│   ├── src/             Servo sweep, LED chase, proximity speed, eye animation, BLE telemetry
│   └── include/          Pin assignments, tunables, BLE UUIDs
├── dashboard/          BLE debug dashboard — ASCII face/sonar/heartbeat, Web Bluetooth (Chrome/Edge only)
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
* BLE telemetry (built-in radio) — feeds the debug dashboard, see below
* Custom single-layer milled PCB (KiCad, `Electronics/`)
* Powered via 5V DC barrel plug

<table>
  <tr>
    <td align="center"><img src="assets/PCB render.jpg" alt="3D render of the populated AC-7 main board" width="300"><br><sub>Populated board render</sub></td>
    <td align="center"><img src="assets/PCB trace view.jpg" alt="AC-7 PCB routing — single-layer milled board, B.Cu traces" width="300"><br><sub>B.Cu routing</sub></td>
    <td align="center"><img src="assets/PCB Milling.jpg" alt="AC-7 board mid-fabrication on a desktop CNC isolation mill" width="300"><br><sub>Desktop CNC isolation milling</sub></td>
  </tr>
</table>

## Getting Started

* Project context, story, and what a viewer experiences: [`docs/design-reference.md`](docs/design-reference.md)
* Firmware build/flash instructions: [`Firmware/README.md`](Firmware/README.md)
* PCB design reference: [`docs/pcb-design-reference.md`](docs/pcb-design-reference.md)
* KiCad project: [`Electronics/EPD 3D G6.kicad_pro`](Electronics/EPD%203D%20G6.kicad_pro)
* BLE debug dashboard: [`dashboard/index.html`](dashboard/index.html) — open directly in Chrome or Edge (desktop/Android; Web Bluetooth isn't supported in Firefox/Safari), click **Connect**, pick "AC-7"
