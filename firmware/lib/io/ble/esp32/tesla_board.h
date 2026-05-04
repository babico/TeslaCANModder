#pragma once
// ── ESP32 board adapter for Tesla BLE ────────────────────────────────────────
//
// Esp32Board implements the Board concept required by tesla/ble/tesla.h using:
//   - Preferences (ESP32 NVS) for key/VIN/role persistence
//   - esp_random() for RNG
//   - TeslaClient (NimBLE) for BLE transport
//   - sendLog / sendError / printStr / printLn for logging
//
// This file also exposes a convenience wrapper matching the legacy call site:
//   Tesla::executeTeslaCommand(sub)  →  Tesla::executeTeslaCommand(s_board, sub)

#if BOARD_ENABLE_BLE

#include <Preferences.h>
#include "io/ble/esp32/ble_client.h"
#include "vehicle/ble/feature/key.h"
#include "vehicle/ble/handler/dispatch.h"
#include <stdint.h>
#include <string.h>

// Forward declarations (defined in io/serial/esp32/output.h via the include chain)
void sendLog(const char *msg);
void sendError(const char *msg);
void printStr(const char *s);
void printLn();

namespace Tesla {

class Esp32Board {
    static constexpr const char *NVS_NS   = "tesla";
    static constexpr const char *NVS_PRIV = "priv_d";
    static constexpr const char *NVS_ROLE = "key_role";
    static constexpr const char *NVS_VIN  = "vin";

public:
    // ── Transport ─────────────────────────────────────────────────────────────
    TeslaClient makeTransport() { return TeslaClient(); }

    // ── RNG ───────────────────────────────────────────────────────────────────
    void random(uint8_t *out, size_t len)
    {
        for (size_t i = 0; i < len; ) {
            uint32_t r = esp_random();
            size_t   n = len - i;
            if (n > 4) n = 4;
            memcpy(out + i, &r, n);
            i += n;
        }
    }

    // ── Key storage ───────────────────────────────────────────────────────────
    bool loadKey(KeyPair &kp)
    {
        Preferences p;
        p.begin(NVS_NS, true);
        size_t n = p.getBytesLength(NVS_PRIV);
        if (n != 32) { p.end(); kp.valid = false; return false; }
        p.getBytes(NVS_PRIV, kp.priv_d, 32);
        p.end();
        return keyDerivePublic(kp);
    }

    bool saveKey(const KeyPair &kp)
    {
        Preferences p;
        p.begin(NVS_NS, false);
        p.putBytes(NVS_PRIV, kp.priv_d, 32);
        p.end();
        return true;
    }

    // ── Role storage ──────────────────────────────────────────────────────────
    void loadRole(char *out, size_t len)
    {
        Preferences p;
        p.begin(NVS_NS, true);
        String r = p.getString(NVS_ROLE, "charging_manager");
        p.end();
        strncpy(out, r.c_str(), len - 1);
        out[len - 1] = '\0';
    }

    void saveRole(const char *role)
    {
        Preferences p;
        p.begin(NVS_NS, false);
        p.putString(NVS_ROLE, role);
        p.end();
    }

    uint8_t roleValue(const char *role)
    {
        if (strcmp(role, "owner") == 0) return ROLE_OWNER;
        return ROLE_CHARGING_MANAGER;
    }

    // ── VIN storage ───────────────────────────────────────────────────────────
    void loadVin(char *out, size_t len)
    {
        Preferences p;
        p.begin(NVS_NS, true);
        String v = p.getString(NVS_VIN, "");
        p.end();
        strncpy(out, v.c_str(), len - 1);
        out[len - 1] = '\0';
    }

    void saveVin(const char *vin)
    {
        Preferences p;
        p.begin(NVS_NS, false);
        p.putString(NVS_VIN, vin);
        p.end();
    }

    // ── Logging ───────────────────────────────────────────────────────────────
    void log(const char *msg)  { sendLog(msg);   }
    void err(const char *msg)  { sendError(msg); }
    void print(const char *s)  { printStr(s);    }
    void println()             { printLn();      }
};

// ── Convenience wrapper ───────────────────────────────────────────────────────
// Matches the existing call site in commands.h so no changes are needed there.
static Esp32Board s_board;
static void executeTeslaCommand(const char *sub)
{
    Tesla::executeTeslaCommand(s_board, sub);
}

} // namespace Tesla

#endif // BOARD_ENABLE_BLE
