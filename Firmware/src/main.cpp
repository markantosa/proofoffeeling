//  Affective Core AC-7 — Firmware v3.0
//  Companion Systems Inc.
//
//  Continuous, proximity-driven behaviour (no discrete interaction modes
//  in this revision — that's a deliberate pivot from v2.0's
//  RESTING/CONTACT/COMMAND/OVERLOAD/SHUTDOWN state machine):
//    - Aux LEDs chase top-to-bottom in a wrapping pulse
//    - Both 180° servos sweep 0→180→0 continuously, mirrored
//    - Ultrasonic proximity continuously scales both speeds — closer
//      object = faster chase and faster sweep
//    - OLED always shows an eye, blinking at random intervals
//
//  Hardware: ESP32-C6-Supermini, 2x MG995 servo (5V, continuous sweep),
//  HC-SR04 ultrasonic (proximity → speed), 8x auxiliary "pulse point"
//  LEDs (4 PWM-faded + 4 on/off — LEDC channel budget, see config.h),
//  I2C SSD1306 OLED (always-on eye animation). BLE telemetry (distance +
//  speed multiplier, ~10Hz notify) for the debug dashboard at
//  dashboard/index.html — see ble_telemetry.h/.cpp. Button (BTN_SIG/SW1)
//  is wired on the board but not read by this revision.

#include <Arduino.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"
#include "ble_telemetry.h"

// ─── State ────────────────────────────────────────────────────────────────────

Servo servoA, servoB;
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

float lastDistanceCm      = PROXIMITY_FAR_CM;
unsigned long lastPingAt  = 0;

bool blinking              = false;
unsigned long blinkStartAt = 0;
unsigned long nextBlinkAt  = 0;

// ─── Ultrasonic Ranging ─────────────────────────────────────────────────────────

// Non-blocking-ish single ping. Returns last good reading if polled too soon
// or if the echo timed out (no reflection in range).
static float pingDistanceCm() {
    unsigned long now = millis();
    if (now - lastPingAt < ULTRASONIC_POLL_MS) return lastDistanceCm;
    lastPingAt = now;

    digitalWrite(PIN_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG, LOW);

    unsigned long durationUs = pulseIn(PIN_ECHO, HIGH, ULTRASONIC_TIMEOUT_US);
    if (durationUs == 0) return lastDistanceCm;   // timeout — keep last value

    lastDistanceCm = (durationUs / 2.0f) / 29.1f;  // speed of sound ≈ 343 m/s
    return lastDistanceCm;
}

// ─── Proximity → Speed ──────────────────────────────────────────────────────────

// 1.0x at/beyond PROXIMITY_FAR_CM, ramps linearly up to SPEED_MULTIPLIER_MAX
// at/below PROXIMITY_NEAR_CM. Applied to both the servo sweep and LED chase.
static float speedMultiplier(float distanceCm) {
    float d = constrain(distanceCm, PROXIMITY_NEAR_CM, PROXIMITY_FAR_CM);
    float t = (PROXIMITY_FAR_CM - d) / (PROXIMITY_FAR_CM - PROXIMITY_NEAR_CM);
    return 1.0f + t * (SPEED_MULTIPLIER_MAX - 1.0f);
}

// ─── Servo Sweep ────────────────────────────────────────────────────────────────

// Continuous triangle-wave sweep between SERVO_MIN_ANGLE/SERVO_MAX_ANGLE.
// Period shrinks as speedMult grows. Servos are mirrored (B = 180 - A) so
// they visibly move in opposite directions rather than in lockstep.
static void updateServos(float speedMult) {
    uint32_t period = (uint32_t)(SERVO_SWEEP_PERIOD_MS / speedMult);
    if (period < 200) period = 200;   // floor — don't let the servo whip itself apart

    float phase    = (float)(millis() % period) / (float)period;         // 0..1
    float triangle = (phase < 0.5f) ? (phase * 2.0f) : (2.0f - phase * 2.0f); // 0..1..0
    int   angle    = SERVO_MIN_ANGLE + (int)(triangle * (SERVO_MAX_ANGLE - SERVO_MIN_ANGLE));

    servoA.write(angle);
    servoB.write(SERVO_MAX_ANGLE - angle);
}

// ─── LED Chase (top → bottom, wrapping) ─────────────────────────────────────────

// Write one aux LED's brightness — PWM-fade if the channel budget allows
// (PIN_LED_AUX_PWM[index] true), otherwise plain on/off at half-brightness
// threshold. See config.h for why only 4 of 8 channels get real PWM.
static void writeAuxLed(uint8_t index, uint8_t brightness) {
    if (PIN_LED_AUX_PWM[index]) {
        analogWrite(PIN_LED_AUX[index], brightness);
    } else {
        digitalWrite(PIN_LED_AUX[index], brightness >= 128 ? HIGH : LOW);
    }
}

// A glow that sweeps top→bottom through PIN_LED_AUX[] and wraps back to the
// top continuously. Period shrinks with speedMult, same as the servos.
static void updateLedChase(float speedMult) {
    uint32_t period = (uint32_t)(LED_PULSE_PERIOD_MS / speedMult);
    if (period < 150) period = 150;

    float pos = (float)(millis() % period) / (float)period * PIN_LED_AUX_COUNT;

    for (uint8_t i = 0; i < PIN_LED_AUX_COUNT; i++) {
        float d = fabsf((float)i - pos);
        d = min(d, (float)PIN_LED_AUX_COUNT - d);   // wrap distance, shortest way around
        float glow = max(0.0f, 1.0f - d / LED_PULSE_WIDTH);
        writeAuxLed(i, (uint8_t)(255 * glow));
    }
}

// ─── OLED Eye ────────────────────────────────────────────────────────────────────

static void scheduleNextBlink() {
    nextBlinkAt = millis() + random(BLINK_MIN_INTERVAL_MS, BLINK_MAX_INTERVAL_MS);
}

// Always-on eye, blinking at random intervals independent of the chase/sweep.
static void updateEye() {
    unsigned long now = millis();

    if (!blinking && now >= nextBlinkAt) {
        blinking     = true;
        blinkStartAt = now;
    } else if (blinking && now - blinkStartAt > BLINK_DURATION_MS) {
        blinking = false;
        scheduleNextBlink();
    }

    display.clearDisplay();
    int cx = OLED_WIDTH / 2, cy = OLED_HEIGHT / 2;
    int rx = OLED_WIDTH / 3, ry = OLED_HEIGHT / 3;

    if (blinking) {
        // Closed eyelid — a thin horizontal bar across the eye's center line.
        display.fillRoundRect(cx - rx, cy - 2, rx * 2, 4, 2, SSD1306_WHITE);
    } else {
        // Open eye — outer eyeball with a dark pupil punched out of the center.
        display.fillCircle(cx, cy, ry, SSD1306_WHITE);
        display.fillCircle(cx, cy, ry / 2, SSD1306_BLACK);
    }
    display.display();
}

// ─── Setup ────────────────────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);
    delay(300);
    Serial.println();
    Serial.println(F("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"));
    Serial.println(F("  AFFECTIVE CORE  Model AC-7  Companion Series"));
    Serial.println(F("  Companion Systems Inc.  |  Firmware v3.0"));
    Serial.println(F("  Synthetic Emotional Verification Organ"));
    Serial.println(F("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"));

    // Allocate LEDC timers for ESP32Servo (C6 has 4 timers: 0–3)
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

    servoA.setPeriodHertz(50);
    servoB.setPeriodHertz(50);
    servoA.attach(PIN_SERVO_A, SERVO_PULSE_MIN, SERVO_PULSE_MAX);
    servoB.attach(PIN_SERVO_B, SERVO_PULSE_MIN, SERVO_PULSE_MAX);
    servoA.write(SERVO_MIN_ANGLE);
    servoB.write(SERVO_MAX_ANGLE);

    // Ultrasonic
    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);
    digitalWrite(PIN_TRIG, LOW);

    // Button — wired, not read by this firmware revision
    pinMode(PIN_BUTTON, INPUT_PULLUP);

    // Auxiliary LED array
    for (uint8_t i = 0; i < PIN_LED_AUX_COUNT; i++) {
        pinMode(PIN_LED_AUX[i], OUTPUT);
        writeAuxLed(i, 0);
    }

    // I2C OLED
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
        Serial.println(F("[AC-7] OLED not found at boot — check J5 wiring/address"));
    } else {
        display.clearDisplay();
        display.display();
    }

    // Seed PRNG from hardware entropy
    randomSeed(esp_random());
    scheduleNextBlink();

    lastPingAt = millis();

    // BLE telemetry — for the debug dashboard (dashboard/index.html)
    bleTelemetryBegin();

    Serial.println(F("[AC-7] Emotional display active. Trust calibration nominal."));
}

// ─── Loop ─────────────────────────────────────────────────────────────────────

void loop() {
    float distance  = pingDistanceCm();
    float speedMult = speedMultiplier(distance);

    updateServos(speedMult);
    updateLedChase(speedMult);
    updateEye();
    bleTelemetryUpdate(distance, speedMult);

    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 2000) {
        lastPrint = millis();
        Serial.printf("[AC-7] dist=%.1fcm  speed=%.2fx\n", distance, speedMult);
    }
}
