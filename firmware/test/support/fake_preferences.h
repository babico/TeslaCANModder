#pragma once
// Fake Preferences (ESP32 NVS) for native tests — in-memory key-value store.

#include <cstdint>
#include <cstring>
#include <map>
#include <string>

class Preferences
{
  public:
	void begin(const char *ns, bool readOnly = false)
	{
		(void)readOnly;
		_ns = ns;
	}
	void end() {}

	void putUChar(const char *key, uint8_t val)
	{
		_store()[fullKey(key)] = (uint64_t)val;
	}

	uint8_t getUChar(const char *key, uint8_t def = 0)
	{
		auto &s = _store();
		auto it = s.find(fullKey(key));
		return it != s.end() ? (uint8_t)it->second : def;
	}

	void putBool(const char *key, bool val)
	{
		_store()[fullKey(key)] = val ? 1u : 0u;
	}

	bool getBool(const char *key, bool def = false)
	{
		auto &s = _store();
		auto it = s.find(fullKey(key));
		return it != s.end() ? (it->second != 0) : def;
	}

	void putUShort(const char *key, uint16_t val)
	{
		_store()[fullKey(key)] = (uint64_t)val;
	}

	uint16_t getUShort(const char *key, uint16_t def = 0)
	{
		auto &s = _store();
		auto it = s.find(fullKey(key));
		return it != s.end() ? (uint16_t)it->second : def;
	}

	static void clearAll()
	{
		_store().clear();
	}

  private:
	std::string _ns;

	std::string fullKey(const char *key) const
	{
		return _ns + "/" + key;
	}

	static std::map<std::string, uint64_t> &_store()
	{
		static std::map<std::string, uint64_t> store;
		return store;
	}
};
