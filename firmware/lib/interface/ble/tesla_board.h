#pragma once

/**
 * @file firmware/lib/interface/ble/tesla_board.h
 * @brief ESP32 board adapter implementing the Board concept for Tesla vehicle BLE commands
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#if BOARD_ENABLE_BLE

#include <Preferences.h>
#include "transport/ble/client.h"
#include "vehicle/ble/feature/key.h"
#include "transport/ble/handler/dispatch.h"
#include <stdint.h>
#include <string.h>

void sendLog(const char *msg);
void sendError(const char *msg);
void printStr(const char *s);
void printLn();

namespace Tesla
{

/**
 * @brief Board implementation for ESP32 providing BLE transport, NVS persistence, and RNG
 *
 * Satisfies the Board concept required by vehicle/ble/tesla.h using ESP32 NVS
 * for key/VIN/role persistence, esp_random() for cryptographic randomness, and
 * TeslaClient (NimBLE) for BLE transport.
 */
class Esp32Board
{
	static constexpr const char *NVS_NS = "tesla";		// NVS namespace for Tesla data
	static constexpr const char *NVS_PRIV = "priv_d";	// NVS key: private key bytes
	static constexpr const char *NVS_ROLE = "key_role"; // NVS key: key-form role string
	static constexpr const char *NVS_VIN = "vin";		// NVS key: vehicle VIN

  public:
	/**
	 * @brief Create a new TeslaClient BLE transport instance
	 * @return A TeslaClient ready for connect()
	 */
	TeslaClient makeTransport()
	{
		return TeslaClient();
	}

	/**
	 * @brief Fill a buffer with cryptographically random bytes from the ESP32 hardware RNG
	 * @param out Destination buffer
	 * @param len Number of random bytes to generate
	 */
	void random(uint8_t *out, size_t len)
	{
		for (size_t i = 0; i < len;)
		{
			uint32_t r = esp_random(); // Hardware TRNG, 32 bits per call
			size_t n = len - i;
			if (n > 4)
				n = 4;
			memcpy(out + i, &r, n);
			i += n;
		}
	}

	/**
	 * @brief Load the ECDH private key from NVS and derive the public key
	 * @param kp KeyPair structure to populate
	 * @return True if a valid 32-byte private key was found and public key derived
	 */
	bool loadKey(KeyPair &kp)
	{
		Preferences p;
		p.begin(NVS_NS, true); // Read-only
		size_t n = p.getBytesLength(NVS_PRIV);
		if (n != 32)
		{
			p.end();
			kp.valid = false;
			return false;
		}
		p.getBytes(NVS_PRIV, kp.priv_d, 32);
		p.end();
		return keyDerivePublic(kp);
	}

	/**
	 * @brief Persist the ECDH private key to NVS
	 * @param kp KeyPair containing the 32-byte private key to store
	 * @return True on success
	 */
	bool saveKey(const KeyPair &kp)
	{
		Preferences p;
		p.begin(NVS_NS, false); // Read-write
		p.putBytes(NVS_PRIV, kp.priv_d, 32);
		p.end();
		return true;
	}

	/**
	 * @brief Load the key-form role string from NVS
	 * @param out Destination buffer for the role string
	 * @param len Size of the destination buffer
	 */
	void loadRole(char *out, size_t len)
	{
		Preferences p;
		p.begin(NVS_NS, true);
		String r = p.getString(NVS_ROLE, "charging_manager");
		p.end();
		strncpy(out, r.c_str(), len - 1);
		out[len - 1] = '\0';
	}

	/**
	 * @brief Persist the key-form role string to NVS
	 * @param role Null-terminated role string (e.g. "owner", "charging_manager")
	 */
	void saveRole(const char *role)
	{
		Preferences p;
		p.begin(NVS_NS, false);
		p.putString(NVS_ROLE, role);
		p.end();
	}

	/**
	 * @brief Convert a role name string to its numeric protocol value
	 * @param role Null-terminated role string
	 * @return Protocol role byte (ROLE_OWNER or ROLE_CHARGING_MANAGER)
	 */
	uint8_t roleValue(const char *role)
	{
		if (strcmp(role, "owner") == 0)
			return ROLE_OWNER;
		return ROLE_CHARGING_MANAGER;
	}

	/**
	 * @brief Load the stored VIN from NVS
	 * @param out Destination buffer for the VIN string
	 * @param len Size of the destination buffer
	 */
	void loadVin(char *out, size_t len)
	{
		Preferences p;
		p.begin(NVS_NS, true);
		String v = p.getString(NVS_VIN, "");
		p.end();
		strncpy(out, v.c_str(), len - 1);
		out[len - 1] = '\0';
	}

	/**
	 * @brief Persist the vehicle VIN to NVS
	 * @param vin Null-terminated VIN string
	 */
	void saveVin(const char *vin)
	{
		Preferences p;
		p.begin(NVS_NS, false);
		p.putString(NVS_VIN, vin);
		p.end();
	}

	/**
	 * @brief Log an informational message via the serial output chain
	 * @param msg Null-terminated message string
	 */
	void log(const char *msg)
	{
		sendLog(msg);
	}

	/**
	 * @brief Log an error message via the serial output chain
	 * @param msg Null-terminated error message string
	 */
	void err(const char *msg)
	{
		sendError(msg);
	}
};

// Singleton board instance used by the convenience wrapper below
static Esp32Board s_board;

/**
 * @brief Convenience wrapper that dispatches a Tesla BLE sub-command using the singleton board
 * @param sub Null-terminated sub-command string (e.g. "unlock", "climate:on")
 */
static void executeTeslaCommand(const char *sub)
{
	Tesla::executeTeslaCommand(s_board, sub);
}

} // namespace Tesla

#endif // BOARD_ENABLE_BLE
