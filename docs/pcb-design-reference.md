# AFFECTIVE CORE AC-7 — PCB DESIGN REFERENCE

## Single-Layer Milled Board — KiCad 10

This revision matches the finalized schematic exactly (2 header rows for
the Supermini, 8 populated aux LEDs, I2C OLED header). Reference
designators follow the schematic's own numbering — see §5 for the full
scheme, in particular: **`D1`–`D11` are LEDs numbered to match their LED
net** (`D1`=`LED1` … `D11`=`LED11`), while **`D12` is the reverse-polarity
protection diode** — this is the opposite convention from earlier drafts
of this doc, so don't cross-reference old revisions.

---

# 1. BOARD OVERVIEW

Fabrication method: desktop CNC isolation milling (single-sided, copper on
bottom only — B.Cu). No vias, no plated through-holes, no top-side routing.
All components are through-hole (THT) so every connection can be made by a
hand-solderable pad reached from one side.

Because this is a single copper layer, every net that would otherwise need
to cross another net must either:

1. Route around it (preferred — and consistent with the "vein / neural
   pathway" decorative trace language in the main design doc), or
2. Jump it with a 0Ω resistor / bare wire link soldered on the top
   (component) side, treated in KiCad as a 2-pin "jumper" footprint.

Budget for 6–10 such jumpers on this board — the ESP32-C6-Supermini sits in
the middle of the layout with pins on both long edges, and the aux LED
array spans both rows, which forces several nets from one side to reach
components/headers on the other.

Board outline target: fits inside the heart shell cradle, roughly
70mm x 50mm. Confirm against the 3D-printed shell's internal mounting boss
positions before finalizing Edge.Cuts.

---

# 2. KICAD 10 PROJECT SETUP

## 2.1 New Project

* File → New Project → "AC7_MainBoard"
* Symbol library: use default KiCad symbol libraries (Device, Connector,
  Switch) — no custom symbols required, see §5.
* Footprint library: default KiCad footprints (Resistor_THT, LED_THT,
  Connector_PinHeader_2.54mm, Connector_PinSocket_2.54mm,
  Button_Switch_THT) — no custom footprint needed, see §5.1.

## 2.2 Board Setup → Physical Stackup

* File → Board Setup → Board Editor Layers
* Untick every layer except: **F.Cu is NOT used** — untick it too.
  Keep only **B.Cu** enabled as the sole copper layer, plus Edge.Cuts,
  F.Silkscreen (for your own reference only — silkscreen will not be
  printed by the mill, but keeping it helps you read the layout on screen),
  and F.Fab/F.Courtyard if you want assembly reference.
* Rationale: all components are placed on the top (F) side and soldered
  through to pads that are milled on the bottom (B.Cu) side. Routing must
  therefore happen entirely on B.Cu.

## 2.3 Design Rules (Board Setup → Design Rules → Constraints)

Tuned for a typical 0.8–1.0mm end-mill / 0.1mm–0.2mm V-bit isolation job:

| Rule                     | Value                        |
|--------------------------|-------------------------------|
| Minimum track width      | 0.4mm (0.5mm preferred for power/servo nets) |
| Minimum clearance        | 0.4mm                        |
| Minimum via              | N/A — vias disabled, single layer |
| Minimum hole size        | 0.8mm (drill), pad ≥ 1.7mm OD |
| Copper-to-edge clearance | 0.5mm                        |

Power nets (`+5V`, `SERVO_5V`, `GND`) should be widened to 0.8–1.0mm
manually where routing space allows, since both servos are MG995 (5V)
units and can draw stall currents approaching 1.5–2.5A each — the
`SERVO_5V` trace and J1/J2/J3 connector current ratings must be sized for
that.

## 2.4 Net Classes

Create two net classes under Board Setup → Design Rules → Net Classes:

* **Default** — 0.4mm track / 0.4mm clearance — signal, sensor, LED, I2C,
  and `+3V3` nets.
* **Power** — 0.8mm track / 0.4mm clearance — assign nets `+5V`, `GND`,
  `SERVO_5V`.

## 2.5 Plotting for the Mill

File → Plot, or File → Fabrication Outputs:

* Export **B.Cu** and **Edge.Cuts** as Gerber (or SVG/DXF if your CAM chain
  — e.g. FlatCAM, pcb2gcode, Candle/GRBL — prefers vector formats).
* Export drill file via File → Fabrication Outputs → Drill Files
  (Excellon, one drill file, PTH+NPTH merged since nothing is plated).
* Because copper is on B.Cu only, **mirror the export** if your CAM
  workflow assumes you're milling the layer you're looking at from the top
  — check a test cut on scrap copper-clad before committing the real board.

---

# 3. POWER ARCHITECTURE

* **Input**: 5V DC via a 5V barrel plug (J1), external supply rated at
  least 5V/3A to cover two MG995 servos' combined stall current plus the
  Supermini and LED array headroom.
* **Two separate 5V nets** — this is the one subtlety worth calling out
  clearly, since it's easy to assume everything downstream of J1 is
  protected the same way:
  * **`+5V`** — raw, straight off J1, *before* the protection diode.
    Feeds the Supermini's own 5V/VIN pin (`U2` pin 1) and the HC-SR04
    (`J4` pin 1) directly.
  * **`SERVO_5V`** — *after* D12 (the reverse-polarity diode), feeds only
    the two servo headers (J2/J3). Only the servo rail is diode-protected
    in this design; the MCU and ultrasonic sensor are not. If reverse
    polarity protection for the whole board matters to you, that's worth
    revisiting, but as drawn, only the high-current/most-exposed servo
    path gets it.
* **ESP32-C6 logic**: 3.3V, generated on-module by the Supermini's onboard
  regulator. The one net distributed off that regulator on this PCB is
  **`+3V3`** (from `U2` pin 3), which powers the I2C OLED (J5) and its
  pull-up resistors (R14/R15). An SSD1306-class OLED draws well under the
  regulator's spare headroom (typically <20mA), so no separate 3.3V
  regulation is needed.
* **HC-SR04**: runs on raw `+5V`. Its ECHO output swings to 5V and MUST be
  divided down before reaching the C6's 3.3V-only GPIO (R12/R13 divider,
  see §4). TRIG is driven by a 3.3V GPIO, which the HC-SR04 reliably reads
  as logic HIGH — no level shifting needed on TRIG.
* **Reverse-polarity protection**: D12, a Schottky diode (e.g. SS54,
  rated ≥3A for MG995 stall current) in series between `+5V` and
  `SERVO_5V`. Costs ~0.3V drop on the servo rail only.

---

# 4. NET LIST

| Net name      | Description                                   | Connects                                   |
|----------------|-----------------------------------------------|---------------------------------------------|
| `+5V`          | Raw 5V input rail (pre-diode)                 | J1.1, D12 anode, U2.1 (Supermini VIN), J4.1 (HC-SR04 VCC), C1 |
| `SERVO_5V`     | Post-diode 5V, servos only                    | D12 cathode, J2.2, J3.2, C2, C3              |
| `GND`          | Common ground                                 | J1.2, U2.2, J2.1, J3.1, J4.4, SW1 (leg 2), J5.4, R13 (divider low side), all LED cathodes (D1–D11) |
| `SERVO_A_PWM`  | Servo A signal                                | U1.3 → J2.3                                  |
| `SERVO_B_PWM`  | Servo B signal                                | U1.4 → J3.3                                  |
| `TRIG`         | Ultrasonic trigger                            | U1.5 → J4.2                                  |
| `ECHO_5V`      | Raw 5V echo output from HC-SR04               | J4.3 → R12                                   |
| `ECHO_3V3`     | Divided echo signal into MCU                  | R12/R13 junction → U1.6                      |
| `BTN_SIG`      | Button input                                  | U2.9 → SW1 (leg 1)                           |
| `LED1`…`LED4`  | Aux LEDs — PWM-faded (4 of 6 free LEDC channels; servos use the other 2) | U1.7/8/9/10 → R1…R4 → D1…D4 anode |
| `LED6`         | Aux LED — on/off                              | U2.4 → R6 → D6 anode                         |
| `LED9`         | Aux LED — on/off                              | U2.7 → R9 → D9 anode                         |
| `LED10`        | Aux LED — on/off                              | U2.8 → R10 → D10 anode                       |
| `LED11`        | Aux LED — on/off. Shares GP8 with the onboard WS2812 DIN (see §5.2) | U2.10 → R11 → D11 anode |
| `+3V3`         | 3.3V, tapped from the Supermini's onboard regulator | U2.3 → J5.3, R14/R15 pull-ups         |
| `I2C_SDA`      | I2C data, OLED                                | U2.6 → J5.1, pull-up to `+3V3`               |
| `I2C_SCL`      | I2C clock, OLED                               | U2.5 → J5.2, pull-up to `+3V3`               |

**LED numbering has gaps by design** — `LED5`, `LED7`, `LED8` don't exist
on this board. `LED5` (GP12) was dropped entirely; `LED7`/`LED8` (GP19/
GP18) were reassigned to `I2C_SCL`/`I2C_SDA` instead. This is expected,
not a missing-component error — see §5.2 for the full pin-by-pin
accounting.

**LED physical position, top → bottom on the shell** (this is the order
the firmware chases through — see `include/config.h` `PIN_LED_AUX[]`):

```
LED6 → LED9 → LED10 → LED11 → LED1 → LED2 → LED3 → LED4
```

| Position (top→bottom) | LED   | GPIO | Resistor | Diode | Drive      |
|------------------------|-------|------|----------|-------|------------|
| 1                       | LED6  | 20   | R6       | D6    | on/off     |
| 2                       | LED9  | 15   | R9       | D9    | on/off     |
| 3                       | LED10 | 14   | R10      | D10   | on/off     |
| 4                       | LED11 | 8 (also WS2812 DIN) | R11 | D11 | on/off |
| 5                       | LED1  | 4    | R1       | D1    | PWM (fade) |
| 6                       | LED2  | 5    | R2       | D2    | PWM (fade) |
| 7                       | LED3  | 6    | R3       | D3    | PWM (fade) |
| 8                       | LED4  | 7    | R4       | D4    | PWM (fade) |

⚠️ **LEDC channel budget**: the ESP32-C6 has 6 total LEDC PWM channels.
The 2 servos consume 2, leaving 4 free — that's why only LED1–LED4 fade;
the rest are driven as plain digitalWrite on/off in firmware
(`writeAuxLed()` in `main.cpp`), not PWM-capable, so no LEDC channel is
needed for them. Physically, the 4 fading LEDs are the second half of the
chase (positions 5–8), not the first — the pulse animation still reads
fine either way since it's a continuous glow sweep, not a fixed brightness
step.

---

# 5. REFERENCE DESIGNATORS, SYMBOLS & FOOTPRINTS

| Ref | Part                          | KiCad Symbol (library:name)              | KiCad Footprint                                  | Notes |
|-----|-------------------------------|-------------------------------------------|---------------------------------------------------|-------|
| U1  | ESP32-C6-Supermini, left pin row (plugs into sockets — see §5.1) | `Connector_Generic:Conn_01x10` — no stock symbol matches the Supermini's specific pinout, label each pin manually | `Connector_PinSocket_2.54mm:PinSocket_1x10_P2.54mm_Vertical` — stock footprint, no custom footprint needed | Only 10 of the row's 12 physical pins are used (pins 1–2 are TX/RX, unconnected); pins 11–12 (GP23/GP22) aren't wired to anything |
| U2  | ESP32-C6-Supermini, right pin row (plugs into sockets — see §5.1) | `Connector_Generic:Conn_01x10` — same caveat as U1 | `Connector_PinSocket_2.54mm:PinSocket_1x10_P2.54mm_Vertical` | Only 10 of the row's 14 physical pins are used — pin 1 (BAT) and pins 12–14 (GP12/GP13/GP21) aren't wired to anything. See §5.2 for the full physical-to-schematic pin mapping |
| J1  | Power input, 5V DC barrel plug | `Connector:Barrel_Jack` | `Connector_BarrelJack:BarrelJack_Horizontal` | Rate for ≥3A; match plug OD/ID to the chosen 5V supply |
| D12 | Schottky diode, reverse-polarity protection (`SERVO_5V` only) | `Diode:D_Schottky` (generic Schottky symbol — set Value field to `SS54`; there is no part-specific `SS54` symbol in the stock library) | `Diode_THT:D_DO-201AD_P15.24mm_Horizontal` | Rated ≥3A for MG995 stall current |
| J2  | Servo A header (3-pin: GND/SERVO_5V/PWM) | `Connector_Generic:Conn_01x03` | `Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical` | Pin order per schematic: GND, SERVO_5V, SERVO_A_PWM — confirm against your MG995 plug wire colors before crimping |
| J3  | Servo B header (3-pin)         | `Connector_Generic:Conn_01x03`            | `Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical` | Same pin order as J2 |
| J4  | HC-SR04 header (4-pin: 5V/TRIG/ECHO_5V/GND) | `Connector_Generic:Conn_01x04`       | `Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical` | Order to match sensor module silkscreen |
| J5  | I2C OLED header (4-pin: SDA/SCL/3V3/GND) | `Connector_Generic:Conn_01x04`       | `Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical` | Pin order is SDA, SCL, 3V3, GND per schematic — this does **not** match every OLED breakout's silkscreen order, double-check before wiring |
| SW1 | Momentary tactile button        | `Switch:SW_Push`                          | `Button_Switch_THT:SW_PUSH_6mm` | Leg 1 → `BTN_SIG`, leg 2 → `GND`. Wired on the board but not read by the current firmware revision |
| D1–D4, D6, D9–D11 | Aux LEDs (8x populated, 3mm or 5mm THT) — designator matches LED net number, **not** sequential (D5/D7/D8 don't exist) | `Device:LED` | `LED_THT:LED_D5.0mm` (or `LED_D3.0mm`) | Route to physical positions on the shell per the top→bottom chase order in §4 |
| R1–R4, R6, R9–R11 | LED current-limit resistors (330Ω for 5mm red/amber LEDs at 3.3V/5V logic; recalc per LED Vf/If) — same gapped numbering as the diodes | `Device:R` | `Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal` | One per LED, in series with GPIO |
| R12 | Echo divider, top (5V side)     | `Device:R` — 1kΩ                          | `Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal` | R12/R13 form a 2:1 divider: 5V × (R13/(R12+R13)) ≈ 3.3V |
| R13 | Echo divider, bottom (GND side) | `Device:R` — 2kΩ                          | `Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal` | Junction of R12/R13 → `ECHO_3V3` → U1.6 |
| R14, R15 | I2C pull-ups — 4.7kΩ, one on SDA one on SCL | `Device:R` | `Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal` | Omit both if your OLED breakout already has onboard SDA/SCL pull-ups (most SSD1306 modules do — check before populating). **Verify which of R14/R15 lands on SDA vs SCL against your schematic file** — both are 4.7kΩ so a swap has no functional effect, but keep your silkscreen/BOM consistent |
| C1  | Decoupling on `+5V`, near U2 pin 1 (Supermini VIN) | `Device:C` — 100nF | `Capacitor_THT:C_Disc_D3.0mm_W2.5mm_P2.50mm` | |
| C2  | Decoupling on `SERVO_5V`, near J2/J3 | `Device:C` — 100nF | `Capacitor_THT:C_Disc_D3.0mm_W2.5mm_P2.50mm` | |
| C3  | Bulk decoupling on `SERVO_5V`, near J2/J3 | `Device:C_Polarized` — 220–470µF, 16V | `Capacitor_THT:CP_Radial_D8.0mm_P3.50mm` | Damps servo current-draw transients so they don't reset the C6 |

## 5.1 ESP32-C6-Supermini — Socketed Mounting (Header Sockets)

The module plugs into header sockets rather than being soldered or laid out
as a custom module footprint. This means the PCB side only needs two stock
2.54mm pin-socket footprints — no custom `.kicad_mod` required:

* Solder two **female 2.54mm pin header sockets** to the PCB, spaced
  ~17.8mm apart center-to-center (verify against the actual module you
  have; Supermini clones vary between vendors even when visually similar
  — measure before milling).
* The physical header rows are 12 pins (left) and 14 pins (right), but
  this schematic only wires up 10 pins on each side — see §5.2 for
  exactly which physical pins are used vs left unconnected. You can
  either socket only the pins you need, or socket the full row and leave
  the unused pads unpopulated/no-connect — either works.
* PCB footprint: stock `Connector_PinSocket_2.54mm:PinSocket_1x10_P2.54mm_Vertical`
  for the 10 populated positions on each row (or size to the full
  physical row length if you'd rather socket the whole thing).
* Schematic symbol for each row: stock `Connector_Generic:Conn_01x10`
  (U1, U2). There is no ESP32-C6-Supermini symbol in the stock KiCad
  library and none is needed for a socketed connector — just rename each
  pin in the symbol properties/net labels to match the module's actual
  silkscreen, rather than modeling a dedicated MCU symbol.
* The module itself needs no PCB footprint at all — it's a separate,
  removable part that carries its own male header pins and simply plugs
  into the sockets. This is also what makes it swappable if a module is
  damaged during the build.

Before laying out nets to U1/U2, confirm your physical module's actual
pin labels with a multimeter continuity check against its
datasheet/silkscreen — clone boards from different vendors are not
pin-identical.

## 5.2 Pinout Quick Reference

Every physical pin on both header rows, its net on this board (from §4),
and what it's used for. `—` means the pin is present on the module but
not wired to anything on this schematic.

**U1 — left row (physical 12 pins, top → bottom; only pins 1–10 wired)**

| Pin | Module Label | Net       | Usage                              |
|-----|--------------|-----------|-------------------------------------|
| 1   | TX           | —         | Unused                              |
| 2   | RX           | —         | Unused                              |
| 3   | GP0          | `SERVO_A_PWM` | Servo A (J2)                    |
| 4   | GP1          | `SERVO_B_PWM` | Servo B (J3)                    |
| 5   | GP2          | `TRIG`    | Ultrasonic trigger (J4)             |
| 6   | GP3          | `ECHO_3V3` | Ultrasonic echo, divided (J4 via R12/R13) |
| 7   | GP4          | `LED1`    | Aux LED D1 (R1) — PWM               |
| 8   | GP5          | `LED2`    | Aux LED D2 (R2) — PWM               |
| 9   | GP6          | `LED3`    | Aux LED D3 (R3) — PWM               |
| 10  | GP7          | `LED4`    | Aux LED D4 (R4) — PWM               |
| 11  | GP23         | —         | Unused, not on schematic            |
| 12  | GP22         | —         | Unused, not on schematic            |

**U2 — right row (physical 14 pins, top → bottom; only pins 2–11 wired)**

| Pin | Module Label | Net       | Usage                              |
|-----|--------------|-----------|-------------------------------------|
| 1   | BAT          | —         | Unused, not on schematic (battery input; board is powered via J1) |
| 2   | 5V           | `+5V`     | Raw 5V in from J1 (pre-diode, powers the module directly) |
| 3   | GND          | `GND`     | Common ground                       |
| 4   | 3V3(OUT)     | `+3V3`    | OLED VCC (J5) + I2C pull-ups (R14/R15) |
| 5   | GP20         | `LED6`    | Aux LED D6 (R6) — on/off             |
| 6   | GP19         | `I2C_SCL` | OLED I2C clock (J5)                  |
| 7   | GP18         | `I2C_SDA` | OLED I2C data (J5)                   |
| 8   | GP15         | `LED9`    | Aux LED D9 (R9) — on/off             |
| 9   | GP14         | `LED10`   | Aux LED D10 (R10) — on/off           |
| 10  | GP9 (BOOT)   | `BTN_SIG` | Momentary button (SW1)              |
| 11  | GP8          | `LED11`   | Aux LED D11 (R11) — on/off. Also drives the onboard WS2812 RGB LED's `DIN`; an external LED here will flicker with whatever pattern is sent, and the onboard WS2812 reacts to the same signal — this pin is doing double duty |
| 12  | GP12         | —         | Unused, not on schematic            |
| 13  | GP13         | —         | Unused, not on schematic. No status LED in this design revision |
| 14  | GP21         | —         | Unused, not on schematic            |

**Top edge** (onboard buttons, not header pins): `BOOT`, `RST` — no net,
no PCB connection.

---

# 6. SCHEMATIC SHEET STRUCTURE

Suggested schematic hierarchy (single flat sheet is fine at this
component count, but grouping helps readability):

1. **Power** — J1, D12, C1, C2, C3, `+5V`/`SERVO_5V`/`GND` labels
2. **MCU** — U1, U2 (socket rows) with all net labels flying to the rest
   of the sheet (avoids drawing every wire across the whole page)
3. **Actuators** — J2, J3 (servo headers)
4. **Sensors** — J4 (ultrasonic), R12/R13 divider, SW1 (button)
5. **LED Array** — D1–D4/D6/D9–D11, R1–R4/R6/R9–R11 (note which are PWM
   vs on/off per §4)
6. **Display** — J5 (OLED header), R14/R15 (I2C pull-ups), `+3V3` label

Use hierarchical labels or global labels (not raw wires) for `+5V`,
`SERVO_5V`, `+3V3`, `GND`, `SERVO_A_PWM`, `SERVO_B_PWM`, `TRIG`,
`ECHO_3V3`, `BTN_SIG`, `LED1`–`LED11` (gapped), `I2C_SDA`, `I2C_SCL` so
ERC can check net connectivity without a tangle of crossing wires on the
schematic canvas.

---

# 7. PHYSICAL LAYOUT & WIRING GUIDE

## 7.1 Placement Strategy

1. Place U1/U2 (Supermini header sockets) roughly centered, oriented so
   U1's GP0–GP3 (servos/ultrasonic) face one edge and U2's GP20/GP19/
   GP18/GP15/GP14/GP9/GP8 face the opposite or adjacent edge — minimizes
   trace length and crossings.
2. Place J2/J3 (servo headers) along the edge nearest U1's GP0/GP1.
3. Place J4 (ultrasonic header) along the edge nearest U1's GP2/GP3, with
   R12/R13 directly adjacent to it (keep the divider physically close to
   the header, not close to the MCU, so `ECHO_5V` stays short and only
   the already-safe `ECHO_3V3` net runs the longer distance to U1).
4. Place J1 (power in) + D12 + C1/C2/C3 at a board edge, away from signal
   headers, to keep the high-current power entry point isolated.
5. Distribute D1–D4/D6/D9–D11 + their resistors around the board
   perimeter facing outward toward the shell's translucent "pulse point"
   cutouts, **in physical top-to-bottom order matching the chase
   animation**: LED6 → LED9 → LED10 → LED11 → LED1 → LED2 → LED3 → LED4
   (see §4). This isn't just cosmetic — the firmware assumes this order
   for the sweeping pulse effect to read as a single wave moving in one
   direction, so keep the physical placement sequential even though the
   nets themselves don't need to route in that order. LED1–LED4 come off
   U1 (left row) while LED6/9/10/11 come off U2 (right row), so this
   array spans both sides of the MCU — expect more jumpers than a
   smaller LED count would need (see §7.3).
6. Place SW1 wherever the shell's physical button cutout requires —
   route `BTN_SIG` to it last, since it's low-priority/low-current (and
   currently unused by firmware).
7. Place J5 (OLED header) + R14/R15 (I2C pull-ups) close together near
   U2's GP19/GP18/3V3(OUT) pins — I2C is not particularly sensitive on a
   slow bus like this, but keeping the pull-ups near the header (not near
   the MCU) keeps the trace length that carries the "raw" bus short and
   off the shared perimeter routing used by the decorative LEDs.

## 7.2 Routing Order (single layer — do the constrained nets first)

1. **GND** — route as wide as possible first; consider a partial ground
   pour (Edit → Fill Zone) on B.Cu in open areas instead of a discrete
   trace, since GND has the most connections. A zone still counts as
   "single layer" — it's just copper fill on B.Cu, no via needed as long
   as every GND pad is a THT pin planted directly into that fill.
2. **`+5V` / `SERVO_5V`** — route next at 0.8–1.0mm width. `+5V` runs
   shortest-path from J1 to D12/U2/J4; `SERVO_5V` runs shortest-path from
   D12 to J2/J3.
3. **`SERVO_A_PWM`, `SERVO_B_PWM`, `TRIG`, `ECHO_3V3`, `BTN_SIG`,
   `I2C_SDA`, `I2C_SCL`** — signal nets, 0.4mm, route with generous
   curves (arcs, not 90° corners — both looks organic and mills more
   reliably than sharp interior corners at small bit diameters). `+3V3`
   can route at 0.4mm too — the OLED's current draw is trivial, it
   doesn't need the 0.8mm power-net width.
4. **`LED1`–`LED11`** (gapped) — route last, since they're the most
   flexible (non-critical, low current, purely decorative routing is
   encouraged).

## 7.3 Handling Crossovers (no second layer available)

Where two nets must cross and cannot be routed around each other:

* Prefer detouring one net around the conflict — with a moderate net
  count and generous board area this is usually possible.
* If unavoidable, insert a 2-pin **jumper footprint**
  (`Jumper:SolderJumper-2_P1.3mm_Bridged` or a plain 0Ω axial resistor
  footprint used as a wire link) in the net that needs to hop over. The
  jumper's two pads sit on either side of the obstruction; a bare wire is
  soldered across the top (component) side, physically bridging over the
  milled trace it's crossing without touching it.
* Document each jumper's location on the silkscreen/F.Fab reference layer
  even though it won't mill, so hand-assembly can find it easily.

## 7.4 Test Points

Consistent with the "visible test points" visual language in the main
design doc: add a handful of small unconnected-pad test points
(`TestPoint:TestPoint_Pad_D1.0mm`) on `+5V`, `GND`, `SERVO_A_PWM`, and
`ECHO_3V3` — functionally optional, but they read as an inspected
industrial artefact under the exhibition lighting and double as real
probe points for bring-up debugging.

---

# 8. BRING-UP / ASSEMBLY CHECKLIST

1. Mill B.Cu + drill, deburr, and continuity-check every net against this
   table with a multimeter **before** populating any parts.
2. Solder passives first (R1–R4, R6, R9–R15, C1–C3), then headers/sockets
   (J1–J5, U1/U2 female sockets), then SW1, then D1–D4/D6/D9–D12 last
   (easiest to damage with heat / easiest to get polarity backwards —
   check flat edge = cathode against the `GND`-side pad before
   soldering).
3. Insert the Supermini into its U1/U2 sockets. Power J1 at 5V with
   servos/HC-SR04 disconnected first; confirm the Supermini boots
   (check serial monitor at 115200 baud for the boot banner in
   `src/main.cpp`) before attaching J2/J3/J4.
4. Attach HC-SR04, verify `ECHO_3V3` never exceeds 3.3V at the R12/R13
   junction with a multimeter before trusting it against GPIO3.
5. Attach servos one at a time, confirm `SERVO_5V` rail voltage doesn't
   sag below ~4.5V under load (add/verify C3 if it does). MG995 stall
   current is significantly higher than a micro servo — verify the 5V
   barrel-plug supply and J1/D12 are rated for both servos stalling
   simultaneously.
6. Attach the OLED to J5, verify `+3V3` measures 3.0–3.3V at J5.3 before
   connecting the display, then power up and confirm the eye animation
   appears. If nothing shows, check R14/R15 are actually populated (or
   that the breakout's onboard pull-ups are present) before assuming the
   display itself is faulty, and confirm J5's SDA/SCL/3V3/GND order
   actually matches your specific OLED breakout's silkscreen — cheap
   breakouts are not consistent about pin order.
7. With everything attached, verify the LED chase visibly sweeps
   top-to-bottom in the order given in §4/§7.1, and that bringing an
   object closer to the HC-SR04 visibly speeds up both the LED chase and
   the servo sweep. If the chase runs in the wrong direction or looks
   scrambled, check `PIN_LED_AUX[]` in `include/config.h` against the
   physical LED placement on the shell.
