#pragma once
// Fake Preferences (ESP32 NVS) for native tests — in-memory key-value store.

#include <cstdint>
#include <cstring>
#include <map>
#include <string>

class Preferences {
public:
  void begin(const char* ns, bool readOnly = false) {
    (void)readOnly;
    _ns = ns;
  }
  void end() {}

  void putUChar(const char* key, uint8_t val) {
    _store()[fullKey(key)] = val;
  }

  uint8_t getUChar(const char* key, uint8_t def = 0) {
    auto& s = _store();
    auto it = s.find(fullKey(key));
    return it != s.end() ? it->second : def;
  }

  static void clearAll() { _store().clear(); }

private:
  std::string _ns;

  std::string fullKey(const char* key) const {
    return _ns + "/" + key;
  }

  static std::map<std::string, uint8_t>& _store() {
    static std::map<std::string, uint8_t> store;
    return store;
  }
};
