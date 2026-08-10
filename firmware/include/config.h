#pragma once

// ─── Pin Assignments ── ESP32-C6-Supermini ────────────────────────────────────
// Matches the finalized KiCad schematic exactly — see
// docs/pcb-design-reference.md §4/§5.2 for the full net-by-net mapping.

// Servos — 180° hobby servos, continuous sweep (see SERVO_* below), not a
// cardiac contraction waveform in this firmware revision.
#define PIN_SERVO_A       0
#define PIN_SERVO_B       1

// Ultrasonic distance sensor (HC-SR04) — drives both the LED chase speed
// and servo sweep speed continuously; closer object = faster motion.
// ECHO is 5V logic on the sensor — it MUST be divided down to 3.3V
// before reaching PIN_ECHO (R12/R13 on the PCB).
#define PIN_TRIG          2
#define PIN_ECHO          3

// Momentary button — wired on the board (BTN_SIG/SW1, GP9/BOOT strap)
// but not read by this firmware revision. Reserved for a future mode.
#define PIN_BUTTON        9

// I2C OLED (SSD1306 128x64 or similar) — always-on random eye-blink
// animation, independent of the LED chase / servo sweep.
#define PIN_I2C_SDA       18
#define PIN_I2C_SCL       19
#define OLED_WIDTH        128
#define OLED_HEIGHT       64
#define OLED_I2C_ADDRESS  0x3C

// Auxiliary "pulse point" LEDs, listed in PHYSICAL TOP-TO-BOTTOM ORDER on
// the shell — this is what makes the chase animation sweep in one
// direction. Matches the schematic's D1/D6/D9/D10/D11 designators;
// GP10/GP12 (former LED5) and GP19/GP18 (former LED7/LED8, now the I2C
// bus) are not part of this board's LED array — see PCB doc §4 for why
// the LED numbering has gaps.
//
// Do not reorder this array without updating the physical LED placement
// on the shell to match — see docs/pcb-design-reference.md §7.1.
#define PIN_LED_AUX_COUNT 8
static const uint8_t PIN_LED_AUX[PIN_LED_AUX_COUNT] =
    { 20, 15, 14, 8, 4, 5, 6, 7 };
    // LED6   LED9  LED10 LED11 LED1 LED2 LED3 LED4

// Which of the above are PWM-capable (fade) vs on/off only. The C6 has
// only 6 total LEDC channels — the 2 servos use 2, leaving 4 free — so
// only 4 of these 8 LEDs can fade; the rest are driven as plain on/off
// (see writeAuxLed() in main.cpp). Must stay index-aligned with
// PIN_LED_AUX[] above.
static const bool PIN_LED_AUX_PWM[PIN_LED_AUX_COUNT] =
    { false, false, false, false, true, true, true, true };

// ─── Servo Sweep ───────────────────────────────────────────────────────────────
// Servo A is mechanically limited to a 90° arc, so it gets its own
// (narrower) range. Servo B sweeps the full 180°. Both share the same
// timing/phase (see updateServos() in main.cpp), so they still reach
// their respective extremes at the same instant: A at its max (90°)
// exactly when B is at 180°, A at its min (0°) exactly when B is at 0°.
#define SERVO_A_MIN_ANGLE      0
#define SERVO_A_MAX_ANGLE      90
#define SERVO_MIN_ANGLE        0
#define SERVO_MAX_ANGLE        180
#define SERVO_PULSE_MIN        500     // µs, standard 500–2400 range
#define SERVO_PULSE_MAX        2400
#define SERVO_SWEEP_PERIOD_MS  4800    // time for one full 0→180→0 cycle, at rest (0.5x speed)

// Hard floor on how short the sweep period is allowed to get, regardless
// of proximity speed-up. An SG92R takes roughly 0.35-0.4s to travel a
// full 180° one-way under light load — a period shorter than ~750ms
// commands the servo to reverse direction before it has physically
// finished traveling, which looks like it "stalls halfway and snaps
// back" rather than completing its full range of motion.
#define SERVO_SWEEP_MIN_PERIOD_MS  750

// ─── LED Chase (top → bottom, wrapping) ─────────────────────────────────────────
#define LED_PULSE_PERIOD_MS    1600    // time for the glow to sweep the full chain, at rest
#define LED_PULSE_WIDTH        2.2f    // glow falloff width, in LED-index units

// ─── Proximity → Speed Mapping ──────────────────────────────────────────────────
// Distance is continuously mapped to a speed multiplier applied to both
// the servo sweep and the LED chase — no discrete interaction modes in
// this firmware revision.
#define PROXIMITY_NEAR_CM      5.0f    // multiplier maxes out at/below this distance
#define PROXIMITY_FAR_CM       60.0f   // multiplier is 1.0x at/beyond this distance
#define SPEED_MULTIPLIER_MAX   5.0f    // how much faster than resting, at closest range
#define ULTRASONIC_TIMEOUT_US  25000UL // ~4.3 m max range, avoids blocking pulseIn()
#define ULTRASONIC_POLL_MS     60      // minimum interval between pings

// ─── OLED Eye Blink ──────────────────────────────────────────────────────────────
#define BLINK_DURATION_MS      140     // how long the eye stays closed per blink
#define BLINK_MIN_INTERVAL_MS  1500    // shortest gap between blinks
#define BLINK_MAX_INTERVAL_MS  6000    // longest gap between blinks
