/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/*
 * CanFeather ESP32 — Tesla FSD CAN Mod + WiFi Arayüzü
 * =====================================================
 * BU KODU YUKLE → ESP32 DevKit V1 (ESP-WROOM-32)
 * Arduino IDE: Tools > Board > ESP32 Dev Module
 *
 * DONANIM BAĞLANTISI:
 *   MCP2515    ESP32
 *   VCC   -->  VIN (5V)
 *   GND   -->  GND
 *   CS    -->  GPIO5
 *   MOSI  -->  GPIO23
 *   MISO  -->  GPIO19
 *   SCK   -->  GPIO18
 *   INT   -->  GPIO4  (kullanılmıyor)
 *
 *   Tesla X179 konnektör:
 *   MCP2515 CAN-H  -->  X179 Pin 13
 *   MCP2515 CAN-L  -->  X179 Pin 14
 *
 * KULLANIM:
 *   1. Kodu yükleyin
 *   2. WiFi: "CanFeather" ağına bağlanın  Şifre: tesla1234
 *   3. Tarayıcı: http://192.168.4.1
 *
 * GEREKLİ KÜTÜPHANELer:
 *   - mcp2515 by autowp            (Library Manager)
 *   - ArduinoJson v6 by Blanchon   (Library Manager — v6 seçin!)
 *   - WiFi + WebServer (ESP32 board paketi ile gelir)
 *
 * ÖNEMLİ NOTLAR:
 *   - MCP2515 modülündeki J1 jumper'ı çıkarın (120Ω terminasyon)
 *   - ESP32 3.3V mantık seviyesi kullanır
 */

#include <memory>
#include <SPI.h>
#include <mcp2515.h>          // autowp/arduino-mcp2515
#include <WiFi.h>             // ESP32 board paketi
#include <WebServer.h>        // ESP32 board paketi
#include <Update.h>           // ESP32 OTA guncelleme
#include <ArduinoJson.h>      // v6.x — StaticJsonDocument kullanıyor
#include <EEPROM.h>           // Kalici ayar kaydi

// ─────────────────────────────────────────────────────────────
//  AYARLAR — Sadece bu bölümü değiştirin
// ─────────────────────────────────────────────────────────────

#define WIFI_SSID      "CanFeather"
#define WIFI_PASSWORD  "tesla1234"

// HW4 FSD V14 options
#define ENABLE_APPROACHING_EMERGENCY_VEHICLE_DETECTION true
#define ENABLE_ISA_SPEED_CHIME_SUPPRESS false
// #define FORCE_FSD

// MCP2515 kristal frekansı — otomatik algılanır (8/16 MHz)

// ESP32 pin tanımları
#define PIN_LED        2
#define PIN_CS         5
#define PIN_SCK        18
#define PIN_MISO       19
#define PIN_MOSI       23

#define EEPROM_SIZE    140
#define EEPROM_MAGIC   0xCF  // Config gecerlilik bayragi
#define WIFI_MAGIC     0xAA  // WiFi ozel sifre bayragi
#define WIFI_EE_START  64    // EEPROM'da WiFi verisi baslangic adresi

// ─────────────────────────────────────────────────────────────
//  ÇALIŞMA ZAMANI KONFİGÜRASYONU
// ─────────────────────────────────────────────────────────────

struct Config {
  uint8_t hw              = 1;     // 0=Legacy  1=HW3  2=HW4
  uint8_t speedProfile    = 2;     // 0-4
  bool    fsdEnabled      = true;
  bool    profileOverride = false; // true = web profili kullan, CAN yok say
  bool    canInjection    = true;  // false = kopru seffaf
  bool    fsdSelectedInUI = false; // CAN'dan okunan FSD secim durumu
  bool    logEnabled      = true;
  uint16_t autoOffMinutes = 0;    // 0 = devre disi, >0 = X dakika sonra enjeksiyon kapanir
  uint16_t logFilter      = 0;    // 0 = tum ID'ler, >0 = sadece bu CAN ID
  bool    snifferMode     = false; // true = tum CAN trafigini yakala
  uint8_t crystal         = 1;    // 0=8MHz, 1=16MHz
  bool    isaSuppress     = ENABLE_ISA_SPEED_CHIME_SUPPRESS;
  bool    emergencyDetect = ENABLE_APPROACHING_EMERGENCY_VEHICLE_DETECTION;
  // Arac kontrol kalici ayarlar (EEPROM addr 11-14)
  uint8_t pedalMode        = 0;    // 0=Std, 1=Chill, 2=Sport
  uint8_t regenLevel       = 100;  // 0=OFF, 50=Low, 100=Std, 200=Max
  uint8_t stopMode         = 2;    // 0=Creep, 1=Roll, 2=Hold
  bool    sentryActive     = false;
};

Config cfg;

// WiFi ayarlari (ayri struct — EEPROM offset 64'ten baslar)
char wifiSSID[33] = "CanFeather";
char wifiPASS[33] = "tesla1234";

// ─────────────────────────────────────────────────────────────
//  ARAC KONTROL — Frame buffer'lari (dinle-degistir-gonder)
// ─────────────────────────────────────────────────────────────

static uint8_t buf_273[8] = {};   // UI_vehicleControl
static uint8_t buf_2F3[5] = {};   // UI_hvacRequest
static uint8_t buf_333[5] = {};   // UI_chargeRequest
static uint8_t buf_334[8] = {};   // UI_powertrainControl
static bool has_273 = false;
static bool has_2F3 = false;
static bool has_333 = false;
static bool has_334 = false;
static bool bootApplied = false; // Kalici ayarlar boot'ta uygulandi mi?

// ─────────────────────────────────────────────────────────────
//  EEPROM — Kalici ayar kaydi
// ─────────────────────────────────────────────────────────────

void saveConfig() {
  EEPROM.write(0, EEPROM_MAGIC);
  EEPROM.write(1, cfg.hw);
  EEPROM.write(2, cfg.speedProfile);
  EEPROM.write(3, cfg.fsdEnabled ? 1 : 0);
  EEPROM.write(4, cfg.profileOverride ? 1 : 0);
  EEPROM.write(5, cfg.canInjection ? 1 : 0);
  EEPROM.write(6, cfg.autoOffMinutes & 0xFF);
  EEPROM.write(7, (cfg.autoOffMinutes >> 8) & 0xFF);
  EEPROM.write(8, cfg.crystal);
  EEPROM.write(9, cfg.isaSuppress ? 1 : 0);
  EEPROM.write(10, cfg.emergencyDetect ? 1 : 0);
  // Arac kontrol kalici ayarlar (addr 11-14)
  EEPROM.write(11, cfg.pedalMode);
  EEPROM.write(12, cfg.regenLevel);
  EEPROM.write(13, cfg.stopMode);
  EEPROM.write(14, cfg.sentryActive ? 1 : 0);
  EEPROM.commit();
  slog("[eeprom] Ayarlar kaydedildi");
}

void saveWiFiConfig() {
  EEPROM.write(WIFI_EE_START, WIFI_MAGIC);
  for (int i = 0; i < 32; i++) EEPROM.write(WIFI_EE_START + 1 + i, wifiSSID[i]);
  for (int i = 0; i < 32; i++) EEPROM.write(WIFI_EE_START + 33 + i, wifiPASS[i]);
  EEPROM.commit();
  slog("[wifi] SSID/sifre EEPROM'a kaydedildi");
}

void loadWiFiConfig() {
  if (EEPROM.read(WIFI_EE_START) != WIFI_MAGIC) return;
  for (int i = 0; i < 32; i++) wifiSSID[i] = EEPROM.read(WIFI_EE_START + 1 + i);
  for (int i = 0; i < 32; i++) wifiPASS[i] = EEPROM.read(WIFI_EE_START + 33 + i);
  wifiSSID[32] = '\0';
  wifiPASS[32] = '\0';
  Serial.printf("[wifi] EEPROM'dan yuklendi: SSID=%s\n", wifiSSID);
}

void resetWiFiConfig() {
  strncpy(wifiSSID, "CanFeather", 32);
  strncpy(wifiPASS, "tesla1234", 32);
  EEPROM.write(WIFI_EE_START, 0xFF);
  EEPROM.commit();
  slog("[wifi] Fabrika ayarina donuldu: CanFeather / tesla1234");
}

void loadConfig() {
  if (EEPROM.read(0) != EEPROM_MAGIC) {
    Serial.println("[eeprom] Kayitli ayar yok, varsayilan kullaniliyor");
    return;
  }
  cfg.hw              = EEPROM.read(1);
  cfg.speedProfile    = EEPROM.read(2);
  cfg.fsdEnabled      = EEPROM.read(3) == 1;
  cfg.profileOverride = EEPROM.read(4) == 1;
  cfg.canInjection    = EEPROM.read(5) == 1;
  cfg.autoOffMinutes  = EEPROM.read(6) | (EEPROM.read(7) << 8);
  cfg.crystal         = EEPROM.read(8);
  if (cfg.crystal > 1) cfg.crystal = 1; // gecersiz deger korumasi
  cfg.isaSuppress     = EEPROM.read(9) == 1;
  cfg.emergencyDetect = EEPROM.read(10) == 1;
  // Arac kontrol kalici ayarlar
  cfg.pedalMode       = EEPROM.read(11);
  cfg.regenLevel      = EEPROM.read(12);
  cfg.stopMode        = EEPROM.read(13);
  cfg.sentryActive    = EEPROM.read(14) == 1;
  if (cfg.pedalMode > 2) cfg.pedalMode = 0;
  if (cfg.stopMode > 2) cfg.stopMode = 2;
  Serial.printf("[eeprom] Yuklendi: hw=%d p=%d fsd=%d pedal=%d regen=%d stop=%d sentry=%d\n",
                cfg.hw, cfg.speedProfile, cfg.fsdEnabled, cfg.pedalMode, cfg.regenLevel, cfg.stopMode, cfg.sentryActive);
}

// ─────────────────────────────────────────────────────────────
//  DURUM TAKIBI
// ─────────────────────────────────────────────────────────────

static uint8_t  lastFollowDist = 0;
static int      lastSpeedOff   = 0;
static uint32_t rxCount = 0, txCount = 0, errCount = 0;
static unsigned long startMillis = 0;
static unsigned long autoOffStart = 0; // autoOff zamanlayici baslangici

// Trafik gecmisi — son 60 saniye, saniyede 1 ornek
#define HIST_LEN 60
static uint16_t rxHist[HIST_LEN] = {0};
static uint8_t  histIdx = 0;
static unsigned long lastHistUpdate = 0;
static uint32_t rxSinceLastHist = 0;

// FSD durum gecmisi
#define FSD_HIST_LEN 16
struct FSDEvent {
  unsigned long ts;  // millis
  bool state;        // true = FSD acildi, false = kapandi
};
static FSDEvent fsdHist[FSD_HIST_LEN];
static uint8_t fsdHistIdx = 0;
static bool lastFSDState = false;

void trackFSDState(bool current) {
  if (current != lastFSDState) {
    fsdHist[fsdHistIdx % FSD_HIST_LEN] = { millis() - startMillis, current };
    fsdHistIdx++;
    lastFSDState = current;
  }
}

// Sniffer tamponu
#define SNIF_CAP 16
static can_frame snifBuf[SNIF_CAP];
static uint8_t snifIdx = 0;

// ─────────────────────────────────────────────────────────────
//  CAN LOG TAMPONU
// ─────────────────────────────────────────────────────────────

#define LOG_CAP 20
static String logBuf[LOG_CAP];
static int    logIdx = 0;

void pushLog(const String& s, uint16_t canId = 0) {
  // Filtre kontrolu
  if (cfg.logFilter > 0 && canId > 0 && canId != cfg.logFilter) return;
  logBuf[logIdx % LOG_CAP] = s;
  logIdx++;
  if (cfg.logEnabled) Serial.println(s);
}

// ─────────────────────────────────────────────────────────────
//  SERIAL LOG (slog) — config / EEPROM / hata mesajlari
// ─────────────────────────────────────────────────────────────

#define SLOG_CAP 16
static String slogBuf[SLOG_CAP];
static int    slogIdx = 0;

void slog(const String& s) {
  Serial.println(s);
  slogBuf[slogIdx % SLOG_CAP] = s;
  slogIdx++;
}

// ─────────────────────────────────────────────────────────────
//  MCP2515
// ─────────────────────────────────────────────────────────────

std::unique_ptr<MCP2515> mcp;

// ─────────────────────────────────────────────────────────────
//  CAN YARDIMCI FONKSİYONLARI
// ─────────────────────────────────────────────────────────────

inline uint8_t muxID(const can_frame& f)     { return f.data[0] & 0x07; }
inline bool    fsdInUI(const can_frame& f)   { return (f.data[4] >> 6) & 0x01; }
inline uint8_t followDist(const can_frame& f){ return (f.data[5] & 0xE0) >> 5; }

inline void setBit(can_frame& f, uint8_t bit, bool val) {
  uint8_t mask = 1u << (bit % 8);
  if (val) f.data[bit/8] |=  mask;
  else     f.data[bit/8] &= ~mask;
}

inline void setSpeedV12V13(can_frame& f, int p) {
  f.data[6] = (f.data[6] & ~0x06) | ((p & 0x03) << 1);
}

inline void canSend(can_frame& f) { mcp->sendMessage(&f); txCount++; }

// ─────────────────────────────────────────────────────────────
//  LEGACY HANDLER
// ─────────────────────────────────────────────────────────────

void handleLegacy(can_frame& f) {
  if (f.can_id == 69) {
    uint8_t pos = f.data[1] >> 5;
    if (!cfg.profileOverride) {
      if      (pos <= 1) cfg.speedProfile = 2;
      else if (pos == 2) cfg.speedProfile = 1;
      else               cfg.speedProfile = 0;
    }
    pushLog("LEGACY id=69 pos=" + String(pos) + " p=" + String(cfg.speedProfile), 69);
    return;
  }
  if (f.can_id != 1006) return;
  uint8_t idx = muxID(f);
  if (idx == 0) cfg.fsdSelectedInUI = fsdInUI(f);
#ifdef FORCE_FSD
  if (idx == 0 && cfg.fsdEnabled) {
#else
  if (idx == 0 && cfg.fsdSelectedInUI && cfg.fsdEnabled) {
#endif
    setBit(f, 46, true);
    setSpeedV12V13(f, cfg.speedProfile);
    canSend(f);
    pushLog("LEGACY mux=0 FSD=1 p=" + String(cfg.speedProfile), 1006);
  }
  if (idx == 1) { setBit(f, 19, false); canSend(f); }
}

// ─────────────────────────────────────────────────────────────
//  HW3 HANDLER
// ─────────────────────────────────────────────────────────────

void handleHW3(can_frame& f) {
  if (f.can_id == 1016) {
    lastFollowDist = followDist(f);
    pushLog("HW3 id=1016 fd=" + String(lastFollowDist), 1016);
    return;
  }
  if (f.can_id != 1021) return;

  uint8_t idx = muxID(f);
  if (idx == 0) cfg.fsdSelectedInUI = fsdInUI(f);
#ifdef FORCE_FSD
  bool fsdOn = cfg.fsdEnabled;
#else
  bool fsdOn = cfg.fsdSelectedInUI && cfg.fsdEnabled;
#endif

  if (idx == 0 && fsdOn) {
    int off      = (int)((f.data[3] >> 1) & 0x3F) - 30;
    int speedOff = constrain(off * 5, 0, 100);
    lastSpeedOff = speedOff;
    int p        = (off == 2) ? 2 : (off == 1) ? 1 : 0;
    if (!cfg.profileOverride) cfg.speedProfile = p;
    setBit(f, 46, true);
    setSpeedV12V13(f, cfg.speedProfile);
    canSend(f);
    pushLog("HW3 mux=0 FSD=1 p=" + String(cfg.speedProfile) + " off=" + String(speedOff), 1021);
  }
  if (idx == 1) { setBit(f, 19, false); canSend(f); }
  if (idx == 2 && fsdOn) {
    int off      = (int)((f.data[3] >> 1) & 0x3F) - 30;
    int speedOff = constrain(off * 5, 0, 100);
    f.data[0] = (f.data[0] & ~0xC0) | ((speedOff & 0x03) << 6);
    f.data[1] = (f.data[1] & ~0x3F) | (speedOff >> 2);
    canSend(f);
  }
}

// ─────────────────────────────────────────────────────────────
//  HW4 HANDLER
// ─────────────────────────────────────────────────────────────

void handleHW4(can_frame& f) {
  // ISA Speed Chime Suppress (CAN ID 921)
  if (cfg.isaSuppress && f.can_id == 921) {
    f.data[1] |= 0x20;
    uint8_t sum = 0;
    for (int i = 0; i < 7; i++) sum += f.data[i];
    sum += (921 & 0xFF) + (921 >> 8);
    f.data[7] = sum & 0xFF;
    canSend(f);
    return;
  }
  if (f.can_id == 1016) {
    uint8_t fd = followDist(f);
    lastFollowDist = fd;
    int p = 0;
    switch (fd) {
      case 1: p=3; break; case 2: p=2; break; case 3: p=1; break;
      case 4: p=0; break; case 5: p=4; break;
    }
    if (!cfg.profileOverride) cfg.speedProfile = p;
    pushLog("HW4 id=1016 fd=" + String(fd) + " p=" + String(cfg.speedProfile), 1016);
    return;
  }
  if (f.can_id != 1021) return;

  uint8_t idx = muxID(f);
  if (idx == 0) cfg.fsdSelectedInUI = fsdInUI(f);
#ifdef FORCE_FSD
  bool fsdOn = cfg.fsdEnabled;
#else
  bool fsdOn = cfg.fsdSelectedInUI && cfg.fsdEnabled;
#endif

  if (idx == 0 && fsdOn) {
    setBit(f, 46, true); setBit(f, 60, true);
    if (cfg.emergencyDetect) {
      setBit(f, 59, true);
    }
    canSend(f);
    pushLog("HW4 mux=0 FSD=1 p=" + String(cfg.speedProfile), 1021);
  }
  if (idx == 1) { setBit(f, 19, false); setBit(f, 47, true); canSend(f); }
  if (idx == 2) {
    f.data[7] = (f.data[7] & ~0x70) | ((cfg.speedProfile & 0x07) << 4);
    canSend(f);
  }
}

// ─────────────────────────────────────────────────────────────
//  ANA FRAME YÖNLENDİRİCİ
// ─────────────────────────────────────────────────────────────

void processFrame(can_frame& f) {
  // FSD durum takibi (enjeksiyon kapali olsa bile)
  trackFSDState(cfg.fsdSelectedInUI && cfg.fsdEnabled);

  // Sniffer modu
  if (cfg.snifferMode) {
    snifBuf[snifIdx % SNIF_CAP] = f;
    snifIdx++;
  }

  // ── Arac kontrol frame buffer'larini surekli guncelle ──
  if (f.can_id == 0x273) { memcpy(buf_273, f.data, 8); has_273 = true; }
  else if (f.can_id == 0x2F3) { memcpy(buf_2F3, f.data, 5); has_2F3 = true; }
  else if (f.can_id == 0x333) { memcpy(buf_333, f.data, 5); has_333 = true; }
  else if (f.can_id == 0x334) { memcpy(buf_334, f.data, 8); has_334 = true; }

  // ── Boot'ta kalici ayarlari otomatik uygula (VehicleBus yakalaninca) ──
  if (!bootApplied && has_273 && has_334) {
    bootApplied = true;
    bool hasNonDefault = (cfg.pedalMode != 0 || cfg.regenLevel != 100 || cfg.stopMode != 2 || cfg.sentryActive);
    if (hasNonDefault) {
      slog("[boot] Kalici ayarlar uygulaniyor...");
      if (cfg.pedalMode == 2)      sendWithChecksum334(0, 0x60, 0x40, "Boot: Pedal Sport");
      else if (cfg.pedalMode == 1) sendWithChecksum334(0, 0x60, 0x20, "Boot: Pedal Chill");
      if (cfg.regenLevel != 100)   sendRegen334(cfg.regenLevel, "Boot: Regen");
      if (cfg.stopMode != 2)       sendWithChecksum334(5, 0x03, cfg.stopMode, "Boot: Stop");
      if (cfg.sentryActive) {
        uint8_t d[5] = {0x20,0,0,0,0};
        sendInjectFrame(0x284, 5, d, 30, "Boot: Sentry ON");
      }
    }
  }

  if (!cfg.canInjection || !cfg.fsdEnabled) return;
  switch (cfg.hw) {
    case 0: handleLegacy(f); break;
    case 1: handleHW3(f);    break;
    case 2: handleHW4(f);    break;
  }
}

// ─────────────────────────────────────────────────────────────
//  ARAC KONTROL KOMUTLARI (10 ozellik)
// ─────────────────────────────────────────────────────────────

// Dinle-degistir-gonder: frame yakalanmissa kullan, yoksa uyar
// Dinle-degistir-gonder: bus'tan frame yakalaninca kullanilir

// Frame yakalanmadiysa komut bekler — CAN hattinda ilgili mesaj gorununce aktif olur

// Bus'tan oku — buf'lari guncelle (komut gonderme dongusu icinde kullanilir)
void refreshBufs() {
  can_frame rx;
  while (mcp->readMessage(&rx) == MCP2515::ERROR_OK) {
    rxCount++;
    if (rx.can_id == 0x273) { memcpy(buf_273, rx.data, 8); has_273 = true; }
    else if (rx.can_id == 0x2F3) { memcpy(buf_2F3, rx.data, 5); has_2F3 = true; }
    else if (rx.can_id == 0x333) { memcpy(buf_333, rx.data, 5); has_333 = true; }
    else if (rx.can_id == 0x334) { memcpy(buf_334, rx.data, 8); has_334 = true; }
  }
}

void sendModified273(uint8_t byteIdx, uint8_t mask, uint8_t val, uint8_t repeat, const char* label) {
  if (!has_273) { slog("[cmd] 0x273 henuz yakalanmadi, bekleniyor"); return; }
  can_frame f; memset(&f, 0, sizeof(f));
  f.can_id = 0x273; f.can_dlc = 8;
  for (uint8_t i = 0; i < repeat; i++) {
    refreshBufs();
    memcpy(f.data, buf_273, 8);
    f.data[byteIdx] = (f.data[byteIdx] & ~mask) | (val & mask);
    canSend(f); delay(20); yield();
  }
  slog(String("[cmd] ") + label + " OK");
}

void sendBitSet273(uint8_t byteIdx, uint8_t bitMask, uint8_t repeat, const char* label) {
  if (!has_273) { slog("[cmd] 0x273 henuz yakalanmadi, bekleniyor"); return; }
  can_frame f; memset(&f, 0, sizeof(f));
  f.can_id = 0x273; f.can_dlc = 8;
  for (uint8_t i = 0; i < repeat; i++) {
    refreshBufs();
    memcpy(f.data, buf_273, 8);
    f.data[byteIdx] |= bitMask;
    canSend(f); delay(20); yield();
  }
  slog(String("[cmd] ") + label + " OK");
}

void sendInjectFrame(uint16_t id, uint8_t dlc, const uint8_t* data, uint8_t repeat, const char* label) {
  can_frame f; memset(&f, 0, sizeof(f));
  f.can_id = id; f.can_dlc = dlc;
  memcpy(f.data, data, dlc);
  for (uint8_t i = 0; i < repeat; i++) { canSend(f); delay(100); yield(); }
  slog(String("[cmd] ") + label + " OK");
}

void sendWithChecksum334(uint8_t byteIdx, uint8_t mask, uint8_t val, const char* label) {
  if (!has_334) { slog("[cmd] 0x334 henuz yakalanmadi, bekleniyor"); return; }
  can_frame f; memset(&f, 0, sizeof(f));
  f.can_id = 0x334; f.can_dlc = 8;
  uint8_t cnt = (buf_334[6] >> 4) & 0x0F;
  for (uint8_t i = 0; i < 30; i++) {
    refreshBufs();
    memcpy(f.data, buf_334, 8);
    f.data[byteIdx] = (f.data[byteIdx] & ~mask) | (val & mask);
    cnt = (cnt + 1) & 0x0F;
    f.data[6] = (f.data[6] & ~0xF0) | (cnt << 4);
    uint8_t sum = 0;
    for (int j = 0; j < 7; j++) sum += f.data[j];
    sum += (820 & 0xFF) + (820 >> 8);
    f.data[7] = sum & 0xFF;
    canSend(f); delay(20); yield();
  }
  slog(String("[cmd] ") + label + " OK");
}

void sendRegen334(uint8_t regenVal, const char* label) {
  if (!has_334) { slog("[cmd] 0x334 henuz yakalanmadi, bekleniyor"); return; }
  can_frame f; memset(&f, 0, sizeof(f));
  f.can_id = 0x334; f.can_dlc = 8;
  uint8_t cnt = (buf_334[6] >> 4) & 0x0F;
  for (uint8_t i = 0; i < 30; i++) {
    refreshBufs();
    memcpy(f.data, buf_334, 8);
    f.data[3] = regenVal;
    cnt = (cnt + 1) & 0x0F;
    f.data[6] = (f.data[6] & ~0xF0) | (cnt << 4);
    uint8_t sum = 0;
    for (int j = 0; j < 7; j++) sum += f.data[j];
    sum += (820 & 0xFF) + (820 >> 8);
    f.data[7] = sum & 0xFF;
    canSend(f); delay(20); yield();
  }
  slog(String("[cmd] ") + label + " OK");
}

void execVehicleCmd(const String& cmd) {
  // ══════════════════════════════════════════════════════════
  //  0x273 UI_vehicleControl — AYNI BUS'TA, dinle-degistir-gonder
  //  DBC: BO_ 627 ID273UI_vehicleControl: 8 VehicleBus
  // ══════════════════════════════════════════════════════════

  // ── Ayna (mirrorFoldRequest 24|2) → 0=idle, 1=Katla, 2=Ac ──
  if (cmd == "mirror_fold")   { sendModified273(3, 0x03, 0x01, 50, "Ayna Katla"); return; }
  if (cmd == "mirror_unfold") { sendModified273(3, 0x03, 0x02, 50, "Ayna Ac"); return; }
  // ── Ayna Isitma (mirrorHeatRequest 26|1) → 1=ON ──
  if (cmd == "mirror_heat")   { sendBitSet273(3, 0x04, 30, "Ayna Isitma"); return; }
  // ── Oto Ayna Katlama (autoFoldMirrorsOn 52|1) → toggle ──
  if (cmd == "mirror_autofold") { sendBitSet273(6, 0x10, 30, "Oto Ayna Katlama"); return; }
  // ── Geri Vites Ayna Egim (mirrorDipOnReverse 53|1) → toggle ──
  if (cmd == "mirror_dip")    { sendBitSet273(6, 0x20, 30, "Geri Ayna Egim"); return; }

  // ── Kilit (lockRequest 17|3) → 0=idle, 1=Lock, 2=Unlock ──
  if (cmd == "lock")          { sendModified273(2, 0x0E, 0x02, 30, "Kilitle"); return; }
  if (cmd == "unlock")        { sendModified273(2, 0x0E, 0x04, 30, "Kilidi Ac"); return; }
  // ── Cocuk Kilidi (childDoorLockOn 16|1) → toggle ──
  if (cmd == "child_lock")    { sendBitSet273(2, 0x01, 30, "Cocuk Kilidi"); return; }

  // ── Frunk (frunkRequest 5|1) → 1=Ac ──
  if (cmd == "frunk")         { sendBitSet273(0, 0x20, 20, "Frunk Ac"); return; }
  // ── Korna (honkHorn 61|1) → 1=Cal ──
  if (cmd == "horn")          { sendBitSet273(7, 0x20, 30, "Korna"); return; }

  // ── Isiklar ──
  // frontFogSwitch 3|1 → 1=ON
  if (cmd == "fog_front")     { sendBitSet273(0, 0x08, 30, "On Sis"); return; }
  // rearFogSwitch 23|1 → 1=ON
  if (cmd == "fog_rear")      { sendBitSet273(2, 0x80, 30, "Arka Sis"); return; }
  // autoHighBeamEnabled 41|1 → 1=ON
  if (cmd == "highbeam_auto") { sendBitSet273(5, 0x02, 30, "Oto Uzun Far"); return; }
  // ambientLightingEnabled 40|1 → 1=ON
  if (cmd == "ambient_light") { sendBitSet273(5, 0x01, 30, "Ambiyans Isik"); return; }
  // seeYouHomeLightingOn 30|1 → 1=ON
  if (cmd == "homelight")     { sendBitSet273(3, 0x40, 30, "Coming Home"); return; }
  // domeLightSwitch 59|2 → 0=off, 1=on, 2=auto
  if (cmd == "dome_off")      { sendModified273(7, 0x18, 0x00, 30, "Tavan Isik OFF"); return; }
  if (cmd == "dome_on")       { sendModified273(7, 0x18, 0x08, 30, "Tavan Isik ON"); return; }
  if (cmd == "dome_auto")     { sendModified273(7, 0x18, 0x10, 30, "Tavan Isik Auto"); return; }

  // ── Silecek (wiperRequest 56|3) → 0=off, 1-6 hiz ──
  if (cmd == "wiper_off")     { sendModified273(7, 0x07, 0x00, 20, "Silecek OFF"); return; }
  if (cmd == "wiper_1")       { sendModified273(7, 0x07, 0x01, 20, "Silecek 1"); return; }
  if (cmd == "wiper_2")       { sendModified273(7, 0x07, 0x02, 20, "Silecek 2"); return; }
  if (cmd == "wiper_3")       { sendModified273(7, 0x07, 0x03, 20, "Silecek 3"); return; }

  // ── Koltuk Isitma (2b enum: 0=Off, 1=Low, 2=Med, 3=High) ──
  if (cmd.startsWith("seat_fl")) { uint8_t v=cmd.substring(8).toInt()&0x03; sendModified273(5,0x0C,v<<2,30,"Koltuk FL"); return; }
  if (cmd.startsWith("seat_fr")) { uint8_t v=cmd.substring(8).toInt()&0x03; sendModified273(5,0x30,v<<4,30,"Koltuk FR"); return; }
  if (cmd.startsWith("seat_rl")) { uint8_t v=cmd.substring(8).toInt()&0x03; sendModified273(5,0xC0,v<<6,30,"Koltuk RL"); return; }
  if (cmd.startsWith("seat_rr")) { uint8_t v=cmd.substring(8).toInt()&0x03; sendModified273(6,0x0C,v<<2,30,"Koltuk RR"); return; }
  if (cmd.startsWith("seat_rc")) { uint8_t v=cmd.substring(8).toInt()&0x03; sendModified273(6,0x03,v,30,"Koltuk RC"); return; }

  // ── Ekran Parlakligi (displayBrightnessLevel 32|8, factor=0.5) 0-127 ──
  if (cmd.startsWith("bright")) { uint8_t v=cmd.substring(7).toInt()&0xFF; sendModified273(4,0xFF,v,20,"Parlaklik"); return; }

  // ── Diger tek-bit toggle'lar ──
  // summonActive 4|1
  if (cmd == "summon")        { sendBitSet273(0, 0x10, 30, "Summon"); return; }
  // accessoryPowerRequest 0|1
  if (cmd == "acc_power")     { sendBitSet273(0, 0x01, 30, "Aksesuar Guc"); return; }
  // powerOff 31|1
  if (cmd == "power_off")     { sendBitSet273(3, 0x80, 30, "Arac Kapat"); return; }
  // driveStateRequest 62|1
  if (cmd == "drive_state")   { sendBitSet273(7, 0x40, 30, "Surus Durumu"); return; }

  // ══════════════════════════════════════════════════════════
  //  FARKLI CAN ID'ler — ayni bus'ta olabilir veya olmayabilir
  // ══════════════════════════════════════════════════════════

  // ── Cam Vent (0x119) ──
  if (cmd == "vent_open")     { uint8_t d[2]={0x1F,100}; sendInjectFrame(0x119,2,d,30,"Cam Ac"); return; }
  if (cmd == "vent_close")    { uint8_t d[2]={0x1F,0};   sendInjectFrame(0x119,2,d,30,"Cam Kapat"); return; }
  // ── Torpido / Bagaj (0x3B3) ──
  if (cmd == "glovebox")      { uint8_t d[4]={0x01,0,0,0}; sendInjectFrame(0x3B3,4,d,20,"Torpido Ac"); return; }
  if (cmd == "trunk")         { uint8_t d[4]={0x02,0,0,0}; sendInjectFrame(0x3B3,4,d,20,"Bagaj Ac"); return; }
  // ── Sentry (0x284) ──
  if (cmd == "sentry_on")     { uint8_t d[5]={0x20,0,0,0,0}; sendInjectFrame(0x284,5,d,30,"Sentry ON"); cfg.sentryActive=true; saveConfig(); return; }
  if (cmd == "sentry_off")    { uint8_t d[5]={0x00,0,0,0,0}; sendInjectFrame(0x284,5,d,30,"Sentry OFF"); cfg.sentryActive=false; saveConfig(); return; }

  // ── Klima (0x2F3) ──
  if (cmd == "climate_keep") {
    if (!has_2F3) { slog("[cmd] 0x2F3 henuz yakalanmadi"); return; }
    can_frame cf; memset(&cf,0,sizeof(cf)); cf.can_id=0x2F3; cf.can_dlc=5;
    for(uint8_t i=0;i<30;i++){refreshBufs();memcpy(cf.data,buf_2F3,5);cf.data[4]=(cf.data[4]&~0x06)|(1<<1);canSend(cf);delay(20);yield();}
    slog("[cmd] Klima Acik Tut OK"); return;
  }
  if (cmd == "climate_off") {
    if (!has_2F3) { slog("[cmd] 0x2F3 henuz yakalanmadi"); return; }
    can_frame cf; memset(&cf,0,sizeof(cf)); cf.can_id=0x2F3; cf.can_dlc=5;
    for(uint8_t i=0;i<30;i++){refreshBufs();memcpy(cf.data,buf_2F3,5);cf.data[4]=(cf.data[4]&~0x06);canSend(cf);delay(20);yield();}
    slog("[cmd] Klima Kapat OK"); return;
  }
  // ── Sarj (0x333) ──
  if (cmd == "charge_start") {
    if (!has_333) { slog("[cmd] 0x333 henuz yakalanmadi"); return; }
    can_frame cf; memset(&cf,0,sizeof(cf)); cf.can_id=0x333; cf.can_dlc=5;
    for(uint8_t i=0;i<20;i++){refreshBufs();memcpy(cf.data,buf_333,5);cf.data[0]|=0x04;canSend(cf);delay(20);yield();}
    slog("[cmd] Sarj Baslat OK"); return;
  }
  if (cmd == "charge_stop") {
    if (!has_333) { slog("[cmd] 0x333 henuz yakalanmadi"); return; }
    can_frame cf; memset(&cf,0,sizeof(cf)); cf.can_id=0x333; cf.can_dlc=5;
    for(uint8_t i=0;i<20;i++){refreshBufs();memcpy(cf.data,buf_333,5);cf.data[0]&=~0x04;canSend(cf);delay(20);yield();}
    slog("[cmd] Sarj Durdur OK"); return;
  }
  if (cmd == "charge_port") {
    if (!has_333) { slog("[cmd] 0x333 henuz yakalanmadi"); return; }
    can_frame cf; memset(&cf,0,sizeof(cf)); cf.can_id=0x333; cf.can_dlc=5;
    for(uint8_t i=0;i<20;i++){refreshBufs();memcpy(cf.data,buf_333,5);cf.data[0]|=0x01;canSend(cf);delay(20);yield();}
    slog("[cmd] Sarj Port Ac OK"); return;
  }
  // 10. Pedal / Regen / Durma (checksum+counter gerekli, EEPROM'a kaydedilir)
  if (cmd == "pedal_sport")   { sendWithChecksum334(0,0x60,0x40,"Pedal Sport"); cfg.pedalMode=2; saveConfig(); return; }
  if (cmd == "pedal_chill")   { sendWithChecksum334(0,0x60,0x20,"Pedal Chill"); cfg.pedalMode=1; saveConfig(); return; }
  if (cmd == "pedal_std")     { sendWithChecksum334(0,0x60,0x00,"Pedal Std"); cfg.pedalMode=0; saveConfig(); return; }
  if (cmd == "regen_off")     { sendRegen334(0,"Regen OFF"); cfg.regenLevel=0; saveConfig(); return; }
  if (cmd == "regen_low")     { sendRegen334(50,"Regen Low"); cfg.regenLevel=50; saveConfig(); return; }
  if (cmd == "regen_std")     { sendRegen334(100,"Regen Std"); cfg.regenLevel=100; saveConfig(); return; }
  if (cmd == "regen_max")     { sendRegen334(200,"Regen Max"); cfg.regenLevel=200; saveConfig(); return; }
  if (cmd == "stop_creep")    { sendWithChecksum334(5,0x03,0x00,"Creep"); cfg.stopMode=0; saveConfig(); return; }
  if (cmd == "stop_roll")     { sendWithChecksum334(5,0x03,0x01,"Roll"); cfg.stopMode=1; saveConfig(); return; }
  if (cmd == "stop_hold")     { sendWithChecksum334(5,0x03,0x02,"Hold"); cfg.stopMode=2; saveConfig(); return; }
  slog("[cmd] Bilinmeyen: " + cmd);
}

// ─────────────────────────────────────────────────────────────
//  WEB SUNUCU
// ─────────────────────────────────────────────────────────────

WebServer server(80);

// ── OTA Guncelleme Sayfasi ────────────────────────────────────
const char OTA_HTML[] PROGMEM = R"HTML(<!DOCTYPE html>
<html lang="tr">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>CanFeather OTA</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
:root{--bg:#0f172a;--sur:#1e293b;--bor:#334155;--acc:#10b981;--acc2:#6366f1;--red:#ef4444;--tx:#f1f5f9;--mu:#94a3b8}
body{background:var(--bg);color:var(--tx);font-family:-apple-system,sans-serif;padding:24px 16px;min-height:100vh;display:flex;align-items:center;justify-content:center}
.w{max-width:420px;width:100%}
.logo{font-family:monospace;font-size:10px;color:var(--acc);letter-spacing:3px;margin-bottom:8px}
h1{font-size:20px;font-weight:300;margin-bottom:4px}h1 span{color:var(--acc);font-weight:500}
.sub{font-size:11px;color:var(--mu);margin-bottom:20px}
.chip{font-family:monospace;font-size:9px;background:#6366f115;border:1px solid #6366f130;color:#a78bfa;border-radius:3px;padding:1px 6px;margin-left:6px}
.card{background:var(--sur);border:1px solid var(--bor);border-radius:12px;padding:24px;margin:12px 0}
.drop{border:2px dashed var(--bor);border-radius:10px;padding:40px 20px;text-align:center;cursor:pointer;transition:.2s}
.drop.over{border-color:var(--acc);background:#10b98108}
.drop.has{border-color:var(--acc2);border-style:solid;background:#6366f108}
.di{font-size:28px;margin-bottom:8px}
.dt{font-size:13px;color:var(--mu)}
.dt b{color:var(--tx);font-weight:500}
.fn{font-family:monospace;font-size:11px;color:var(--acc2);margin-top:8px;word-break:break-all}
.fs{font-size:10px;color:var(--mu);margin-top:4px}
.bar-wrap{width:100%;height:6px;background:var(--bor);border-radius:3px;margin-top:16px;overflow:hidden;display:none}
.bar-fill{height:100%;width:0%;background:var(--acc);border-radius:3px;transition:width .3s}
.pct{font-family:monospace;font-size:11px;color:var(--acc);margin-top:6px;text-align:center;display:none}
.sb{width:100%;padding:13px;border-radius:8px;border:none;background:var(--acc);color:#000;font-size:14px;font-weight:500;cursor:pointer;margin-top:16px;transition:.15s;font-family:inherit}
.sb:hover{opacity:.88}.sb:active{transform:scale(.98)}
.sb:disabled{opacity:.4;cursor:not-allowed}
.back{display:inline-block;margin-top:14px;font-size:11px;color:var(--mu);text-decoration:none;transition:.15s}
.back:hover{color:var(--acc)}
.msg{font-size:12px;margin-top:12px;padding:10px;border-radius:8px;display:none}
.msg.ok{display:block;background:#10b98115;border:1px solid #10b98130;color:var(--acc)}
.msg.err{display:block;background:#ef444415;border:1px solid #ef444430;color:var(--red)}
.lang-sw{position:absolute;top:16px;right:16px;display:flex;gap:0;font-size:11px;font-family:monospace}
.lang-sw button{padding:4px 10px;border:1px solid var(--bor);background:transparent;color:var(--mu);cursor:pointer;transition:.15s}
.lang-sw button:first-child{border-radius:4px 0 0 4px}
.lang-sw button:last-child{border-radius:0 4px 4px 0;border-left:none}
.lang-sw button.on{background:#10b98115;border-color:var(--acc);color:var(--acc)}
</style>
</head>
<body>
<div style="position:relative;width:100%;max-width:420px">
<div class="lang-sw">
  <button id="lTR" onclick="setLang('tr')">TR</button>
  <button id="lEN" onclick="setLang('en')">EN</button>
</div>
<div class="w">
  <div class="logo">// CanFeather</div>
  <h1 data-tr="Firmware Guncelleme" data-en="Firmware Update">Firmware Guncelleme</h1>
  <div class="sub" data-tr="Derlenmi&#351; .bin dosyasini yukleyin" data-en="Upload compiled .bin file">Derlenmis .bin dosyasini yukleyin</div>

  <div class="card">
    <div class="drop" id="drop" onclick="document.getElementById('file').click()">
      <div class="di" id="icon">&#128228;</div>
      <div class="dt" id="dtxt" data-tr="Dosyayi surukle-birak veya <b>gozat</b>" data-en="Drag &amp; drop file or <b>browse</b>">Dosyayi surukle-birak veya <b>gozat</b></div>
      <div class="fn" id="fname"></div>
      <div class="fs" id="fsize"></div>
    </div>
    <input type="file" id="file" accept=".bin" style="display:none">
    <div class="bar-wrap" id="barw"><div class="bar-fill" id="barf"></div></div>
    <div class="pct" id="pct">0%</div>
    <div class="msg" id="msg"></div>
    <button class="sb" id="upbtn" disabled onclick="doUpload()" data-tr="Yukle" data-en="Upload">Yukle</button>
  </div>
  <a class="back" href="/" data-tr="&#8592; Kontrol paneline don" data-en="&#8592; Back to dashboard">&#8592; Kontrol paneline don</a>
</div>
</div>
<script>
const L={tr:{uploading:'Yukleniyor...',success:'Basarili! Yeniden baslatiliyor...',fail:'Yukleme hatasi!',selectFile:'Once dosya secin'},en:{uploading:'Uploading...',success:'Success! Restarting...',fail:'Upload failed!',selectFile:'Select a file first'}};
let lang=localStorage.getItem('cf_lang')||'tr',selFile=null;
function setLang(l){lang=l;localStorage.setItem('cf_lang',l);document.getElementById('lTR').classList.toggle('on',l==='tr');document.getElementById('lEN').classList.toggle('on',l==='en');document.querySelectorAll('[data-'+l+']').forEach(e=>{const v=e.getAttribute('data-'+l);if(e.tagName==='INPUT'||e.tagName==='BUTTON'&&!e.children.length)e.textContent=v;else e.innerHTML=v;});}
setLang(lang);
const drop=document.getElementById('drop'),fi=document.getElementById('file');
drop.addEventListener('dragover',e=>{e.preventDefault();drop.classList.add('over');});
drop.addEventListener('dragleave',()=>drop.classList.remove('over'));
drop.addEventListener('drop',e=>{e.preventDefault();drop.classList.remove('over');if(e.dataTransfer.files.length)pickFile(e.dataTransfer.files[0]);});
fi.addEventListener('change',()=>{if(fi.files.length)pickFile(fi.files[0]);});
function pickFile(f){selFile=f;drop.classList.add('has');document.getElementById('icon').textContent='\u2705';document.getElementById('fname').textContent=f.name;document.getElementById('fsize').textContent=(f.size/1024).toFixed(1)+' KB';document.getElementById('upbtn').disabled=false;document.getElementById('msg').className='msg';document.getElementById('msg').style.display='none';}
function doUpload(){if(!selFile)return;const fd=new FormData();fd.append('update',selFile);const xhr=new XMLHttpRequest();const barw=document.getElementById('barw'),barf=document.getElementById('barf'),pct=document.getElementById('pct'),msg=document.getElementById('msg'),btn=document.getElementById('upbtn');barw.style.display='block';pct.style.display='block';btn.disabled=true;btn.textContent=L[lang].uploading;xhr.upload.onprogress=e=>{if(e.lengthComputable){const p=Math.round(e.loaded/e.total*100);barf.style.width=p+'%';pct.textContent=p+'%';}};xhr.onload=()=>{if(xhr.status===200){msg.className='msg ok';msg.textContent=L[lang].success;msg.style.display='block';setTimeout(()=>location.href='/',5000);}else{msg.className='msg err';msg.textContent=L[lang].fail;msg.style.display='block';btn.disabled=false;btn.textContent=L[lang].selectFile;}};xhr.onerror=()=>{msg.className='msg err';msg.textContent=L[lang].fail;msg.style.display='block';btn.disabled=false;};xhr.open('POST','/update');xhr.send(fd);}
</script>
</body>
</html>)HTML";

// HTML arayüzü — PROGMEM'e alındı
const char HTML[] PROGMEM = R"HTML(<!DOCTYPE html>
<html lang="tr">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>CanFeather</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
:root{--bg:#0f172a;--sur:#1e293b;--bor:#334155;--acc:#10b981;--acc2:#6366f1;--red:#ef4444;--tx:#f1f5f9;--mu:#94a3b8}
body{background:var(--bg);color:var(--tx);font-family:-apple-system,sans-serif;padding:24px 16px}
.w{max-width:460px;margin:0 auto}
.logo{font-family:monospace;font-size:10px;color:var(--acc);letter-spacing:3px;margin-bottom:8px}
h1{font-size:20px;font-weight:300}h1 span{color:var(--acc);font-weight:500}
.badge{font-family:monospace;font-size:10px;background:#10b98118;color:var(--acc);border:1px solid #10b98130;border-radius:4px;padding:2px 8px;margin-left:8px;vertical-align:middle}
.sub{font-size:11px;color:var(--mu);margin-top:5px}
.dot{width:6px;height:6px;border-radius:50%;background:var(--acc);display:inline-block;margin-right:5px;animation:p 2s infinite}
@keyframes p{0%,100%{opacity:1}50%{opacity:.3}}
.card{background:var(--sur);border:1px solid var(--bor);border-radius:14px;padding:20px;margin:14px 0;box-shadow:0 2px 8px rgba(0,0,0,.2)}
.ct{font-size:13px;font-weight:500;margin-bottom:3px}
.cd{font-size:11px;color:var(--mu);margin-bottom:14px;line-height:1.5}
.seg{display:flex;gap:5px}
.seg button{flex:1;padding:12px 6px;border-radius:10px;border:1px solid var(--bor);background:transparent;color:var(--mu);font-size:13px;cursor:pointer;transition:.2s;font-family:inherit}
.seg button.on{background:#10b98120;border-color:var(--acc);color:var(--acc);font-weight:600;box-shadow:0 0 12px #10b98115}
.seg button:hover:not(.on){border-color:#2a2a3a;color:var(--tx)}
.pr{display:flex;gap:5px;flex-wrap:wrap}
.pb{flex:1;min-width:60px;padding:12px 6px;border-radius:10px;border:1px solid var(--bor);background:transparent;color:var(--mu);font-size:13px;cursor:pointer;transition:.2s;font-family:inherit}
.pb.on{background:#6366f120;border-color:var(--acc2);color:var(--acc2);font-weight:600;box-shadow:0 0 12px #6366f115}
.pb:hover:not(.on){border-color:#2a2a3a;color:var(--tx)}
.tr{display:flex;align-items:center;justify-content:space-between}
.tg{position:relative;width:44px;height:26px;flex-shrink:0}
.tg input{opacity:0;width:0;height:0}
.sl{position:absolute;inset:0;border-radius:13px;background:#334155;border:1px solid var(--bor);cursor:pointer;transition:.2s}
.sl:before{content:'';position:absolute;width:18px;height:18px;left:3px;top:3px;border-radius:50%;background:var(--mu);transition:.2s}
input:checked+.sl{background:#10b98130;border-color:var(--acc)}
input:checked+.sl:before{transform:translateX(18px);background:var(--acc);box-shadow:0 0 8px #10b98140}
.db{width:100%;padding:12px;border-radius:8px;border:1px solid #ef444440;background:#ef444410;color:var(--red);font-size:13px;font-weight:500;cursor:pointer;transition:.15s;font-family:inherit}
.db:hover{background:#ef444425;border-color:var(--red)}
.lb{font-family:monospace;font-size:10px;background:#0f172a;border:1px solid var(--bor);border-radius:8px;padding:10px;height:120px;overflow-y:auto;line-height:1.9}
.ll{color:#4a9eff}.ll:last-child{color:var(--acc)}
.sb{width:100%;padding:13px;border-radius:8px;border:none;background:linear-gradient(135deg,var(--acc),#059669);color:#fff;font-size:14px;font-weight:500;cursor:pointer;margin-top:4px;transition:.15s;font-family:inherit;box-shadow:0 2px 12px #10b98130}
.sb:hover{opacity:.88}.sb:active{transform:scale(.98)}
.toast{position:fixed;bottom:20px;left:50%;transform:translateX(-50%) translateY(60px);background:var(--acc);color:#000;padding:9px 20px;border-radius:8px;font-size:12px;font-weight:500;transition:.3s;opacity:0;pointer-events:none}
.toast.show{transform:translateX(-50%) translateY(0);opacity:1}
.chip{font-family:monospace;font-size:9px;background:#6366f115;border:1px solid #6366f130;color:#a78bfa;border-radius:3px;padding:1px 6px;margin-left:6px}
.stb{font-family:monospace;font-size:11px;line-height:2;background:#162032;border-color:#1e4035;border-left:3px solid var(--acc)}
.stl{display:flex;gap:12px;flex-wrap:wrap}.stl b{color:var(--acc)}
.dvd{margin-top:14px;padding-top:14px;border-top:1px solid var(--bor)}
.inp{background:#0f172a;border:1px solid var(--bor);border-radius:6px;color:var(--tx);font-family:monospace;font-size:12px;padding:6px 8px;width:70px;text-align:center}
.inp:focus{outline:none;border-color:var(--acc)}
.row{display:flex;gap:6px;align-items:center;flex-wrap:wrap}
.mbtn{padding:10px 16px;border-radius:8px;border:1px solid var(--bor);background:transparent;color:var(--mu);font-size:11px;cursor:pointer;font-family:inherit;transition:.15s}
.mbtn:hover{border-color:var(--acc);color:var(--acc)}
.mbtn.act{background:#10b98115;border-color:var(--acc);color:var(--acc)}
.spark{height:40px;display:flex;align-items:flex-end;gap:1px}
.spark div{flex:1;background:var(--acc);border-radius:1px 1px 0 0;min-width:2px;opacity:.6;transition:height .3s}
.fhist{font-family:monospace;font-size:10px;line-height:1.8;color:var(--mu)}
.fhist b{color:var(--acc)}
.snif{font-family:monospace;font-size:9px;background:#0f172a;border:1px solid var(--bor);border-radius:8px;padding:8px;height:100px;overflow-y:auto;line-height:1.8;color:#f59e0b}
.lang-sw{display:flex;gap:0;font-size:11px;font-family:monospace;position:absolute;top:0;right:0}
.lang-sw button{padding:4px 10px;border:1px solid var(--bor);background:transparent;color:var(--mu);cursor:pointer;transition:.15s}
.lang-sw button:first-child{border-radius:4px 0 0 4px}
.lang-sw button:last-child{border-radius:0 4px 4px 0;border-left:none}
.lang-sw button.on{background:#10b98115;border-color:var(--acc);color:var(--acc)}
</style>
</head>
<body>
<div class="w">
  <div style="margin-bottom:24px;position:relative">
    <div class="lang-sw">
      <button id="lTR" onclick="setLang('tr')">TR</button>
      <button id="lEN" onclick="setLang('en')">EN</button>
    </div>
    <div class="logo">// CanFeather</div>
    <h1><span data-tr="Kontroller" data-en="Controls">Kontroller</span> <span class="badge" id="hwb">HW3</span><span class="chip">ESP32</span></h1>
    <div class="sub"><span class="dot"></span><span data-tr="CAN kopru ayarlari. Donanim ve surus profiline gore yapilandir." data-en="CAN bridge settings. Configure by hardware and driving profile.">CAN kopru ayarlari. Donanim ve surus profiline gore yapilandir.</span></div>
  </div>

  <div class="card stb" id="stb">
    <div class="stl"><span>CAN: <b id="sC">—</b></span> <span>FSD: <b id="sF">—</b></span> <span>NAG: <b id="sN">—</b></span></div>
    <div class="stl"><span>HW: <b id="sH">—</b></span> <span>Profile: <b id="sP">—</b></span></div>
    <div class="stl"><span>Follow dist: <b id="sD">—</b></span> <span>Offset: <b id="sO">—</b></span></div>
    <div class="stl"><span>RX: <b id="sR">0</b></span> <span>TX: <b id="sT">0</b></span> <span>INJ: <b id="sI">—</b></span></div>
    <div class="stl"><span>Uptime: <b id="sU">—</b></span> <span>Err: <b id="sE">0</b></span></div>
  </div>

  <div class="card">
    <div class="ct" data-tr="&#128202; Trafik (son 60sn)" data-en="&#128202; Traffic (last 60s)">&#128202; Trafik (son 60sn)</div>
    <div class="spark" id="spark"></div>
  </div>

  <div class="card">
    <div class="ct" data-tr="&#128268; Donanim" data-en="&#128268; Hardware">&#128268; Donanim</div>
    <div class="seg">
      <button onclick="setHW(0)" id="h0">Legacy</button>
      <button onclick="setHW(1)" id="h1" class="on">HW3</button>
      <button onclick="setHW(2)" id="h2">HW4</button>
    </div>
    <div class="dvd">
      <div class="ct" style="margin-bottom:10px" data-tr="Kristal Frekans" data-en="Crystal Frequency">Kristal Frekans</div>
      <div class="seg">
        <button onclick="setCrystal(0)" id="cr0">8 MHz</button>
        <button onclick="setCrystal(1)" id="cr1" class="on">16 MHz</button>
      </div>
    </div>
  </div>

  <div class="card">
    <div class="ct" data-tr="&#9889; Hiz profili" data-en="&#9889; Speed profile">&#9889; Hiz profili</div>
    <div class="pr" id="pr"></div>
  </div>

  <div class="card">
    <div class="ct" data-tr="&#128137; Enjeksiyon" data-en="&#128137; Injection">&#128137; Enjeksiyon</div>
    <div class="tr">
      <div><div style="font-size:13px;font-weight:500" data-tr="FSD etkin" data-en="FSD enabled">FSD etkin</div>
      <div style="font-size:11px;color:var(--mu);margin-top:2px" data-tr="Veri yolunda enjeksiyon mantigi" data-en="Injection logic on the data bus">Veri yolunda enjeksiyon mantigi</div></div>
      <label class="tg"><input type="checkbox" id="ft" checked><span class="sl"></span></label>
    </div>
    <div class="tr dvd">
      <div><div style="font-size:13px;font-weight:500" data-tr="Profili buradan uygula" data-en="Apply profile from here">Profili buradan uygula</div>
      <div style="font-size:11px;color:var(--mu);margin-top:2px" data-tr="CAN profil secimi yok sayilir" data-en="CAN profile selection is ignored">CAN profil secimi yok sayilir</div></div>
      <label class="tg"><input type="checkbox" id="ov"><span class="sl"></span></label>
    </div>
    <div class="tr dvd">
      <div><div style="font-size:13px;font-weight:500" data-tr="CAN enjeksiyonu" data-en="CAN injection">CAN enjeksiyonu</div>
      <div style="font-size:11px;color:var(--mu);margin-top:2px" data-tr="Kapali = kopru seffaf" data-en="Off = bridge transparent">Kapali = kopru seffaf</div></div>
      <label class="tg"><input type="checkbox" id="ci" checked><span class="sl"></span></label>
    </div>
    <div class="tr dvd">
      <div><div style="font-size:13px;font-weight:500" data-tr="Otomatik kapanma" data-en="Auto shutdown">Otomatik kapanma</div>
      <div style="font-size:11px;color:var(--mu);margin-top:2px" data-tr="0 = devre disi" data-en="0 = disabled">0 = devre disi</div></div>
      <div class="row"><input class="inp" id="ao" type="number" min="0" max="999" value="0"><span style="font-size:11px;color:var(--mu)" data-tr="dk" data-en="min">dk</span></div>
    </div>
    <div class="tr dvd">
      <div><div style="font-size:13px;font-weight:500" data-tr="ISA hiz uyari bastirma" data-en="ISA speed chime suppress">ISA hiz uyari bastirma</div>
      <div style="font-size:11px;color:var(--mu);margin-top:2px" data-tr="Hiz limiti isareti bos kalir" data-en="Speed limit sign will be empty while driving">Hiz limiti isareti bos kalir</div></div>
      <label class="tg"><input type="checkbox" id="isa"><span class="sl"></span></label>
    </div>
    <div class="tr dvd">
      <div><div style="font-size:13px;font-weight:500" data-tr="Acil arac algilama" data-en="Emergency vehicle detection">Acil arac algilama</div>
      <div style="font-size:11px;color:var(--mu);margin-top:2px" data-tr="Yaklasan acil arac tespiti" data-en="Approaching emergency vehicle detection">Yaklasan acil arac tespiti</div></div>
      <label class="tg"><input type="checkbox" id="evd" checked><span class="sl"></span></label>
    </div>
  </div>

  <div class="card">
    <div class="ct" data-tr="&#128340; FSD Durum Gecmisi" data-en="&#128340; FSD State History">&#128340; FSD Durum Gecmisi</div>
    <div class="fhist" id="fhist" data-tr="— bekleniyor —" data-en="— waiting —">— bekleniyor —</div>
  </div>

  <div class="card">
    <div class="ct" style="margin-bottom:10px" data-tr="&#128220; CAN Log" data-en="&#128220; CAN Log">&#128220; CAN Log</div>
    <div class="row" style="margin-bottom:8px">
      <span style="font-size:11px;color:var(--mu)" data-tr="Filtre ID:" data-en="Filter ID:">Filtre ID:</span>
      <input class="inp" id="flt" type="number" min="0" max="2047" value="0" placeholder="0=hepsi">
      <button class="mbtn" onclick="applyFilter()" data-tr="Uygula" data-en="Apply">Uygula</button>
    </div>
    <div class="lb" id="log"><span style="color:#2a2a3a">// bekleniyor...</span></div>
  </div>

  <div class="card">
    <div class="ct" data-tr="&#128065; CAN Sniffer" data-en="&#128065; CAN Sniffer">&#128065; CAN Sniffer</div>
    <div class="cd" data-tr="Tum CAN tratigini yakalar — debug icin" data-en="Captures all CAN traffic — for debug">Tum CAN tratigini yakalar — debug icin</div>
    <div class="tr" style="margin-bottom:10px">
      <div style="font-size:13px;font-weight:500" data-tr="Sniffer modu" data-en="Sniffer mode">Sniffer modu</div>
      <label class="tg"><input type="checkbox" id="snf" onchange="toggleSnif()"><span class="sl"></span></label>
    </div>
    <div class="snif" id="snif"><span style="color:#2a2a3a">// sniffer kapali</span></div>
  </div>

  <div class="card">
    <div class="ct" data-tr="&#128187; Serial Log (Canli)" data-en="&#128187; Serial Log (Live)">&#128187; Serial Log (Canli)</div>
    <div class="cd" data-tr="ESP'nin Serial ciktisi — config, EEPROM, hata mesajlari" data-en="ESP Serial output — config, EEPROM, error messages">ESP'nin Serial ciktisi — config, EEPROM, hata mesajlari</div>
    <div class="lb" id="slog" style="height:90px;color:#f59e0b"><span style="color:#2a2a3a">// bekleniyor...</span></div>
  </div>

  <div class="card">
    <div class="ct" data-tr="&#128228; CAN Mesaj Gonder" data-en="&#128228; Send CAN Message">&#128228; CAN Mesaj Gonder</div>
    <div class="cd" data-tr="Manuel CAN frame gonderme — test/debug icin" data-en="Manual CAN frame send — for test/debug">Manuel CAN frame gonderme — test/debug icin</div>
    <div class="row" style="margin-bottom:8px">
      <span style="font-size:11px;color:var(--mu)">ID:</span>
      <input class="inp" id="sid" type="number" min="0" max="2047" value="1021">
      <span style="font-size:11px;color:var(--mu)">DLC:</span>
      <input class="inp" id="sdlc" type="number" min="0" max="8" value="8" style="width:40px">
    </div>
    <div class="row" style="margin-bottom:8px">
      <span style="font-size:11px;color:var(--mu)">Data (hex):</span>
      <input class="inp" id="sdata" type="text" value="00 00 00 00 00 00 00 00" style="width:200px;text-align:left">
    </div>
    <button class="mbtn" onclick="sendCAN()" data-tr="Gonder" data-en="Send">Gonder</button>
  </div>

  <div class="card">
    <div class="ct" data-tr="&#128663; Arac Kontrol" data-en="&#128663; Vehicle Control">&#128663; Arac Kontrol</div>
    <div class="cd" data-tr="CAN bus uzerinden arac komutlari" data-en="Vehicle commands via CAN bus">CAN bus uzerinden arac komutlari</div>
    <div style="font-size:10px;color:var(--mu);margin-bottom:6px">ID 0x273 &mdash; VehicleBus &mdash; dinle-degistir-gonder</div>
    <div class="ct dvd" style="font-size:11px;color:#818cf8">&#128274; Ayna &amp; Kilit</div>
    <div style="display:grid;grid-template-columns:1fr 1fr;gap:5px;margin:6px 0">
      <button class="mbtn" onclick="vcmd('mirror_fold')">Ayna Katla</button>
      <button class="mbtn" onclick="vcmd('mirror_unfold')">Ayna Ac</button>
      <button class="mbtn" onclick="vcmd('mirror_heat')">Ayna Isit</button>
      <button class="mbtn" onclick="vcmd('mirror_autofold')">Oto Katlama</button>
      <button class="mbtn" onclick="vcmd('lock')">Kilitle</button>
      <button class="mbtn" onclick="vcmd('unlock')">Kilidi Ac</button>
      <button class="mbtn" onclick="vcmd('child_lock')">Cocuk Kilidi</button>
    </div>
    <div class="ct dvd" style="font-size:11px;color:#818cf8">&#128663; Bagaj &amp; Kaput</div>
    <div style="display:grid;grid-template-columns:1fr 1fr 1fr;gap:5px;margin:6px 0">
      <button class="mbtn" onclick="vcmd('frunk')">Frunk</button>
      <button class="mbtn" onclick="vcmd('trunk')">Bagaj</button>
      <button class="mbtn" onclick="vcmd('glovebox')">Torpido</button>
    </div>
    <div class="ct dvd" style="font-size:11px;color:#818cf8">&#128161; Isiklar</div>
    <div style="display:grid;grid-template-columns:1fr 1fr;gap:5px;margin:6px 0">
      <button class="mbtn" onclick="vcmd('fog_front')">On Sis</button>
      <button class="mbtn" onclick="vcmd('fog_rear')">Arka Sis</button>
      <button class="mbtn" onclick="vcmd('highbeam_auto')">Oto Uzun Far</button>
      <button class="mbtn" onclick="vcmd('ambient_light')">Ambiyans</button>
      <button class="mbtn" onclick="vcmd('homelight')">Coming Home</button>
      <button class="mbtn" onclick="vcmd('mirror_dip')">Geri Ayna Egim</button>
    </div>
    <div style="font-size:10px;color:var(--mu);margin-top:4px">Tavan Isik:</div>
    <div class="seg" style="margin:4px 0"><button onclick="vcmd('dome_off')">OFF</button><button onclick="vcmd('dome_on')">ON</button><button onclick="vcmd('dome_auto')">Auto</button></div>
    <div class="ct dvd" style="font-size:11px;color:#818cf8">&#127786; Silecek</div>
    <div class="seg" style="margin:4px 0"><button onclick="vcmd('wiper_off')">OFF</button><button onclick="vcmd('wiper_1')">1</button><button onclick="vcmd('wiper_2')">2</button><button onclick="vcmd('wiper_3')">3</button></div>
    <div class="ct dvd" style="font-size:11px;color:#818cf8">&#128267; Diger</div>
    <div style="display:grid;grid-template-columns:1fr 1fr;gap:5px;margin:6px 0">
      <button class="mbtn" onclick="vcmd('horn')">Korna</button>
      <button class="mbtn" onclick="vcmd('summon')">Summon</button>
      <button class="mbtn" onclick="vcmd('acc_power')">Aksesuar Guc</button>
      <button class="mbtn" onclick="vcmd('power_off')" style="border-color:#ef444440;color:var(--red)">Arac Kapat</button>
    </div>
    <div class="ct dvd" style="font-size:11px;color:#818cf8">&#128296; Ekran</div>
    <div class="row" style="margin:6px 0"><span style="font-size:10px;width:55px">Parlaklik:</span><input class="inp" id="brVal" type="range" min="0" max="127" value="64" style="flex:1"><button class="mbtn" onclick="vcmd('bright'+document.getElementById('brVal').value)" style="padding:6px 10px">Ayarla</button></div>
    <div class="ct dvd" style="font-size:11px;color:#818cf8">&#128293; Koltuk Isitma</div>
    <div style="display:grid;grid-template-columns:1fr 1fr;gap:4px;margin-top:6px;font-size:11px">
      <div class="row"><span style="width:28px">FL:</span><button class="mbtn" onclick="vcmd('seat_fl 0')" style="padding:6px 8px">0</button><button class="mbtn" onclick="vcmd('seat_fl 1')" style="padding:6px 8px">1</button><button class="mbtn" onclick="vcmd('seat_fl 2')" style="padding:6px 8px">2</button><button class="mbtn" onclick="vcmd('seat_fl 3')" style="padding:6px 8px">3</button></div>
      <div class="row"><span style="width:28px">FR:</span><button class="mbtn" onclick="vcmd('seat_fr 0')" style="padding:6px 8px">0</button><button class="mbtn" onclick="vcmd('seat_fr 1')" style="padding:6px 8px">1</button><button class="mbtn" onclick="vcmd('seat_fr 2')" style="padding:6px 8px">2</button><button class="mbtn" onclick="vcmd('seat_fr 3')" style="padding:6px 8px">3</button></div>
      <div class="row"><span style="width:28px">RL:</span><button class="mbtn" onclick="vcmd('seat_rl 0')" style="padding:6px 8px">0</button><button class="mbtn" onclick="vcmd('seat_rl 1')" style="padding:6px 8px">1</button><button class="mbtn" onclick="vcmd('seat_rl 2')" style="padding:6px 8px">2</button><button class="mbtn" onclick="vcmd('seat_rl 3')" style="padding:6px 8px">3</button></div>
      <div class="row"><span style="width:28px">RR:</span><button class="mbtn" onclick="vcmd('seat_rr 0')" style="padding:6px 8px">0</button><button class="mbtn" onclick="vcmd('seat_rr 1')" style="padding:6px 8px">1</button><button class="mbtn" onclick="vcmd('seat_rr 2')" style="padding:6px 8px">2</button><button class="mbtn" onclick="vcmd('seat_rr 3')" style="padding:6px 8px">3</button></div>
      <div class="row"><span style="width:28px">RC:</span><button class="mbtn" onclick="vcmd('seat_rc 0')" style="padding:6px 8px">0</button><button class="mbtn" onclick="vcmd('seat_rc 1')" style="padding:6px 8px">1</button><button class="mbtn" onclick="vcmd('seat_rc 2')" style="padding:6px 8px">2</button><button class="mbtn" onclick="vcmd('seat_rc 3')" style="padding:6px 8px">3</button></div>
    </div>
    <div class="ct dvd" style="font-size:10px;color:var(--mu)">&#9888; Farkli CAN ID (bus'ta yoksa calismaz)</div>
    <div style="display:grid;grid-template-columns:1fr 1fr;gap:5px;margin:6px 0">
      <button class="mbtn" onclick="vcmd('vent_open')">Cam Ac</button>
      <button class="mbtn" onclick="vcmd('vent_close')">Cam Kapat</button>
      <button class="mbtn" onclick="vcmd('sentry_on')">Sentry ON</button>
      <button class="mbtn" onclick="vcmd('sentry_off')">Sentry OFF</button>
      <button class="mbtn" onclick="vcmd('climate_keep')">Klima Tut</button>
      <button class="mbtn" onclick="vcmd('climate_off')">Klima Kapat</button>
      <button class="mbtn" onclick="vcmd('charge_start')">Sarj Baslat</button>
      <button class="mbtn" onclick="vcmd('charge_stop')">Sarj Durdur</button>
      <button class="mbtn" onclick="vcmd('charge_port')">Sarj Port</button>
    </div>
    <div class="ct dvd" data-tr="&#9881; Surus Modu" data-en="&#9881; Drive Mode">&#9881; Surus Modu</div>
    <div style="margin-top:6px;font-size:11px;color:var(--mu)">Pedal:</div>
    <div class="seg" style="margin:4px 0" id="seg-pedal"><button onclick="setPedal(0)">Std</button><button onclick="setPedal(1)">Chill</button><button onclick="setPedal(2)">Sport</button></div>
    <div style="font-size:11px;color:var(--mu)">Regen:</div>
    <div class="seg" style="margin:4px 0" id="seg-regen"><button onclick="setRegen(0)">OFF</button><button onclick="setRegen(50)">Low</button><button onclick="setRegen(100)">Std</button><button onclick="setRegen(200)">Max</button></div>
    <div style="font-size:11px;color:var(--mu)">Durma:</div>
    <div class="seg" style="margin:4px 0" id="seg-stop"><button onclick="setStop(0)">Creep</button><button onclick="setStop(1)">Roll</button><button onclick="setStop(2)">Hold</button></div>
  </div>

  <div class="card">
    <div class="ct" data-tr="&#128246; WiFi Ayarlari" data-en="&#128246; WiFi Settings">&#128246; WiFi Ayarlari</div>
    <div class="cd" data-tr="SSID ve sifre degistir (min 8 karakter)" data-en="Change SSID and password (min 8 chars)">SSID ve sifre degistir (min 8 karakter)</div>
    <div class="row" style="margin:6px 0"><span style="font-size:11px;color:var(--mu);width:45px">SSID:</span><input class="inp" id="wSSID" style="flex:1;width:auto;text-align:left" value=""></div>
    <div class="row" style="margin:6px 0"><span style="font-size:11px;color:var(--mu);width:45px">Sifre:</span><input class="inp" id="wPASS" type="password" style="flex:1;width:auto;text-align:left" value="" placeholder="min 8 karakter"></div>
    <div class="row" style="margin:8px 0;gap:6px"><button class="mbtn" onclick="saveWifi()" style="background:#10b98118;border-color:var(--acc);color:var(--acc)">Kaydet &amp; Yeniden Baslat</button><button class="mbtn" onclick="resetWifi()" style="border-color:#ef444440;color:var(--red)">Fabrika Ayari</button></div>
  </div>

  <div class="card">
    <div class="ct" style="margin-bottom:12px" data-tr="&#128721; Guvenlik" data-en="&#128721; Safety">&#128721; Guvenlik</div>
    <button class="db" onclick="disableAll()" data-tr="Tum enjeksiyonlari devre disi birak" data-en="Disable all injections">Tum enjeksiyonlari devre disi birak</button>
  </div>

  <div class="row" style="margin-top:12px;gap:8px">
    <button class="sb" style="flex:1" onclick="save()" data-tr="Kaydet &amp; EEPROM" data-en="Save &amp; EEPROM">Kaydet &amp; EEPROM</button>
    <button class="mbtn" onclick="location.href='/update'">OTA</button>
  </div>
</div>
<div class="toast" id="t"></div>
<script>
const L={tr:{saved:'Ayarlar kaydedildi',connErr:'Baglanti hatasi',disabled:'Tum enjeksiyonlar durduruldu',eeprom:'EEPROM kaydedildi',snifOn:'Sniffer acik',snifOff:'Sniffer kapali',canSent:'CAN frame gonderildi',canErr:'Gonderme hatasi',fsdOn:'\u25B6 FSD ACILDI',fsdOff:'\u25A0 FSD KAPANDI',noEvent:'\u2014 henuz olay yok \u2014'},en:{saved:'Settings saved',connErr:'Connection error',disabled:'All injections stopped',eeprom:'EEPROM saved',snifOn:'Sniffer on',snifOff:'Sniffer off',canSent:'CAN frame sent',canErr:'Send error',fsdOn:'\u25B6 FSD ACTIVATED',fsdOff:'\u25A0 FSD DEACTIVATED',noEvent:'\u2014 no events yet \u2014'}};
let lang=localStorage.getItem('cf_lang')||'tr';
let hw=1,prof=2,crystal=1,synced=false;
const pN=['Chill','Normal','Hurry','Max','Sloth'];
const pS=[[2,1,0],[2,1,0],[3,2,1,0,4]];
function setLang(l){lang=l;localStorage.setItem('cf_lang',l);document.getElementById('lTR').classList.toggle('on',l==='tr');document.getElementById('lEN').classList.toggle('on',l==='en');document.querySelectorAll('[data-'+l+']').forEach(e=>{const v=e.getAttribute('data-'+l);if(e.tagName==='INPUT')e.placeholder=v;else if(e.tagName==='BUTTON'&&!e.children.length)e.textContent=v;else e.innerHTML=v;});}
function setHW(n){hw=n;[0,1,2].forEach(i=>document.getElementById('h'+i).classList.toggle('on',i===n));document.getElementById('hwb').textContent=['Legacy','HW3','HW4'][n];buildP();}
function setCrystal(n){crystal=n;[0,1].forEach(i=>document.getElementById('cr'+i).classList.toggle('on',i===n));}
function buildP(){const s=pS[hw],c=document.getElementById('pr');c.innerHTML='';s.forEach(v=>{const b=document.createElement('button');b.className='pb'+(v===prof?' on':'');b.textContent=pN[v];b.onclick=()=>setP(v);c.appendChild(b);});}
function setP(n){prof=n;document.querySelectorAll('.pb').forEach(b=>b.classList.toggle('on',b.textContent===pN[n]));}
function disableAll(){document.getElementById('ft').checked=false;document.getElementById('ci').checked=false;fetch('/disable').then(()=>toast(L[lang].disabled));}
function save(){fetch('/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({hw,profile:prof,crystal,fsd:document.getElementById('ft').checked,ovr:document.getElementById('ov').checked,inj:document.getElementById('ci').checked,autoOff:parseInt(document.getElementById('ao').value)||0,logFilter:parseInt(document.getElementById('flt').value)||0,isa:document.getElementById('isa').checked,ev:document.getElementById('evd').checked})}).then(r=>r.json()).then(()=>fetch('/saveee')).then(()=>{toast(L[lang].saved);document.getElementById('slog').innerHTML='';document.getElementById('log').innerHTML='';synced=false;setTimeout(fetchStatus,500);}).catch(()=>toast(L[lang].connErr));}
function applyFilter(){save();}
function toggleSnif(){fetch('/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({snif:document.getElementById('snf').checked})}).then(()=>toast(document.getElementById('snf').checked?L[lang].snifOn:L[lang].snifOff));}
function sendCAN(){const id=parseInt(document.getElementById('sid').value)||0;const dlc=parseInt(document.getElementById('sdlc').value)||8;const hex=document.getElementById('sdata').value.trim().split(/[\s,]+/).map(h=>parseInt(h,16)||0);fetch('/send',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({id,dlc,data:hex})}).then(r=>r.json()).then(d=>{if(d.ok)toast(L[lang].canSent);else toast(L[lang].canErr);}).catch(()=>toast(L[lang].connErr));}
function vcmd(c){fetch('/cmd?c='+encodeURIComponent(c)).then(r=>r.json()).then(d=>{toast(d.cmd+' gonderildi');}).catch(()=>toast('Hata'));}
const pedalCmds=['pedal_std','pedal_chill','pedal_sport'];
const regenVals=[0,50,100,200];
const regenCmds=['regen_off','regen_low','regen_std','regen_max'];
const stopCmds=['stop_creep','stop_roll','stop_hold'];
function segHL(id,idx){document.querySelectorAll('#'+id+' button').forEach((b,i)=>b.classList.toggle('on',i===idx));}
function setPedal(n){vcmd(pedalCmds[n]);segHL('seg-pedal',n);}
function setRegen(v){const i=regenVals.indexOf(v);vcmd(regenCmds[i]);segHL('seg-regen',i);}
function setStop(n){vcmd(stopCmds[n]);segHL('seg-stop',n);}
function syncDriveMode(d){if(d.pedal!=null)segHL('seg-pedal',d.pedal);if(d.regen!=null){const i=regenVals.indexOf(d.regen);if(i>=0)segHL('seg-regen',i);}if(d.stopM!=null)segHL('seg-stop',d.stopM);}
function saveWifi(){const s=document.getElementById('wSSID').value;const p=document.getElementById('wPASS').value;if(!s){toast('SSID bos olamaz');return;}if(p&&p.length<8){toast('Sifre min 8 karakter');return;}const body={};if(s)body.ssid=s;if(p)body.pass=p;fetch('/wifi',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)}).then(r=>r.json()).then(d=>{toast(d.msg||'Kaydedildi');}).catch(()=>toast('Hata'));}
function resetWifi(){if(!confirm('Fabrika ayarina don? (CanFeather/tesla1234)'))return;fetch('/wifireset').then(r=>r.json()).then(d=>{toast(d.msg||'Sifirlandirildi');}).catch(()=>toast('Hata'));}
function loadWifiInfo(){fetch('/wifi').then(r=>r.json()).then(d=>{document.getElementById('wSSID').value=d.ssid||'';}).catch(()=>{});}
function toast(m){const e=document.getElementById('t');e.textContent=m;e.classList.add('show');setTimeout(()=>e.classList.remove('show'),2600);}
function fmtTime(ms){const s=Math.floor(ms/1000);const m=Math.floor(s/60);const h=Math.floor(m/60);if(h>0)return h+'s '+m%60+'dk';if(m>0)return m+'dk '+s%60+'sn';return s+'sn';}
function updateSB(d){
  document.getElementById('sC').textContent=d.canOk?'AKTIF':'HATA';document.getElementById('sC').style.color=d.canOk?'#10b981':'#ef4444';
  document.getElementById('sF').textContent=d.fsd?'ON':'OFF';document.getElementById('sF').style.color=d.fsd?'#10b981':'#ef4444';
  document.getElementById('sN').textContent=d.fsd?'Disabled':'\u2014';
  document.getElementById('sH').textContent=['Legacy','HW3','HW4'][d.hw||0];
  document.getElementById('sP').textContent=d.profile+' ('+pN[d.profile]+')';
  document.getElementById('sD').textContent=d.fd||0;document.getElementById('sO').textContent=d.off||0;
  document.getElementById('sR').textContent=d.rx||0;document.getElementById('sT').textContent=d.tx||0;
  document.getElementById('sI').textContent=d.inj?'ON':'OFF';document.getElementById('sI').style.color=d.inj?'#10b981':'#ef4444';
  document.getElementById('sU').textContent=fmtTime(d.uptime||0);
  document.getElementById('sE').textContent=d.err||0;
}
function drawSpark(hist){const c=document.getElementById('spark');if(!hist||!hist.length)return;const mx=Math.max(...hist,1);c.innerHTML=hist.map(v=>'<div style="height:'+Math.max(1,v/mx*38)+'px"></div>').join('');}
function drawFHist(events){const c=document.getElementById('fhist');if(!events||!events.length){c.innerHTML=L[lang].noEvent;return;}c.innerHTML=events.map(e=>'<div>'+(e.s?'<b style="color:#10b981">'+L[lang].fsdOn+'</b>':'<b style="color:#ef4444">'+L[lang].fsdOff+'</b>')+' <span style="color:#94a3b8">'+fmtTime(e.t)+'</span></div>').join('');}
function fetchStatus(){fetch('/status').then(r=>r.json()).then(d=>{if(!synced){setHW(d.hw!=null?d.hw:1);setP(d.profile!=null?d.profile:2);setCrystal(d.crystal!=null?d.crystal:1);document.getElementById('ft').checked=!!d.fsd;document.getElementById('ov').checked=!!d.ovr;document.getElementById('ci').checked=d.inj!==false;document.getElementById('ao').value=d.autoOff||0;document.getElementById('flt').value=d.logFilter||0;document.getElementById('snf').checked=!!d.snif;document.getElementById('isa').checked=!!d.isa;document.getElementById('evd').checked=d.ev!==false;synced=true;}updateSB(d);syncDriveMode(d);}).catch(()=>{});}
function fetchLog(){fetch('/log').then(r=>r.json()).then(d=>{if(!d.lines||!d.lines.length)return;const b=document.getElementById('log');b.innerHTML=d.lines.map(l=>'<div class="ll">'+l+'</div>').join('');b.scrollTop=b.scrollHeight;}).catch(()=>{});}
function fetchStats(){fetch('/stats').then(r=>r.json()).then(d=>{drawSpark(d.hist);drawFHist(d.fsdHist);}).catch(()=>{});}
function fetchSnif(){if(!document.getElementById('snf').checked)return;fetch('/sniff').then(r=>r.json()).then(d=>{if(!d.frames||!d.frames.length)return;const b=document.getElementById('snif');b.innerHTML=d.frames.map(f=>'<div>ID:'+f.id+' ['+f.dlc+'] '+f.data+'</div>').join('');b.scrollTop=b.scrollHeight;}).catch(()=>{});}
function fetchSlog(){fetch('/slog').then(r=>r.json()).then(d=>{if(!d.lines||!d.lines.length)return;const b=document.getElementById('slog');b.innerHTML=d.lines.map(l=>'<div class="ll" style="color:#f59e0b">'+l+'</div>').join('');b.scrollTop=b.scrollHeight;}).catch(()=>{});}
setLang(lang);buildP();fetchStatus();fetchStats();loadWifiInfo();
setInterval(fetchStatus,2000);setInterval(fetchLog,2000);setInterval(fetchStats,5000);setInterval(fetchSnif,1000);setInterval(fetchSlog,3000);
</script>
</body>
</html>)HTML";

// ─────────────────────────────────────────────────────────────
//  KRISTAL FREKANS UYGULAMA
// ─────────────────────────────────────────────────────────────

void applyCrystal() {
  mcp->reset();
  CAN_CLOCK clk = (cfg.crystal == 0) ? MCP_8MHZ : MCP_16MHZ;
  MCP2515::ERROR e = mcp->setBitrate(CAN_500KBPS, clk);
  if (e == MCP2515::ERROR_OK) {
    char buf[64]; snprintf(buf, sizeof(buf), "[can] MCP2515 hazir @ 500 kbps (%s kristal)", cfg.crystal == 0 ? "8 MHz" : "16 MHz");
    slog(String(buf));
  } else {
    slog("[can] HATA: setBitrate basarisiz!");
    errCount++;
  }
  mcp->setNormalMode();
}

void onVehicleCmd() {
  String c = server.arg("c");
  if (c.length() == 0) { server.send(400, "application/json", "{\"err\":\"c param gerekli\"}"); return; }
  execVehicleCmd(c);
  server.send(200, "application/json", "{\"ok\":true,\"cmd\":\"" + c + "\"}");
}

void onWiFiConfig() {
  if (server.method() == HTTP_GET) {
    String json = "{\"ssid\":\"" + String(wifiSSID) + "\"}";
    server.send(200, "application/json", json);
    return;
  }
  StaticJsonDocument<128> d;
  if (deserializeJson(d, server.arg("plain"))) { server.send(400); return; }
  if (!d["ssid"].isNull()) {
    String s = d["ssid"].as<String>();
    if (s.length() > 0 && s.length() <= 31) strncpy(wifiSSID, s.c_str(), 32);
  }
  if (!d["pass"].isNull()) {
    String p = d["pass"].as<String>();
    if (p.length() >= 8 && p.length() <= 31) strncpy(wifiPASS, p.c_str(), 32);
    else { server.send(400, "application/json", "{\"err\":\"Sifre min 8 karakter\"}"); return; }
  }
  saveWiFiConfig();
  server.send(200, "application/json", "{\"ok\":true,\"msg\":\"Kaydedildi. Yeniden baslatiliyor...\"}");
  delay(500);
  ESP.restart();
}

void onWiFiReset() {
  resetWiFiConfig();
  server.send(200, "application/json", "{\"ok\":true,\"msg\":\"Fabrika ayari. Yeniden baslatiliyor...\"}");
  delay(500);
  ESP.restart();
}

// ── Route handler'ları ─────────────────────────────────────────

void onRoot() {
  server.send_P(200, "text/html", HTML);
}

void onStatus() {
  StaticJsonDocument<384> d;
  d["hw"]        = cfg.hw;
  d["profile"]   = cfg.speedProfile;
  d["fsd"]       = cfg.fsdEnabled;
  d["ovr"]       = cfg.profileOverride;
  d["inj"]       = cfg.canInjection;
  d["canOk"]     = true;
  d["fsdUI"]     = cfg.fsdSelectedInUI;
  d["fd"]        = lastFollowDist;
  d["off"]       = lastSpeedOff;
  d["rx"]        = rxCount;
  d["tx"]        = txCount;
  d["err"]       = errCount;
  d["uptime"]    = millis() - startMillis;
  d["autoOff"]   = cfg.autoOffMinutes;
  d["logFilter"] = cfg.logFilter;
  d["snif"]      = cfg.snifferMode;
  d["crystal"]   = cfg.crystal;
  d["isa"]       = cfg.isaSuppress;
  d["ev"]        = cfg.emergencyDetect;
  d["pedal"]     = cfg.pedalMode;
  d["regen"]     = cfg.regenLevel;
  d["stopM"]     = cfg.stopMode;
  d["sentry"]    = cfg.sentryActive;
  String o; serializeJson(d, o);
  server.send(200, "application/json", o);
}

void onConfig() {
  if (server.method() != HTTP_POST) { server.send(405); return; }
  StaticJsonDocument<256> d;
  if (deserializeJson(d, server.arg("plain"))) { server.send(400); return; }
  if (!d["hw"].isNull())        cfg.hw              = (uint8_t)d["hw"];
  if (!d["profile"].isNull())   cfg.speedProfile    = (uint8_t)d["profile"];
  if (!d["fsd"].isNull())       cfg.fsdEnabled      = (bool)d["fsd"];
  if (!d["ovr"].isNull())       cfg.profileOverride = (bool)d["ovr"];
  if (!d["inj"].isNull())       cfg.canInjection    = (bool)d["inj"];
  if (!d["autoOff"].isNull())   {
    cfg.autoOffMinutes = (uint16_t)d["autoOff"];
    if (cfg.autoOffMinutes > 0) autoOffStart = millis();
    else autoOffStart = 0;
  }
  if (!d["logFilter"].isNull()) cfg.logFilter       = (uint16_t)d["logFilter"];
  if (!d["snif"].isNull())      cfg.snifferMode     = (bool)d["snif"];
  if (!d["crystal"].isNull()) {
    cfg.crystal = (uint8_t)d["crystal"];
    if (cfg.crystal > 1) cfg.crystal = 1;
    applyCrystal();
  }
  if (!d["isa"].isNull())       cfg.isaSuppress     = (bool)d["isa"];
  if (!d["ev"].isNull())        cfg.emergencyDetect = (bool)d["ev"];
  { char buf[128]; snprintf(buf, sizeof(buf), "[config] hw=%d p=%d fsd=%d ovr=%d inj=%d autoOff=%d filter=%d snif=%d crystal=%d isa=%d ev=%d",
                cfg.hw, cfg.speedProfile, cfg.fsdEnabled, cfg.profileOverride,
                cfg.canInjection, cfg.autoOffMinutes, cfg.logFilter, cfg.snifferMode, cfg.crystal, cfg.isaSuppress, cfg.emergencyDetect);
    slog(String(buf)); }
  server.send(200, "application/json", "{\"ok\":true}");
}

void onDisable() {
  cfg.fsdEnabled   = false;
  cfg.canInjection = false;
  slog("[!] Tum enjeksiyonlar devre disi");
  server.send(200, "application/json", "{\"ok\":true}");
}

void onLog() {
  StaticJsonDocument<1024> d;
  JsonArray arr = d.createNestedArray("lines");
  int total = min(logIdx, LOG_CAP);
  int start = (logIdx >= LOG_CAP) ? logIdx % LOG_CAP : 0;
  for (int i = 0; i < total; i++) arr.add(logBuf[(start + i) % LOG_CAP]);
  String o; serializeJson(d, o);
  server.send(200, "application/json", o);
}

void onSaveEE() {
  saveConfig();
  server.send(200, "application/json", "{\"ok\":true}");
}

void onStats() {
  StaticJsonDocument<512> d;
  // Trafik gecmisi
  JsonArray hist = d.createNestedArray("hist");
  for (int i = 0; i < HIST_LEN; i++) hist.add(rxHist[(histIdx + i) % HIST_LEN]);
  // FSD gecmisi
  JsonArray fh = d.createNestedArray("fsdHist");
  int total = min((int)fsdHistIdx, FSD_HIST_LEN);
  int start = (fsdHistIdx >= FSD_HIST_LEN) ? fsdHistIdx % FSD_HIST_LEN : 0;
  for (int i = 0; i < total; i++) {
    JsonObject ev = fh.createNestedObject();
    FSDEvent& e = fsdHist[(start + i) % FSD_HIST_LEN];
    ev["t"] = e.ts;
    ev["s"] = e.state;
  }
  String o; serializeJson(d, o);
  server.send(200, "application/json", o);
}

void onSniff() {
  StaticJsonDocument<1024> d;
  JsonArray arr = d.createNestedArray("frames");
  int total = min((int)snifIdx, SNIF_CAP);
  int start = (snifIdx >= SNIF_CAP) ? snifIdx % SNIF_CAP : 0;
  for (int i = 0; i < total; i++) {
    can_frame& f = snifBuf[(start + i) % SNIF_CAP];
    JsonObject fr = arr.createNestedObject();
    fr["id"] = f.can_id;
    fr["dlc"] = f.can_dlc;
    char hex[24]; snprintf(hex, sizeof(hex), "%02X %02X %02X %02X %02X %02X %02X %02X",
      f.data[0],f.data[1],f.data[2],f.data[3],f.data[4],f.data[5],f.data[6],f.data[7]);
    fr["data"] = hex;
  }
  String o; serializeJson(d, o);
  server.send(200, "application/json", o);
}

void onSlog() {
  StaticJsonDocument<768> d;
  JsonArray arr = d.createNestedArray("lines");
  int total = min(slogIdx, SLOG_CAP);
  int start = (slogIdx >= SLOG_CAP) ? slogIdx % SLOG_CAP : 0;
  for (int i = 0; i < total; i++) arr.add(slogBuf[(start + i) % SLOG_CAP]);
  String o; serializeJson(d, o);
  server.send(200, "application/json", o);
}

void onSend() {
  if (server.method() != HTTP_POST) { server.send(405); return; }
  StaticJsonDocument<256> d;
  if (deserializeJson(d, server.arg("plain"))) { server.send(400); return; }
  can_frame f;
  memset(&f, 0, sizeof(f));
  f.can_id  = (uint16_t)(d["id"] | 0);
  f.can_dlc = (uint8_t)(d["dlc"] | 8);
  JsonArray data = d["data"];
  for (int i = 0; i < min((int)f.can_dlc, 8); i++) f.data[i] = (uint8_t)(data[i] | 0);
  mcp->sendMessage(&f);
  txCount++;
  Serial.printf("[send] ID=%d DLC=%d\n", f.can_id, f.can_dlc);
  server.send(200, "application/json", "{\"ok\":true}");
}

// ─────────────────────────────────────────────────────────────
//  SETUP
// ─────────────────────────────────────────────────────────────

void setup() {
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);  // ESP32 normal mantık: HIGH = açık

  delay(1000);
  Serial.begin(115200);
  Serial.println("\n=== CanFeather ESP32 ===");

  // EEPROM
  EEPROM.begin(EEPROM_SIZE);
  loadConfig();
  startMillis = millis();

  // WiFi ayarlarini EEPROM'dan yukle
  loadWiFiConfig();

  // WiFi Access Point
  WiFi.mode(WIFI_AP);
  WiFi.softAP(wifiSSID, wifiPASS);
  Serial.printf("[wifi] SSID: %s  IP: %s\n", wifiSSID, WiFi.softAPIP().toString().c_str());

  // Web sunucu
  server.on("/",        HTTP_GET,  onRoot);
  server.on("/status",  HTTP_GET,  onStatus);
  server.on("/config",  HTTP_POST, onConfig);
  server.on("/disable", HTTP_GET,  onDisable);
  server.on("/log",     HTTP_GET,  onLog);
  server.on("/saveee",  HTTP_GET,  onSaveEE);
  server.on("/stats",   HTTP_GET,  onStats);
  server.on("/sniff",   HTTP_GET,  onSniff);
  server.on("/slog",    HTTP_GET,  onSlog);
  server.on("/send",    HTTP_POST, onSend);
  server.on("/cmd",     HTTP_GET,  onVehicleCmd);
  server.on("/wifi",    HTTP_GET,  onWiFiConfig);
  server.on("/wifi",    HTTP_POST, onWiFiConfig);
  server.on("/wifireset", HTTP_GET, onWiFiReset);

  // OTA guncelleme: http://192.168.4.1/update
  server.on("/update", HTTP_GET, []() {
    server.send_P(200, "text/html", OTA_HTML);
  });
  server.on("/update", HTTP_POST, []() {
    server.send(200, "text/plain", Update.hasError() ? "HATA" : "OK — yeniden baslatiliyor...");
    delay(500);
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Update.begin(UPDATE_SIZE_UNKNOWN);
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      Update.write(upload.buf, upload.currentSize);
    } else if (upload.status == UPLOAD_FILE_END) {
      Update.end(true);
    }
  });

  server.begin();
  Serial.println("[web] http://192.168.4.1");
  Serial.println("[ota] http://192.168.4.1/update");

  // SPI + MCP2515
  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);
  mcp = std::make_unique<MCP2515>(PIN_CS);
  applyCrystal();

  digitalWrite(PIN_LED, LOW);  // LED kapat
  Serial.println("[ok] Hazir.\n");
}

// ─────────────────────────────────────────────────────────────
//  LOOP
// ─────────────────────────────────────────────────────────────

void loop() {
  server.handleClient();

  // Trafik gecmisi guncelle (saniyede 1)
  unsigned long now = millis();
  if (now - lastHistUpdate >= 1000) {
    rxHist[histIdx % HIST_LEN] = rxSinceLastHist;
    histIdx++;
    rxSinceLastHist = 0;
    lastHistUpdate = now;
  }

  // Otomatik kapanma zamanlayici
  if (cfg.autoOffMinutes > 0 && autoOffStart > 0) {
    if (now - autoOffStart >= (unsigned long)cfg.autoOffMinutes * 60000UL) {
      cfg.fsdEnabled = false;
      cfg.canInjection = false;
      cfg.autoOffMinutes = 0;
      autoOffStart = 0;
      slog("[autooff] Zamanlayici doldu, enjeksiyonlar kapatildi");
    }
  }

  can_frame frame;
  if (mcp->readMessage(&frame) == MCP2515::ERROR_OK) {
    rxCount++;
    rxSinceLastHist++;
    digitalWrite(PIN_LED, HIGH);  // LED yak (normal mantık)
    processFrame(frame);
    digitalWrite(PIN_LED, LOW);   // LED söndür
  }
}
