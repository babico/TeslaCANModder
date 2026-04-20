#pragma once
// AUTO-GENERATED from dashboard.html — do not edit directly.
#include <pgmspace.h>

static const char DASH_HTML[] PROGMEM = R"HTML(<!doctype html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width,initial-scale=1" />
    <title>TeslaCANModder Dashboard</title>
    <style>
      * {
        box-sizing: border-box;
        margin: 0;
        padding: 0;
      }

      :root {
        --bg: #f4f6f8;
        --surface: #ffffff;
        --surface-2: #fffaf6;
        --text: #1f2937;
        --muted: #5f6b7a;
        --border: #d5dde6;
        --accent: #d73c1f;
        --accent-dark: #b93118;
        --ok: #0f8b57;
        --warn: #d68400;
        --shadow: 0 18px 34px rgba(22, 33, 49, 0.12);
      }

      body {
        min-height: 100vh;
        background:
          radial-gradient(1000px 460px at 0% 0%, #fff6eb 0%, transparent 70%),
          radial-gradient(800px 420px at 100% 100%, #e4edf6 0%, transparent 70%),
          var(--bg);
        color: var(--text);
        font-family: "Trebuchet MS", "Segoe UI", sans-serif;
        padding: 14px;
      }

      .wrap {
        max-width: 980px;
        margin: 0 auto;
        display: grid;
        gap: 12px;
      }

      .hero {
        background: linear-gradient(135deg, #111827, #253047);
        color: #f9fafb;
        border-radius: 18px;
        box-shadow: var(--shadow);
        border: 1px solid #374151;
        padding: 14px 16px;
        display: flex;
        justify-content: space-between;
        align-items: center;
        gap: 10px;
        flex-wrap: wrap;
      }

      .hero h1 {
        font-size: 20px;
        font-weight: 700;
        letter-spacing: 0.2px;
      }

      .hero p {
        color: #cbd5e1;
        font-size: 12px;
      }

      .pulse {
        width: 10px;
        height: 10px;
        border-radius: 999px;
        background: #22c55e;
        margin-right: 8px;
        display: inline-block;
        animation: pulse 1.8s infinite;
      }

      @keyframes pulse {
        0%,
        100% {
          opacity: 1;
          transform: scale(1);
        }
        50% {
          opacity: 0.35;
          transform: scale(0.85);
        }
      }

      .chips {
        display: flex;
        flex-wrap: wrap;
        gap: 6px;
      }

      .chip {
        border-radius: 999px;
        padding: 5px 10px;
        font-size: 11px;
        font-weight: 700;
        border: 1px solid rgba(255, 255, 255, 0.25);
        color: #e5e7eb;
      }

      .grid {
        display: grid;
        grid-template-columns: repeat(12, minmax(0, 1fr));
        gap: 12px;
      }

      .card {
        background: var(--surface);
        border: 1px solid var(--border);
        border-radius: 16px;
        box-shadow: var(--shadow);
        padding: 14px;
      }

      .col-12 {
        grid-column: span 12;
      }

      .col-6 {
        grid-column: span 6;
      }

      .title {
        font-size: 14px;
        font-weight: 800;
        margin-bottom: 10px;
      }

      .kv {
        font-family: ui-monospace, Menlo, Consolas, monospace;
        font-size: 12px;
        line-height: 1.8;
        color: #374151;
      }

      .kv b {
        color: var(--accent);
      }

      .seg {
        display: grid;
        grid-template-columns: repeat(3, minmax(0, 1fr));
        gap: 6px;
      }

      .seg.wide {
        grid-template-columns: repeat(6, minmax(0, 1fr));
      }

      .seg button,
      .btn {
        background: var(--surface-2);
        border: 1px solid var(--border);
        color: var(--muted);
        border-radius: 10px;
        padding: 9px 8px;
        font-size: 12px;
        font-weight: 700;
        cursor: pointer;
        transition: 0.16s ease;
      }

      .seg button:hover,
      .btn:hover {
        border-color: var(--accent);
        color: var(--accent);
      }

      .seg button.on {
        background: #ffe8e9;
        border-color: #f8b4b6;
        color: var(--accent);
      }

      .row {
        display: flex;
        justify-content: space-between;
        align-items: center;
        gap: 8px;
        margin-bottom: 10px;
      }

      .switch {
        position: relative;
        width: 46px;
        height: 26px;
      }

      .switch input {
        display: none;
      }

      .slider {
        position: absolute;
        inset: 0;
        background: #d1d5db;
        border-radius: 999px;
        cursor: pointer;
        transition: 0.2s;
      }

      .slider:before {
        content: "";
        position: absolute;
        width: 18px;
        height: 18px;
        left: 4px;
        top: 4px;
        border-radius: 999px;
        background: #ffffff;
        box-shadow: 0 1px 4px rgba(0, 0, 0, 0.25);
        transition: 0.2s;
      }

      .switch input:checked + .slider {
        background: #fecaca;
      }

      .switch input:checked + .slider:before {
        transform: translateX(20px);
        background: var(--accent);
      }

      .input {
        width: 100%;
        border: 1px solid var(--border);
        border-radius: 10px;
        padding: 9px 10px;
        background: #ffffff;
        color: var(--text);
        font-size: 12px;
      }

      .label {
        display: block;
        color: var(--muted);
        font-size: 11px;
        margin: 8px 0 5px;
      }

      .mono {
        font-family: ui-monospace, Menlo, Consolas, monospace;
      }

      .log {
        background: #111827;
        color: #d1d5db;
        border-radius: 10px;
        border: 1px solid #374151;
        min-height: 96px;
        max-height: 140px;
        overflow: auto;
        padding: 8px;
        font-size: 11px;
        line-height: 1.7;
        font-family: ui-monospace, Menlo, Consolas, monospace;
      }

      .status-line {
        color: var(--muted);
        font-size: 11px;
        margin-top: 6px;
      }

      .toast {
        position: fixed;
        bottom: 18px;
        left: 50%;
        transform: translateX(-50%) translateY(40px);
        opacity: 0;
        transition: 0.2s ease;
        background: var(--accent);
        color: #fff;
        padding: 8px 14px;
        border-radius: 10px;
        font-size: 12px;
        font-weight: 700;
        z-index: 20;
      }

      .toast.show {
        transform: translateX(-50%) translateY(0);
        opacity: 1;
      }

      @media (max-width: 840px) {
        .col-6 {
          grid-column: span 12;
        }

        .seg.wide {
          grid-template-columns: repeat(3, minmax(0, 1fr));
        }
      }
    </style>
  </head>
  <body>
    <div class="wrap">
      <section class="hero">
        <div>
          <h1>TeslaCANModder Control Deck</h1>
          <p><span class="pulse"></span>Live vehicle state, firmware controls, and transport diagnostics</p>
        </div>
        <div class="chips">
          <span class="chip" id="chipVariant">Variant: ?</span>
          <span class="chip" id="chipCan">CAN: ?</span>
          <span class="chip" id="chipUptime">Up: 0s</span>
        </div>
      </section>

      <section class="grid">
        <div class="card col-6">
          <div class="title">Board Status</div>
          <div class="kv" id="st">Loading...</div>
        </div>

        <div class="card col-6">
          <div class="title">Hardware Variant</div>
          <div class="seg" id="hwSeg">
            <button onclick="setVariant('hw4')">HW4</button>
            <button onclick="setVariant('hw3')">HW3</button>
            <button onclick="setVariant('legacy')">Legacy</button>
          </div>
          <div class="status-line">Choose the active firmware mapping mode.</div>
        </div>

        <div class="card col-6">
          <div class="title">FSD Controls</div>
          <div class="row">
            <span>FSD Enabled</span>
            <label class="switch"><input type="checkbox" id="fsdTg" onchange="toggleFsd()" /><span class="slider"></span></label>
          </div>
          <div class="row">
            <span>Nag Suppress</span>
            <label class="switch"><input type="checkbox" id="nagTg" onchange="toggleNag()" /><span class="slider"></span></label>
          </div>
          <div class="row">
            <span>ISA Chime Suppress</span>
            <label class="switch"><input type="checkbox" id="isaTg" onchange="toggleIsa()" /><span class="slider"></span></label>
          </div>
          <div class="row" style="margin-bottom: 4px">
            <span>Mirror Auto-Fold on Lock</span>
            <label class="switch"><input type="checkbox" id="mirrorAutoTg" onchange="toggleMirrorAutoFold()" /><span class="slider"></span></label>
          </div>

          <div class="title" style="margin-top: 10px; margin-bottom: 8px">Speed Profile</div>
          <div class="seg wide" id="spSeg">
            <button onclick="setProfile(0)">Chill</button>
            <button onclick="setProfile(1)">Normal</button>
            <button onclick="setProfile(2)">Hurry</button>
            <button onclick="setProfile(3)">Max</button>
            <button onclick="setProfile(4)">Sloth</button>
            <button onclick="cmd('profile:auto')">Auto</button>
          </div>
        </div>

        <div class="card col-6">
          <div class="title">Vehicle Actions</div>
          <div class="seg wide">
            <button onclick="cmd('lock')">Lock</button>
            <button onclick="cmd('unlock')">Unlock</button>
            <button onclick="cmd('horn')">Horn</button>
            <button onclick="cmd('frunk')">Frunk</button>
            <button onclick="cmd('trunk:open')">Trunk</button>
            <button onclick="cmd('sentry:on')">Sentry</button>
          </div>
          <div class="seg wide" style="margin-top: 8px">
            <button onclick="cmd('mirror:fold')">Mirror Fold</button>
            <button onclick="cmd('mirror:unfold')">Mirror Unfold</button>
            <button onclick="cmd('climate:keep')">Climate Keep</button>
            <button onclick="cmd('climate:off')">Climate Off</button>
            <button onclick="cmd('summon:fwd')">Summon Fwd</button>
            <button onclick="cmd('summon:stop')">Summon Stop</button>
          </div>

          <div class="title" style="margin-top: 12px; margin-bottom: 8px">Window Vent Position</div>
          <input type="range" min="0" max="100" value="0" id="ventRange" oninput="ventPreview()" />
          <div class="row" style="margin-top: 8px; margin-bottom: 0">
            <span class="mono">window:vent:<span id="ventVal">0</span></span>
            <button class="btn" onclick="sendVent()">Send</button>
          </div>
        </div>

        <div class="card col-6">
          <div class="title">WiFi Settings</div>
          <div class="kv" id="wifiSt">Loading...</div>
          <div class="title" style="margin-top: 10px; margin-bottom: 8px">Mode</div>
          <div class="seg" id="wifiModeSeg">
            <button onclick="setWifiMode('ap')">AP</button>
            <button onclick="setWifiMode('sta')">STA</button>
          </div>
          <div id="staFields" style="display: none">
            <label class="label">Station SSID</label>
            <input type="text" class="input" id="staSSID" placeholder="WiFi network name" maxlength="32" />
            <label class="label">Station Password</label>
            <input type="password" class="input" id="staPW" placeholder="WiFi password" maxlength="63" />
            <div style="margin-top: 8px"><button class="btn" onclick="connectSTA()">Connect STA</button></div>
          </div>
          <div id="apFields" style="display: none">
            <label class="label">AP SSID</label>
            <input type="text" class="input" id="apSSID" placeholder="Access point name" maxlength="32" />
            <label class="label">AP Password</label>
            <input type="password" class="input" id="apPW" placeholder="AP password (min 8 chars)" maxlength="63" />
            <div style="margin-top: 8px"><button class="btn" onclick="saveAP()">Save AP Config</button></div>
          </div>
        </div>

        <div class="card col-6" id="bleCard">
          <div class="title">BLE Settings</div>
          <div class="kv" id="bleSt">Loading...</div>
          <div class="row" style="margin-top: 10px; margin-bottom: 0">
            <span>BLE Enabled</span>
            <label class="switch"><input type="checkbox" id="bleTg" onchange="toggleBle()" /><span class="slider"></span></label>
          </div>
        </div>

        <div class="card col-12">
          <div class="title">System Log</div>
          <div class="log" id="logBox"></div>
        </div>
      </section>
    </div>

    <div class="toast" id="toast"></div>

    <script>
      let S = {},
        W = {},
        B = {};

      function $(id) {
        return document.getElementById(id);
      }

      function appendLog(text) {
        const box = $("logBox");
        const stamp = new Date().toLocaleTimeString();
        box.innerHTML = `[${stamp}] ${text}<br>` + box.innerHTML;
      }

      function toast(m) {
        const t = $("toast");
        t.textContent = m;
        t.classList.add("show");
        setTimeout(() => t.classList.remove("show"), 1500);
      }

      function cmd(c) {
        fetch("/api/command", {
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify({ cmd: c }),
        })
          .then((r) => r.json())
          .then((d) => {
            S = d;
            render();
            appendLog(`CMD ${c}`);
            toast(c);
          })
          .catch(() => toast("Error"));
      }

      function toggleFsd() {
        cmd(S.fsd ? "fsd:off" : "fsd:on");
      }

      function toggleNag() {
        cmd(S.nag ? "nag:off" : "nag:on");
      }

      function toggleIsa() {
        cmd(S.isaChime ? "isa-chime:off" : "isa-chime:on");
      }

      function toggleMirrorAutoFold() {
        cmd(S.mirrorAutoFold ? "mirror:autofold:off" : "mirror:autofold:on");
      }

      function setVariant(v) {
        cmd("variant:" + v);
      }

      function setProfile(p) {
        cmd("profile:" + p);
      }

      function ventPreview() {
        $("ventVal").textContent = $("ventRange").value;
      }

      function sendVent() {
        const v = parseInt($("ventRange").value || "0", 10);
        cmd(`window:vent:${v}`);
      }

      function setWifiMode(m) {
        if (m === "sta") {
          $("staFields").style.display = "block";
          $("apFields").style.display = "none";
        } else {
          $("staFields").style.display = "none";
          $("apFields").style.display = "block";
        }
        document.querySelectorAll("#wifiModeSeg button").forEach((b) => b.classList.toggle("on", b.textContent.toLowerCase() === m));
      }

      function connectSTA() {
        const ssid = $("staSSID").value.trim();
        const pw = $("staPW").value;
        if (!ssid) {
          toast("Enter SSID");
          return;
        }
        fetch("/api/wifi/config", {
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify({ mode: "sta", ssid: ssid, password: pw }),
        })
          .then((r) => r.json())
          .then((d) => {
            W = d;
            renderWifi();
            appendLog(`WIFI STA ${ssid}`);
            toast("Connecting to " + ssid + "...");
          })
          .catch(() => toast("Error"));
      }

      function saveAP() {
        const ssid = $("apSSID").value.trim();
        const pw = $("apPW").value;
        if (!ssid) {
          toast("Enter AP SSID");
          return;
        }
        if (pw.length > 0 && pw.length < 8) {
          toast("AP password must be 8+ chars");
          return;
        }
        fetch("/api/wifi/config", {
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify({ mode: "ap", ssid: ssid, password: pw }),
        })
          .then((r) => r.json())
          .then((d) => {
            W = d;
            renderWifi();
            appendLog(`WIFI AP ${ssid}`);
            toast("AP config saved");
          })
          .catch(() => toast("Error"));
      }

      function toggleBle() {
        const en = $("bleTg").checked;
        fetch("/api/ble/config", {
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify({ enabled: en }),
        })
          .then((r) => r.json())
          .then((d) => {
            B = d;
            renderBle();
            appendLog(en ? "BLE enabled" : "BLE disabled");
            toast(en ? "BLE enabled" : "BLE disabled");
          })
          .catch(() => toast("Error"));
      }

      function renderWifi() {
        let h = "<b>Mode:</b> " + (W.mode || "?").toUpperCase();
        h += " &nbsp; <b>IP:</b> " + (W.ip || "N/A");
        if (W.mode === "sta") {
          h += "<br><b>SSID:</b> " + (W.ssid || "N/A");
          h += " &nbsp; <b>RSSI:</b> " + (W.rssi || "?") + " dBm";
          h += "<br><b>Status:</b> " + (W.connected ? "Connected" : "Disconnected");
        } else {
          h += "<br><b>SSID:</b> " + (W.ssid || "N/A");
          h += " &nbsp; <b>Clients:</b> " + (W.clients || 0);
          h += "<br><b>Channel:</b> " + (W.channel || "?");
        }
        $("wifiSt").innerHTML = h;
        document.querySelectorAll("#wifiModeSeg button").forEach((b) => b.classList.toggle("on", b.textContent.toLowerCase() === W.mode));
        if (W.mode === "sta") {
          $("staFields").style.display = "block";
          $("apFields").style.display = "none";
          if (W.ssid) $("staSSID").placeholder = W.ssid;
        } else {
          $("staFields").style.display = "none";
          $("apFields").style.display = "block";
          if (W.ssid) $("apSSID").placeholder = W.ssid;
        }
      }

      function renderBle() {
        let h = "<b>Status:</b> " + (B.enabled ? "Enabled" : "Disabled");
        h += " &nbsp; <b>Connected:</b> " + (B.connected ? "Yes" : "No");
        if (B.deviceName) h += "<br><b>Device:</b> " + B.deviceName;
        $("bleSt").innerHTML = h;
        $("bleTg").checked = !!B.enabled;
      }

      function render() {
        let h = "<b>Variant:</b> " + (S.variant || "?");
        h += " &nbsp; <b>CAN:</b> " + (S.chassisOnline ? "Online" : "Offline");
        h += "<br><b>FSD:</b> " + (S.fsd ? "ON" : "OFF");
        h += " &nbsp; <b>Nag:</b> " + (S.nag ? "ON" : "OFF");
        h += " &nbsp; <b>Mirror Auto:</b> " + (S.mirrorAutoFold ? "ON" : "OFF");
        const pn = ["Chill", "Normal", "Hurry", "Max", "Sloth"][S.profile] || S.profile;
        h += "<br><b>Profile:</b> " + pn + (S.profilePin ? " (pinned)" : " (auto)");
        h += " &nbsp; <b>ISA:</b> " + (S.isaChime ? "Suppressed" : "Normal");
        h += "<br><b>Uptime:</b> " + Math.floor((S.uptime || 0) / 1000) + "s";
        if (S.hardware) {
          const b = [];
          if (S.hardware.busChassis) b.push("Chassis");
          if (S.hardware.busVehicle) b.push("Vehicle");
          if (S.hardware.busBody) b.push("Body");
          h += " &nbsp; <b>Buses:</b> " + (b.length ? b.join(" + ") : "none");
        }
        $("st").innerHTML = h;

        $("chipVariant").textContent = "Variant: " + (S.variant || "?");
        $("chipCan").textContent = "CAN: " + (S.chassisOnline ? "Online" : "Offline");
        $("chipUptime").textContent = "Up: " + Math.floor((S.uptime || 0) / 1000) + "s";

        $("fsdTg").checked = !!S.fsd;
        $("nagTg").checked = !!S.nag;
        $("isaTg").checked = !!S.isaChime;
        $("mirrorAutoTg").checked = !!S.mirrorAutoFold;

        document.querySelectorAll("#hwSeg button").forEach((b) => b.classList.toggle("on", b.textContent.toLowerCase() === S.variant));
        document.querySelectorAll("#spSeg button").forEach((b, i) => {
          if (i < 5) b.classList.toggle("on", S.profilePin && S.profile === i);
          if (i === 5) b.classList.toggle("on", !S.profilePin);
        });
      }

      function poll() {
        fetch("/api/status")
          .then((r) => r.json())
          .then((d) => {
            S = d;
            render();
          })
          .catch(() => {});

        fetch("/api/wifi/status")
          .then((r) => r.json())
          .then((d) => {
            W = d;
            renderWifi();
          })
          .catch(() => {});

        fetch("/api/ble/status")
          .then((r) => r.json())
          .then((d) => {
            B = d;
            renderBle();
          })
          .catch(() => {});
      }

      setInterval(poll, 2000);
      ventPreview();
      appendLog("Dashboard booted");
      poll();
    </script>
  </body>
</html>
)HTML";
