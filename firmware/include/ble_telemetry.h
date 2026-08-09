#pragma once
#include <Arduino.h>

// BLE telemetry for the debug dashboard (dashboard/index.html). One
// service, one notify characteristic, an 8-byte struct broadcast at a
// fixed rate — no request/response, the dashboard just listens.
//
// Device name: "AC-7". Advertises continuously; safe to leave connected
// or disconnected indefinitely, doesn't affect the servo/LED/eye loop.

void bleTelemetryBegin();
void bleTelemetryUpdate(float distanceCm, float speedMultiplier);
