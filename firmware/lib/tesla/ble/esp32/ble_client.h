#pragma once
// ── NimBLE central client for Tesla vehicle BLE ───────────────────────────────
//
// The Tesla vehicle exposes three GATT characteristics:
//   Service: 00000211-b2d1-43f0-9b88-960cebf8b91e
//   TX char (write to vehicle): 00000212-b2d1-43f0-9b88-960cebf8b91e
//   RX char (notify from vehicle): 00000213-b2d1-43f0-9b88-960cebf8b91e
//
// BLE message framing: 2-byte big-endian length prefix + raw protobuf payload.
//
// NimBLEDevice must already be initialised (bleInit() in io/ble/esp32/init.h
// is called before any Tesla commands can reach here).
//
// Thread model: all calls block in the Arduino loop task.  Scan + connect can
// take several seconds; keep timeout reasonable.

#if BOARD_ENABLE_BLE

#include <NimBLEDevice.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

namespace Tesla {

// Tesla BLE service / characteristic UUIDs
static const char *TESLA_SVC_UUID = "00000211-b2d1-43f0-9b88-960cebf8b91e";
static const char *TESLA_TX_UUID  = "00000212-b2d1-43f0-9b88-960cebf8b91e";
static const char *TESLA_RX_UUID  = "00000213-b2d1-43f0-9b88-960cebf8b91e";

// Maximum receive buffer: 600 bytes covers all known Tesla BLE responses
static const size_t TESLA_RX_BUF = 600;

// ── Receive state shared with the notification callback ──────────────────────
struct TeslaRxState {
    uint8_t  raw[TESLA_RX_BUF];
    size_t   rawLen;
    bool     complete;          // full message received
    bool     overflow;
};

static TeslaRxState s_rx;

// ── NimBLE notification callback ─────────────────────────────────────────────
static void teslaNotifyCallback(NimBLERemoteCharacteristic * /*ch*/,
                                uint8_t *data, size_t len, bool /*notify*/)
{
    if (s_rx.overflow) return;

    // First two bytes of *first* chunk are the 2-byte big-endian length.
    // After that, accumulate until rawLen == expected.
    if (s_rx.rawLen == 0 && len >= 2) {
        // Peek expected length
        uint16_t expected = ((uint16_t)data[0] << 8) | data[1];
        if (expected + 2 > TESLA_RX_BUF) {
            s_rx.overflow = true;
            return;
        }
    }

    if (s_rx.rawLen + len > TESLA_RX_BUF) {
        s_rx.overflow = true;
        return;
    }

    memcpy(s_rx.raw + s_rx.rawLen, data, len);
    s_rx.rawLen += len;

    // Check if we have the full message
    if (s_rx.rawLen >= 2) {
        uint16_t expected = ((uint16_t)s_rx.raw[0] << 8) | s_rx.raw[1];
        if (s_rx.rawLen >= (size_t)(expected + 2)) {
            s_rx.complete = true;
        }
    }
}

// ── TeslaClient ──────────────────────────────────────────────────────────────
class TeslaClient {
public:
    TeslaClient() : _pClient(nullptr), _pTxChar(nullptr), _pRxChar(nullptr) {}

    ~TeslaClient() { disconnect(); }

    // Scan for up to scanSec seconds for a device advertising the Tesla service.
    // If vinSuffix is non-null (e.g. last 8 chars of VIN), filter by device name suffix.
    // Returns true if a matching device was found and connect() succeeded.
    bool connect(const char *vinSuffix = nullptr, uint32_t scanSec = 8)
    {
        if (_pClient && _pClient->isConnected()) return true;

        NimBLEScan *scan = NimBLEDevice::getScan();
        scan->setActiveScan(true);
        scan->setInterval(97);
        scan->setWindow(37);

        NimBLEAdvertisedDevice *target = nullptr;
        NimBLEScanResults results = scan->start((uint32_t)scanSec, false);

        for (int i = 0; i < results.getCount(); i++) {
            NimBLEAdvertisedDevice dev = results.getDevice(i);
            if (!dev.isAdvertisingService(NimBLEUUID(TESLA_SVC_UUID))) continue;
            if (vinSuffix && strlen(vinSuffix) > 0) {
                String name = dev.getName().c_str();
                if (name.length() < strlen(vinSuffix)) continue;
                if (!name.endsWith(vinSuffix)) continue;
            }
            // Found a suitable device – take its address
            _addr = dev.getAddress();
            target = &dev;
            break;
        }
        scan->stop();
        scan->clearResults();

        if (!target) return false;

        _pClient = NimBLEDevice::createClient();
        _pClient->setConnectionParams(12, 12, 0, 51);
        _pClient->setConnectTimeout(10);

        if (!_pClient->connect(_addr)) {
            NimBLEDevice::deleteClient(_pClient);
            _pClient = nullptr;
            return false;
        }

        NimBLERemoteService *svc = _pClient->getService(TESLA_SVC_UUID);
        if (!svc) { disconnect(); return false; }

        _pTxChar = svc->getCharacteristic(TESLA_TX_UUID);
        _pRxChar = svc->getCharacteristic(TESLA_RX_UUID);
        if (!_pTxChar || !_pRxChar) { disconnect(); return false; }

        // Subscribe to RX notifications
        if (!_pRxChar->subscribe(true, teslaNotifyCallback)) {
            disconnect();
            return false;
        }

        return true;
    }

    bool isConnected() const
    {
        return _pClient && _pClient->isConnected();
    }

    void disconnect()
    {
        if (_pClient) {
            if (_pClient->isConnected()) _pClient->disconnect();
            NimBLEDevice::deleteClient(_pClient);
            _pClient  = nullptr;
        }
        _pTxChar = nullptr;
        _pRxChar = nullptr;
    }

    // Send a raw protobuf payload (without framing).  Prepends 2-byte BE length.
    bool send(const uint8_t *payload, size_t len)
    {
        if (!isConnected() || !_pTxChar) return false;

        uint16_t mtu = _pClient->getMTU();
        uint16_t chunkPayload = (mtu > 3) ? (mtu - 3) : 20;

        // Build framed message: [len_hi, len_lo, payload...]
        static uint8_t frameBuf[600 + 2];
        if (len + 2 > sizeof(frameBuf)) return false;
        frameBuf[0] = (uint8_t)((len >> 8) & 0xFF);
        frameBuf[1] = (uint8_t)(len & 0xFF);
        memcpy(frameBuf + 2, payload, len);
        size_t total = len + 2;

        // Write in MTU-sized chunks
        for (size_t offset = 0; offset < total; offset += chunkPayload) {
            size_t chunk = total - offset;
            if (chunk > chunkPayload) chunk = chunkPayload;
            if (!_pTxChar->writeValue(frameBuf + offset, chunk, false))
                return false;
        }
        return true;
    }

    // Wait for a complete notification response.
    // On success, sets *outPayload to point inside internal buffer (after 2-byte length header),
    // sets *outLen to payload byte count.
    // Valid until next send() or disconnect().
    bool receive(uint32_t timeoutMs, const uint8_t **outPayload, size_t *outLen)
    {
        s_rx.rawLen   = 0;
        s_rx.complete = false;
        s_rx.overflow = false;

        uint32_t deadline = millis() + timeoutMs;
        while (!s_rx.complete && !s_rx.overflow) {
            if (millis() > deadline) return false;
            delay(5);
        }
        if (s_rx.overflow || s_rx.rawLen < 2) return false;

        uint16_t payloadLen = ((uint16_t)s_rx.raw[0] << 8) | s_rx.raw[1];
        if (s_rx.rawLen < (size_t)(payloadLen + 2)) return false;

        *outPayload = s_rx.raw + 2;
        *outLen     = payloadLen;
        return true;
    }

    // Convenience: send + receive in one call.
    bool exchange(const uint8_t *payload, size_t len,
                  const uint8_t **outPayload, size_t *outLen,
                  uint32_t timeoutMs = 10000)
    {
        if (!send(payload, len)) return false;
        return receive(timeoutMs, outPayload, outLen);
    }

private:
    NimBLEClient             *_pClient;
    NimBLERemoteCharacteristic *_pTxChar;
    NimBLERemoteCharacteristic *_pRxChar;
    NimBLEAddress              _addr;
};

} // namespace Tesla

#endif // BOARD_ENABLE_BLE
