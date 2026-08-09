#include "ble_telemetry.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Custom 128-bit UUIDs built on the standard Bluetooth base UUID, using
// AC71/AC72 as the "16-bit" portion — easy to recognise and hardcode on
// both the firmware and dashboard side. These aren't SIG-registered
// (fine for a one-off hobby device; don't reuse commercially).
static const char *SERVICE_UUID = "0000ac71-0000-1000-8000-00805f9b34fb";
static const char *CHAR_UUID    = "0000ac72-0000-1000-8000-00805f9b34fb";

#define BLE_NOTIFY_INTERVAL_MS 100   // ~10Hz

struct __attribute__((packed)) Telemetry {
    float distanceCm;
    float speedMultiplier;
};

static BLEServer         *server        = nullptr;
static BLECharacteristic *telemetryChar = nullptr;
static bool                deviceConnected = false;
static unsigned long       lastNotifyAt    = 0;

class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *s) override {
        deviceConnected = true;
    }
    void onDisconnect(BLEServer *s) override {
        deviceConnected = false;
        s->getAdvertising()->start();   // resume advertising so the dashboard can reconnect
    }
};

void bleTelemetryBegin() {
    BLEDevice::init("AC-7");

    server = BLEDevice::createServer();
    server->setCallbacks(new ServerCallbacks());

    BLEService *service = server->createService(SERVICE_UUID);
    telemetryChar = service->createCharacteristic(
        CHAR_UUID,
        BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ
    );
    telemetryChar->addDescriptor(new BLE2902());   // required for notify to work with most clients
    service->start();

    BLEAdvertising *advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(SERVICE_UUID);
    advertising->setScanResponse(true);
    BLEDevice::startAdvertising();
}

void bleTelemetryUpdate(float distanceCm, float speedMultiplier) {
    if (!deviceConnected) return;

    unsigned long now = millis();
    if (now - lastNotifyAt < BLE_NOTIFY_INTERVAL_MS) return;
    lastNotifyAt = now;

    Telemetry t = { distanceCm, speedMultiplier };
    telemetryChar->setValue((uint8_t *)&t, sizeof(t));
    telemetryChar->notify();
}
