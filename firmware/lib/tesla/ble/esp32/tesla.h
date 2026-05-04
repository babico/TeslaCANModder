#pragma once
// ── Tesla vehicle BLE command dispatch ───────────────────────────────────────
//
// Called from commands.h when cmd prefix is "tesla:".
// All operations block in the Arduino loop task.
//
// Supported commands:
//   tesla:key:gen                → generate + save ECDSA P-256 key pair
//   tesla:key:show               → emit {"t":"tesla_key","pub":"<hex65>","role":"..."}
//   tesla:key:role:owner         → store role = owner
//   tesla:key:role:charging_manager → store role = charging_manager
//   tesla:vin:<VIN>              → store default VIN (17 chars)
//   tesla:key:send               → VCSEC add-key-request to stored VIN (NFC tap needed)
//   tesla:wake                   → wake vehicle (authenticated)
//   tesla:charge:start           → charge start
//   tesla:charge:stop            → charge stop
//   tesla:charge:amps:<n>        → set charging amps (1–32)
//   tesla:charge:limit:<n>       → set charge limit % (50–100)
//   tesla:climate:on             → climate on
//   tesla:climate:off            → climate off

#if BOARD_ENABLE_BLE

#include "key.h"
#include "msg.h"
#include "ble_client.h"
#include "session.h"
#include <stdint.h>
#include <string.h>

// Forward declarations (defined in io/serial/esp32/output.h + common.h,
// which are included before this header via commands.h → messages.h chain)
void sendLog(const char *msg);
void sendError(const char *msg);
void printStr(const char *s);
void printLn();

namespace Tesla {

// ── helper: send a log line via the existing JSON logger ─────────────────────
static void tlog(const char *msg) { sendLog(msg); }
static void terr(const char *msg) { sendError(msg); }

// ── helper: connect to vehicle by stored VIN ──────────────────────────────────
static bool connectToVehicle(TeslaClient &client)
{
    String vin = vinGet();
    // Use last 8 characters of VIN as BLE name suffix for filtering, or empty.
    const char *suffix = nullptr;
    char        sfxBuf[9] = {};
    if (vin.length() >= 8) {
        vin.substring(vin.length() - 8).toCharArray(sfxBuf, sizeof(sfxBuf));
        suffix = sfxBuf;
    }

    tlog("Tesla: scanning for vehicle...");
    if (!client.connect(suffix, 10)) {
        terr("Tesla: vehicle not found via BLE");
        return false;
    }
    tlog("Tesla: connected");
    return true;
}

// ── helper: establish authenticated session + run an action ──────────────────
using ActionBuilder = bool (*)(uint8_t *out, size_t cap, size_t &outLen);

static bool runAuthCommand(ActionBuilder buildFn, const char *desc)
{
    TeslaClient client;
    if (!connectToVehicle(client)) return false;

    TeslaSession session;
    tlog("Tesla: establishing ECDH session...");
    if (!session.establish(client, DOMAIN_INFOTAINMENT)) {
        terr("Tesla: session establishment failed");
        client.disconnect();
        return false;
    }

    uint8_t actionBuf[128];
    size_t  actionLen = 0;
    if (!buildFn(actionBuf, sizeof(actionBuf), actionLen)) {
        terr("Tesla: message encoding failed");
        client.disconnect();
        return false;
    }

    if (!session.sendCommand(client, actionBuf, actionLen)) {
        terr("Tesla: command send failed");
        client.disconnect();
        return false;
    }

    tlog(desc);
    client.disconnect();
    return true;
}

// ── executeTeslaCommand ───────────────────────────────────────────────────────
// sub = cmd after the "tesla:" prefix (caller strips "tesla:")
static void executeTeslaCommand(const char *sub)
{
    // ── tesla:key:gen ─────────────────────────────────────────────────────────
    if (strcmp(sub, "key:gen") == 0) {
        KeyPair kp;
        if (!keyGenerate(kp)) { terr("Tesla: key generation failed"); return; }
        if (!keySave(kp))     { terr("Tesla: key save failed"); return; }
        tlog("Tesla: key generated and saved");
        return;
    }

    // ── tesla:key:show ────────────────────────────────────────────────────────
    if (strcmp(sub, "key:show") == 0) {
        KeyPair kp;
        if (!keyLoad(kp)) { terr("Tesla: no key stored (run tesla:key:gen)"); return; }

        char hexBuf[131]; // 65 bytes * 2 + NUL
        hexEncode(kp.pub_xy, 65, hexBuf);
        String role = keyRoleGet();

        // Emit JSON line: {"t":"tesla_key","pub":"<130 hex chars>","role":"..."}
        // Build it manually to avoid pulling in extra dependencies
        char line[256];
        snprintf(line, sizeof(line),
                 "{\"t\":\"tesla_key\",\"pub\":\"%s\",\"role\":\"%s\"}",
                 hexBuf, role.c_str());
        // printStr + printLn (already declared via forward-include chain)
        printStr(line);
        printLn();
        return;
    }

    // ── tesla:key:role:<role> ─────────────────────────────────────────────────
    if (strncmp(sub, "key:role:", 9) == 0) {
        const char *role = sub + 9;
        if (strcmp(role, "owner") == 0 || strcmp(role, "charging_manager") == 0) {
            keyRoleSet(role);
            char msg[48];
            snprintf(msg, sizeof(msg), "Tesla: role set to %s", role);
            tlog(msg);
        } else {
            terr("Tesla: unknown role (use 'owner' or 'charging_manager')");
        }
        return;
    }

    // ── tesla:vin:<VIN> ───────────────────────────────────────────────────────
    if (strncmp(sub, "vin:", 4) == 0) {
        const char *vin = sub + 4;
        if (strlen(vin) < 5 || strlen(vin) > 17) {
            terr("Tesla: VIN must be 5–17 characters");
            return;
        }
        vinSet(vin);
        char msg[48];
        snprintf(msg, sizeof(msg), "Tesla: VIN stored: %s", vin);
        tlog(msg);
        return;
    }

    // ── tesla:key:send ────────────────────────────────────────────────────────
    // Sends VCSEC add-key-request to vehicle; owner must tap NFC card to confirm.
    if (strcmp(sub, "key:send") == 0) {
        KeyPair kp;
        if (!keyLoad(kp)) { terr("Tesla: no key stored (run tesla:key:gen)"); return; }

        String roleStr  = keyRoleGet();
        uint8_t role    = roleValue(roleStr);

        TeslaClient client;
        if (!connectToVehicle(client)) return;

        uint8_t uuid[16]; genRandom16(uuid);
        uint8_t reqBuf[200];
        size_t  reqLen = 0;
        if (!buildAddKeyRequest(kp.pub_xy, role, KEY_FORM_FACTOR_NFC_CARD,
                                 uuid, reqBuf, sizeof(reqBuf), reqLen)) {
            terr("Tesla: add-key-request encoding failed");
            client.disconnect();
            return;
        }

        if (!client.send(reqBuf, reqLen)) {
            terr("Tesla: add-key-request send failed");
            client.disconnect();
            return;
        }

        tlog("Tesla: add-key-request sent - owner must tap NFC card on vehicle");
        client.disconnect();
        return;
    }

    // ── tesla:wake ────────────────────────────────────────────────────────────
    if (strcmp(sub, "wake") == 0) {
        runAuthCommand(buildWakeAction, "Tesla: wake sent");
        return;
    }

    // ── tesla:charge:start ────────────────────────────────────────────────────
    if (strcmp(sub, "charge:start") == 0) {
        runAuthCommand(buildChargeStartAction, "Tesla: charge start sent");
        return;
    }

    // ── tesla:charge:stop ─────────────────────────────────────────────────────
    if (strcmp(sub, "charge:stop") == 0) {
        runAuthCommand(buildChargeStopAction, "Tesla: charge stop sent");
        return;
    }

    // ── tesla:charge:amps:<n> ─────────────────────────────────────────────────
    if (strncmp(sub, "charge:amps:", 12) == 0) {
        int amps = atoi(sub + 12);
        if (amps < 1 || amps > 32) {
            terr("Tesla: amps must be 1–32");
            return;
        }
        // Capture amps in a lambda-compatible way via a static with care;
        // use a module-level trampoline approach instead:
        struct Ctx { int a; } ctx = { amps };
        (void)ctx;
        // We can't pass a closure to a function pointer; call directly.
        TeslaClient cl;
        if (!connectToVehicle(cl)) return;
        TeslaSession sess;
        if (!sess.establish(cl, DOMAIN_INFOTAINMENT)) {
            terr("Tesla: session failed"); cl.disconnect(); return;
        }
        uint8_t buf[64]; size_t len = 0;
        if (!buildSetAmpsAction(amps, buf, sizeof(buf), len)) {
            terr("Tesla: encoding failed"); cl.disconnect(); return;
        }
        if (!sess.sendCommand(cl, buf, len)) {
            terr("Tesla: send failed"); cl.disconnect(); return;
        }
        char msg[48]; snprintf(msg, sizeof(msg), "Tesla: charge amps set to %d", amps);
        tlog(msg);
        cl.disconnect();
        return;
    }

    // ── tesla:charge:limit:<n> ────────────────────────────────────────────────
    if (strncmp(sub, "charge:limit:", 13) == 0) {
        int pct = atoi(sub + 13);
        if (pct < 50 || pct > 100) {
            terr("Tesla: limit must be 50–100");
            return;
        }
        TeslaClient cl;
        if (!connectToVehicle(cl)) return;
        TeslaSession sess;
        if (!sess.establish(cl, DOMAIN_INFOTAINMENT)) {
            terr("Tesla: session failed"); cl.disconnect(); return;
        }
        uint8_t buf[64]; size_t len = 0;
        if (!buildSetLimitAction(pct, buf, sizeof(buf), len)) {
            terr("Tesla: encoding failed"); cl.disconnect(); return;
        }
        if (!sess.sendCommand(cl, buf, len)) {
            terr("Tesla: send failed"); cl.disconnect(); return;
        }
        char msg[48]; snprintf(msg, sizeof(msg), "Tesla: charge limit set to %d%%", pct);
        tlog(msg);
        cl.disconnect();
        return;
    }

    // ── tesla:climate:on ──────────────────────────────────────────────────────
    if (strcmp(sub, "climate:on") == 0) {
        TeslaClient cl;
        if (!connectToVehicle(cl)) return;
        TeslaSession sess;
        if (!sess.establish(cl, DOMAIN_INFOTAINMENT)) {
            terr("Tesla: session failed"); cl.disconnect(); return;
        }
        uint8_t buf[64]; size_t len = 0;
        if (!buildClimateAction(true, buf, sizeof(buf), len)) {
            terr("Tesla: encoding failed"); cl.disconnect(); return;
        }
        if (!sess.sendCommand(cl, buf, len)) {
            terr("Tesla: send failed"); cl.disconnect(); return;
        }
        tlog("Tesla: climate on sent");
        cl.disconnect();
        return;
    }

    // ── tesla:climate:off ─────────────────────────────────────────────────────
    if (strcmp(sub, "climate:off") == 0) {
        TeslaClient cl;
        if (!connectToVehicle(cl)) return;
        TeslaSession sess;
        if (!sess.establish(cl, DOMAIN_INFOTAINMENT)) {
            terr("Tesla: session failed"); cl.disconnect(); return;
        }
        uint8_t buf[64]; size_t len = 0;
        if (!buildClimateAction(false, buf, sizeof(buf), len)) {
            terr("Tesla: encoding failed"); cl.disconnect(); return;
        }
        if (!sess.sendCommand(cl, buf, len)) {
            terr("Tesla: send failed"); cl.disconnect(); return;
        }
        tlog("Tesla: climate off sent");
        cl.disconnect();
        return;
    }

    terr("Tesla: unknown sub-command");
}

} // namespace Tesla

#endif // BOARD_ENABLE_BLE
