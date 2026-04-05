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
 * ESP32-C3 WiFi Bridge — CanFeather RP2040 için
 * ================================================
 * BU KODU YUKLE → ESP32-C3 Super Mini (Lolin / WeAct)
 * Arduino IDE: Tools > Board > ESP32C3 Dev Module
 * NOT: RP2040'a ayrica CanFeather_RP2040.ino yukleyin!
 *
 * Bu firmware ESP32-C3 Super Mini üzerinde çalışır.
 * RP2040 CAN Feather'a UART ile bağlanır ve WiFi AP +
 * web arayüzü sağlar. Tüm CAN işlemleri RP2040'ta yapılır,
 * ESP32-C3 sadece köprü görevi görür.
 *
 * DONANIM BAĞLANTISI:
 *   ESP32-C3      RP2040 Feather
 *   RX (GPIO20)   TX (GPIO0)
 *   TX (GPIO21)   RX (GPIO1)
 *   3V3            3.3V
 *   GND            GND
 *
 * BOARD SEÇİMİ:
 *   Tools > Board > ESP32C3 Dev Module
 *
 * BOARD MANAGER URL:
 *   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

// ─────────────────────────────────────────────────────────────
//  AYARLAR
// ─────────────────────────────────────────────────────────────

#define WIFI_SSID     "CanFeather"
#define WIFI_PASSWORD "tesla1234"

// ESP32-C3 Super Mini UART pinleri
#define BRIDGE_RX     20   // ← RP2040 TX
#define BRIDGE_TX     21   // → RP2040 RX

#define EEPROM_SIZE    140
#define WIFI_MAGIC     0xAA  // WiFi ozel sifre bayragi
#define WIFI_EE_START  64    // EEPROM'da WiFi verisi baslangic adresi

// ─────────────────────────────────────────────────────────────
//  WiFi ayarlari (EEPROM offset 64'ten baslar)
// ─────────────────────────────────────────────────────────────

char wifiSSID[33] = "CanFeather";
char wifiPASS[33] = "tesla1234";

void saveWiFiConfig() {
  EEPROM.write(WIFI_EE_START, WIFI_MAGIC);
  for (int i = 0; i < 32; i++) EEPROM.write(WIFI_EE_START + 1 + i, wifiSSID[i]);
  for (int i = 0; i < 32; i++) EEPROM.write(WIFI_EE_START + 33 + i, wifiPASS[i]);
  EEPROM.commit();
  Serial.println("[wifi] SSID/sifre EEPROM'a kaydedildi");
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
  Serial.println("[wifi] Fabrika ayarina donuldu: CanFeather / tesla1234");
}

// ─────────────────────────────────────────────────────────────
//  RP2040'A KOMUT GÖNDER, CEVABI AL
// ─────────────────────────────────────────────────────────────

String sendToRP2040(const String& cmd, unsigned long timeout = 500) {
  // Tamponu temizle
  while (Serial1.available()) Serial1.read();

  Serial1.println(cmd);

  unsigned long start = millis();
  String response = "";
  while (millis() - start < timeout) {
    if (Serial1.available()) {
      response = Serial1.readStringUntil('\n');
      response.trim();
      if (response.length() > 0) break;
    }
  }
  return response;
}

// ─────────────────────────────────────────────────────────────
//  WEB SUNUCU
// ─────────────────────────────────────────────────────────────

WebServer server(80);

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
.seg button{flex:1;padding:12px 6px;border-radius:8px;border:1px solid var(--bor);background:transparent;color:var(--mu);font-size:12px;cursor:pointer;transition:.15s;font-family:inherit}
.seg button.on{background:#10b98115;border-color:var(--acc);color:var(--acc);font-weight:500}
.seg button:hover:not(.on){border-color:#475569;color:var(--tx)}
.pr{display:flex;gap:5px;flex-wrap:wrap}
.pb{flex:1;min-width:60px;padding:12px 6px;border-radius:8px;border:1px solid var(--bor);background:transparent;color:var(--mu);font-size:12px;cursor:pointer;transition:.15s;font-family:inherit}
.pb.on{background:#6366f120;border-color:var(--acc2);color:var(--acc2);font-weight:500}
.pb:hover:not(.on){border-color:#475569;color:var(--tx)}
.tr{display:flex;align-items:center;justify-content:space-between}
.tg{position:relative;width:44px;height:26px;flex-shrink:0}
.tg input{opacity:0;width:0;height:0}
.sl{position:absolute;inset:0;border-radius:13px;background:#334155;border:1px solid var(--bor);cursor:pointer;transition:.2s}
.sl:before{content:'';position:absolute;width:18px;height:18px;left:3px;top:3px;border-radius:50%;background:var(--mu);transition:.2s}
input:checked+.sl{background:#10b98120;border-color:var(--acc)}
input:checked+.sl:before{transform:translateX(18px);background:var(--acc)}
.db{width:100%;padding:12px;border-radius:8px;border:1px solid #ef444440;background:#ef444410;color:var(--red);font-size:13px;font-weight:500;cursor:pointer;transition:.15s;font-family:inherit}
.db:hover{background:#ef444425;border-color:var(--red)}
.lb{font-family:monospace;font-size:10px;background:#0f172a;border:1px solid var(--bor);border-radius:8px;padding:10px;height:120px;overflow-y:auto;line-height:1.9}
.ll{color:#4a9eff}.ll:last-child{color:var(--acc)}
.sb{width:100%;padding:13px;border-radius:8px;border:none;background:linear-gradient(135deg,var(--acc),var(--acc2));color:#000;font-size:14px;font-weight:500;cursor:pointer;margin-top:4px;transition:.15s;font-family:inherit}
.sb:hover{opacity:.88}.sb:active{transform:scale(.98)}
.toast{position:fixed;bottom:20px;left:50%;transform:translateX(-50%) translateY(60px);background:var(--acc);color:#000;padding:9px 20px;border-radius:8px;font-size:12px;font-weight:500;transition:.3s;opacity:0;pointer-events:none}
.toast.show{transform:translateX(-50%) translateY(0);opacity:1}
.chip{font-family:monospace;font-size:9px;background:#06b6d415;border:1px solid #06b6d430;color:#67e8f9;border-radius:3px;padding:1px 6px;margin-left:6px}
.stb{font-family:monospace;font-size:11px;line-height:2;background:#162032;border-color:#1a3a2a;border-left:3px solid var(--acc)}
.stl{display:flex;gap:12px;flex-wrap:wrap}.stl b{color:var(--acc)}
.dvd{margin-top:14px;padding-top:14px;border-top:1px solid var(--bor)}
.inp{background:#0f172a;border:1px solid var(--bor);border-radius:6px;color:var(--tx);font-family:monospace;font-size:12px;padding:6px 8px;width:70px;text-align:center}
.inp:focus{outline:none;border-color:var(--acc)}
.row{display:flex;gap:6px;align-items:center;flex-wrap:wrap}
.mbtn{padding:10px 18px;border-radius:6px;border:1px solid var(--bor);background:transparent;color:var(--mu);font-size:11px;cursor:pointer;font-family:inherit;transition:.15s}
.mbtn:hover{border-color:var(--acc);color:var(--acc)}
.mbtn.act{background:#10b98115;border-color:var(--acc);color:var(--acc)}
.spark{height:40px;display:flex;align-items:flex-end;gap:1px}
.spark div{flex:1;background:var(--acc);border-radius:1px 1px 0 0;min-width:2px;opacity:.6;transition:height .3s}
.fhist{font-family:monospace;font-size:10px;line-height:1.8;color:var(--mu)}
.fhist b{color:var(--acc)}
.snif{font-family:monospace;font-size:9px;background:#0f172a;border:1px solid var(--bor);border-radius:8px;padding:8px;height:100px;overflow-y:auto;line-height:1.8;color:#f59e0b}
.lang-sw{display:inline-flex;gap:2px;margin-left:12px;vertical-align:middle}
.lang-sw button{padding:2px 8px;border-radius:4px;border:1px solid var(--bor);background:transparent;color:var(--mu);font-size:10px;font-family:monospace;cursor:pointer;transition:.15s}
.lang-sw button.on{background:#10b98118;border-color:var(--acc);color:var(--acc);font-weight:600}
.lang-sw button:hover:not(.on){border-color:#475569;color:var(--tx)}
</style>
</head>
<body>
<div class="w">
  <div style="margin-bottom:24px">
    <div class="logo">// CanFeather</div>
    <h1><span data-tr="Kontroller" data-en="Controller">Kontroller</span></h1>
    <div style="margin-top:6px">
      <span class="badge" id="hwb">HW3</span><span class="chip">RP2040+C3</span>
      <span class="lang-sw">
        <button id="lTR" class="on" onclick="setLang('tr')">TR</button>
        <button id="lEN" onclick="setLang('en')">EN</button>
      </span>
    </div>
    <div class="sub"><span class="dot"></span><span data-tr="CAN kopru ayarlari. Donanim ve surus profiline gore yapilandir." data-en="CAN bridge settings. Configure by hardware and drive profile.">CAN kopru ayarlari. Donanim ve surus profiline gore yapilandir.</span></div>
  </div>

  <div class="card stb" id="stb">
    <div class="stl"><span>CAN: <b id="sC">&mdash;</b></span> <span>FSD: <b id="sF">&mdash;</b></span> <span>NAG: <b id="sN">&mdash;</b></span></div>
    <div class="stl"><span>HW: <b id="sH">&mdash;</b></span> <span><span data-tr="Profile" data-en="Profile">Profile</span>: <b id="sP">&mdash;</b></span></div>
    <div class="stl"><span>Follow dist: <b id="sD">&mdash;</b></span> <span>Offset: <b id="sO">&mdash;</b></span></div>
    <div class="stl"><span>RX: <b id="sR">0</b></span> <span>TX: <b id="sT">0</b></span> <span>INJ: <b id="sI">&mdash;</b></span></div>
    <div class="stl"><span>Uptime: <b id="sU">&mdash;</b></span> <span>Err: <b id="sE">0</b></span></div>
  </div>

  <div class="card">
    <div class="ct" data-tr="&#x1F4CA; Trafik (son 60sn)" data-en="&#x1F4CA; Traffic (last 60s)">&#x1F4CA; Trafik (son 60sn)</div>
    <div class="spark" id="spark"></div>
  </div>

  <div class="card">
    <div class="ct" data-tr="&#x1F527; Donanim" data-en="&#x1F527; Hardware">&#x1F527; Donanim</div>
    <div class="seg">
      <button onclick="setHW(0)" id="h0">Legacy</button>
      <button onclick="setHW(1)" id="h1" class="on">HW3</button>
      <button onclick="setHW(2)" id="h2">HW4</button>
    </div>
    <div class="ct" style="margin-top:14px" data-tr="&#x1F48E; MCP2515 Kristal" data-en="&#x1F48E; MCP2515 Crystal">&#x1F48E; MCP2515 Kristal</div>
    <div class="seg" style="margin-top:6px">
      <button onclick="setCrystal(0)" id="cr0" data-tr="8 MHz" data-en="8 MHz">8 MHz</button>
      <button onclick="setCrystal(1)" id="cr1" class="on" data-tr="16 MHz" data-en="16 MHz">16 MHz</button>
    </div>
  </div>

  <div class="card">
    <div class="ct" data-tr="&#x1F3CE;&#xFE0F; Hiz profili" data-en="&#x1F3CE;&#xFE0F; Speed profile">&#x1F3CE;&#xFE0F; Hiz profili</div>
    <div class="pr" id="pr"></div>
  </div>

  <div class="card">
    <div class="ct" data-tr="&#x1F489; Enjeksiyon" data-en="&#x1F489; Injection">&#x1F489; Enjeksiyon</div>
    <div class="tr">
      <div><div style="font-size:13px;font-weight:500" data-tr="FSD etkin" data-en="FSD enabled">FSD etkin</div>
      <div style="font-size:11px;color:var(--mu);margin-top:2px" data-tr="Veri yolunda enjeksiyon mantigi" data-en="Injection logic on the data bus">Veri yolunda enjeksiyon mantigi</div></div>
      <label class="tg"><input type="checkbox" id="ft" checked><span class="sl"></span></label>
    </div>
    <div class="tr dvd">
      <div><div style="font-size:13px;font-weight:500" data-tr="Profili buradan uygula" data-en="Apply profile from here">Profili buradan uygula</div>
      <div style="font-size:11px;color:var(--mu);margin-top:2px" data-tr="CAN profil secimi yok sayilir" data-en="CAN profile selection ignored">CAN profil secimi yok sayilir</div></div>
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
      <div style="font-size:11px;color:var(--mu);margin-top:2px" data-tr="Hiz limiti isareti bos kalir" data-en="Speed limit sign will be empty">Hiz limiti isareti bos kalir</div></div>
      <label class="tg"><input type="checkbox" id="isa"><span class="sl"></span></label>
    </div>
    <div class="tr dvd">
      <div><div style="font-size:13px;font-weight:500" data-tr="Acil arac algilama" data-en="Emergency vehicle detect">Acil arac algilama</div>
      <div style="font-size:11px;color:var(--mu);margin-top:2px" data-tr="Yaklasan acil arac tespiti" data-en="Approaching emergency vehicle detection">Yaklasan acil arac tespiti</div></div>
      <label class="tg"><input type="checkbox" id="ev" checked><span class="sl"></span></label>
    </div>
  </div>

  <div class="card">
    <div class="ct" data-tr="&#x1F4DC; FSD Durum Gecmisi" data-en="&#x1F4DC; FSD State History">&#x1F4DC; FSD Durum Gecmisi</div>
    <div class="fhist" id="fhist" data-tr="&mdash; bekleniyor &mdash;" data-en="&mdash; waiting &mdash;">&mdash; bekleniyor &mdash;</div>
  </div>

  <div class="card">
    <div class="ct" style="margin-bottom:10px" data-tr="&#x1F4CB; CAN Log" data-en="&#x1F4CB; CAN Log">&#x1F4CB; CAN Log</div>
    <div class="row" style="margin-bottom:8px">
      <span style="font-size:11px;color:var(--mu)" data-tr="Filtre ID:" data-en="Filter ID:">Filtre ID:</span>
      <input class="inp" id="flt" type="number" min="0" max="2047" value="0" placeholder="0=all">
      <button class="mbtn" onclick="applyFilter()" data-tr="Uygula" data-en="Apply">Uygula</button>
    </div>
    <div class="lb" id="log"><span style="color:#475569">// bekleniyor...</span></div>
  </div>

  <div class="card">
    <div class="ct" style="margin-bottom:10px" data-tr="&#x1F4DF; Serial Log" data-en="&#x1F4DF; Serial Log">&#x1F4DF; Serial Log</div>
    <div class="cd" data-tr="RP2040 sistem mesajlari (son 16)" data-en="RP2040 system messages (last 16)">RP2040 sistem mesajlari (son 16)</div>
    <div class="lb" id="slog"><span style="color:#475569">// bekleniyor...</span></div>
  </div>

  <div class="card">
    <div class="ct" data-tr="&#x1F50D; CAN Sniffer" data-en="&#x1F50D; CAN Sniffer">&#x1F50D; CAN Sniffer</div>
    <div class="cd" data-tr="Tum CAN tratigini yakalar &mdash; debug icin" data-en="Captures all CAN traffic &mdash; for debug">Tum CAN tratigini yakalar &mdash; debug icin</div>
    <div class="tr" style="margin-bottom:10px">
      <div style="font-size:13px;font-weight:500" data-tr="Sniffer modu" data-en="Sniffer mode">Sniffer modu</div>
      <label class="tg"><input type="checkbox" id="snf" onchange="toggleSnif()"><span class="sl"></span></label>
    </div>
    <div class="snif" id="snif"><span style="color:#475569" data-tr="// sniffer kapali" data-en="// sniffer off">// sniffer kapali</span></div>
  </div>

  <div class="card">
    <div class="ct" data-tr="&#x1F4E8; CAN Mesaj Gonder" data-en="&#x1F4E8; Send CAN Message">&#x1F4E8; CAN Mesaj Gonder</div>
    <div class="cd" data-tr="Manuel CAN frame gonderme &mdash; test/debug icin" data-en="Manual CAN frame send &mdash; for test/debug">Manuel CAN frame gonderme &mdash; test/debug icin</div>
    <div class="row" style="margin-bottom:8px">
      <span style="font-size:11px;color:var(--mu)">ID:</span>
      <input class="inp" id="sid" type="number" min="0" max="2047" value="1021">
      <span style="font-size:11px;color:var(--mu)">DLC:</span>
      <input class="inp" id="sdlc" type="number" min="0" max="8" value="8" style="width:40px">
    </div>
    <div class="row" style="margin-bottom:8px">
      <span style="font-size:11px;color:var(--mu)">Data (dec):</span>
      <input class="inp" id="sdata" type="text" value="0 0 0 0 0 0 0 0" style="width:200px;text-align:left">
    </div>
    <button class="mbtn" onclick="sendCAN()" data-tr="Gonder" data-en="Send">Gonder</button>
  </div>

  <div class="card">
    <div class="ct" data-tr="&#x1F697; Arac Kontrol" data-en="&#x1F697; Vehicle Control">&#x1F697; Arac Kontrol</div>
    <div class="cd" data-tr="CAN bus uzerinden arac komutlari (RP2040 uzerinden)" data-en="Vehicle commands via CAN bus (through RP2040)">CAN bus uzerinden arac komutlari (RP2040 uzerinden)</div>
    <div style="font-size:10px;color:var(--mu);margin-bottom:6px">ID 0x273 &mdash; VehicleBus &mdash; dinle-degistir-gonder</div>
    <div class="ct dvd" style="font-size:11px;color:#818cf8">&#x1F512; Ayna &amp; Kilit</div>
    <div style="display:grid;grid-template-columns:1fr 1fr;gap:5px;margin:6px 0">
      <button class="mbtn" onclick="vcmd('mirror_fold')">Ayna Katla</button>
      <button class="mbtn" onclick="vcmd('mirror_unfold')">Ayna Ac</button>
      <button class="mbtn" onclick="vcmd('mirror_heat')">Ayna Isit</button>
      <button class="mbtn" onclick="vcmd('mirror_autofold')">Oto Katlama</button>
      <button class="mbtn" onclick="vcmd('lock')">Kilitle</button>
      <button class="mbtn" onclick="vcmd('unlock')">Kilidi Ac</button>
      <button class="mbtn" onclick="vcmd('child_lock')">Cocuk Kilidi</button>
    </div>
    <div class="ct dvd" style="font-size:11px;color:#818cf8">&#x1F697; Bagaj &amp; Kaput</div>
    <div style="display:grid;grid-template-columns:1fr 1fr 1fr;gap:5px;margin:6px 0">
      <button class="mbtn" onclick="vcmd('frunk')">Frunk</button>
      <button class="mbtn" onclick="vcmd('trunk')">Bagaj</button>
      <button class="mbtn" onclick="vcmd('glovebox')">Torpido</button>
    </div>
    <div class="ct dvd" style="font-size:11px;color:#818cf8">&#x1F4A1; Isiklar</div>
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
    <div class="ct dvd" style="font-size:11px;color:#818cf8">&#x1F32A; Silecek</div>
    <div class="seg" style="margin:4px 0"><button onclick="vcmd('wiper_off')">OFF</button><button onclick="vcmd('wiper_1')">1</button><button onclick="vcmd('wiper_2')">2</button><button onclick="vcmd('wiper_3')">3</button></div>
    <div class="ct dvd" style="font-size:11px;color:#818cf8">&#x1F50B; Diger</div>
    <div style="display:grid;grid-template-columns:1fr 1fr;gap:5px;margin:6px 0">
      <button class="mbtn" onclick="vcmd('horn')">Korna</button>
      <button class="mbtn" onclick="vcmd('summon')">Summon</button>
      <button class="mbtn" onclick="vcmd('acc_power')">Aksesuar Guc</button>
      <button class="mbtn" onclick="vcmd('power_off')" style="border-color:#ef444440;color:var(--red)">Arac Kapat</button>
    </div>
    <div class="ct dvd" style="font-size:11px;color:#818cf8">&#x1F528; Ekran</div>
    <div class="row" style="margin:6px 0"><span style="font-size:10px;width:55px">Parlaklik:</span><input class="inp" id="brVal" type="range" min="0" max="127" value="64" style="flex:1"><button class="mbtn" onclick="vcmd('bright'+document.getElementById('brVal').value)" style="padding:6px 10px">Ayarla</button></div>
    <div class="ct dvd" style="font-size:11px;color:#818cf8">&#x1F525; Koltuk Isitma</div>
    <div style="display:grid;grid-template-columns:1fr 1fr;gap:4px;margin-top:6px;font-size:11px">
      <div class="row"><span style="width:28px">FL:</span><button class="mbtn" onclick="vcmd('seat_fl 0')" style="padding:6px 8px">0</button><button class="mbtn" onclick="vcmd('seat_fl 1')" style="padding:6px 8px">1</button><button class="mbtn" onclick="vcmd('seat_fl 2')" style="padding:6px 8px">2</button><button class="mbtn" onclick="vcmd('seat_fl 3')" style="padding:6px 8px">3</button></div>
      <div class="row"><span style="width:28px">FR:</span><button class="mbtn" onclick="vcmd('seat_fr 0')" style="padding:6px 8px">0</button><button class="mbtn" onclick="vcmd('seat_fr 1')" style="padding:6px 8px">1</button><button class="mbtn" onclick="vcmd('seat_fr 2')" style="padding:6px 8px">2</button><button class="mbtn" onclick="vcmd('seat_fr 3')" style="padding:6px 8px">3</button></div>
      <div class="row"><span style="width:28px">RL:</span><button class="mbtn" onclick="vcmd('seat_rl 0')" style="padding:6px 8px">0</button><button class="mbtn" onclick="vcmd('seat_rl 1')" style="padding:6px 8px">1</button><button class="mbtn" onclick="vcmd('seat_rl 2')" style="padding:6px 8px">2</button><button class="mbtn" onclick="vcmd('seat_rl 3')" style="padding:6px 8px">3</button></div>
      <div class="row"><span style="width:28px">RR:</span><button class="mbtn" onclick="vcmd('seat_rr 0')" style="padding:6px 8px">0</button><button class="mbtn" onclick="vcmd('seat_rr 1')" style="padding:6px 8px">1</button><button class="mbtn" onclick="vcmd('seat_rr 2')" style="padding:6px 8px">2</button><button class="mbtn" onclick="vcmd('seat_rr 3')" style="padding:6px 8px">3</button></div>
      <div class="row"><span style="width:28px">RC:</span><button class="mbtn" onclick="vcmd('seat_rc 0')" style="padding:6px 8px">0</button><button class="mbtn" onclick="vcmd('seat_rc 1')" style="padding:6px 8px">1</button><button class="mbtn" onclick="vcmd('seat_rc 2')" style="padding:6px 8px">2</button><button class="mbtn" onclick="vcmd('seat_rc 3')" style="padding:6px 8px">3</button></div>
    </div>
    <div class="ct dvd" style="font-size:10px;color:var(--mu)">&#x26A0; Farkli CAN ID (bus'ta yoksa calismaz)</div>
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
    <div class="ct dvd" data-tr="&#x2699; Surus Modu" data-en="&#x2699; Drive Mode">&#x2699; Surus Modu</div>
    <div style="margin-top:6px;font-size:11px;color:var(--mu)">Pedal:</div>
    <div class="seg" style="margin:4px 0" id="seg-pedal"><button onclick="setPedal(0)">Std</button><button onclick="setPedal(1)">Chill</button><button onclick="setPedal(2)">Sport</button></div>
    <div style="font-size:11px;color:var(--mu)">Regen:</div>
    <div class="seg" style="margin:4px 0" id="seg-regen"><button onclick="setRegen(0)">OFF</button><button onclick="setRegen(50)">Low</button><button onclick="setRegen(100)">Std</button><button onclick="setRegen(200)">Max</button></div>
    <div style="font-size:11px;color:var(--mu)">Durma:</div>
    <div class="seg" style="margin:4px 0" id="seg-stop"><button onclick="setStop(0)">Creep</button><button onclick="setStop(1)">Roll</button><button onclick="setStop(2)">Hold</button></div>
  </div>

  <div class="card">
    <div class="ct" data-tr="&#x1F4F6; WiFi Ayarlari" data-en="&#x1F4F6; WiFi Settings">&#x1F4F6; WiFi Ayarlari</div>
    <div class="cd" data-tr="SSID ve sifre degistir (min 8 karakter)" data-en="Change SSID and password (min 8 chars)">SSID ve sifre degistir (min 8 karakter)</div>
    <div class="row" style="margin:6px 0"><span style="font-size:11px;color:var(--mu);width:45px">SSID:</span><input class="inp" id="wSSID" style="flex:1;width:auto;text-align:left" value=""></div>
    <div class="row" style="margin:6px 0"><span style="font-size:11px;color:var(--mu);width:45px" data-tr="Sifre:" data-en="Pass:">Sifre:</span><input class="inp" id="wPASS" type="password" style="flex:1;width:auto;text-align:left" value="" placeholder="min 8 karakter"></div>
    <div class="row" style="margin:8px 0;gap:6px"><button class="mbtn" onclick="saveWifi()" style="background:#10b98118;border-color:var(--acc);color:var(--acc)" data-tr="Kaydet &amp; Yeniden Baslat" data-en="Save &amp; Restart">Kaydet &amp; Yeniden Baslat</button><button class="mbtn" onclick="resetWifi()" style="border-color:#ef444440;color:var(--red)" data-tr="Fabrika Ayari" data-en="Factory Reset">Fabrika Ayari</button></div>
  </div>

  <div class="card">
    <div class="ct" style="margin-bottom:12px" data-tr="&#x1F6E1;&#xFE0F; Guvenlik" data-en="&#x1F6E1;&#xFE0F; Safety">&#x1F6E1;&#xFE0F; Guvenlik</div>
    <button class="db" onclick="disableAll()" data-tr="Tum enjeksiyonlari devre disi birak" data-en="Disable all injections">Tum enjeksiyonlari devre disi birak</button>
  </div>

  <div class="row" style="margin-top:12px;gap:8px">
    <button class="sb" style="flex:1" onclick="save()" data-tr="Kaydet &amp; EEPROM" data-en="Save &amp; EEPROM">Kaydet &amp; EEPROM</button>
  </div>
</div>
<div class="toast" id="t"></div>
<script>
let hw=1,prof=2,crystal=1,synced=false;
let lang=localStorage.getItem('cflang')||'tr';
const pN=['Chill','Normal','Hurry','Max','Sloth'];
const pS=[[2,1,0],[2,1,0],[3,2,1,0,4]];

const L={
  tr:{
    saved:'Ayarlar kaydedildi',
    connErr:'Baglanti hatasi',
    disabled:'Tum enjeksiyonlar durduruldu',
    eeprom:'EEPROM kaydedildi',
    filterOk:'Filtre uygulandi',
    snifOn:'Sniffer acik',
    snifOff:'Sniffer kapali',
    canSent:'CAN frame gonderildi',
    noEvent:'\u2014 henuz olay yok \u2014',
    fsdOn:'\u25B6 FSD ACILDI',
    fsdOff:'\u25A0 FSD KAPANDI'
  },
  en:{
    saved:'Settings saved',
    connErr:'Connection error',
    disabled:'All injections stopped',
    eeprom:'EEPROM saved',
    filterOk:'Filter applied',
    snifOn:'Sniffer on',
    snifOff:'Sniffer off',
    canSent:'CAN frame sent',
    noEvent:'\u2014 no events yet \u2014',
    fsdOn:'\u25B6 FSD ENABLED',
    fsdOff:'\u25A0 FSD DISABLED'
  }
};

function setLang(l){
  lang=l;localStorage.setItem('cflang',l);
  document.getElementById('lTR').classList.toggle('on',l==='tr');
  document.getElementById('lEN').classList.toggle('on',l==='en');
  document.querySelectorAll('[data-'+l+']').forEach(function(el){el.textContent=el.getAttribute('data-'+l);});
}

function cmd(c){return fetch('/cmd?c='+encodeURIComponent(c)).then(r=>r.text());}
function setHW(n){hw=n;[0,1,2].forEach(i=>document.getElementById('h'+i).classList.toggle('on',i===n));document.getElementById('hwb').textContent=['Legacy','HW3','HW4'][n];buildP();}
function setCrystal(n){crystal=n;[0,1].forEach(i=>document.getElementById('cr'+i).classList.toggle('on',i===n));}
function buildP(){const s=pS[hw],c=document.getElementById('pr');c.innerHTML='';s.forEach(v=>{const b=document.createElement('button');b.className='pb'+(v===prof?' on':'');b.textContent=pN[v];b.onclick=()=>setP(v);c.appendChild(b);});}
function setP(n){prof=n;document.querySelectorAll('.pb').forEach(b=>b.classList.toggle('on',b.textContent===pN[n]));}
function disableAll(){document.getElementById('ft').checked=false;document.getElementById('ci').checked=false;cmd('disable').then(()=>toast(L[lang].disabled));}
function save(){
  cmd('hw'+hw)
  .then(()=>cmd('p'+prof))
  .then(()=>{const fsd=document.getElementById('ft').checked;return cmd(fsd?'on':'off');})
  .then(()=>{const ovr=document.getElementById('ov').checked;return cmd(ovr?'ovron':'ovroff');})
  .then(()=>{const inj=document.getElementById('ci').checked;return cmd(inj?'injon':'injoff');})
  .then(()=>{const ao=parseInt(document.getElementById('ao').value)||0;return cmd('autooff '+ao);})
  .then(()=>{const flt=parseInt(document.getElementById('flt').value)||0;return cmd('filter '+flt);})
  .then(()=>cmd(crystal===0?'crystal8':'crystal16'))
  .then(()=>{const isa=document.getElementById('isa').checked;return cmd(isa?'isaon':'isaoff');})
  .then(()=>{const ev=document.getElementById('ev').checked;return cmd(ev?'evon':'evoff');})
  .then(()=>cmd('save'))
  .then(()=>toast(L[lang].saved))
  .catch(()=>toast(L[lang].connErr));
}
function applyFilter(){const flt=parseInt(document.getElementById('flt').value)||0;cmd('filter '+flt).then(()=>toast(L[lang].filterOk));}
function toggleSnif(){const on=document.getElementById('snf').checked;cmd(on?'snifon':'snifoff').then(()=>toast(on?L[lang].snifOn:L[lang].snifOff));}
function sendCAN(){const id=parseInt(document.getElementById('sid').value)||0;const dlc=parseInt(document.getElementById('sdlc').value)||8;const vals=document.getElementById('sdata').value.trim().split(/[\s,]+/).map(v=>parseInt(v)||0);let c='send '+id+' '+dlc;for(let i=0;i<dlc;i++)c+=' '+(vals[i]||0);cmd(c).then(()=>toast(L[lang].canSent)).catch(()=>toast(L[lang].connErr));}
function vcmd(c){fetch('/vcmd?c='+encodeURIComponent(c)).then(r=>r.json()).then(d=>{toast((d.cmd||c)+' gonderildi');}).catch(()=>toast('Hata'));}
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
function fmtTime(ms){const s=Math.floor(ms/1000);const m=Math.floor(s/60);const h=Math.floor(m/60);if(h>0)return h+'h '+m%60+'m';if(m>0)return m+'m '+s%60+'s';return s+'s';}
function updateSB(d){
  document.getElementById('sC').textContent=d.canOk?(lang==='tr'?'AKTIF':'ACTIVE'):(lang==='tr'?'HATA':'ERROR');document.getElementById('sC').style.color=d.canOk?'#10b981':'#ef4444';
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
function fetchStatus(){cmd('status').then(d=>{try{let j=JSON.parse(d);if(!synced){setHW(j.hw!=null?j.hw:1);setP(j.profile!=null?j.profile:2);document.getElementById('ft').checked=!!j.fsd;document.getElementById('ov').checked=!!j.ovr;document.getElementById('ci').checked=j.inj!==false;document.getElementById('ao').value=j.autoOff||0;document.getElementById('flt').value=j.logFilter||0;document.getElementById('snf').checked=!!j.snif;setCrystal(j.crystal!=null?j.crystal:1);document.getElementById('isa').checked=!!j.isa;document.getElementById('ev').checked=j.ev!==false;synced=true;}updateSB(j);syncDriveMode(j);}catch(e){}}).catch(()=>{});}
function fetchLog(){cmd('log').then(d=>{try{let j=JSON.parse(d);if(!j.lines||!j.lines.length)return;const b=document.getElementById('log');b.innerHTML=j.lines.map(l=>'<div class="ll">'+l+'</div>').join('');b.scrollTop=b.scrollHeight;}catch(e){}}).catch(()=>{});}
function fetchStats(){cmd('stats').then(d=>{try{let j=JSON.parse(d);drawSpark(j.hist);drawFHist(j.fsdHist);}catch(e){}}).catch(()=>{});}
function fetchSlog(){cmd('slog').then(d=>{try{let j=JSON.parse(d);if(!j.lines||!j.lines.length)return;const b=document.getElementById('slog');b.innerHTML=j.lines.map(l=>'<div class="ll">'+l+'</div>').join('');b.scrollTop=b.scrollHeight;}catch(e){}}).catch(()=>{});}
function fetchSnif(){if(!document.getElementById('snf').checked)return;cmd('sniflog').then(d=>{try{let j=JSON.parse(d);if(!j.frames||!j.frames.length)return;const b=document.getElementById('snif');b.innerHTML=j.frames.map(f=>'<div>ID:'+f.id+' ['+f.dlc+'] '+f.data+'</div>').join('');b.scrollTop=b.scrollHeight;}catch(e){}}).catch(()=>{});}
buildP();setLang(lang);fetchStatus();fetchStats();fetchSlog();loadWifiInfo();
setInterval(fetchStatus,2000);setInterval(fetchLog,2000);setInterval(fetchStats,5000);setInterval(fetchSnif,1000);setInterval(fetchSlog,3000);
</script>
</body>
</html>)HTML";

// ── Route handler'ları ────────────────────────────────────────

void onRoot() {
  server.send_P(200, "text/html", HTML);
}

void onCmd() {
  String cmd = server.arg("c");
  if (cmd.length() == 0) {
    server.send(400, "text/plain", "missing ?c=");
    return;
  }
  String response = sendToRP2040(cmd);
  // JSON response ise content-type ayarla
  if (response.startsWith("{") || response.startsWith("[")) {
    server.send(200, "application/json", response);
  } else {
    server.send(200, "text/plain", response);
  }
}

void onVehicleCmd() {
  String c = server.arg("c");
  if (c.length() == 0) {
    server.send(400, "application/json", "{\"err\":\"c param gerekli\"}");
    return;
  }
  // Vehicle komutlarini UART uzerinden RP2040'a gonder
  String response = sendToRP2040(c, 8000);  // uzun timeout — bazi komutlar 3-5sn surer
  if (response.startsWith("{") || response.startsWith("[")) {
    server.send(200, "application/json", response);
  } else {
    server.send(200, "application/json", "{\"ok\":true,\"cmd\":\"" + c + "\"}");
  }
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

// ─────────────────────────────────────────────────────────────
//  SETUP
// ─────────────────────────────────────────────────────────────

void setup() {
  delay(500);
  Serial.begin(115200);   // USB debug
  Serial1.begin(115200, SERIAL_8N1, BRIDGE_RX, BRIDGE_TX);  // UART → RP2040

  Serial.println("\n=== ESP32-C3 WiFi Bridge ===");

  // EEPROM
  EEPROM.begin(EEPROM_SIZE);
  loadWiFiConfig();

  // WiFi Access Point
  WiFi.mode(WIFI_AP);
  WiFi.softAP(wifiSSID, wifiPASS);
  Serial.printf("[wifi] SSID: %s  IP: %s\n", wifiSSID, WiFi.softAPIP().toString().c_str());

  // Web sunucu
  server.on("/",    HTTP_GET, onRoot);
  server.on("/cmd", HTTP_GET, onCmd);
  server.on("/vcmd", HTTP_GET, onVehicleCmd);
  server.on("/wifi", HTTP_GET, onWiFiConfig);
  server.on("/wifi", HTTP_POST, onWiFiConfig);
  server.on("/wifireset", HTTP_GET, onWiFiReset);
  server.begin();
  Serial.println("[web] http://192.168.4.1");
  Serial.println("[ok] Bridge hazir.");
}

// ─────────────────────────────────────────────────────────────
//  LOOP
// ─────────────────────────────────────────────────────────────

void loop() {
  server.handleClient();
}
