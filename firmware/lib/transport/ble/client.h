#pragma once

/**
 * @file firmware/lib/transport/ble/client.h
 * @brief NimBLE central (client) for communicating with the Tesla vehicle over BLE
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#if BOARD_ENABLE_BLE

#include <NimBLEDevice.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

namespace Tesla {

// Tesla vehicle BLE GATT service UUID
static const char *TESLA_SVC_UUID = "00000211-b2d1-43f0-9b88-960cebf8b91e";

// TX characteristic — write commands to the vehicle
static const char *TESLA_TX_UUID  = "00000212-b2d1-43f0-9b88-960cebf8b91e";

// RX characteristic — receive notifications from the vehicle
static const char *TESLA_RX_UUID  = "00000213-b2d1-43f0-9b88-960cebf8b91e";

// Maximum receive buffer size; 600 bytes covers all known Tesla BLE responses
static const size_t TESLA_RX_BUF = 600;

/**
 * @brief Receive state shared between the notification callback and TeslaClient
 */
struct TeslaRxState {
	uint8_t  raw[TESLA_RX_BUF]; // Accumulated raw bytes (length header + payload)
	size_t   rawLen;            // Current number of bytes received
	bool     complete;          // True when the full framed message has arrived
	bool     overflow;          // True if incoming data exceeded buffer capacity
};

static TeslaRxState s_rx;

/**
 * @brief NimBLE notification callback that reassembles framed Tesla BLE messages
 * @param data Pointer to the incoming notification chunk
 * @param len Length of the incoming chunk in bytes
 *
 * Tesla BLE framing: first 2 bytes are a big-endian payload length, followed by
 * the raw protobuf payload. Chunks may arrive across multiple notifications.
 */
static void teslaNotifyCallback(NimBLERemoteCharacteristic * /*ch*/,
                                uint8_t *data, size_t len, bool /*notify*/)
{
	// Do NOT short-circuit on s_rx.overflow: that flag is only reset inside
	// receive(), and a single transient overflow would otherwise permanently
	// drop every subsequent notification for the lifetime of the program.
	// The bounds checks below still set s_rx.overflow and bail, and the
	// consumer's next receive() call clears the flag before reading.

	// On the first chunk, validate the declared payload length fits the buffer
	if (s_rx.rawLen == 0 && len >= 2) {
		uint16_t expected = ((uint16_t)data[0] << 8) | data[1]; // Big-endian length prefix
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

	// Check whether the complete framed message has been received
	if (s_rx.rawLen >= 2) {
		uint16_t expected = ((uint16_t)s_rx.raw[0] << 8) | s_rx.raw[1];
		if (s_rx.rawLen >= (size_t)(expected + 2)) {
			s_rx.complete = true;
		}
	}
}

/**
 * @brief BLE central client that scans for, connects to, and exchanges messages with a Tesla vehicle
 */
class TeslaClient {
public:
	TeslaClient() : _pClient(nullptr), _pTxChar(nullptr), _pRxChar(nullptr) {}

	~TeslaClient() { disconnect(); }

	/**
	 * @brief Scan for and connect to a Tesla vehicle advertising the Tesla BLE service
	 * @param vinSuffix Optional VIN suffix filter (last 8 chars); nullptr to accept any Tesla
	 * @param scanSec Maximum scan duration in seconds
	 * @return True if a matching device was found and connection established
	 */
	bool connect(const char *vinSuffix = nullptr, uint32_t scanSec = 8)
	{
		if (_pClient && _pClient->isConnected()) return true;

		NimBLEScan *scan = NimBLEDevice::getScan();
		scan->setActiveScan(true);
		scan->setInterval(97);  // Scan interval in 0.625 ms units
		scan->setWindow(37);    // Scan window in 0.625 ms units

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
			_addr = dev.getAddress();
			target = &dev;
			break;
		}
		scan->stop();
		scan->clearResults();

		if (!target) return false;

		_pClient = NimBLEDevice::createClient();
		_pClient->setConnectionParams(12, 12, 0, 51); // min/max interval, latency, timeout
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

		// Subscribe to RX notifications to receive vehicle responses
		if (!_pRxChar->subscribe(true, teslaNotifyCallback)) {
			disconnect();
			return false;
		}

		return true;
	}

	/**
	 * @brief Check whether the client is currently connected to a vehicle
	 * @return True if connected
	 */
	bool isConnected() const
	{
		return _pClient && _pClient->isConnected();
	}

	/**
	 * @brief Disconnect from the vehicle and release BLE resources
	 */
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

	/**
	 * @brief Send a raw protobuf payload to the vehicle with BLE framing
	 * @param payload Pointer to the protobuf-encoded message bytes
	 * @param len Length of the payload in bytes
	 * @return True if all chunks were written successfully
	 *
	 * Prepends a 2-byte big-endian length header and splits the framed message
	 * into MTU-sized chunks for transmission.
	 */
	bool send(const uint8_t *payload, size_t len)
	{
		if (!isConnected() || !_pTxChar) return false;

		uint16_t mtu = _pClient->getMTU();
		uint16_t chunkPayload = (mtu > 3) ? (mtu - 3) : 20; // ATT overhead is 3 bytes

		// Build framed message: [len_hi, len_lo, payload...]
		static uint8_t frameBuf[600 + 2];
		if (len + 2 > sizeof(frameBuf)) return false;
		frameBuf[0] = (uint8_t)((len >> 8) & 0xFF); // Length high byte
		frameBuf[1] = (uint8_t)(len & 0xFF);         // Length low byte
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

	/**
	 * @brief Wait for a complete notification response from the vehicle
	 * @param timeoutMs Maximum time to wait in milliseconds
	 * @param outPayload Set to point at the payload (past the 2-byte length header)
	 * @param outLen Set to the payload byte count
	 * @return True if a complete message was received within the timeout
	 *
	 * The returned pointer is valid until the next send() or disconnect() call.
	 */
	bool receive(uint32_t timeoutMs, const uint8_t **outPayload, size_t *outLen)
	{
		s_rx.rawLen   = 0;
		s_rx.complete = false;
		s_rx.overflow = false;

		uint32_t deadline = millis() + timeoutMs;
		while (!s_rx.complete && !s_rx.overflow) {
			if (millis() > deadline) return false;
			delay(5); // Yield to allow BLE stack to process notifications
		}
		if (s_rx.overflow || s_rx.rawLen < 2) return false;

		uint16_t payloadLen = ((uint16_t)s_rx.raw[0] << 8) | s_rx.raw[1];
		if (s_rx.rawLen < (size_t)(payloadLen + 2)) return false;

		*outPayload = s_rx.raw + 2; // Skip the 2-byte length header
		*outLen     = payloadLen;
		return true;
	}

	/**
	 * @brief Send a message and wait for the vehicle's response in one call
	 * @param payload Pointer to the protobuf-encoded request bytes
	 * @param len Length of the request payload
	 * @param outPayload Set to point at the response payload
	 * @param outLen Set to the response payload byte count
	 * @param timeoutMs Maximum time to wait for the response
	 * @return True if send and receive both succeeded
	 */
	bool exchange(const uint8_t *payload, size_t len,
	              const uint8_t **outPayload, size_t *outLen,
	              uint32_t timeoutMs = 10000)
	{
		if (!send(payload, len)) return false;
		return receive(timeoutMs, outPayload, outLen);
	}

private:
	NimBLEClient             *_pClient;  // Underlying NimBLE client connection
	NimBLERemoteCharacteristic *_pTxChar; // Remote TX characteristic (write to vehicle)
	NimBLERemoteCharacteristic *_pRxChar; // Remote RX characteristic (notifications)
	NimBLEAddress              _addr;     // Address of the connected vehicle
};

} // namespace Tesla

#endif // BOARD_ENABLE_BLE
