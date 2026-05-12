#pragma once

/**
 * @file firmware/test/support/fake_preferences.h
 * @brief Fake ESP32 Preferences (NVS) for native tests — in-memory key-value store
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <cstdint>
#include <cstring>
#include <map>
#include <string>

/**
 * @brief In-memory replacement for the ESP32 Preferences class used in native tests
 *
 * Stores values in a static map keyed by "namespace/key" so multiple test
 * namespaces can coexist. Call clearAll() between tests to reset state.
 */
class Preferences
{
  public:
	/**
	 * @brief Open a namespace (stored for key prefixing; readOnly flag is ignored)
	 * @param ns Namespace string
	 * @param readOnly Ignored in the fake implementation
	 */
	void begin(const char *ns, bool readOnly = false)
	{
		(void)readOnly;
		_ns = ns;
	}

	/**
	 * @brief No-op close (nothing to flush in memory)
	 */
	void end() {}

	/**
	 * @brief Store an unsigned 8-bit value
	 * @param key NVS key name
	 * @param val Value to store
	 */
	void putUChar(const char *key, uint8_t val)
	{
		_store()[fullKey(key)] = (uint64_t)val;
	}

	/**
	 * @brief Retrieve an unsigned 8-bit value
	 * @param key NVS key name
	 * @param def Default value if key is absent
	 * @return Stored value or def
	 */
	uint8_t getUChar(const char *key, uint8_t def = 0)
	{
		auto &s = _store();
		auto it = s.find(fullKey(key));
		return it != s.end() ? (uint8_t)it->second : def;
	}

	/**
	 * @brief Store a boolean value (internally 1 or 0)
	 * @param key NVS key name
	 * @param val Boolean to store
	 */
	void putBool(const char *key, bool val)
	{
		_store()[fullKey(key)] = val ? 1u : 0u;
	}

	/**
	 * @brief Retrieve a boolean value
	 * @param key NVS key name
	 * @param def Default value if key is absent
	 * @return Stored boolean or def
	 */
	bool getBool(const char *key, bool def = false)
	{
		auto &s = _store();
		auto it = s.find(fullKey(key));
		return it != s.end() ? (it->second != 0) : def;
	}

	/**
	 * @brief Store an unsigned 16-bit value
	 * @param key NVS key name
	 * @param val Value to store
	 */
	void putUShort(const char *key, uint16_t val)
	{
		_store()[fullKey(key)] = (uint64_t)val;
	}

	/**
	 * @brief Retrieve an unsigned 16-bit value
	 * @param key NVS key name
	 * @param def Default value if key is absent
	 * @return Stored value or def
	 */
	uint16_t getUShort(const char *key, uint16_t def = 0)
	{
		auto &s = _store();
		auto it = s.find(fullKey(key));
		return it != s.end() ? (uint16_t)it->second : def;
	}

	/**
	 * @brief Reset the entire in-memory store (call between tests)
	 */
	static void clearAll()
	{
		_store().clear();
	}

  private:
	std::string _ns;

	/**
	 * @brief Build a composite key from namespace and key name
	 * @param key Raw key name
	 * @return "namespace/key" string used as map key
	 */
	std::string fullKey(const char *key) const
	{
		return _ns + "/" + key;
	}

	/**
	 * @brief Access the singleton static store shared across all Preferences instances
	 * @return Reference to the global key-value map
	 */
	static std::map<std::string, uint64_t> &_store()
	{
		static std::map<std::string, uint64_t> store;
		return store;
	}
};
