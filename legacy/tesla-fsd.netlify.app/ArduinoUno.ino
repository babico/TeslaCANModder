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
 * CanFeather Arduino Uno / Nano — Tesla FSD CAN Mod (WiFi'siz, lean versiyon)
 * ===========================================================================
 * BU KODU YUKLE → Arduino Uno R3  veya  Arduino Nano (KLASIK 5V, ATmega328P)
 *
 * Arduino IDE board seciminde:
 *   Uno  : Tools > Board > Arduino Uno
 *   Nano : Tools > Board > Arduino Nano
 *          (eski Cin klonlari icin: Processor > "ATmega328P (Old Bootloader)")
 *
 * !!! UYARI — DESTEKLENMEYEN NANO VARYANTLARI !!!
 *   ✗ Arduino Nano 33 BLE       (3.3V, nRF52840 ARM)
 *   ✗ Arduino Nano 33 IoT       (3.3V, SAMD21 ARM)
 *   ✗ Arduino Nano RP2040       (3.3V, RP2040 ARM)
 * Bu kartlar farkli mimaride, bu kod onlarda DERLENMEZ.
 * Sadece klasik 5V ATmega328P tabanli Uno/Nano (orijinal veya CH340 klonu) destekleniyor.
 *
 * Uno ve Nano AYNI cipi (ATmega328P) ve AYNI pin numaralarini kullanir
 * (D10/D11/D12/D13 = SPI), kod arasinda hicbir fark yoktur.
 *
 * Bu, ATmega328P'nin sinirli RAM (2 KB) ve Flash (32 KB) butcesine sigan
 * minimal CanFeather varyantidir. ESP8266/ESP32 versiyonlarinin aksine:
 *
 *   ✓ HW3 / HW4 / Legacy FSD bypass — temel CanFeather mantigi
 *   ✓ ISA Speed Override (gercek hiz offset enjeksiyonu)
 *   ✓ ISA Speed Chime suppress
 *   ✓ Emergency vehicle detection
 *   ✓ EEPROM persistence (boot'ta kalici ayarlar)
 *   ✓ Serial komut interface (runtime ayar degisimi)
 *   ✗ Web arayuzu YOK (WiFi yok)
 *   ✗ Vehicle control komutlari YOK (ayna/kilit/klima vb.)
 *   ✗ OTA YOK
 *   ✗ Log/history grafikleri YOK
 *
 * DONANIM BAĞLANTISI (Arduino Uno R3 / Nano + MCP2515):
 *   MCP2515         Uno / Nano
 *   ─────────       ───────────
 *   VCC      -->    5V
 *   GND      -->    GND
 *   CS       -->    D10  (SS)
 *   MOSI     -->    D11  (sabit)
 *   MISO     -->    D12  (sabit)
 *   SCK      -->    D13  (sabit)
 *   INT      -->    D2   (kullanilmiyor, polling)
 *
 * Uno ve Nano'da pin numaralari ve isimleri AYNIDIR. Tek fark form factor:
 * Uno tam boy, Nano breadboard dostu kucuk. Aynı kodu, ayni baglantilari
 * kullanin.
 *
 *   Tesla X179 konnektor:
 *   MCP2515 CAN-H  -->  X179 Pin 13
 *   MCP2515 CAN-L  -->  X179 Pin 14
 *
 * KULLANIM:
 *   1. Asagidaki #define'lardan default ayarlari yap (HW, profile, vb.)
 *   2. Kodu yukle
 *   3. Serial Monitor ac (9600 baud) — durum ve komut listesi gosterilir
 *   4. Calismaya basladiginda her sey otomatik (Serial Monitor'u kapatabilirsin)
 *
 * RUNTIME SERIAL KOMUTLARI:
 *   ?           Yardim — komut listesi
 *   s           Status — anlik durum (HW, profile, FSD, RX/TX/ERR)
 *   hw 0|1|2    HW seciminin degistir (0=Legacy 1=HW3 2=HW4)
 *   p 0..4      Speed profile (0=Chill .. 4=Sloth)
 *   fsd on|off  FSD enjeksiyonu acık/kapali
 *   inj on|off  Tum CAN enjeksiyonu acık/kapali (FSD dahil)
 *   isa on|off  ISA speed chime suppress
 *   ev  on|off  Emergency vehicle detection
 *   tlssc on|off  Bypass "Trafik Isigi" gerekligi
 *   ovr on|off  Profile override (CAN'dan auto-mapping yerine sabit profile)
 *   isaovr on|off ISA speed override (gercek hizi nav hiz limitine ezdirme)
 *   isamul N    ISA hiz carpan (1-15, default 7)
 *   save        Mevcut ayarlari EEPROM'a yaz
 *   load        EEPROM'dan ayarlari geri yukle
 *   reset       EEPROM'u temizle (varsayilan ayarlara don)
 *
 * GEREKLİ KÜTÜPHANE:
 *   - mcp2515 by autowp  (Library Manager > "mcp2515")
 *   - SPI                (built-in)
 *   - EEPROM             (built-in)
 *
 * MCP2515 NOTLARI:
 *   - Modul uzerindeki J1 jumper'i (120Ω terminasyon) CIKARIN — Tesla bus
 *     zaten kendi terminasyonuna sahip, ikincisi error'a yol acar
 *   - 8 MHz vs 16 MHz kristal: asagida CAN_CLOCK_MHZ ile sec
 */

#include <SPI.h>
#include <mcp2515.h>          // autowp/arduino-mcp2515
#include <EEPROM.h>           // Kalici ayar kaydi

// ─────────────────────────────────────────────────────────────
//  AYARLAR — Compile-time defaults
// ─────────────────────────────────────────────────────────────
//
// Bu degerler EEPROM'da gecerli kayit yoksa kullanilir. EEPROM'a
// "save" komutu ile yazip kalici hale getirebilirsin.

#define FW_VERSION       "v2.9-avr"   /* Uno + Nano (ATmega328P) ortak */
#define FW_DATE          "08.04.2026"

// HW: 0=Legacy(HW1/HW2)  1=HW3  2=HW4
#define DEFAULT_HW             2

// Speed profile: 0=Chill 1=Normal 2=Hurry 3=Max(HW4 only) 4=Sloth(HW4 only)
#define DEFAULT_PROFILE        2

// FSD enjeksiyonu acik mi?
#define DEFAULT_FSD_ENABLED    true

// Tum CAN enjeksiyonu acik mi? (false = sadece dinleme, hicbir frame yazilmaz)
#define DEFAULT_CAN_INJECTION  true

// ISA speed chime suppress (HW3/HW4 — 0x399 frame'i bastir)
#define DEFAULT_ISA_SUPPRESS   false

// Emergency vehicle detection (HW3/HW4 — bit59 set)
#define DEFAULT_EMERG_DETECT   true

// Bypass "Trafik Isigi ve Stop Sign Control" (FSD'yi UI ayarsiz aktif et)
#define DEFAULT_BYPASS_TLSSC   false

// Profile override: true ise CAN'dan gelen takip mesafesini yoksay,
// sadece sabit DEFAULT_PROFILE kullan
#define DEFAULT_PROFILE_OVR    false

// ISA speed override: true ise gercek hizi nav hiz limitine ezdirme
#define DEFAULT_ISA_SPEED_OVR  true

// ISA speed multiplier (1-15, default 7) — gercek hiz offset carpan
#define DEFAULT_ISA_MUL        7

// MCP2515 kristal (8 veya 16) — modul uzerindeki kristale gore sec
#define CAN_CLOCK_MHZ          16

// MCP2515 CS pin
#define PIN_CS                 10

// EEPROM ayarlari
#define EEPROM_MAGIC           0xCF
#define EEPROM_ADDR_MAGIC      0
#define EEPROM_ADDR_HW         1
#define EEPROM_ADDR_PROFILE    2
#define EEPROM_ADDR_FSD        3
#define EEPROM_ADDR_INJ        4
#define EEPROM_ADDR_ISA        5
#define EEPROM_ADDR_EV         6
#define EEPROM_ADDR_BTLSSC     7
#define EEPROM_ADDR_OVR        8
#define EEPROM_ADDR_ISAOVR     9
#define EEPROM_ADDR_ISAMUL     10

// ─────────────────────────────────────────────────────────────
//  ÇALIŞMA ZAMANI KONFİGÜRASYONU
// ─────────────────────────────────────────────────────────────

struct Config {
  uint8_t hw;
  uint8_t speedProfile;
  bool    fsdEnabled;
  bool    canInjection;
  bool    fsdSelectedInUI;  // CAN'dan okunan, runtime
  bool    isaSuppress;
  bool    emergencyDetect;
  bool    bypassTLSSC;
  bool    profileOverride;
  bool    isaSpeedOverride;
  uint8_t isaSpeedMul;
};

Config cfg;

// ─────────────────────────────────────────────────────────────
//  DURUM
// ─────────────────────────────────────────────────────────────

MCP2515 mcp(PIN_CS);

static uint8_t  lastFollowDist = 0;
static int      lastSpeedOff   = 0;
static uint32_t rxCount = 0;
static uint32_t txCount = 0;
static uint32_t errCount = 0;

// Serial komut buffer'i (RAM tasarrufu icin kucuk)
#define CMD_BUF_SIZE 32
static char cmdBuf[CMD_BUF_SIZE];
static uint8_t cmdLen = 0;

// ─────────────────────────────────────────────────────────────
//  EEPROM
// ─────────────────────────────────────────────────────────────

void loadConfigDefaults() {
  cfg.hw              = DEFAULT_HW;
  cfg.speedProfile    = DEFAULT_PROFILE;
  cfg.fsdEnabled      = DEFAULT_FSD_ENABLED;
  cfg.canInjection    = DEFAULT_CAN_INJECTION;
  cfg.fsdSelectedInUI = false;
  cfg.isaSuppress     = DEFAULT_ISA_SUPPRESS;
  cfg.emergencyDetect = DEFAULT_EMERG_DETECT;
  cfg.bypassTLSSC     = DEFAULT_BYPASS_TLSSC;
  cfg.profileOverride = DEFAULT_PROFILE_OVR;
  cfg.isaSpeedOverride = DEFAULT_ISA_SPEED_OVR;
  cfg.isaSpeedMul     = DEFAULT_ISA_MUL;
}

void saveConfig() {
  EEPROM.update(EEPROM_ADDR_MAGIC,   EEPROM_MAGIC);
  EEPROM.update(EEPROM_ADDR_HW,      cfg.hw);
  EEPROM.update(EEPROM_ADDR_PROFILE, cfg.speedProfile);
  EEPROM.update(EEPROM_ADDR_FSD,     cfg.fsdEnabled ? 1 : 0);
  EEPROM.update(EEPROM_ADDR_INJ,     cfg.canInjection ? 1 : 0);
  EEPROM.update(EEPROM_ADDR_ISA,     cfg.isaSuppress ? 1 : 0);
  EEPROM.update(EEPROM_ADDR_EV,      cfg.emergencyDetect ? 1 : 0);
  EEPROM.update(EEPROM_ADDR_BTLSSC,  cfg.bypassTLSSC ? 1 : 0);
  EEPROM.update(EEPROM_ADDR_OVR,     cfg.profileOverride ? 1 : 0);
  EEPROM.update(EEPROM_ADDR_ISAOVR,  cfg.isaSpeedOverride ? 1 : 0);
  EEPROM.update(EEPROM_ADDR_ISAMUL,  cfg.isaSpeedMul);
  Serial.println(F("[ee] kaydedildi"));
}

void loadConfig() {
  if (EEPROM.read(EEPROM_ADDR_MAGIC) != EEPROM_MAGIC) {
    Serial.println(F("[ee] kayit yok, default kullaniliyor"));
    loadConfigDefaults();
    return;
  }
  cfg.hw              = EEPROM.read(EEPROM_ADDR_HW);
  cfg.speedProfile    = EEPROM.read(EEPROM_ADDR_PROFILE);
  cfg.fsdEnabled      = EEPROM.read(EEPROM_ADDR_FSD) == 1;
  cfg.canInjection    = EEPROM.read(EEPROM_ADDR_INJ) == 1;
  cfg.fsdSelectedInUI = false;
  cfg.isaSuppress     = EEPROM.read(EEPROM_ADDR_ISA) == 1;
  cfg.emergencyDetect = EEPROM.read(EEPROM_ADDR_EV) == 1;
  cfg.bypassTLSSC     = EEPROM.read(EEPROM_ADDR_BTLSSC) == 1;
  cfg.profileOverride = EEPROM.read(EEPROM_ADDR_OVR) == 1;
  cfg.isaSpeedOverride = EEPROM.read(EEPROM_ADDR_ISAOVR) == 1;
  cfg.isaSpeedMul     = EEPROM.read(EEPROM_ADDR_ISAMUL);
  // Sanity
  if (cfg.hw > 2) cfg.hw = DEFAULT_HW;
  if (cfg.speedProfile > 4) cfg.speedProfile = DEFAULT_PROFILE;
  if (cfg.isaSpeedMul < 1 || cfg.isaSpeedMul > 15) cfg.isaSpeedMul = DEFAULT_ISA_MUL;
  Serial.println(F("[ee] yuklendi"));
}

void resetConfig() {
  EEPROM.update(EEPROM_ADDR_MAGIC, 0xFF);
  loadConfigDefaults();
  Serial.println(F("[ee] reset, default ayarlar yuklendi"));
}

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

inline void canSend(can_frame& f) {
  if (mcp.sendMessage(&f) == MCP2515::ERROR_OK) txCount++;
  else errCount++;
}

// MCP2515 hardware filter — sadece bizim islememiz gereken CAN ID'ler
// CPU'ya gelir. Tesla VehicleBus ~500 fps yayinliyor, bizim ihtiyacimiz
// olan 3 ID ~210 fps. Filter ile %50+ azalma. Uno'nun 16 MHz CPU'su icin
// kritik kazanc. Vehicle control yok, dolayisiyla sade.
// HW degisirse runtime'da yeniden cagrilir (execCommand "hw" sonrasi).
void applyCanFilters() {
  if (cfg.hw == 0) {
    // Legacy (HW1/HW2): 0x3EE (1006) ve 0x45 (69)
    mcp.setFilterMask(MCP2515::MASK0, false, 0x7FF);
    mcp.setFilter(MCP2515::RXF0, false, 1006);
    mcp.setFilter(MCP2515::RXF1, false, 69);
    mcp.setFilterMask(MCP2515::MASK1, false, 0x7FF);
    mcp.setFilter(MCP2515::RXF2, false, 1006);
    mcp.setFilter(MCP2515::RXF3, false, 1006);
    mcp.setFilter(MCP2515::RXF4, false, 1006);
    mcp.setFilter(MCP2515::RXF5, false, 1006);
  } else {
    // HW3/HW4: 0x3FD (1021), 0x3F8 (1016), 0x399 (921)
    mcp.setFilterMask(MCP2515::MASK0, false, 0x7FF);
    mcp.setFilter(MCP2515::RXF0, false, 1021);
    mcp.setFilter(MCP2515::RXF1, false, 921);
    mcp.setFilterMask(MCP2515::MASK1, false, 0x7FF);
    mcp.setFilter(MCP2515::RXF2, false, 1016);
    mcp.setFilter(MCP2515::RXF3, false, 1016);
    mcp.setFilter(MCP2515::RXF4, false, 1016);
    mcp.setFilter(MCP2515::RXF5, false, 1016);
  }
}

void applyCanClock() {
  mcp.reset();
  CAN_CLOCK clk = (CAN_CLOCK_MHZ == 8) ? MCP_8MHZ : MCP_16MHZ;
  if (mcp.setBitrate(CAN_500KBPS, clk) != MCP2515::ERROR_OK) {
    Serial.println(F("[can] HATA: setBitrate basarisiz"));
    errCount++;
  }
  applyCanFilters();
  mcp.setNormalMode();
}

// ─────────────────────────────────────────────────────────────
//  LEGACY HANDLER (HW1/HW2)
// ─────────────────────────────────────────────────────────────

void handleLegacy(can_frame& f) {
  if (f.can_id == 69) {
    uint8_t pos = f.data[1] >> 5;
    if (!cfg.profileOverride) {
      if      (pos <= 1) cfg.speedProfile = 2;
      else if (pos == 2) cfg.speedProfile = 1;
      else               cfg.speedProfile = 0;
    }
    return;
  }
  if (f.can_id != 1006) return;
  uint8_t idx = muxID(f);
  if (idx == 0) cfg.fsdSelectedInUI = fsdInUI(f);
  if (idx == 0 && cfg.fsdEnabled && (cfg.bypassTLSSC || cfg.fsdSelectedInUI)) {
    setBit(f, 46, true);
    setSpeedV12V13(f, cfg.speedProfile);
    canSend(f);
  }
  if (idx == 1) { setBit(f, 19, false); canSend(f); }
}

// ─────────────────────────────────────────────────────────────
//  HW3 HANDLER
// ─────────────────────────────────────────────────────────────

void handleHW3(can_frame& f) {
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

  // 1016: takip mesafesi — sadece status icin oku
  // HW3 fd->profile auto-mapping kaldirildi (eski switch kirikti).
  // HW3'te profil sadece manuel secilir.
  if (f.can_id == 1016) {
    lastFollowDist = followDist(f);
    return;
  }

  if (f.can_id != 1021) return;
  uint8_t idx = muxID(f);

  if (idx == 0) {
    cfg.fsdSelectedInUI = fsdInUI(f);
    int off = (int)((f.data[3] >> 1) & 0x3F) - 30;
    int v = off * cfg.isaSpeedMul;
    if (v < 0) v = 0;
    if (v > 200) v = 200;
    lastSpeedOff = v;
  }

  bool fsdOn = cfg.fsdEnabled && (cfg.bypassTLSSC || cfg.fsdSelectedInUI);

  if (idx == 0 && fsdOn) {
    setBit(f, 46, true);
    setBit(f, 60, true);
    if (cfg.emergencyDetect) setBit(f, 59, true);
    canSend(f);
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
      case 1: p = 3; break;
      case 2: p = 2; break;
      case 3: p = 1; break;
      case 4: p = 0; break;
      case 5: p = 4; break;
    }
    if (!cfg.profileOverride) cfg.speedProfile = p;
    return;
  }
  if (f.can_id != 1021) return;

  uint8_t idx = muxID(f);
  if (idx == 0) {
    cfg.fsdSelectedInUI = fsdInUI(f);
    int off = (int)((f.data[3] >> 1) & 0x3F) - 30;
    int v = off * cfg.isaSpeedMul;
    if (v < 0) v = 0;
    if (v > 200) v = 200;
    lastSpeedOff = v;
  }
  bool fsdOn = cfg.fsdEnabled && (cfg.bypassTLSSC || cfg.fsdSelectedInUI);

  if (idx == 0 && fsdOn) {
    setBit(f, 46, true);
    setBit(f, 60, true);
    if (cfg.emergencyDetect) setBit(f, 59, true);
    canSend(f);
  }
  if (idx == 1) {
    setBit(f, 19, false);
    setBit(f, 47, true);
    if (cfg.isaSpeedOverride) f.data[2] &= ~0x08;
    canSend(f);
  }
  if (idx == 2) {
    f.data[7] = (f.data[7] & ~0x70) | ((cfg.speedProfile & 0x07) << 4);
    if (cfg.isaSpeedOverride && fsdOn) {
      f.data[0] = (f.data[0] & ~0xC0) | ((lastSpeedOff & 0x03) << 6);
      f.data[1] = (f.data[1] & ~0x3F) | (lastSpeedOff >> 2);
    }
    canSend(f);
  }
}

// ─────────────────────────────────────────────────────────────
//  ANA FRAME YÖNLENDİRİCİ
// ─────────────────────────────────────────────────────────────

void processFrame(can_frame& f) {
  if (!cfg.canInjection || !cfg.fsdEnabled) return;
  switch (cfg.hw) {
    case 0: handleLegacy(f); break;
    case 1: handleHW3(f);    break;
    case 2: handleHW4(f);    break;
  }
}

// ─────────────────────────────────────────────────────────────
//  SERIAL KOMUT INTERFACE
// ─────────────────────────────────────────────────────────────

void printHelp() {
  Serial.println(F("=== CanFeather Uno/Nano komutlari ==="));
  Serial.println(F("?              Yardim"));
  Serial.println(F("s              Status"));
  Serial.println(F("hw 0|1|2       0=Legacy 1=HW3 2=HW4"));
  Serial.println(F("p 0..4         Speed profile"));
  Serial.println(F("fsd on|off     FSD enjeksiyonu"));
  Serial.println(F("inj on|off     Tum CAN enjeksiyonu"));
  Serial.println(F("isa on|off     ISA chime suppress"));
  Serial.println(F("ev  on|off     Emergency vehicle"));
  Serial.println(F("tlssc on|off   Bypass Trafik Isigi"));
  Serial.println(F("ovr on|off     Profile override"));
  Serial.println(F("isaovr on|off  ISA speed override"));
  Serial.println(F("isamul N       ISA carpan (1-15)"));
  Serial.println(F("save           EEPROM'a yaz"));
  Serial.println(F("load           EEPROM'dan oku"));
  Serial.println(F("reset          Default ayarlar"));
}

void printStatus() {
  Serial.print(F("FW="));   Serial.print(F(FW_VERSION));
  Serial.print(F(" HW="));  Serial.print(cfg.hw);
  Serial.print(F(" P="));   Serial.print(cfg.speedProfile);
  Serial.print(F(" FSD=")); Serial.print(cfg.fsdEnabled ? F("ON") : F("OFF"));
  Serial.print(F(" INJ=")); Serial.print(cfg.canInjection ? F("ON") : F("OFF"));
  Serial.print(F(" UI="));  Serial.print(cfg.fsdSelectedInUI ? F("ON") : F("OFF"));
  Serial.print(F(" FD="));  Serial.print(lastFollowDist);
  Serial.print(F(" OFF=")); Serial.print(lastSpeedOff);
  Serial.print(F(" RX="));  Serial.print(rxCount);
  Serial.print(F(" TX="));  Serial.print(txCount);
  Serial.print(F(" ERR=")); Serial.println(errCount);
}

// "x on" / "x off" parser. Returns 1=on, 0=off, -1=invalid
int8_t parseOnOff(const char* s) {
  if (strcmp(s, "on") == 0)  return 1;
  if (strcmp(s, "off") == 0) return 0;
  return -1;
}

void execCommand(char* line) {
  // Tokenize: ilk kelime cmd, sonraki opsiyonel arg
  char* cmd = strtok(line, " ");
  if (!cmd) return;
  char* arg = strtok(NULL, " ");

  if (strcmp(cmd, "?") == 0)        { printHelp(); return; }
  if (strcmp(cmd, "s") == 0)        { printStatus(); return; }
  if (strcmp(cmd, "save") == 0)     { saveConfig(); return; }
  if (strcmp(cmd, "load") == 0)     { loadConfig(); applyCanClock(); printStatus(); return; }
  if (strcmp(cmd, "reset") == 0)    { resetConfig(); applyCanClock(); printStatus(); return; }

  if (!arg) { Serial.println(F("? arg eksik")); return; }

  if (strcmp(cmd, "hw") == 0) {
    int v = atoi(arg);
    if (v >= 0 && v <= 2) {
      cfg.hw = v;
      // HW degistiginde Legacy ile HW3/HW4 farkli CAN ID'ler kullaniyor,
      // hardware filter'i yeniden ayarlamak gerek (mcp.setFilter normal
      // mode disinda yapilmali → reset + filter + normal mode).
      mcp.reset();
      CAN_CLOCK clk = (CAN_CLOCK_MHZ == 8) ? MCP_8MHZ : MCP_16MHZ;
      mcp.setBitrate(CAN_500KBPS, clk);
      applyCanFilters();
      mcp.setNormalMode();
      Serial.print(F("hw=")); Serial.println(v);
    }
    else Serial.println(F("? 0..2"));
    return;
  }
  if (strcmp(cmd, "p") == 0) {
    int v = atoi(arg);
    if (v >= 0 && v <= 4) { cfg.speedProfile = v; Serial.print(F("p=")); Serial.println(v); }
    else Serial.println(F("? 0..4"));
    return;
  }
  if (strcmp(cmd, "isamul") == 0) {
    int v = atoi(arg);
    if (v >= 1 && v <= 15) { cfg.isaSpeedMul = v; Serial.print(F("isamul=")); Serial.println(v); }
    else Serial.println(F("? 1..15"));
    return;
  }

  // Boolean komutlar
  int8_t b = parseOnOff(arg);
  if (b < 0) { Serial.println(F("? on|off")); return; }

  if      (strcmp(cmd, "fsd")    == 0) cfg.fsdEnabled       = b;
  else if (strcmp(cmd, "inj")    == 0) cfg.canInjection     = b;
  else if (strcmp(cmd, "isa")    == 0) cfg.isaSuppress      = b;
  else if (strcmp(cmd, "ev")     == 0) cfg.emergencyDetect  = b;
  else if (strcmp(cmd, "tlssc")  == 0) cfg.bypassTLSSC      = b;
  else if (strcmp(cmd, "ovr")    == 0) cfg.profileOverride  = b;
  else if (strcmp(cmd, "isaovr") == 0) cfg.isaSpeedOverride = b;
  else { Serial.println(F("? bilinmeyen komut, ? yaz")); return; }

  Serial.print(cmd); Serial.print('=');
  Serial.println(b ? F("ON") : F("OFF"));
}

void pollSerial() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      cmdBuf[cmdLen] = '\0';
      if (cmdLen > 0) execCommand(cmdBuf);
      cmdLen = 0;
      continue;
    }
    if (cmdLen < CMD_BUF_SIZE - 1) {
      cmdBuf[cmdLen++] = c;
    }
  }
}

// ─────────────────────────────────────────────────────────────
//  SETUP
// ─────────────────────────────────────────────────────────────

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.print(F("=== CanFeather Uno/Nano "));
  Serial.print(F(FW_VERSION));
  Serial.println(F(" ==="));

  loadConfig();

  SPI.begin();
  applyCanClock();

  printStatus();
  Serial.println(F("Komut icin '?' yaz"));
}

// ─────────────────────────────────────────────────────────────
//  LOOP
// ─────────────────────────────────────────────────────────────

void loop() {
  // CAN frame'lerini OKU (kuyrugu bosalt)
  // 'while' kullaniyoruz cunku Tesla VehicleBus saniyede ~500 frame
  // uretiyor ve MCP2515 RX FIFO sadece 2 frame.
  can_frame frame;
  while (mcp.readMessage(&frame) == MCP2515::ERROR_OK) {
    rxCount++;
    processFrame(frame);
  }

  // Serial komut polling (nadiren bir karakter olur)
  pollSerial();
}
