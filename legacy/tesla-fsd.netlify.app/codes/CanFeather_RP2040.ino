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
 * CanFeather RP2040 — Tesla FSD CAN Bus Mod + ESP32-C3 WiFi Bridge
 * =================================================================
 * BU KODU YUKLE → Adafruit RP2040 CAN Bus Feather (#5724)
 * Arduino IDE: Tools > Board > Adafruit Feather RP2040
 * NOT: ESP32-C3 icin ayrica ESP32C3_WiFiBridge.ino yukleyin!
 *
 * Bu versiyon: Adafruit RP2040 CAN Bus Feather (#5724)
 *   MCP2515 dahili — harici modül gerekmez
 *   SPI1 bus (SCK=14, MISO=12, MOSI=15, CS=13, INT=11)
 *   UART1 (TX=GPIO0, RX=GPIO1) → ESP32-C3 Mini WiFi bridge
 *
 * DONANIM BAĞLANTISI:
 *   Feather CANH  -->  X179 Pin 13
 *   Feather CANL  -->  X179 Pin 14
 *   Feather TX0   -->  ESP32-C3 RX (GPIO20)
 *   Feather RX1   -->  ESP32-C3 TX (GPIO21)
 *   Feather 3.3V  -->  ESP32-C3 3V3
 *   Feather GND   -->  ESP32-C3 GND
 *
 *   ÖNEMLİ: JP1 jumper çıkar veya R10 kaldır
 *
 * KULLANIM:
 *   1. Bu kodu RP2040'a yükleyin
 *   2. ESP32C3_WiFiBridge.ino'yu ESP32-C3'e yükleyin
 *   3. WiFi: "CanFeather" ağına bağlanın → 192.168.4.1
 *   4. Serial Monitor da çalışır (115200 baud, debug için)
 *
 * GEREKLİ KÜTÜPHANELer:
 *   - mcp2515 by autowp   (Library Manager)
 *   - ArduinoJson v6      (Library Manager — v6 seçin, v7 değil)
 *
 * BOARD SEÇİMİ:
 *   Tools > Board > Adafruit Feather RP2040
 */

#include <SPI.h>
#include <mcp2515.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

// ─────────────────────────────────────────────────────────────
//  AYARLAR
// ─────────────────────────────────────────────────────────────

#define FW_VERSION     "v3.1"
#define FW_DATE        "15.04.2026"

#define PIN_CAN_CS   13
#define PIN_CAN_INT  11
#define PIN_CAN_SCK  14
#define PIN_CAN_MISO 12
#define PIN_CAN_MOSI 15
#define PIN_LED      25

// MCP2515 kristal frekansi — serial komutla secilir (crystal8/crystal16)

// UART1 → ESP32-C3 (GPIO0=TX, GPIO1=RX)
#define BRIDGE_SERIAL Serial1
#define BRIDGE_BAUD   115200

// HW4 FSD V14 options
#define ENABLE_APPROACHING_EMERGENCY_VEHICLE_DETECTION true
#define ENABLE_ISA_SPEED_CHIME_SUPPRESS false // suppresses ISA speed chime, but speed limit sign will be empty while driving

// #define FORCE_FSD  // FSD'yi her zaman aktif et — "Trafik Isigi" ayari gerekmez

#define EEPROM_SIZE    64
#define EEPROM_MAGIC   0xCF  // Config gecerlilik bayragi

// ─────────────────────────────────────────────────────────────
//  ÇALIŞMA ZAMANI KONFİGÜRASYONU
// ─────────────────────────────────────────────────────────────

struct Config {
  uint8_t hw              = 1;    // 0=Legacy  1=HW3  2=HW4
  uint8_t speedProfile    = 2;    // 0-4
  bool    fsdEnabled      = true;
  bool    profileOverride = false; // true = web profili kullan, CAN yok say
  bool    canInjection    = true;  // false = kopru seffaf
  bool    fsdSelectedInUI = false; // CAN'dan okunan FSD secim durumu
  bool    logEnabled      = true;
  uint16_t autoOffMinutes = 0;    // 0 = devre disi, >0 = X dakika sonra enjeksiyon kapanir
  uint16_t logFilter      = 0;    // 0 = tum ID'ler, >0 = sadece bu CAN ID
  uint8_t crystal         = 1;    // 0=8MHz, 1=16MHz
  bool    isaSuppress     = false; // ISA speed chime suppress
  bool    emergencyDetect = true;  // approaching emergency vehicle detection
  // Arac kontrol kalici ayarlar (EEPROM addr 14-17)
  uint8_t pedalMode        = 0;    // 0=Std, 1=Chill, 2=Sport
  uint8_t regenLevel       = 100;  // 0=OFF, 50=Low, 100=Std, 200=Max
  uint8_t stopMode         = 2;    // 0=Creep, 1=Roll, 2=Hold
  bool    sentryActive     = false;
  // v2.3 Yeni: gelismis ozellikler (EEPROM addr 18-20)
  bool    nagKiller        = false; // Autosteer Nag Killer — direksiyona dokunmadan surme (CAN 880)
  bool    bypassTLSSC      = false; // FSD'yi "Trafik Isigi" gerekmeden zorla acik tut
  bool    busAutoRecover   = true;  // CAN bus-off otomatik kurtarma
  // v2.4 Yeni: ISA Speed Override (EEPROM addr 21)
  bool    isaSpeedOverride = false; // HW4 mux 2 — gercek hizi navigasyon hiz limitine ezdirme
  // v2.4 Yeni: ISA Speed Multiplier (EEPROM addr 22)
  uint8_t isaSpeedMul      = 7;     // v2.4: offset carpan (1-15) — default 7, web UI'dan ayar
};

Config cfg;

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
  EEPROM.write(8, cfg.logFilter & 0xFF);
  EEPROM.write(9, (cfg.logFilter >> 8) & 0xFF);
  // byte 10 bos (eski snifferMode, kullanilmiyor)
  EEPROM.write(11, cfg.crystal);
  EEPROM.write(12, cfg.isaSuppress ? 1 : 0);
  EEPROM.write(13, cfg.emergencyDetect ? 1 : 0);
  // Arac kontrol kalici ayarlar (addr 14-17)
  EEPROM.write(14, cfg.pedalMode);
  EEPROM.write(15, cfg.regenLevel);
  EEPROM.write(16, cfg.stopMode);
  EEPROM.write(17, cfg.sentryActive ? 1 : 0);
  // v2.3 yeni ayarlar (addr 18-20)
  EEPROM.write(18, cfg.nagKiller ? 1 : 0);
  EEPROM.write(19, cfg.bypassTLSSC ? 1 : 0);
  EEPROM.write(20, cfg.busAutoRecover ? 1 : 0);
  EEPROM.write(21, cfg.isaSpeedOverride ? 1 : 0);
  EEPROM.write(22, cfg.isaSpeedMul);
  EEPROM.commit();
  slog("[eeprom] Ayarlar kaydedildi");
}

void loadConfig() {
  if (EEPROM.read(0) != EEPROM_MAGIC) {
    Serial.println("[eeprom] Kayitli ayar yok, varsayilan kullaniliyor");
    cfg.isaSuppress     = ENABLE_ISA_SPEED_CHIME_SUPPRESS;
    cfg.emergencyDetect = ENABLE_APPROACHING_EMERGENCY_VEHICLE_DETECTION;
    return;
  }
  cfg.hw              = EEPROM.read(1);
  cfg.speedProfile    = EEPROM.read(2);
  cfg.fsdEnabled      = EEPROM.read(3) == 1;
  cfg.profileOverride = EEPROM.read(4) == 1;
  cfg.canInjection    = EEPROM.read(5) == 1;
  cfg.autoOffMinutes  = EEPROM.read(6) | (EEPROM.read(7) << 8);
  cfg.logFilter       = EEPROM.read(8) | (EEPROM.read(9) << 8);
  // byte 10 atla (eski snifferMode, kullanilmiyor)
  cfg.crystal         = EEPROM.read(11);
  if (cfg.crystal > 1) cfg.crystal = 1; // guvenlik: gecersiz deger ise 16MHz varsayilan
  cfg.isaSuppress     = EEPROM.read(12) == 1;
  cfg.emergencyDetect = EEPROM.read(13) == 1;
  // Arac kontrol kalici ayarlar
  cfg.pedalMode       = EEPROM.read(14);
  cfg.regenLevel      = EEPROM.read(15);
  cfg.stopMode        = EEPROM.read(16);
  cfg.sentryActive    = EEPROM.read(17) == 1;
  // v2.3 yeni ayarlar
  uint8_t v18 = EEPROM.read(18);
  uint8_t v19 = EEPROM.read(19);
  uint8_t v20 = EEPROM.read(20);
  uint8_t v21 = EEPROM.read(21);
  uint8_t v22 = EEPROM.read(22);
  if (v18 != 0xFF) cfg.nagKiller        = v18 == 1;
  if (v19 != 0xFF) cfg.bypassTLSSC      = v19 == 1;
  if (v20 != 0xFF) cfg.busAutoRecover   = v20 == 1;
  if (v21 != 0xFF) cfg.isaSpeedOverride = v21 == 1;
  if (v22 != 0xFF && v22 >= 1 && v22 <= 15) cfg.isaSpeedMul = v22;
  if (cfg.pedalMode > 2) cfg.pedalMode = 0;
  if (cfg.stopMode > 2) cfg.stopMode = 2;
  Serial.printf("[eeprom] Yuklendi: hw=%d p=%d fsd=%d pedal=%d regen=%d stop=%d sentry=%d\n",
                cfg.hw, cfg.speedProfile, cfg.fsdEnabled, cfg.pedalMode, cfg.regenLevel, cfg.stopMode, cfg.sentryActive);
}

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

// ─────────────────────────────────────────────────────────────
//  LOG
// ─────────────────────────────────────────────────────────────

#define LOG_CAP 24
static String logBuf[LOG_CAP];
static int    logIdx = 0;

void pushLog(const String& s, uint16_t canId = 0) {
  // Filtre kontrolu
  if (cfg.logFilter > 0 && canId > 0 && canId != cfg.logFilter) return;
  logBuf[logIdx % LOG_CAP] = s;
  logIdx++;
  if (cfg.logEnabled) Serial.println(s);
}

// ── Serial Log (slog) — web UI'siz RP2040 icin serial gecmisi ──
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

MCP2515 mcp(PIN_CAN_CS, &SPI1);

// MCP2515 hardware filter — bizim islememiz gereken CAN ID'ler CPU'ya gelir.
// Vehicle control olan firmware oldugu icin RXB1'de mask trick kullanilir.
// Yanlis pozitifler processFrame()'de reddedilir.
void applyCanFilters() {
  if (cfg.hw == 0) {
    // Legacy
    mcp.setFilterMask(MCP2515::MASK0, false, 0x7FF);
    mcp.setFilter(MCP2515::RXF0, false, 1006);  // 0x3EE
    mcp.setFilter(MCP2515::RXF1, false, 69);    // 0x45
  } else {
    // HW3/HW4
    mcp.setFilterMask(MCP2515::MASK0, false, 0x7FF);
    mcp.setFilter(MCP2515::RXF0, false, 1021);  // 0x3FD
    mcp.setFilter(MCP2515::RXF1, false, 1016);  // 0x3F8
  }
  // Vehicle control + ISA chime — mask trick (alt 3 bit don't care)
  mcp.setFilterMask(MCP2515::MASK1, false, 0x7F8);
  mcp.setFilter(MCP2515::RXF2, false, 0x370);   // 0x370-0x377 (nag killer)
  mcp.setFilter(MCP2515::RXF3, false, 0x2F0);   // 0x2F0-0x2F7 (0x2F3)
  mcp.setFilter(MCP2515::RXF4, false, 0x330);   // 0x330-0x337 (0x333, 0x334)
  mcp.setFilter(MCP2515::RXF5, false, 0x398);   // 0x398-0x39F (0x399)
}

void applyCrystal() {
  CAN_CLOCK clk = (cfg.crystal == 0) ? MCP_8MHZ : MCP_16MHZ;
  mcp.reset();
  MCP2515::ERROR e = mcp.setBitrate(CAN_500KBPS, clk);
  if (e == MCP2515::ERROR_OK) {
    Serial.printf("[can] MCP2515 hazir @ 500 kbps (%s kristal)\n",
                  (cfg.crystal == 0) ? "8 MHz" : "16 MHz");
  } else {
    slog("[can] HATA: setBitrate basarisiz!");
    errCount++;
  }
  applyCanFilters();
  mcp.setNormalMode();
}

// ─────────────────────────────────────────────────────────────
//  CAN YARDIMCILAR
// ─────────────────────────────────────────────────────────────

inline uint8_t muxID(const can_frame& f)      { return f.data[0] & 0x07; }
inline bool    fsdInUI(const can_frame& f)     { return (f.data[4] >> 6) & 0x01; }
inline uint8_t followDist(const can_frame& f)  { return (f.data[5] & 0xE0) >> 5; }

inline void setBit(can_frame& f, uint8_t bit, bool val) {
  uint8_t mask = 1u << (bit % 8);
  if (val) f.data[bit/8] |=  mask;
  else     f.data[bit/8] &= ~mask;
}

inline void setSpeedV12V13(can_frame& f, int p) {
  f.data[6] = (f.data[6] & ~0x06) | ((p & 0x03) << 1);
}

inline void canSend(can_frame& f) { mcp.sendMessage(&f); txCount++; }

// ─────────────────────────────────────────────────────────────
//  HANDS-FREE STEERING — Direksiyona dokunmadan surme
//  CAN 880 (EPAS3P_sysStatus) uzerinden torque degeri sahtelenir,
//  AP ECU direksiyonun tutuldugunu sanir → uyari gelmez.
//  NOT: Chassis CAN (bus 4) erisimi gerektirir.
// ─────────────────────────────────────────────────────────────

inline void handleNagKiller(can_frame& f) {
  if (!cfg.nagKiller || f.can_id != 880) return;
  uint8_t realHandsOn = (f.data[4] >> 6) & 0x03;
  if (realHandsOn != 0) return;
  f.data[3] = 0xB6;
  f.data[4] = (f.data[4] & ~0xC0) | 0x40;
  uint8_t cnt = f.data[1] & 0x0F;
  cnt = (cnt + 1) & 0x0F;
  f.data[1] = (f.data[1] & ~0x0F) | cnt;
  uint8_t sum = 0;
  for (int i = 0; i < 7; i++) sum += f.data[i];
  f.data[7] = (sum + 0x73) & 0xFF;
  canSend(f);
}

// ─────────────────────────────────────────────────────────────
//  LEGACY HANDLER
// ─────────────────────────────────────────────────────────────

void handleLegacy(can_frame& f) {
  // STW_ACTN_RQ (0x045 = 69): Follow-Distance-Stalk as Source for Profile Mapping
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
  // bypassTLSSC = "Trafik Isigi" gerekmeden FSD aktif et (runtime toggle)
  if (idx == 0 && cfg.fsdEnabled && (cfg.bypassTLSSC || cfg.fsdSelectedInUI)) {
    setBit(f, 46, true);
    setSpeedV12V13(f, cfg.speedProfile);
    canSend(f);
    pushLog("LEGACY mux=0 FSD=1 profile=" + String(cfg.speedProfile), 1006);
  }
  if (idx == 1) { setBit(f, 19, false); canSend(f); }
}

// ─────────────────────────────────────────────────────────────
//  HW3 HANDLER
// ─────────────────────────────────────────────────────────────

void handleHW3(can_frame& f) {
  // ISA Speed Chime Suppress (CAN ID 921) — paylasilan kodla birebir
  if (cfg.isaSuppress && f.can_id == 921) {
    f.data[1] |= 0x20;
    uint8_t sum = 0;
    for (int i = 0; i < 7; i++) sum += f.data[i];
    sum += (921 & 0xFF) + (921 >> 8);
    f.data[7] = sum & 0xFF;
    canSend(f);
    return;
  }

  // 1016: takip mesafesi — sadece UI status icin oku
  // HW3 fd->profile auto-mapping kaldirildi: eski switch kirikti
  // (fd=1/7 case yok, UI'da olmayan Max/Sloth degerleri cfg'ye yaziliyordu).
  // HW3'te profil sadece UI'dan manuel secilir — pS[1]=[2,1,0].
  if (f.can_id == 1016) {
    lastFollowDist = followDist(f);
    pushLog("HW3 id=1016 fd=" + String(lastFollowDist), 1016);
    return;
  }

  if (f.can_id != 1021) return;
  uint8_t idx = muxID(f);

  if (idx == 0) {
    cfg.fsdSelectedInUI = fsdInUI(f);
    // Gercek hiz offset'i — paylasilan kod birebir (*7, 0-200)
    int off = (int)((f.data[3] >> 1) & 0x3F) - 30;
    lastSpeedOff = constrain(off * cfg.isaSpeedMul, 0, 200);
  }

  // bypassTLSSC = "Trafik Isigi" gerekmeden FSD aktif et
  bool fsdOn = cfg.fsdEnabled && (cfg.bypassTLSSC || cfg.fsdSelectedInUI);

  if (idx == 0 && fsdOn) {
    setBit(f, 46, true);
    setBit(f, 60, true);
    if (cfg.emergencyDetect) setBit(f, 59, true);
    canSend(f);
    pushLog("HW3 mux=0 FSD=1 off=" + String(lastSpeedOff), 1021);
  }
  if (idx == 1) {
    setBit(f, 19, false);
    setBit(f, 47, true);
    if (cfg.isaSpeedOverride) f.data[2] &= ~0x08;
    canSend(f);
  }
  if (idx == 2 && fsdOn && cfg.isaSpeedOverride) {
    f.data[7] = (f.data[7] & ~0x70) | ((cfg.speedProfile & 0x07) << 4);
    f.data[0] = (f.data[0] & ~0xC0) | ((lastSpeedOff & 0x03) << 6);
    f.data[1] = (f.data[1] & ~0x3F) | (lastSpeedOff >> 2);
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
      case 1: p = 3; break; case 2: p = 2; break;
      case 3: p = 1; break; case 4: p = 0; break;
      case 5: p = 4; break;
    }
    if (!cfg.profileOverride) cfg.speedProfile = p;
    pushLog("HW4 id=1016 fd=" + String(fd) + " p=" + String(cfg.speedProfile), 1016);
    return;
  }
  if (f.can_id != 1021) return;

  uint8_t idx   = muxID(f);
  if (idx == 0) {
    cfg.fsdSelectedInUI = fsdInUI(f);
    // v2.4: gercek hiz offset'i hesapla (ISA Speed Override icin)
    int off = (int)((f.data[3] >> 1) & 0x3F) - 30;
    lastSpeedOff = constrain(off * cfg.isaSpeedMul, 0, 200);
  }
  // bypassTLSSC = "Trafik Isigi" gerekmeden FSD aktif et (runtime toggle)
  bool    fsdOn = cfg.fsdEnabled && (cfg.bypassTLSSC || cfg.fsdSelectedInUI);

  if (idx == 0 && fsdOn) {
    setBit(f, 46, true); setBit(f, 60, true);
    if (cfg.emergencyDetect) {
      setBit(f, 59, true);
    }
    canSend(f);
    pushLog("HW4 id=1021 mux=0 FSD=1 profile=" + String(cfg.speedProfile), 1021);
  }
  if (idx == 1) {
    setBit(f, 19, false); setBit(f, 47, true);
    // v2.4: ek nag bit temizle
    if (cfg.isaSpeedOverride) f.data[2] &= ~0x08;
    canSend(f);
  }
  if (idx == 2) {
    f.data[7] = (f.data[7] & ~0x70) | ((cfg.speedProfile & 0x07) << 4);
    // v2.4: ISA Speed Override — gercek hiz offset'ini byte[0]/byte[1]'e enjekte et
    // Bu sayede nav hiz limiti (orn 30) gercek hizi (orn 80) ezmez
    if (cfg.isaSpeedOverride && fsdOn) {
      f.data[0] = (f.data[0] & ~0xC0) | ((lastSpeedOff & 0x03) << 6);
      f.data[1] = (f.data[1] & ~0x3F) | (lastSpeedOff >> 2);
    }
    canSend(f);
  }
}

// ─────────────────────────────────────────────────────────────
//  FRAME YÖNLENDİRİCİ
// ─────────────────────────────────────────────────────────────

void processFrame(can_frame& f) {
  // FSD durum takibi (enjeksiyon kapali olsa bile)
  trackFSDState(cfg.fsdSelectedInUI && cfg.fsdEnabled);

  // ── Arac kontrol frame buffer'larini surekli guncelle ──
  if (f.can_id == 0x273) { memcpy(buf_273, f.data, 8); has_273 = true; }
  else if (f.can_id == 0x2F3) { memcpy(buf_2F3, f.data, 5); has_2F3 = true; }
  else if (f.can_id == 0x333) { memcpy(buf_333, f.data, 5); has_333 = true; }
  else if (f.can_id == 0x334) { memcpy(buf_334, f.data, 8); has_334 = true; }

  // ── Boot'ta kalici ayarlari otomatik uygula (frame yakalandiktan sonra) ──
  if (!bootApplied && has_273 && has_334) {
    bootApplied = true;
    bool hasNonDefault = (cfg.pedalMode != 0 || cfg.regenLevel != 100 || cfg.stopMode != 2 || cfg.sentryActive);
    if (hasNonDefault) {
      slog("[boot] Kalici ayarlar uygulaniyor (inject)...");
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

  // Hands-free steering (CAN 880) — injection'dan bagimsiz calisir
  handleNagKiller(f);

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
  while (mcp.readMessage(&rx) == MCP2515::ERROR_OK) {
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
    canSend(f); delay(20);
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
    canSend(f); delay(20);
  }
  slog(String("[cmd] ") + label + " OK");
}

void sendInjectFrame(uint16_t id, uint8_t dlc, const uint8_t* data, uint8_t repeat, const char* label) {
  can_frame f; memset(&f, 0, sizeof(f));
  f.can_id = id; f.can_dlc = dlc;
  memcpy(f.data, data, dlc);
  for (uint8_t i = 0; i < repeat; i++) { canSend(f); delay(100); }
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
    canSend(f); delay(20);
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
    canSend(f); delay(20);
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
    for(uint8_t i=0;i<30;i++){refreshBufs();memcpy(cf.data,buf_2F3,5);cf.data[4]=(cf.data[4]&~0x06)|(1<<1);canSend(cf);delay(20);}
    slog("[cmd] Klima Acik Tut OK"); return;
  }
  if (cmd == "climate_off") {
    if (!has_2F3) { slog("[cmd] 0x2F3 henuz yakalanmadi"); return; }
    can_frame cf; memset(&cf,0,sizeof(cf)); cf.can_id=0x2F3; cf.can_dlc=5;
    for(uint8_t i=0;i<30;i++){refreshBufs();memcpy(cf.data,buf_2F3,5);cf.data[4]=(cf.data[4]&~0x06);canSend(cf);delay(20);}
    slog("[cmd] Klima Kapat OK"); return;
  }
  // ── Sarj (0x333) ──
  if (cmd == "charge_start") {
    if (!has_333) { slog("[cmd] 0x333 henuz yakalanmadi"); return; }
    can_frame cf; memset(&cf,0,sizeof(cf)); cf.can_id=0x333; cf.can_dlc=5;
    for(uint8_t i=0;i<20;i++){refreshBufs();memcpy(cf.data,buf_333,5);cf.data[0]|=0x04;canSend(cf);delay(20);}
    slog("[cmd] Sarj Baslat OK"); return;
  }
  if (cmd == "charge_stop") {
    if (!has_333) { slog("[cmd] 0x333 henuz yakalanmadi"); return; }
    can_frame cf; memset(&cf,0,sizeof(cf)); cf.can_id=0x333; cf.can_dlc=5;
    for(uint8_t i=0;i<20;i++){refreshBufs();memcpy(cf.data,buf_333,5);cf.data[0]&=~0x04;canSend(cf);delay(20);}
    slog("[cmd] Sarj Durdur OK"); return;
  }
  if (cmd == "charge_port") {
    if (!has_333) { slog("[cmd] 0x333 henuz yakalanmadi"); return; }
    can_frame cf; memset(&cf,0,sizeof(cf)); cf.can_id=0x333; cf.can_dlc=5;
    for(uint8_t i=0;i<20;i++){refreshBufs();memcpy(cf.data,buf_333,5);cf.data[0]|=0x01;canSend(cf);delay(20);}
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
//  SERİAL + BRIDGE KOMUTLAR
//  Hem Serial (USB debug) hem UART1 (ESP32-C3) dinlenir
// ─────────────────────────────────────────────────────────────

void processCommand(const String& cmd, Stream& reply) {
  if      (cmd == "hw0") { cfg.hw = 0; applyCrystal(); reply.println("[ok] HW=Legacy"); }
  else if (cmd == "hw1") { cfg.hw = 1; applyCrystal(); reply.println("[ok] HW=HW3"); }
  else if (cmd == "hw2") { cfg.hw = 2; applyCrystal(); reply.println("[ok] HW=HW4"); }
  else if (cmd == "p0")  { cfg.speedProfile = 0; reply.println("[ok] profile=0 Chill"); }
  else if (cmd == "p1")  { cfg.speedProfile = 1; reply.println("[ok] profile=1 Normal"); }
  else if (cmd == "p2")  { cfg.speedProfile = 2; reply.println("[ok] profile=2 Hurry"); }
  else if (cmd == "p3")  { cfg.speedProfile = 3; reply.println("[ok] profile=3 Max"); }
  else if (cmd == "p4")  { cfg.speedProfile = 4; reply.println("[ok] profile=4 Sloth"); }
  else if (cmd == "on")  { cfg.fsdEnabled = true;  reply.println("[ok] FSD etkin"); }
  else if (cmd == "off") { cfg.fsdEnabled = false; reply.println("[ok] FSD devre disi"); }
  else if (cmd == "ovron")  { cfg.profileOverride = true;  reply.println("[ok] override acik"); }
  else if (cmd == "ovroff") { cfg.profileOverride = false; reply.println("[ok] override kapali"); }
  else if (cmd == "injon")  { cfg.canInjection = true;  reply.println("[ok] enjeksiyon acik"); }
  else if (cmd == "injoff") { cfg.canInjection = false; reply.println("[ok] enjeksiyon kapali"); }
  else if (cmd == "isaon")  { cfg.isaSuppress = true;  reply.println("[ok] ISA suppress acik"); }
  else if (cmd == "isaoff") { cfg.isaSuppress = false; reply.println("[ok] ISA suppress kapali"); }
  else if (cmd == "evon")   { cfg.emergencyDetect = true;  reply.println("[ok] acil arac algilama acik"); }
  else if (cmd == "evoff")  { cfg.emergencyDetect = false; reply.println("[ok] acil arac algilama kapali"); }
  else if (cmd == "nagon")    { cfg.nagKiller = true;        reply.println("[ok] nag killer acik"); }
  else if (cmd == "nagoff")   { cfg.nagKiller = false;       reply.println("[ok] nag killer kapali"); }
  else if (cmd == "btlsscon") { cfg.bypassTLSSC = true;     reply.println("[ok] bypass TLSSC acik"); }
  else if (cmd == "btlsscoff"){ cfg.bypassTLSSC = false;    reply.println("[ok] bypass TLSSC kapali"); }
  else if (cmd == "recovon")  { cfg.busAutoRecover = true;  reply.println("[ok] auto-recover acik"); }
  else if (cmd == "recovoff") { cfg.busAutoRecover = false; reply.println("[ok] auto-recover kapali"); }
  else if (cmd == "isaovron") { cfg.isaSpeedOverride = true;  reply.println("[ok] ISA Speed Override acik"); }
  else if (cmd == "isaovroff"){ cfg.isaSpeedOverride = false; reply.println("[ok] ISA Speed Override kapali"); }
  else if (cmd.startsWith("isamul ")) {
    int val = cmd.substring(7).toInt();
    if (val >= 1 && val <= 15) {
      cfg.isaSpeedMul = (uint8_t)val;
      reply.printf("[ok] ISA carpan = %d\n", cfg.isaSpeedMul);
    } else {
      reply.println("[err] isamul N (1-15)");
    }
  }
  else if (cmd == "disable") {
    cfg.fsdEnabled = false; cfg.canInjection = false;
    reply.println("[ok] tum enjeksiyonlar devre disi");
  }
  // ── Yeni komutlar: autooff, filter, send, save/load, stats ──
  else if (cmd.startsWith("autooff ")) {
    int val = cmd.substring(8).toInt();
    cfg.autoOffMinutes = (uint16_t)constrain(val, 0, 9999);
    if (cfg.autoOffMinutes > 0) autoOffStart = millis();
    else autoOffStart = 0;
    reply.println("[ok] autoOff=" + String(cfg.autoOffMinutes) + " dk");
  }
  else if (cmd.startsWith("filter ")) {
    int val = cmd.substring(7).toInt();
    cfg.logFilter = (uint16_t)constrain(val, 0, 2047);
    reply.println("[ok] logFilter=" + String(cfg.logFilter));
  }
  else if (cmd == "crystal8") {
    cfg.crystal = 0; applyCrystal(); saveConfig();
    reply.println("[ok] crystal=8MHz");
  }
  else if (cmd == "crystal16") {
    cfg.crystal = 1; applyCrystal(); saveConfig();
    reply.println("[ok] crystal=16MHz");
  }
  else if (cmd.startsWith("send ")) {
    // send ID DLC D0 D1 D2 D3 D4 D5 D6 D7
    int parts[10] = {0};
    int count = 0;
    String rest = cmd.substring(5);
    rest.trim();
    while (rest.length() > 0 && count < 10) {
      int sp = rest.indexOf(' ');
      String tok;
      if (sp == -1) { tok = rest; rest = ""; }
      else { tok = rest.substring(0, sp); rest = rest.substring(sp + 1); rest.trim(); }
      parts[count++] = tok.toInt();
    }
    if (count >= 2) {
      can_frame sf;
      memset(&sf, 0, sizeof(sf));
      sf.can_id  = (uint16_t)parts[0];
      sf.can_dlc = (uint8_t)constrain(parts[1], 0, 8);
      for (int i = 0; i < min((int)sf.can_dlc, 8); i++) {
        if (i + 2 < count) sf.data[i] = (uint8_t)parts[i + 2];
      }
      mcp.sendMessage(&sf);
      txCount++;
      reply.printf("[ok] sent ID=%d DLC=%d\n", sf.can_id, sf.can_dlc);
    } else {
      reply.println("[?] send ID DLC D0 D1 D2 D3 D4 D5 D6 D7");
    }
  }
  else if (cmd == "save") {
    saveConfig();
    reply.println("[ok] EEPROM kaydedildi");
  }
  else if (cmd == "load") {
    loadConfig();
    reply.println("[ok] EEPROM yuklendi");
  }
  else if (cmd == "stats") {
    StaticJsonDocument<512> d;
    d["uptime"]  = millis() - startMillis;
    d["rx"]      = rxCount;
    d["tx"]      = txCount;
    d["err"]     = errCount;
    d["autoOff"] = cfg.autoOffMinutes;
    d["filter"]  = cfg.logFilter;
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
    String out; serializeJson(d, out);
    reply.println(out);
  }
  // ── Mevcut komutlar ──
  else if (cmd == "status") {
    StaticJsonDocument<768> d;
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
    d["crystal"]   = cfg.crystal;
    d["isa"]       = cfg.isaSuppress;
    d["ev"]        = cfg.emergencyDetect;
    d["pedal"]     = cfg.pedalMode;
    d["regen"]     = cfg.regenLevel;
    d["stopM"]     = cfg.stopMode;
    d["sentry"]    = cfg.sentryActive;
    // v2.3 yeni
    d["nag"]       = cfg.nagKiller;
    d["btlssc"]    = cfg.bypassTLSSC;
    d["recov"]     = cfg.busAutoRecover;
    d["isaOvr"]    = cfg.isaSpeedOverride;
    d["isaMul"]    = cfg.isaSpeedMul;
    // v2.7: Versiyon bilgisi (ESP32-C3 bunu forward eder)
    d["fwVersion"] = FW_VERSION;
    d["fwDate"]    = FW_DATE;
    String out; serializeJson(d, out);
    reply.println(out);
  }
  else if (cmd == "log") {
    int total = min(logIdx, LOG_CAP);
    int start = (logIdx >= LOG_CAP) ? logIdx % LOG_CAP : 0;
    StaticJsonDocument<1024> d;
    JsonArray arr = d.createNestedArray("lines");
    for (int i = 0; i < total; i++)
      arr.add(logBuf[(start + i) % LOG_CAP]);
    String out; serializeJson(d, out);
    reply.println(out);
  }
  else if (cmd == "slog") {
    int total = min(slogIdx, SLOG_CAP);
    int start = (slogIdx >= SLOG_CAP) ? slogIdx % SLOG_CAP : 0;
    StaticJsonDocument<768> d;
    JsonArray arr = d.createNestedArray("lines");
    for (int i = 0; i < total; i++)
      arr.add(slogBuf[(start + i) % SLOG_CAP]);
    String out; serializeJson(d, out);
    reply.println(out);
  }
  else if (cmd == "logon")  { cfg.logEnabled = true;  reply.println("[ok] log acik"); }
  else if (cmd == "logoff") { cfg.logEnabled = false; reply.println("[ok] log kapali"); }
  // ── Arac kontrol komutlari (vehicle commands) ──
  else if (cmd == "mirror_fold" || cmd == "mirror_unfold" || cmd == "lock" || cmd == "unlock" ||
           cmd == "frunk" || cmd == "horn" || cmd == "vent_open" || cmd == "vent_close" ||
           cmd == "glovebox" || cmd == "trunk" ||
           cmd.startsWith("seat_") || cmd == "sentry_on" || cmd == "sentry_off" ||
           cmd == "climate_keep" || cmd == "climate_off" ||
           cmd == "charge_start" || cmd == "charge_stop" || cmd == "charge_port" ||
           cmd == "pedal_sport" || cmd == "pedal_chill" || cmd == "pedal_std" ||
           cmd == "regen_off" || cmd == "regen_low" || cmd == "regen_std" || cmd == "regen_max" ||
           cmd == "stop_creep" || cmd == "stop_roll" || cmd == "stop_hold") {
    execVehicleCmd(cmd);
    reply.println("{\"ok\":true,\"cmd\":\"" + cmd + "\"}");
  }
  else {
    reply.println("[?] hw0/1/2 | p0-p4 | on | off | ovron/ovroff | injon/injoff | isaon/isaoff | evon/evoff | disable | crystal8/crystal16 | autooff N | filter N | send ID DLC D0..D7 | save | load | stats | status | log | slog | mirror_fold/unfold | lock/unlock | frunk | trunk | glovebox | horn | vent_open/close | seat_XX N | sentry_on/off | climate_keep/off | charge_start/stop/port | pedal_sport/chill/std | regen_off/low/std/max | stop_creep/roll/hold");
  }
}

void handleSerial() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim(); cmd.toLowerCase();
    processCommand(cmd, Serial);
  }
}

void handleBridge() {
  if (BRIDGE_SERIAL.available()) {
    String cmd = BRIDGE_SERIAL.readStringUntil('\n');
    cmd.trim(); cmd.toLowerCase();
    processCommand(cmd, BRIDGE_SERIAL);
  }
}

// ─────────────────────────────────────────────────────────────
//  SETUP
// ─────────────────────────────────────────────────────────────

void setup() {
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);

  delay(1000);
  Serial.begin(115200);
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 3000) {}

  // EEPROM
  EEPROM.begin(EEPROM_SIZE);
  loadConfig();
  startMillis = millis();

  // UART1 → ESP32-C3 (TX=GPIO0, RX=GPIO1, varsayılan pinler)
  BRIDGE_SERIAL.begin(BRIDGE_BAUD);

  Serial.println("\n=== CanFeather RP2040 + ESP32-C3 ===");

  // SPI1 pinlerini tanımla
  SPI1.setRX(PIN_CAN_MISO);
  SPI1.setTX(PIN_CAN_MOSI);
  SPI1.setSCK(PIN_CAN_SCK);
  SPI1.begin();

  applyCrystal();

  Serial.println("[uart] Bridge UART1 hazir @ " + String(BRIDGE_BAUD));
  digitalWrite(PIN_LED, LOW);
  Serial.println("[ok] Hazir.");
  { char buf[100];
    snprintf(buf, sizeof(buf), "[config] hw=%d profile=%d fsd=%d autoOff=%d filter=%d xtal=%d",
             cfg.hw, cfg.speedProfile, (int)cfg.fsdEnabled,
             cfg.autoOffMinutes, cfg.logFilter, cfg.crystal);
    slog(buf);
  }
}

// ─────────────────────────────────────────────────────────────
//  LOOP
// ─────────────────────────────────────────────────────────────

void loop() {
  handleSerial();   // USB Serial (debug)
  handleBridge();   // UART1 → ESP32-C3 (WiFi komutları)

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

  // CAN bus-off auto-recovery (her 2 saniyede bir kontrol)
  static unsigned long lastBusCheck = 0;
  if (cfg.busAutoRecover && (now - lastBusCheck > 2000)) {
    lastBusCheck = now;
    uint8_t eflg = mcp.getErrorFlags();
    if (eflg & 0x20) {  // BUS-OFF state
      slog("[recover] CAN BUS-OFF tespit edildi, MCP2515 reset...");
      mcp.reset();
      applyCrystal();
      errCount++;
    }
  }

  // v2.7 fix: 'if' yerine 'while' — Tesla VehicleBus saniyede ~500 frame
  // uretiyor, MCP2515 RX buffer sadece 2 frame. Eski 'if' frame loss'a yol
  // aciyordu (UART bridge isleminde gecikme oldukca kritik FSD frame'i kayboluyor).
  can_frame frame;
  while (mcp.readMessage(&frame) == MCP2515::ERROR_OK) {
    rxCount++;
    rxSinceLastHist++;
    digitalWrite(PIN_LED, HIGH);
    processFrame(frame);
    digitalWrite(PIN_LED, LOW);
  }
}

