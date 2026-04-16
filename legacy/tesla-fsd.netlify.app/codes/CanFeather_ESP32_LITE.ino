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
 * CanFeather ESP32 LITE — Tesla FSD CAN Mod (BASE / WiFi'siz)
 * ============================================================
 * BU KODU YUKLE → ESP32 DevKit V1 (ESP-WROOM-32)
 * Arduino IDE: Tools > Board > "ESP32 Dev Module"
 *
 * Bu, ESP32_WiFi.ino'nun "lite" / "base" varyantidir. Tum WiFi ve
 * web arayuzu kaldirildi. Sadece dogrudan calisan FSD bypass mantigi var.
 * Tum ayarlar kodun en ustunde sabit (#define), runtime degisim yok.
 *
 * NEDEN LITE? Bazi araclarda WiFi acikken FSD enable bit'i sik kayboluyor
 * (HTTP request handling MCP2515 buffer'ini tasiriyor). Bu lite varyant
 * WiFi stack'i hic baslatmadigi icin hicbir interrupt baskisi yok, frame
 * loss riski sifirlanir. Aracinizda WiFi'li versiyonda "FSD bir geliyor
 * bir gidiyor" yasiyorsaniz, bu surumu deneyin.
 *
 * NELER VAR?
 *   ✓ HW3 / HW4 / Legacy FSD bypass (compile-time secim)
 *   ✓ FSD enjeksiyonu (mux 0/1/2 tam handler)
 *   ✓ Bypass TLSSC (UI ayarsiz FSD aktif)
 *   ✓ ISA Speed Override (gercek hizi nav hiz limitine ezdirme)
 *   ✓ Bus-off auto-recovery
 *   ✓ Status LED (GPIO2 dahili LED)
 *   ✓ Serial debug print (115200 baud, opsiyonel)
 *
 * NELER YOK?
 *   ✗ WiFi / web arayuzu
 *   ✗ EEPROM persistence
 *   ✗ OTA guncelleme
 *   ✗ Vehicle control komutlari (ayna/kilit/klima/sentry vb.)
 *   ✗ Log buffer / FSD history grafikleri
 *   ✗ Runtime ayar degisimi
 *
 * AYAR DEGISTIRMEK ICIN: Asagidaki #define'lardan istediginizi degistirin,
 * derleyin, yukleyin. Tek seferlik degisim icin yorum satirlarini takip edin.
 *
 * DONANIM BAGLANTISI (ESP32 DevKit V1 + MCP2515):
 *   MCP2515         ESP32 DevKit V1
 *   ─────────       ───────────────
 *   VCC      -->    VIN (5V) veya 5V pin
 *   GND      -->    GND
 *   CS       -->    GPIO5
 *   SCK      -->    GPIO18
 *   MISO     -->    GPIO19
 *   MOSI     -->    GPIO23
 *   INT      -->    (kullanilmiyor)
 *
 *   Tesla X179 konnektor:
 *   MCP2515 CAN-H  -->  X179 Pin 13
 *   MCP2515 CAN-L  -->  X179 Pin 14
 *
 * GEREKLI KUTUPHANE:
 *   - mcp2515 by autowp  (Library Manager > "mcp2515")
 *
 * MCP2515 NOTLARI:
 *   - Modul uzerindeki J1 jumper'ini (120Ω terminasyon) CIKARIN
 *   - Modulun kristal frekansini (8 MHz veya 16 MHz) asagidan ayarlayin
 */

#include <SPI.h>
#include <mcp2515.h>          // autowp/arduino-mcp2515
#include <memory>

#define FW_VERSION       "v2.9-lite"
#define FW_DATE          "08.04.2026"

// ═════════════════════════════════════════════════════════════
//                          AYARLAR
//          Burayi degistirip yeniden yukleyebilirsiniz
// ═════════════════════════════════════════════════════════════

// ─── DONANIM SECIMI ─────────────────────────────────────────
// 0 = Legacy (HW1/HW2 — Model S/X 2016-2019)
// 1 = HW3
// 2 = HW4
#define HW_VERSION             1   // <-- HW3 (degistirmek icin: 0, 1 veya 2)

// ─── MCP2515 KRISTAL FREKANSI ───────────────────────────────
// Modul uzerindeki kristale gore sec. Yanlis ayar = bus okuyamaz.
// MCP_8MHZ veya MCP_16MHZ
#define CAN_CRYSTAL            MCP_8MHZ   // <-- 8 MHz (16 MHz icin: MCP_16MHZ)

// ─── HIZ PROFILI ────────────────────────────────────────────
// 0 = Chill   1 = Normal   2 = Hurry   3 = Max (HW4)   4 = Sloth (HW4)
#define SPEED_PROFILE          2   // <-- Hurry

// ─── FSD ENJEKSIYON KONTROLU ────────────────────────────────
#define FSD_ENABLED            true   // FSD bit enjeksiyonu aktif
#define CAN_INJECTION          true   // Tum CAN enjeksiyonu aktif (master switch)

// ─── BYPASS TLSSC ───────────────────────────────────────────
// "Trafik Isigi ve Stop Sign Control" UI ayarini gerektirmeden FSD'yi aktif eder
#define BYPASS_TLSSC           true

// ─── BUS-OFF AUTO-RECOVERY ──────────────────────────────────
// CAN bus error sayaci taskiniginda MCP2515'i otomatik resetler
#define BUS_AUTO_RECOVER       true

// ─── ISA SPEED OVERRIDE ─────────────────────────────────────
// Gercek hiz offset'ini nav hiz limiti yerine zorla — hiz limitine
// otomatik dusmesini onler. ISA_SPEED_MUL = 1..15 (default 7)
#define ISA_SPEED_OVERRIDE     true
#define ISA_SPEED_MUL          7

// ─── ISA SPEED CHIME SUPPRESS ───────────────────────────────
// Hiz limit asim sesini bastir (HW3/HW4 — 0x399 frame)
// Default kapali; istersen true yap
#define ISA_SUPPRESS           false

// ─── EMERGENCY VEHICLE DETECTION ────────────────────────────
// Acil arac algilama bit'i (HW3/HW4 — bit59)
// Default kapali; istersen true yap
#define EMERGENCY_DETECT       false

// ─── PROFILE OVERRIDE ───────────────────────────────────────
// true ise CAN'dan gelen takip mesafesi auto-mapping'ini yoksay,
// yukaridaki SPEED_PROFILE sabit kullanilsin
#define PROFILE_OVERRIDE       false

// ─── SERIAL DEBUG ───────────────────────────────────────────
// Serial Monitor'a periyodik durum yazsin mi? Production'da kapatabilirsin.
#define SERIAL_DEBUG           true
#define DEBUG_INTERVAL_MS      5000   // 5 saniyede bir status yaz

// ═════════════════════════════════════════════════════════════
//                  PIN TANIMLARI (ESP32 DevKit V1)
// ═════════════════════════════════════════════════════════════

#define PIN_LED        2     // Dahili LED (HIGH = acik)
#define PIN_CS         5     // SPI Chip Select
#define PIN_SCK        18    // SPI Clock
#define PIN_MISO       19    // SPI MISO
#define PIN_MOSI       23    // SPI MOSI

// ═════════════════════════════════════════════════════════════
//                       DURUM (RUNTIME)
// ═════════════════════════════════════════════════════════════

// Runtime'da degisen kucuk durumlar — config DEGIL, sadece izleme.
static bool      g_fsdSelectedInUI = false;
static uint8_t   g_lastFollowDist  = 0;
static int       g_lastSpeedOff    = 0;
static uint32_t  g_rxCount = 0;
static uint32_t  g_txCount = 0;
static uint32_t  g_errCount = 0;

std::unique_ptr<MCP2515> mcp;

// ═════════════════════════════════════════════════════════════
//                    CAN YARDIMCI FONKSIYONLAR
// ═════════════════════════════════════════════════════════════

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
  if (mcp->sendMessage(&f) == MCP2515::ERROR_OK) g_txCount++;
  else g_errCount++;
}

// MCP2515 hardware filter setup — sadece bizim islememiz gereken CAN ID'ler
// CPU'ya gelir. Tesla VehicleBus ~500 fps yayinliyor, bizim ihtiyacimiz olan
// 3 ID toplam ~210 fps. Filter ile %50+ azalma.
// LITE'ta vehicle control yok, dolayisiyla sade ve risksiz.
void applyCanFilters() {
#if HW_VERSION == 0
  // Legacy (HW1/HW2): 0x3EE (1006) ve 0x45 (69)
  mcp->setFilterMask(MCP2515::MASK0, false, 0x7FF);
  mcp->setFilter(MCP2515::RXF0, false, 1006);
  mcp->setFilter(MCP2515::RXF1, false, 69);
  mcp->setFilterMask(MCP2515::MASK1, false, 0x7FF);
  mcp->setFilter(MCP2515::RXF2, false, 1006);
  mcp->setFilter(MCP2515::RXF3, false, 1006);
  mcp->setFilter(MCP2515::RXF4, false, 1006);
  mcp->setFilter(MCP2515::RXF5, false, 1006);
#else
  // HW3/HW4: 0x3FD (1021), 0x3F8 (1016), 0x399 (921)
  mcp->setFilterMask(MCP2515::MASK0, false, 0x7FF);
  mcp->setFilter(MCP2515::RXF0, false, 1021);  // 0x3FD UI_autopilotControl
  mcp->setFilter(MCP2515::RXF1, false, 921);   // 0x399 UI_status (ISA chime)
  mcp->setFilterMask(MCP2515::MASK1, false, 0x7FF);
  mcp->setFilter(MCP2515::RXF2, false, 1016);  // 0x3F8 UI_dasControl
  mcp->setFilter(MCP2515::RXF3, false, 1016);  // bos slotlar duplicate
  mcp->setFilter(MCP2515::RXF4, false, 1016);
  mcp->setFilter(MCP2515::RXF5, false, 1016);
#endif
}

void applyCanClock() {
  mcp->reset();
  if (mcp->setBitrate(CAN_500KBPS, CAN_CRYSTAL) != MCP2515::ERROR_OK) {
    Serial.println(F("[can] HATA: setBitrate basarisiz"));
    g_errCount++;
  }
  applyCanFilters();
  mcp->setNormalMode();
}

// ═════════════════════════════════════════════════════════════
//                       LEGACY HANDLER (HW1/HW2)
// ═════════════════════════════════════════════════════════════

void handleLegacy(can_frame& f) {
  if (f.can_id == 69) {
    // Legacy follow distance: profile auto-mapping (yalniz override kapaliysa)
    // Lite'ta bunu zaten sabit tutmak istiyoruz; PROFILE_OVERRIDE ile koruma
    return;
  }
  if (f.can_id != 1006) return;
  uint8_t idx = muxID(f);
  if (idx == 0) g_fsdSelectedInUI = fsdInUI(f);
  bool fsdOn = FSD_ENABLED && (BYPASS_TLSSC || g_fsdSelectedInUI);
  if (idx == 0 && fsdOn) {
    setBit(f, 46, true);
    setSpeedV12V13(f, SPEED_PROFILE);
    canSend(f);
  }
  if (idx == 1) { setBit(f, 19, false); canSend(f); }
}

// ═════════════════════════════════════════════════════════════
//                          HW3 HANDLER
// ═════════════════════════════════════════════════════════════

void handleHW3(can_frame& f) {
  // ISA Speed Chime Suppress (CAN ID 921)
  if (ISA_SUPPRESS && f.can_id == 921) {
    f.data[1] |= 0x20;
    uint8_t sum = 0;
    for (int i = 0; i < 7; i++) sum += f.data[i];
    sum += (921 & 0xFF) + (921 >> 8);
    f.data[7] = sum & 0xFF;
    canSend(f);
    return;
  }

  // 1016: takip mesafesi — sadece status icin oku, profile dokunma
  if (f.can_id == 1016) {
    g_lastFollowDist = followDist(f);
    return;
  }

  if (f.can_id != 1021) return;
  uint8_t idx = muxID(f);

  if (idx == 0) {
    g_fsdSelectedInUI = fsdInUI(f);
    int off = (int)((f.data[3] >> 1) & 0x3F) - 30;
    int v = off * ISA_SPEED_MUL;
    if (v < 0) v = 0;
    if (v > 200) v = 200;
    g_lastSpeedOff = v;
  }

  bool fsdOn = FSD_ENABLED && (BYPASS_TLSSC || g_fsdSelectedInUI);

  if (idx == 0 && fsdOn) {
    setBit(f, 46, true);
    setBit(f, 60, true);
    if (EMERGENCY_DETECT) setBit(f, 59, true);
    canSend(f);
  }
  if (idx == 1) {
    setBit(f, 19, false);
    setBit(f, 47, true);
    if (ISA_SPEED_OVERRIDE) f.data[2] &= ~0x08;
    canSend(f);
  }
  if (idx == 2 && fsdOn && ISA_SPEED_OVERRIDE) {
    f.data[7] = (f.data[7] & ~0x70) | ((SPEED_PROFILE & 0x07) << 4);
    f.data[0] = (f.data[0] & ~0xC0) | ((g_lastSpeedOff & 0x03) << 6);
    f.data[1] = (f.data[1] & ~0x3F) | (g_lastSpeedOff >> 2);
    canSend(f);
  }
}

// ═════════════════════════════════════════════════════════════
//                          HW4 HANDLER
// ═════════════════════════════════════════════════════════════

void handleHW4(can_frame& f) {
  // ISA Speed Chime Suppress (CAN ID 921)
  if (ISA_SUPPRESS && f.can_id == 921) {
    f.data[1] |= 0x20;
    uint8_t sum = 0;
    for (int i = 0; i < 7; i++) sum += f.data[i];
    sum += (921 & 0xFF) + (921 >> 8);
    f.data[7] = sum & 0xFF;
    canSend(f);
    return;
  }
  if (f.can_id == 1016) {
    g_lastFollowDist = followDist(f);
    return;
  }
  if (f.can_id != 1021) return;

  uint8_t idx = muxID(f);
  if (idx == 0) {
    g_fsdSelectedInUI = fsdInUI(f);
    int off = (int)((f.data[3] >> 1) & 0x3F) - 30;
    int v = off * ISA_SPEED_MUL;
    if (v < 0) v = 0;
    if (v > 200) v = 200;
    g_lastSpeedOff = v;
  }
  bool fsdOn = FSD_ENABLED && (BYPASS_TLSSC || g_fsdSelectedInUI);

  if (idx == 0 && fsdOn) {
    setBit(f, 46, true);
    setBit(f, 60, true);
    if (EMERGENCY_DETECT) setBit(f, 59, true);
    canSend(f);
  }
  if (idx == 1) {
    setBit(f, 19, false);
    setBit(f, 47, true);
    if (ISA_SPEED_OVERRIDE) f.data[2] &= ~0x08;
    canSend(f);
  }
  if (idx == 2) {
    f.data[7] = (f.data[7] & ~0x70) | ((SPEED_PROFILE & 0x07) << 4);
    if (ISA_SPEED_OVERRIDE && fsdOn) {
      f.data[0] = (f.data[0] & ~0xC0) | ((g_lastSpeedOff & 0x03) << 6);
      f.data[1] = (f.data[1] & ~0x3F) | (g_lastSpeedOff >> 2);
    }
    canSend(f);
  }
}

// ═════════════════════════════════════════════════════════════
//                    ANA FRAME YONLENDIRICI
// ═════════════════════════════════════════════════════════════

void processFrame(can_frame& f) {
  if (!CAN_INJECTION || !FSD_ENABLED) return;
#if HW_VERSION == 0
  handleLegacy(f);
#elif HW_VERSION == 1
  handleHW3(f);
#elif HW_VERSION == 2
  handleHW4(f);
#else
  #error "HW_VERSION must be 0 (Legacy), 1 (HW3), or 2 (HW4)"
#endif
}

// ═════════════════════════════════════════════════════════════
//                       SERIAL DEBUG PRINT
// ═════════════════════════════════════════════════════════════

#if SERIAL_DEBUG
void printStatus() {
  Serial.print(F("HW="));   Serial.print(HW_VERSION);
  Serial.print(F(" P="));   Serial.print(SPEED_PROFILE);
  Serial.print(F(" UI="));  Serial.print(g_fsdSelectedInUI ? F("ON") : F("OFF"));
  Serial.print(F(" FD="));  Serial.print(g_lastFollowDist);
  Serial.print(F(" OFF=")); Serial.print(g_lastSpeedOff);
  Serial.print(F(" RX="));  Serial.print(g_rxCount);
  Serial.print(F(" TX="));  Serial.print(g_txCount);
  Serial.print(F(" ERR=")); Serial.println(g_errCount);
}
#endif

// ═════════════════════════════════════════════════════════════
//                            SETUP
// ═════════════════════════════════════════════════════════════

void setup() {
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);   // boot sirasinda yansin

  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.print(F("=== CanFeather ESP32 LITE "));
  Serial.print(F(FW_VERSION));
  Serial.println(F(" ==="));
  Serial.print(F("HW=")); Serial.print(HW_VERSION);
  Serial.print(F(" Crystal=")); Serial.println((CAN_CRYSTAL == MCP_8MHZ) ? F("8MHz") : F("16MHz"));

  // SPI + MCP2515
  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);
  mcp = std::make_unique<MCP2515>(PIN_CS);
  applyCanClock();

  digitalWrite(PIN_LED, LOW);
  Serial.println(F("[ok] hazir, FSD bypass aktif"));
}

// ═════════════════════════════════════════════════════════════
//                            LOOP
// ═════════════════════════════════════════════════════════════

void loop() {
  // CAN frame'lerini OKU (kuyrugu bosalt)
  // 'while' kullaniyoruz cunku Tesla VehicleBus saniyede ~500 frame
  // uretiyor ve MCP2515 RX FIFO sadece 2 frame.
  can_frame frame;
  while (mcp && mcp->readMessage(&frame) == MCP2515::ERROR_OK) {
    g_rxCount++;
    digitalWrite(PIN_LED, HIGH);
    processFrame(frame);
    digitalWrite(PIN_LED, LOW);
  }

  // Bus-off auto-recovery (her 2 saniyede bir kontrol)
#if BUS_AUTO_RECOVER
  static unsigned long lastBusCheck = 0;
  unsigned long now = millis();
  if (mcp && (now - lastBusCheck > 2000)) {
    lastBusCheck = now;
    uint8_t eflg = mcp->getErrorFlags();
    if (eflg & 0x20) {  // BUS-OFF state
      Serial.println(F("[recover] CAN BUS-OFF tespit edildi, MCP2515 reset..."));
      mcp->reset();
      applyCanClock();
      g_errCount++;
    }
  }
#endif

  // Periyodik debug print
#if SERIAL_DEBUG
  static unsigned long lastDebug = 0;
  unsigned long now2 = millis();
  if (now2 - lastDebug > DEBUG_INTERVAL_MS) {
    lastDebug = now2;
    printStatus();
  }
#endif
}

