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
#include <Update.h>           // v2.6: OTA firmware guncelleme
#include <esp_ota_ops.h>      // v2.6: dual-partition rollback
#include <ArduinoJson.h>
#include <EEPROM.h>

// ─────────────────────────────────────────────────────────────
//  AYARLAR
// ─────────────────────────────────────────────────────────────

#define FW_VERSION     "v3.1"
#define FW_DATE        "15.04.2026"
#define DEFAULT_PASS   "tesla1234"

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
.locked-card{background:repeating-linear-gradient(45deg,var(--sur),var(--sur) 10px,#1a2540 10px,#1a2540 20px);border:1px solid var(--bor);border-radius:14px;padding:14px 16px;margin:14px 0;opacity:.6;cursor:not-allowed;position:relative}
.locked-card .lc-head{display:flex;align-items:center;justify-content:space-between;font-size:13px;font-weight:500}
.locked-card .lc-badge{background:linear-gradient(135deg,#f59e0b,#d97706);color:#fff;font-size:9px;font-weight:700;padding:2px 8px;border-radius:4px;letter-spacing:1px}
.locked-card .lc-hidden{display:none}
.spark{height:40px;display:flex;align-items:flex-end;gap:1px}
.spark div{flex:1;background:var(--acc);border-radius:1px 1px 0 0;min-width:2px;opacity:.6;transition:height .3s}
.fhist{font-family:monospace;font-size:10px;line-height:1.8;color:var(--mu)}
.fhist b{color:var(--acc)}
.ltab{flex:1;padding:8px 12px;background:transparent;border:none;color:var(--mu);font-size:12px;cursor:pointer;border-bottom:2px solid transparent;font-family:inherit;transition:.2s}
.ltab:hover{color:var(--tx)}
.ltab.on{color:var(--acc);border-bottom-color:var(--acc);font-weight:600}
.lang-sw{display:inline-flex;gap:2px;margin-left:12px;vertical-align:middle}
.lang-sw button{padding:2px 8px;border-radius:4px;border:1px solid var(--bor);background:transparent;color:var(--mu);font-size:10px;font-family:monospace;cursor:pointer;transition:.15s}
.lang-sw button.on{background:#10b98118;border-color:var(--acc);color:var(--acc);font-weight:600}
.lang-sw button:hover:not(.on){border-color:#475569;color:var(--tx)}
</style>
</head>
<body>
<div class="w">
  <div style="margin-bottom:24px">
    <div class="logo">// CanFeather <span id="fwVer" style="color:var(--mu);font-size:9px;margin-left:6px">v?.?</span></div>
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

  <div id="defPwBanner" style="display:none;background:linear-gradient(135deg,#7f1d1d,#991b1b);border:2px solid #ef4444;border-radius:10px;padding:12px 16px;margin:12px 0;color:#fff;font-size:12px;line-height:1.6">
    <div style="font-weight:700;font-size:13px;margin-bottom:6px">&#9888; ZORUNLU: WiFi Sifrenizi Degistirin</div>
    <div data-tr="Hala default sifre kullaniliyor (tesla1234). Cevredeki herkes cihaza baglanip aracinizi kontrol edebilir!" data-en="Still using default password (tesla1234). Anyone nearby can connect and control your vehicle!">Hala default sifre kullaniliyor (<b>tesla1234</b>). Cevredeki herkes cihaza baglanip aracinizi kontrol edebilir!</div>
    <a href="#wifiCard" onclick="document.getElementById('wifiCard').scrollIntoView({behavior:'smooth'});return false" style="display:inline-block;margin-top:8px;background:#fff;color:#991b1b;padding:6px 14px;border-radius:6px;text-decoration:none;font-weight:600;font-size:11px">&#128272; Sifre Degistir &rarr;</a>
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
    <div class="tr dvd">
      <div><div style="font-size:13px;font-weight:500;color:#a78bfa" data-tr="&#128737; Hands-Free Steering" data-en="&#128737; Hands-Free Steering">&#128737; Hands-Free Steering</div>
      <div style="font-size:11px;color:var(--mu);margin-top:2px" data-tr="Direksiyona dokunmadan surme — CAN 880 torque sahteleme" data-en="Drive without touching steering — CAN 880 torque spoofing">Direksiyona dokunmadan surme &mdash; CAN 880 torque sahteleme</div></div>
      <label class="tg"><input type="checkbox" id="nag"><span class="sl"></span></label>
    </div>
    <div class="tr dvd">
      <div><div style="font-size:13px;font-weight:500;color:#a78bfa" data-tr="&#9889; Bypass TLSSC" data-en="&#9889; Bypass TLSSC">&#9889; Bypass TLSSC</div>
      <div style="font-size:11px;color:var(--mu);margin-top:2px" data-tr="Trafik Isigi gerekmeden FSD ac" data-en="FSD without Traffic Light requirement">Trafik Isigi gerekmeden FSD ac</div></div>
      <label class="tg"><input type="checkbox" id="btlssc"><span class="sl"></span></label>
    </div>
    <div class="tr dvd">
      <div><div style="font-size:13px;font-weight:500;color:#a78bfa" data-tr="&#9851; Bus-off Auto Recovery" data-en="&#9851; Bus-off Auto Recovery">&#9851; Bus-off Auto Recovery</div>
      <div style="font-size:11px;color:var(--mu);margin-top:2px" data-tr="CAN bus hata olunca otomatik reset" data-en="Auto reset on CAN bus error">CAN bus hata olunca otomatik reset</div></div>
      <label class="tg"><input type="checkbox" id="recov" checked><span class="sl"></span></label>
    </div>
    <div class="tr dvd">
      <div><div style="font-size:13px;font-weight:500;color:#06b6d4" data-tr="&#128586; ISA Speed Override" data-en="&#128586; ISA Speed Override">&#128586; ISA Speed Override</div>
      <div style="font-size:11px;color:var(--mu);margin-top:2px" data-tr="Nav hiz limiti gercek hizi ezmesin (HW4)" data-en="Nav speed limit doesn't override real speed (HW4)">Nav hiz limiti gercek hizi ezmesin (HW4)</div></div>
      <label class="tg"><input type="checkbox" id="isaOvr"><span class="sl"></span></label>
    </div>
    <div class="tr dvd">
      <div><div style="font-size:13px;font-weight:500;color:#06b6d4" data-tr="🎚 ISA Carpan" data-en="🎚 ISA Multiplier">🎚 ISA Carpan</div>
      <div style="font-size:11px;color:var(--mu);margin-top:2px" data-tr="Offset carpani (1-15, default 7) — yuksek=daha guclu" data-en="Offset multiplier (1-15, default 7) — higher=stronger">Offset carpani (1-15, default 7) — yuksek=daha guclu</div></div>
      <input class="inp" id="isaMul" type="number" min="1" max="15" value="7" style="width:54px">
    </div>
  </div>

  <div class="card">
    <div class="ct" data-tr="&#x1F4DC; FSD Durum Gecmisi" data-en="&#x1F4DC; FSD State History">&#x1F4DC; FSD Durum Gecmisi</div>
    <div class="fhist" id="fhist" data-tr="&mdash; bekleniyor &mdash;" data-en="&mdash; waiting &mdash;">&mdash; bekleniyor &mdash;</div>
  </div>

  <div class="card">
    <div class="logtabs" style="display:flex;gap:4px;margin-bottom:10px;border-bottom:1px solid var(--bor)">
      <button class="ltab on" id="lt-can" onclick="switchLogTab('can')" data-tr="&#x1F4CB; CAN Log" data-en="&#x1F4CB; CAN Log">&#x1F4CB; CAN Log</button>
      <button class="ltab" id="lt-serial" onclick="switchLogTab('serial')" data-tr="&#x1F4DF; Serial Log" data-en="&#x1F4DF; Serial Log">&#x1F4DF; Serial Log</button>
    </div>
    <div id="ltp-can">
      <div class="row" style="margin-bottom:8px">
        <span style="font-size:11px;color:var(--mu)" data-tr="Filtre ID:" data-en="Filter ID:">Filtre ID:</span>
        <input class="inp" id="flt" type="number" min="0" max="2047" value="0" placeholder="0=all">
        <button class="mbtn" onclick="applyFilter()" data-tr="Uygula" data-en="Apply">Uygula</button>
      </div>
      <div class="lb" id="log"><span style="color:#475569">// bekleniyor...</span></div>
    </div>
    <div id="ltp-serial" style="display:none">
      <div class="cd" style="margin-bottom:6px" data-tr="RP2040 sistem mesajlari (son 16)" data-en="RP2040 system messages (last 16)">RP2040 sistem mesajlari (son 16)</div>
      <div class="lb" id="slog" style="color:#f59e0b"><span style="color:#475569">// bekleniyor...</span></div>
    </div>
  </div>

  <div class="locked-card">
    <div class="lc-head">
      <span data-tr="🚗 Arac Kontrol" data-en="🚗 Vehicle Control">&#128663; Arac Kontrol</span>
      <span class="lc-badge" data-tr="YAKINDA" data-en="SOON">YAKINDA</span>
    </div>
    <div style="font-size:10px;color:var(--mu);margin-top:6px" data-tr="Test asamasinda — yakinda aktif olacak" data-en="In testing — coming soon">Test asamasinda &mdash; yakinda aktif olacak</div>
    <div class="lc-hidden">
    <div class="cd" style="margin-top:10px" data-tr="CAN bus uzerinden arac komutlari (RP2040 uzerinden)" data-en="Vehicle commands via CAN bus (through RP2040)">CAN bus uzerinden arac komutlari (RP2040 uzerinden)</div>
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
  </div>

  <div class="card" id="wifiCard">
    <div class="ct" data-tr="&#x1F4F6; WiFi Ayarlari" data-en="&#x1F4F6; WiFi Settings">&#x1F4F6; WiFi Ayarlari</div>
    <div class="cd" data-tr="SSID ve sifre degistir (min 8 karakter)" data-en="Change SSID and password (min 8 chars)">SSID ve sifre degistir (min 8 karakter)</div>
    <div id="defPwWarn" style="display:none;background:#ef444415;border:1px solid #ef4444;border-radius:8px;padding:10px 12px;margin:8px 0;font-size:11px;color:#fca5a5;line-height:1.5">
      <div style="font-weight:600;color:#ef4444;margin-bottom:6px">&#9888; GUVENLIK UYARISI</div>
      <div data-tr="Hala default sifre kullaniliyor (tesla1234). Cihaza herkes baglanabilir!" data-en="Still using default password (tesla1234). Anyone can connect!">Hala default sifre kullaniliyor (<b>tesla1234</b>). Cihaza herkes baglanabilir!</div>
      <div style="margin-top:6px" data-tr="Lutfen yeni bir sifre belirleyin (min 8 karakter)." data-en="Please set a new password (min 8 chars).">Lutfen yeni bir sifre belirleyin (min 8 karakter).</div>
    </div>
    <div class="row" style="margin:6px 0"><span style="font-size:11px;color:var(--mu);width:45px">SSID:</span><input class="inp" id="wSSID" style="flex:1;width:auto;text-align:left" value=""></div>
    <div class="row" style="margin:6px 0"><span style="font-size:11px;color:var(--mu);width:45px" data-tr="Sifre:" data-en="Pass:">Sifre:</span><input class="inp" id="wPASS" type="password" style="flex:1;width:auto;text-align:left" value="" placeholder="min 8 karakter, default kullanilmaz"></div>
    <div class="row" style="margin:8px 0;gap:6px"><button class="mbtn" onclick="saveWifi()" style="background:#10b98118;border-color:var(--acc);color:var(--acc)" data-tr="Kaydet &amp; Yeniden Baslat" data-en="Save &amp; Restart">Kaydet &amp; Yeniden Baslat</button><button class="mbtn" onclick="resetWifi()" style="border-color:#ef444440;color:var(--red)" data-tr="Fabrika Ayari" data-en="Factory Reset">Fabrika Ayari</button></div>
    <div id="afterSaveHelp" style="display:none;background:#10b98115;border:1px solid #10b981;border-radius:8px;padding:10px 12px;margin:10px 0 0;font-size:11px;color:#86efac;line-height:1.6">
      <div style="font-weight:600;color:#10b981;margin-bottom:6px">&#10003; SIFRE DEGISTIRILDI</div>
      <div data-tr="1) Cihaz yeniden baslatildi. WiFi kesilecek." data-en="1) Device restarting. WiFi will disconnect.">1) Cihaz yeniden baslatildi. WiFi kesilecek.</div>
      <div data-tr="2) Telefonunuzdan/bilgisayarinizdan 'CanFeather' agina YENI sifreniz ile baglanin." data-en="2) Reconnect to 'CanFeather' from your phone/computer with the NEW password.">2) Telefonunuzdan/bilgisayarinizdan <b>CanFeather</b> agina <b>YENI sifreniz</b> ile baglanin.</div>
      <div data-tr="3) Baglanamiyorsaniz: Telefon WiFi ayarlarinda 'CanFeather' agini UNUT (Forget Network) yapip yeniden baglanin." data-en="3) If you can't connect: Forget the 'CanFeather' network in WiFi settings, then reconnect.">3) Baglanamiyorsaniz: Telefon WiFi ayarlarinda <b>CanFeather</b> agini <b>UNUT (Forget Network)</b> yapip yeniden baglanin.</div>
      <div style="margin-top:6px;color:#fca5a5" data-tr="Eski sifre cache'de kaldigi icin bazen otomatik baglanma calismaz." data-en="Auto-connect sometimes fails because the old password stays cached.">Eski sifre cache'de kaldigi icin bazen otomatik baglanma calismaz.</div>
    </div>
  </div>

  <div class="card">
    <div class="ct" style="margin-bottom:12px" data-tr="&#x1F6E1;&#xFE0F; Guvenlik" data-en="&#x1F6E1;&#xFE0F; Safety">&#x1F6E1;&#xFE0F; Guvenlik</div>
    <button class="db" onclick="disableAll()" data-tr="Tum enjeksiyonlari devre disi birak" data-en="Disable all injections">Tum enjeksiyonlari devre disi birak</button>
  </div>

  <div class="row" style="margin-top:12px;gap:8px">
    <button class="sb" style="flex:1" onclick="save()" data-tr="Kaydet &amp; EEPROM" data-en="Save &amp; EEPROM">Kaydet &amp; EEPROM</button>
    <button class="mbtn" onclick="location.href='/update'" title="C3 firmware OTA">OTA</button>
    <button class="mbtn" onclick="rollbackFw()" style="background:#f59e0b15;border-color:#f59e0b;color:#f59e0b" title="C3 firmware rollback">&#8634; Rollback</button>
  </div>
  <div style="margin-top:6px;font-size:10px;color:var(--mu);text-align:center">&#9888; OTA + Rollback sadece <b>C3 bridge</b> firmware'ini etkiler. RP2040 USB ile flash'lanir.</div>
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
function rollbackFw(){fetch('/partinfo').then(r=>r.json()).then(d=>{if(!d.canRollback){alert('Onceki C3 firmware bulunamadi. En az 1 OTA yapilmis olmasi lazim.');return;}const v=d.previous&&d.previous.version?d.previous.version:'?';if(!confirm('C3 bridge firmware\u0027ini onceki versiyona (v'+v+') dondur. RP2040 etkilenmez. Devam?'))return;fetch('/rollback').then(r=>r.json()).then(d=>{if(d.ok){toast(d.msg||'Rollback OK');}else{alert('Hata: '+(d.err||'bilinmeyen'));}}).catch(()=>toast('Baglanti hatasi'));}).catch(()=>alert('Partition bilgisi alinamadi'));}
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
  .then(()=>{const ng=document.getElementById('nag').checked;return cmd(ng?'nagon':'nagoff');})
  .then(()=>{const bt=document.getElementById('btlssc').checked;return cmd(bt?'btlsscon':'btlsscoff');})
  .then(()=>{const rc=document.getElementById('recov').checked;return cmd(rc?'recovon':'recovoff');})
  .then(()=>{const io=document.getElementById('isaOvr').checked;return cmd(io?'isaovron':'isaovroff');})
  .then(()=>{const im=parseInt(document.getElementById('isaMul').value)||7;return cmd('isamul '+im);})
  .then(()=>cmd('save'))
  .then(()=>toast(L[lang].saved))
  .catch(()=>toast(L[lang].connErr));
}
function applyFilter(){const flt=parseInt(document.getElementById('flt').value)||0;cmd('filter '+flt).then(()=>toast(L[lang].filterOk));}
function switchLogTab(tab){var ct=document.getElementById('lt-can'),st=document.getElementById('lt-serial'),cp=document.getElementById('ltp-can'),sp=document.getElementById('ltp-serial');if(tab==='can'){ct.classList.add('on');st.classList.remove('on');cp.style.display='';sp.style.display='none';}else{st.classList.add('on');ct.classList.remove('on');sp.style.display='';cp.style.display='none';}}
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
function saveWifi(){const s=document.getElementById('wSSID').value;const p=document.getElementById('wPASS').value;if(!s){toast('SSID bos olamaz');return;}if(p&&p.length<8){toast('Sifre min 8 karakter');return;}if(p==='tesla1234'){toast('Default sifre kullanilamaz!');return;}var msg='WiFi ayarlari kaydedilecek ve cihaz YENIDEN BASLATILACAK!\n\nSSID: '+s+(p?'\nYeni sifre belirlendi':'\nSifre degisikligi yok')+'\n\nBaglantiniz kesilecek. Telefonunuzdan/bilgisayarinizdan yeni sifre ile tekrar baglanmaniz gerekecek.\n\nDevam etmek istiyor musunuz?';if(!confirm(msg))return;const body={};if(s)body.ssid=s;if(p)body.pass=p;fetch('/wifi',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)}).then(r=>r.json()).then(d=>{if(d.err){toast('Hata: '+d.err);return;}var h=document.getElementById('afterSaveHelp');if(h)h.style.display='block';toast(d.msg||'Kaydedildi. Yeniden baslatiliyor...');}).catch(()=>toast('Hata'));}
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
  // v2.7: Versiyon ve default sifre uyarisi
  if(d.fwVersion){var fv=document.getElementById('fwVer');if(fv)fv.textContent=d.fwVersion;}
  var defB=document.getElementById('defPwBanner');var defW=document.getElementById('defPwWarn');
  if(defB)defB.style.display=d.mustChgPw?'block':'none';
  if(defW)defW.style.display=d.mustChgPw?'block':'none';
}
function drawSpark(hist){const c=document.getElementById('spark');if(!hist||!hist.length)return;const mx=Math.max(...hist,1);c.innerHTML=hist.map(v=>'<div style="height:'+Math.max(1,v/mx*38)+'px"></div>').join('');}
function drawFHist(events){const c=document.getElementById('fhist');if(!events||!events.length){c.innerHTML=L[lang].noEvent;return;}c.innerHTML=events.map(e=>'<div>'+(e.s?'<b style="color:#10b981">'+L[lang].fsdOn+'</b>':'<b style="color:#ef4444">'+L[lang].fsdOff+'</b>')+' <span style="color:#94a3b8">'+fmtTime(e.t)+'</span></div>').join('');}
function fetchStatus(){cmd('status').then(d=>{try{let j=JSON.parse(d);if(!synced){setHW(j.hw!=null?j.hw:1);setP(j.profile!=null?j.profile:2);document.getElementById('ft').checked=!!j.fsd;document.getElementById('ov').checked=!!j.ovr;document.getElementById('ci').checked=j.inj!==false;document.getElementById('ao').value=j.autoOff||0;document.getElementById('flt').value=j.logFilter||0;setCrystal(j.crystal!=null?j.crystal:1);document.getElementById('isa').checked=!!j.isa;document.getElementById('ev').checked=j.ev!==false;document.getElementById('nag').checked=!!j.nag;document.getElementById('btlssc').checked=!!j.btlssc;document.getElementById('recov').checked=j.recov!==false;document.getElementById('isaOvr').checked=!!j.isaOvr;document.getElementById('isaMul').value=j.isaMul||7;synced=true;}updateSB(j);syncDriveMode(j);}catch(e){}}).catch(()=>{});}
function fetchLog(){cmd('log').then(d=>{try{let j=JSON.parse(d);if(!j.lines||!j.lines.length)return;const b=document.getElementById('log');b.innerHTML=j.lines.map(l=>'<div class="ll">'+l+'</div>').join('');b.scrollTop=b.scrollHeight;}catch(e){}}).catch(()=>{});}
function fetchStats(){cmd('stats').then(d=>{try{let j=JSON.parse(d);drawSpark(j.hist);drawFHist(j.fsdHist);}catch(e){}}).catch(()=>{});}
function fetchSlog(){cmd('slog').then(d=>{try{let j=JSON.parse(d);if(!j.lines||!j.lines.length)return;const b=document.getElementById('slog');b.innerHTML=j.lines.map(l=>'<div class="ll">'+l+'</div>').join('');b.scrollTop=b.scrollHeight;}catch(e){}}).catch(()=>{});}
buildP();setLang(lang);fetchStatus();fetchStats();fetchSlog();loadWifiInfo();
setInterval(fetchStatus,2000);setInterval(fetchLog,2000);setInterval(fetchStats,5000);setInterval(fetchSlog,3000);
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
  // v2.7: status yanitina C3 tarafli ek alanlari enjekte et
  // (fwVersion/fwDate RP2040'tan geliyor; C3 mustChgPw + c3Version ekler)
  if (cmd == "status" && response.startsWith("{") && response.endsWith("}")) {
    bool mustChg = (strcmp(wifiPASS, DEFAULT_PASS) == 0);
    String extra = ",\"mustChgPw\":";
    extra += (mustChg ? "true" : "false");
    extra += ",\"c3Version\":\"" FW_VERSION "\",\"c3Date\":\"" FW_DATE "\"";
    // Eger RP2040 cevabinda fwVersion yoksa (eski RP2040) C3'unkini kullan
    if (response.indexOf("\"fwVersion\"") < 0) {
      extra += ",\"fwVersion\":\"" FW_VERSION "\",\"fwDate\":\"" FW_DATE "\"";
    }
    response = response.substring(0, response.length() - 1) + extra + "}";
  }
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
    if (p.length() < 8 || p.length() > 31) {
      server.send(400, "application/json", "{\"err\":\"Sifre min 8 karakter\"}");
      return;
    }
    // v2.7: Default sifre kullanimini engelle
    if (p == DEFAULT_PASS) {
      server.send(400, "application/json", "{\"err\":\"Default sifre kullanilamaz! Farkli bir sifre secin.\"}");
      return;
    }
    strncpy(wifiPASS, p.c_str(), 32);
    wifiPASS[32] = '\0';
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

  // v2.6: OTA — sadece C3 firmware'i guncellenir (RP2040 USB ile)
  server.on("/update", HTTP_GET, []() {
    server.send(200, "text/html",
      "<html><body style='font-family:sans-serif;background:#0f172a;color:#fff;padding:20px'>"
      "<h2>C3 Bridge OTA</h2>"
      "<p>Bu sayfa sadece <b>ESP32-C3 WiFi Bridge</b> firmware'ini gunceller. "
      "RP2040 firmware'i USB ile flashlanir.</p>"
      "<form method='POST' enctype='multipart/form-data'>"
      "<input type='file' name='update' accept='.bin' style='margin:10px 0'><br>"
      "<input type='submit' value='Yukle' style='padding:10px 20px;background:#10b981;color:#000;border:none;border-radius:6px;cursor:pointer'>"
      "</form><p><a href='/' style='color:#10b981'>&larr; Geri</a></p></body></html>");
  });
  server.on("/update", HTTP_POST, []() {
    server.send(200, "text/plain", Update.hasError() ? "HATA" : "OK");
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

  // v2.6: OTA Rollback (C3 dual-partition)
  server.on("/rollback", HTTP_GET, []() {
    const esp_partition_t* running = esp_ota_get_running_partition();
    const esp_partition_t* next = esp_ota_get_next_update_partition(NULL);
    if (!next || next == running) {
      server.send(400, "application/json", "{\"err\":\"Onceki firmware bulunamadi\"}");
      return;
    }
    esp_app_desc_t app_info;
    if (esp_ota_get_partition_description(next, &app_info) != ESP_OK) {
      server.send(400, "application/json", "{\"err\":\"Diger partition bos veya bozuk\"}");
      return;
    }
    esp_err_t err = esp_ota_set_boot_partition(next);
    if (err != ESP_OK) {
      char buf[80];
      snprintf(buf, sizeof(buf), "{\"err\":\"esp_ota_set_boot_partition: 0x%x\"}", err);
      server.send(500, "application/json", buf);
      return;
    }
    Serial.println("[rollback] C3: Onceki firmware'a donuluyor...");
    server.send(200, "application/json", "{\"ok\":true,\"msg\":\"Rollback OK\"}");
    delay(500);
    ESP.restart();
  });

  server.on("/partinfo", HTTP_GET, []() {
    const esp_partition_t* running = esp_ota_get_running_partition();
    const esp_partition_t* next = esp_ota_get_next_update_partition(NULL);
    StaticJsonDocument<384> d;
    d["mcu"] = "ESP32-C3";
    if (running) {
      d["current"]["label"] = running->label;
      d["current"]["size"] = running->size;
      esp_app_desc_t info;
      if (esp_ota_get_partition_description(running, &info) == ESP_OK) {
        d["current"]["version"] = info.version;
        d["current"]["date"] = info.date;
      }
    }
    if (next && next != running) {
      esp_app_desc_t info;
      if (esp_ota_get_partition_description(next, &info) == ESP_OK) {
        d["previous"]["label"] = next->label;
        d["previous"]["version"] = info.version;
        d["previous"]["date"] = info.date;
        d["canRollback"] = true;
      } else {
        d["canRollback"] = false;
        d["msg"] = "Onceki partition bos";
      }
    } else {
      d["canRollback"] = false;
    }
    String o; serializeJson(d, o);
    server.send(200, "application/json", o);
  });

  server.begin();
  Serial.println("[web] http://192.168.4.1");
  Serial.println("[ota] http://192.168.4.1/update (C3 firmware)");
  Serial.println("[ok] Bridge hazir.");
}

// ─────────────────────────────────────────────────────────────
//  LOOP
// ─────────────────────────────────────────────────────────────

void loop() {
  server.handleClient();
}

