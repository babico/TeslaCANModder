#pragma once
#include <NimBLEDevice.h>
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

// Ring buffer for received BLE data
static char bleRxBuf[256];
static volatile uint16_t bleRxHead = 0;
static volatile uint16_t bleRxTail = 0;

class BLEServerCB : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* s, NimBLEConnInfo& connInfo) override {
    (void)s; (void)connInfo;
    bleDeviceConnected = true;
  }
  void onDisconnect(NimBLEServer* s, NimBLEConnInfo& connInfo, int reason) override {
    (void)s; (void)connInfo; (void)reason;
    bleDeviceConnected = false;
    // Restart advertising so device can reconnect
    NimBLEDevice::startAdvertising();
  }
};

class BLERxCallback : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
    (void)connInfo;
    std::string val = pChar->getValue();
    for (size_t i = 0; i < val.length(); i++) {
      uint16_t next = (bleRxHead + 1) % sizeof(bleRxBuf);
      if (next != bleRxTail) {
        bleRxBuf[bleRxHead] = val[i];
        bleRxHead = next;
      }
    }
  }
};

void bleInit() {
  NimBLEDevice::init(BLE_DEVICE_NAME);
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);

  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new BLEServerCB());

  NimBLEService* pService = pServer->createService(BLE_SERVICE_UUID);

  pTxChar = pService->createCharacteristic(
    BLE_TX_UUID,
    NIMBLE_PROPERTY::NOTIFY
  );

  pRxChar = pService->createCharacteristic(
    BLE_RX_UUID,
    NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
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
  return (bleRxHead - bleRxTail + sizeof(bleRxBuf)) % sizeof(bleRxBuf);
}

char bleRead() {
  if (bleRxHead == bleRxTail) return 0;
  char c = bleRxBuf[bleRxTail];
  bleRxTail = (bleRxTail + 1) % sizeof(bleRxBuf);
  return c;
}

// ── Runtime BLE Control ──────────────────────────────────────────────────────

void bleStop() {
  if (!bleReady) return;
  NimBLEDevice::stopAdvertising();
  if (pServer) {
    pServer->disconnectClient(0);
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
