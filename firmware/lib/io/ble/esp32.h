#pragma once
#include <NimBLEDevice.h>
#include <atomic>
#include "core/config/esp32.h"
#include "core/types.h"

// ── BLE GATT Service for iPhone ──────────────────────────────────────────────
// Uses NimBLE for proper BLE support (works with iOS, unlike Bluetooth Classic SPP).
//
// Service UUID:  6e400001-b5a3-f393-e0a9-e50e24dcca9e (Nordic UART Service)
// RX Char UUID:  6e400002-... (write: phone → device, commands)
// TX Char UUID:  6e400003-... (notify: device → phone, responses)

#define BLE_SERVICE_UUID "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define BLE_RX_UUID      "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define BLE_TX_UUID      "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

static NimBLEServer* pServer = nullptr;
static NimBLECharacteristic* pTxChar = nullptr;
static NimBLECharacteristic* pRxChar = nullptr;
static bool bleDeviceConnected = false;
static bool bleReady = false;
static char bleDeviceName[33] = BLE_DEVICE_NAME;

// Ring buffer for received BLE data (atomic for dual-core ESP32 safety)
static char bleRxBuf[256];
static std::atomic<uint16_t> bleRxHead{0};
static std::atomic<uint16_t> bleRxTail{0};

class BLEServerCB : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* s) override {
    (void)s;
    bleDeviceConnected = true;
  }
  void onDisconnect(NimBLEServer* s) override {
    (void)s;
    bleDeviceConnected = false;
    // Restart advertising so device can reconnect
    NimBLEDevice::startAdvertising();
  }
};

class BLERxCallback : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* pChar) override {
    std::string val = pChar->getValue();
    uint16_t head = bleRxHead.load(std::memory_order_relaxed);
    for (size_t i = 0; i < val.length(); i++) {
      uint16_t next = (head + 1) % sizeof(bleRxBuf);
      if (next != bleRxTail.load(std::memory_order_acquire)) {
        bleRxBuf[head] = val[i];
        head = next;
      }
    }
    bleRxHead.store(head, std::memory_order_release);
  }
};

void bleStop();

static void setBleDeviceNameValue(const char* name) {
  if (!name) return;
  strncpy(bleDeviceName, name, sizeof(bleDeviceName) - 1);
  bleDeviceName[sizeof(bleDeviceName) - 1] = '\0';
}

void bleInit() {
  NimBLEDevice::init(bleDeviceName);
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);

  // Enable bonding + MITM protection + secure connections
  NimBLEDevice::setSecurityAuth(true, true, true);
  NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);

  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new BLEServerCB());

  NimBLEService* pService = pServer->createService(BLE_SERVICE_UUID);

  pTxChar = pService->createCharacteristic(
    BLE_TX_UUID,
    NIMBLE_PROPERTY::NOTIFY
  );

  pRxChar = pService->createCharacteristic(
    BLE_RX_UUID,
    NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR | NIMBLE_PROPERTY::WRITE_ENC
  );
  pRxChar->setCallbacks(new BLERxCallback());

  pService->start();

  NimBLEAdvertising* pAdv = NimBLEDevice::getAdvertising();
  pAdv->addServiceUUID(BLE_SERVICE_UUID);
  pAdv->setScanResponse(true);
  pAdv->start();

  bleReady = true;
}

bool bleIsReady() { return bleReady; }
bool bleIsConnected() { return bleDeviceConnected; }
const char* bleGetDeviceName() { return bleDeviceName; }

bool bleSetDeviceName(const char* name) {
  if (!name) return false;
  size_t len = strlen(name);
  if (len == 0 || len > 32) return false;
  if (strcmp(bleDeviceName, name) == 0) return true;

  bool wasReady = bleReady;
  if (wasReady) {
    bleStop();
  }

  setBleDeviceNameValue(name);

  if (wasReady) {
    bleInit();
  }

  return true;
}

// ── Print API (same interface as old BT, called from serial output helpers) ──

void blePrint(const char* s) {
  if (bleReady && bleDeviceConnected && pTxChar) {
    pTxChar->setValue((const uint8_t*)s, strlen(s));
    pTxChar->notify();
  }
}

void blePrint(const __FlashStringHelper* s) {
  if (bleReady && bleDeviceConnected && pTxChar) {
    String str(s);
    pTxChar->setValue((const uint8_t*)str.c_str(), str.length());
    pTxChar->notify();
  }
}

void blePrintNum(long n) {
  char buf[12];
  snprintf(buf, sizeof(buf), "%ld", n);
  blePrint(buf);
}

void blePrintHex(uint8_t b) {
  char buf[4];
  snprintf(buf, sizeof(buf), "%02X", b);
  blePrint(buf);
}

void blePrintLn() {
  blePrint("\r\n");
}

int bleAvailable() {
  if (!bleReady) return 0;
  uint16_t head = bleRxHead.load(std::memory_order_acquire);
  uint16_t tail = bleRxTail.load(std::memory_order_relaxed);
  return (head - tail + sizeof(bleRxBuf)) % sizeof(bleRxBuf);
}

char bleRead() {
  uint16_t head = bleRxHead.load(std::memory_order_acquire);
  uint16_t tail = bleRxTail.load(std::memory_order_relaxed);
  if (head == tail) return 0;
  char c = bleRxBuf[tail];
  bleRxTail.store((tail + 1) % sizeof(bleRxBuf), std::memory_order_release);
  return c;
}

// ── Runtime BLE Control ──────────────────────────────────────────────────────

void bleStop() {
  if (!bleReady) return;
  NimBLEDevice::stopAdvertising();
  if (pServer) {
    pServer->disconnect(0);
  }
  NimBLEDevice::deinit(true);
  bleReady = false;
  bleDeviceConnected = false;
  pServer = nullptr;
  pTxChar = nullptr;
  pRxChar = nullptr;
}

void bleRestart() {
  if (bleReady) return;
  bleInit();
}
