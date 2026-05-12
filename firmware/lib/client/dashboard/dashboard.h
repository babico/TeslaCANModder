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

			.warning-box {
				background: #fff4e5;
				border: 1px solid #f2c16d;
				border-radius: 10px;
				padding: 8px 10px;
				font-size: 11px;
				font-weight: 700;
				color: #7a4900;
				margin-bottom: 8px;
			}

			.hidden-card {
				display: none;
			}

			.hidden-card.revealed {
				display: block;
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
					<p>
						<span class="pulse"></span>Live vehicle state, firmware controls, and
						transport diagnostics
					</p>
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
					<div class="status-line" style="margin-bottom: 8px">
						Driver-assist toggles for FSD behavior, nag handling, and speed-profile
						shaping.
					</div>
					<div class="row">
						<span>FSD Enabled</span>
						<label class="switch"
							><input type="checkbox" id="fsdTg" onchange="toggleFsd()" /><span
								class="slider"
							></span
						></label>
					</div>
					<div class="row">
						<span>Nag Mode</span>
						<select
							class="input"
							id="nagModeSel"
							onchange="setNagMode(this.value)"
							style="width: auto"
						>
							<option value="off">off — no suppression</option>
							<option value="bit19">bit19 — clear ECE R79 bit</option>
							<option value="legacy">legacy — EPAS echo, fixed 0 Nm</option>
							<option value="safe">safe — DAS-gated echo</option>
							<option value="natural">natural — Gaussian 0.08-0.18 Nm</option>
							<option value="organic">organic — DAS state machine</option>
							<option value="full">full — bit19 + organic</option>
						</select>
					</div>
					<div class="row">
						<span>Nag Organic Driver Bypass</span>
						<label class="switch"
							><input
								type="checkbox"
								id="nagBypassTg"
								onchange="toggleNagBypass()"
							/><span class="slider"></span
						></label>
					</div>
					<div class="row">
						<span>ISA Chime Suppress</span>
						<label class="switch"
							><input type="checkbox" id="isaTg" onchange="toggleIsa()" /><span
								class="slider"
							></span
						></label>
					</div>
					<div class="row" style="margin-bottom: 4px">
						<span>Mirror Auto-Fold on Lock</span>
						<label class="switch"
							><input
								type="checkbox"
								id="mirrorAutoTg"
								onchange="toggleMirrorAutoFold()" /><span class="slider"></span
						></label>
					</div>

					<div class="title" style="margin-top: 10px; margin-bottom: 8px">
						Speed Profile
					</div>
					<div class="seg wide" id="spSeg">
						<button onclick="setProfile(0)">Chill</button>
						<button onclick="setProfile(1)">Normal</button>
						<button onclick="setProfile(2)">Hurry</button>
						<button onclick="setProfile(3)">Max</button>
						<button onclick="setProfile(4)">Sloth</button>
						<button onclick="cmd('profile:auto')">Auto</button>
					</div>

					<div class="title" style="margin-top: 14px; margin-bottom: 6px">
						Gamepad / DAS Drive Speed
					</div>
					<div class="warning-box">
						Raises the absolute longitudinal-request ceiling sent to the car. Default 25
						km/h is parking-lot safe; higher values are for closed-track /
						private-property use only. Hard upper bound is the protocol byte limit.
					</div>
					<div class="row">
						<span>DAS Drive Enabled</span>
						<label class="switch"
							><input
								type="checkbox"
								id="dasDriveTg"
								onchange="toggleDasDrive()" /><span class="slider"></span
						></label>
					</div>
					<div class="row" style="gap: 8px; flex-wrap: wrap">
						<span>User limit (km/h)</span>
						<input
							type="number"
							id="dasSpeedLimitIn"
							min="1"
							style="width: 70px"
							onchange="sendDasSpeedLimit()"
						/>
						<span>Hard cap (km/h)</span>
						<input
							type="number"
							id="dasSpeedCapIn"
							min="1"
							style="width: 70px"
							onchange="sendDasSpeedCap()"
						/>
						<span class="status-line" id="dasSpeedCapMaxLbl"></span>
					</div>
				</div>

				<div class="card col-6">
					<div class="title">AP Injection Gate</div>
					<div class="warning-box">
						High-risk safety control. Disabling the gate can permit live-frame injection
						paths.
					</div>
					<div class="status-line" style="margin-bottom: 8px">
						Safety gate for write/injection paths. Closed gate blocks high-risk transmit
						actions.
					</div>
					<div class="row">
						<span>Gate Enabled</span>
						<label class="switch"
							><input type="checkbox" id="apGateTg" onchange="toggleApGate()" /><span
								class="slider"
							></span
						></label>
					</div>
					<div class="kv" id="apGateSt" style="margin-top: 8px">Loading...</div>
				</div>

				<div class="card col-6">
					<div class="title">Write Probe (TX to RX Confirmation)</div>
					<div class="status-line" style="margin-bottom: 8px">
						Sends a controlled command and verifies response echo plus follow-up status
						confirmation.
					</div>
					<label class="label">Probe Command</label>
					<select class="input" id="probeCmd">
						<option value="apgate:on">apgate:on → apGateEnabled=true</option>
						<option value="apgate:off">apgate:off → apGateEnabled=false</option>
						<option value="nag:mode:off">nag:mode:off → nagMode=off</option>
						<option value="nag:mode:organic">nag:mode:organic → nagMode=organic</option>
						<option value="mirror:autofold:on">
							mirror:autofold:on → mirrorAutoFold=true
						</option>
						<option value="mirror:autofold:off">
							mirror:autofold:off → mirrorAutoFold=false
						</option>
					</select>
					<div style="margin-top: 8px">
						<button class="btn" onclick="runWriteProbe()">Run Probe</button>
					</div>
					<div class="kv" id="probeSt" style="margin-top: 8px"><b>Status:</b> idle</div>
				</div>

				<div class="card col-6">
					<div class="title">Vehicle Actions</div>
					<div class="warning-box">
						These commands affect a live vehicle. Confirm each action and use only when
						safe.
					</div>
					<div class="status-line" style="margin-bottom: 8px">
						One-shot cabin/body commands. Use carefully while connected to a live
						vehicle.
					</div>
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

					<div class="title" style="margin-top: 12px; margin-bottom: 8px">
						Window Vent Position
					</div>
					<input
						type="range"
						min="0"
						max="100"
						value="0"
						id="ventRange"
						oninput="ventPreview()"
					/>
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
						<input
							type="text"
							class="input"
							id="staSSID"
							placeholder="WiFi network name"
							maxlength="32"
						/>
						<label class="label">Station Password</label>
						<input
							type="password"
							class="input"
							id="staPW"
							placeholder="WiFi password"
							maxlength="63"
						/>
						<div style="margin-top: 8px">
							<button class="btn" onclick="connectSTA()">Connect STA</button>
						</div>
					</div>
					<div id="apFields" style="display: none">
						<label class="label">AP SSID</label>
						<input
							type="text"
							class="input"
							id="apSSID"
							placeholder="Access point name"
							maxlength="32"
						/>
						<label class="label">AP Password</label>
						<input
							type="password"
							class="input"
							id="apPW"
							placeholder="AP password (min 8 chars)"
							maxlength="63"
						/>
						<div style="margin-top: 8px">
							<button class="btn" onclick="saveAP()">Save AP Config</button>
						</div>
					</div>
				</div>

				<div class="card col-6" id="bleCard">
					<div class="title">BLE Settings</div>
					<div class="kv" id="bleSt">Loading...</div>
					<div class="row" style="margin-top: 10px; margin-bottom: 0">
						<span>BLE Enabled</span>
						<label class="switch"
							><input type="checkbox" id="bleTg" onchange="toggleBle()" /><span
								class="slider"
							></span
						></label>
					</div>
				</div>

				<div class="card col-6" id="gamepadCard">
					<div class="title">Gamepad Control</div>
					<div class="kv" id="gpSt">Loading...</div>
					<div class="seg wide" style="margin-top: 10px">
						<button onclick="gpScan()">Scan</button>
						<button onclick="gpUnpair()">Unpair</button>
					</div>
					<div id="gpScanResults" style="display: none; margin-top: 10px">
						<div style="font-size: 12px; color: var(--muted); margin-bottom: 6px">
							Found HID devices:
						</div>
						<div id="gpDeviceList"></div>
					</div>
					<div style="margin-top: 14px">
						<div
							style="
								font-size: 12px;
								font-weight: 600;
								color: var(--muted);
								margin-bottom: 6px;
								text-transform: uppercase;
								letter-spacing: 0.05em;
							"
						>
							Button Bindings
						</div>
						<table
							id="gpBindTable"
							style="width: 100%; border-collapse: collapse; font-size: 12px"
						>
							<thead>
								<tr>
									<th
										style="
											text-align: left;
											padding: 3px 6px;
											color: var(--muted);
											font-weight: 600;
										"
									>
										Button
									</th>
									<th
										style="
											text-align: left;
											padding: 3px 6px;
											color: var(--muted);
											font-weight: 600;
										"
									>
										Command
									</th>
									<th style="padding: 3px 6px"></th>
								</tr>
							</thead>
							<tbody id="gpBindBody"></tbody>
						</table>
					</div>
				</div>

				<div class="card col-6">
					<div class="title">CAN Recorder</div>
					<div class="status-line" style="margin-bottom: 8px">
						Capture inbound CAN frames to a bounded ring buffer for troubleshooting.
					</div>
					<div class="seg wide">
						<button onclick="startRecorder()">Start</button>
						<button onclick="stopRecorder()">Stop</button>
						<button onclick="downloadRecorder()">Download CSV</button>
					</div>
					<div class="kv" id="recSt" style="margin-top: 8px">Loading...</div>
				</div>

				<div class="card col-6 hidden-card" id="hiddenApCard">
					<div class="title">Hidden AP Setting (Advanced)</div>
					<div class="warning-box">
						Experimental control. Keep disabled unless you understand EAP/Summon side
						effects.
					</div>
					<div class="row">
						<span>Enhanced Autopilot (EAP bit 46)</span>
						<label class="switch"
							><input
								type="checkbox"
								id="hiddenApTg"
								onchange="toggleHiddenAp()" /><span class="slider"></span
						></label>
					</div>
					<div class="kv" id="hiddenApSt">Hidden until revealed.</div>
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
				B = {},
				R = {},
				G = {};

			const HIGH_RISK_COMMANDS = new Set([
				"lock",
				"unlock",
				"horn",
				"frunk",
				"trunk:open",
				"sentry:on",
				"mirror:fold",
				"mirror:unfold",
				"climate:keep",
				"climate:off",
				"summon:fwd",
				"summon:stop",
				"apgate:off",
			]);

			const WRITE_PROBE_CASES = {
				"apgate:on": { key: "apGateEnabled", expected: true },
				"apgate:off": { key: "apGateEnabled", expected: false },
				"nag:mode:off": { key: "nagMode", expected: "off" },
				"nag:mode:organic": { key: "nagMode", expected: "organic" },
				"mirror:autofold:on": { key: "mirrorAutoFold", expected: true },
				"mirror:autofold:off": { key: "mirrorAutoFold", expected: false },
			};

			let hiddenApVisible = false;

			function $(id) {
				return document.getElementById(id);
			}

			function escapeHtml(str) {
				if (str == null) return "";
				return String(str)
					.replace(/&/g, "&amp;")
					.replace(/</g, "&lt;")
					.replace(/>/g, "&gt;")
					.replace(/"/g, "&quot;")
					.replace(/'/g, "&#39;");
			}

			function appendLog(text) {
				const box = $("logBox");
				const stamp = new Date().toLocaleTimeString();
				box.innerHTML = `[${escapeHtml(stamp)}] ${escapeHtml(text)}<br>` + box.innerHTML;
			}

			function toast(m) {
				const t = $("toast");
				t.textContent = m;
				t.classList.add("show");
				setTimeout(() => t.classList.remove("show"), 1500);
			}

			function isHighRiskCommand(c) {
				if (HIGH_RISK_COMMANDS.has(c)) return true;
				if (c.startsWith("window:vent:")) return true;
				if (c.startsWith("summon:")) return true;
				return false;
			}

			function cmd(c, opts) {
				if (!(opts && opts.skipConfirm) && isHighRiskCommand(c)) {
					const proceed = window.confirm(
						`Confirm high-risk command: ${c}. Continue only if vehicle conditions are safe.`,
					);
					if (!proceed) {
						appendLog(`Canceled ${c}`);
						toast("Canceled");
						return Promise.resolve(null);
					}
				}

				return fetch("/api/command", {
					method: "POST",
					headers: { "Content-Type": "application/json" },
					body: JSON.stringify({ cmd: c }),
				})
					.then((r) => {
						if (!r.ok) throw new Error(r.status);
						// POST returns RpcResponse Ack — fetch updated state separately.
						return fetch("/api/status");
					})
					.then((r) => r.json())
					.then((d) => {
						S = d;
						render();
						appendLog(`CMD ${c}`);
						toast(c);
						return d;
					})
					.catch(() => {
						toast("Error");
						return null;
					});
			}

			function probeStateHtml(status, detail, ok) {
				const color = ok ? "var(--ok)" : "var(--accent)";
				return `<b>Status:</b> <span style="color:${color}">${escapeHtml(status)}</span><br><b>Detail:</b> ${escapeHtml(detail)}`;
			}

			async function runWriteProbe() {
				const probeCmd = $("probeCmd").value;
				const probeCfg = WRITE_PROBE_CASES[probeCmd];
				if (!probeCfg) {
					$("probeSt").innerHTML = probeStateHtml(
						"FAILED",
						"Unknown probe command",
						false,
					);
					return;
				}

				const started = Date.now();
				$("probeSt").innerHTML = "<b>Status:</b> running...";
				appendLog(`PROBE TX ${probeCmd}`);

				const txAck = await cmd(probeCmd, { skipConfirm: true });
				if (!txAck) {
					$("probeSt").innerHTML = probeStateHtml(
						"FAILED",
						"No command acknowledgment from API",
						false,
					);
					appendLog(`PROBE FAIL ${probeCmd}: no ack`);
					return;
				}

				const txOk = txAck[probeCfg.key] === probeCfg.expected;
				let rxOk = txOk;
				let verify = txAck;

				for (let i = 0; !rxOk && i < 4; i++) {
					await new Promise((resolve) => setTimeout(resolve, 250));
					try {
						verify = await fetch("/api/status").then((r) => r.json());
						rxOk = verify[probeCfg.key] === probeCfg.expected;
					} catch (_err) {
						rxOk = false;
					}
				}

				const elapsed = Date.now() - started;
				const summary = `${probeCfg.key}=${String(verify[probeCfg.key])}, expected=${String(probeCfg.expected)}, ${elapsed}ms`;
				if (txOk && rxOk) {
					$("probeSt").innerHTML = probeStateHtml("PASS", summary, true);
					appendLog(`PROBE PASS ${probeCmd} (${elapsed}ms)`);
					return;
				}

				$("probeSt").innerHTML = probeStateHtml("FAILED", summary, false);
				appendLog(`PROBE FAIL ${probeCmd} (${elapsed}ms)`);
			}

			function toggleFsd() {
				cmd(S.fsd ? "fsd:off" : "fsd:on");
			}

			function setNagMode(mode) {
				cmd("nag:mode:" + mode);
			}

			function toggleNagBypass() {
				cmd(S.nagOrgBypass ? "nag:bypass:off" : "nag:bypass:on");
			}

			function toggleIsa() {
				cmd(S.isaChime ? "isa-chime:off" : "isa-chime:on");
			}

			function toggleMirrorAutoFold() {
				cmd(S.mirrorAutoFold ? "mirror:autofold:off" : "mirror:autofold:on");
			}

			function toggleApGate() {
				cmd(S.apGateEnabled ? "apgate:off" : "apgate:on");
			}

			function setHiddenApVisibility(visible) {
				hiddenApVisible = visible;
				$("hiddenApCard").classList.toggle("revealed", visible);
				if (visible) {
					appendLog("Hidden AP controls revealed");
					toast("Advanced AP controls revealed");
				}
			}

			function toggleHiddenAp() {
				cmd(S.enhancedAutopilot ? "eap:off" : "eap:on");
			}

			function setVariant(v) {
				cmd("variant:" + v);
			}

			function setProfile(p) {
				cmd("profile:" + p);
			}

			function toggleDasDrive() {
				cmd(S.dasDriveEnabled ? "drive:off" : "drive:on");
			}

			function sendDasSpeedLimit() {
				const v = parseInt($("dasSpeedLimitIn").value || "0", 10);
				if (v >= 1) cmd("drive:speed:" + v);
			}

			function sendDasSpeedCap() {
				const v = parseInt($("dasSpeedCapIn").value || "0", 10);
				if (v < 1) return;
				const max = S.dasSpeedCapMaxKph || 200;
				if (v > max) {
					alert("Cap cannot exceed protocol max " + max + " km/h.");
					return;
				}
				if (
					v > 25 &&
					!confirm(
						"Raising the DAS hard cap above 25 km/h is for closed-track use only. Continue?",
					)
				)
					return;
				cmd("drive:cap:" + v);
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
				document
					.querySelectorAll("#wifiModeSeg button")
					.forEach((b) => b.classList.toggle("on", b.textContent.toLowerCase() === m));
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
				cmd(en ? "ble:on" : "ble:off").then((d) => {
					if (d && d.ble) {
						B = d.ble;
						renderBle();
					}
				});
			}

			function renderGamepad() {
				const enabled = !!G.enabled;
				const connected = !!G.connected;
				const scanning = !!G.scanning;
				const addr = G.pairedAddr || "";
				let h = "<b>Status:</b> ";
				if (!enabled) {
					h += '<span style="color:var(--muted)">Disabled</span>';
				} else if (connected) {
					h += '<span style="color:var(--ok)">Connected</span>';
					h +=
						" &nbsp; <b>Buttons:</b> 0x" +
						(G.buttons || 0).toString(16).padStart(4, "0");
					if (G.axes && G.axes.length >= 6) {
						h +=
							"<br><b>Axes:</b> LX=" +
							G.axes[0] +
							" LY=" +
							G.axes[1] +
							" RX=" +
							G.axes[2] +
							" RY=" +
							G.axes[3] +
							" LT=" +
							G.axes[4] +
							" RT=" +
							G.axes[5];
					}
				} else if (addr) {
					h += '<span style="color:var(--warn)">Disconnected</span>';
					h += " &nbsp; reconnecting...";
				} else {
					h += '<span style="color:var(--muted)">No device paired</span>';
				}
				if (addr) h += "<br><b>Paired:</b> " + escapeHtml(addr);
				if (scanning) h += " &nbsp; <span style='color:var(--warn)'>Scanning...</span>";
				$("gpSt").innerHTML = h;

				// Scan results panel
				if (G.scanCount > 0 || scanning) {
					$("gpScanResults").style.display = "block";
				} else {
					$("gpScanResults").style.display = "none";
				}

				// Build binding table rows if not yet populated
				if (!$("gpBindBody").dataset.loaded && G.bindings) {
					renderGpBindings(G.bindings);
					$("gpBindBody").dataset.loaded = "1";
				}
			}

			function renderGpScan(devices) {
				let h = "";
				(devices || []).forEach(function (d) {
					h +=
						'<div style="display:flex;align-items:center;gap:8px;margin:3px 0">' +
						'<span style="font-family:monospace;font-size:11px">' +
						escapeHtml(d.addr) +
						"</span>" +
						" " +
						escapeHtml(d.name) +
						' <button style="padding:2px 8px;font-size:11px" onclick="gpPair(\'' +
						escapeHtml(d.addr) +
						"')\">Pair</button>" +
						"</div>";
				});
				$("gpDeviceList").innerHTML =
					h || '<span style="color:var(--muted)">No HID devices found yet</span>';
				$("gpScanResults").style.display = "block";
			}

			function renderGpBindings(bindings) {
				let rows = "";
				(bindings || []).forEach(function (b) {
					rows +=
						"<tr>" +
						'<td style="padding:2px 6px;white-space:nowrap">' +
						escapeHtml(b.button) +
						"</td>" +
						'<td style="padding:2px 6px"><input type="text" class="input" style="padding:3px 6px;font-size:12px;width:100%" ' +
						'id="gpBind' +
						b.index +
						'" value="' +
						escapeHtml(b.command) +
						'" placeholder="command e.g. fsd:on" /></td>' +
						'<td style="padding:2px 6px"><button style="padding:2px 8px;font-size:11px" onclick="gpSaveBind(' +
						b.index +
						')">Save</button></td>' +
						"</tr>";
				});
				$("gpBindBody").innerHTML = rows;
			}

			function gpScan() {
				cmd("gamepad:scan").then(() => {
					$("gpScanResults").style.display = "block";
					$("gpDeviceList").innerHTML =
						'<span style="color:var(--muted)">Scanning…</span>';
					// Poll /api/status after 7 s to render scan results.
					setTimeout(function () {
						fetch("/api/status")
							.then((r) => r.json())
							.then((d) => {
								S = d;
								if (d.gamepad) renderGpScan(d.gamepad.devices);
							})
							.catch(() => {});
					}, 7000);
				});
			}

			function gpPair(addr) {
				cmd("gamepad:pair:" + addr).then(() => {
					$("gpBindBody").dataset.loaded = ""; // reload bindings on next render
				});
			}

			function gpUnpair() {
				if (!confirm("Unpair gamepad?")) return;
				cmd("gamepad:unpair");
			}

			function gpSaveBind(idx) {
				const el = $("gpBind" + idx);
				if (!el) return;
				const command = el.value.trim();
				cmd("gamepad:bind:" + idx + ":" + command);
			}

			function renderRecorder() {
				let h = "<b>Status:</b> " + (R.enabled ? "Running" : "Stopped");
				h += " &nbsp; <b>Frames:</b> " + escapeHtml(R.count || 0);
				h += " / " + escapeHtml(R.capacity || 0);
				h += "<br><b>Captured:</b> " + escapeHtml(R.captured || 0);
				h += " &nbsp; <b>Dropped:</b> " + escapeHtml(R.dropped || 0);
				h += "<br><b>Last Capture:</b> " + escapeHtml(R.lastCaptureMs || 0) + " ms";
				$("recSt").innerHTML = h;
			}

			function startRecorder() {
				cmd("recorder:on").then((d) => {
					if (d && d.recorder) {
						R = d.recorder;
						renderRecorder();
					}
				});
			}

			function stopRecorder() {
				cmd("recorder:off").then((d) => {
					if (d && d.recorder) {
						R = d.recorder;
						renderRecorder();
					}
				});
			}

			function downloadRecorder() {
				fetch("/api/recorder/download")
					.then((r) => {
						if (!r.ok) throw new Error("download failed");
						return r.text();
					})
					.then((csv) => {
						const blob = new Blob([csv], { type: "text/csv;charset=utf-8" });
						const url = URL.createObjectURL(blob);
						const a = document.createElement("a");
						a.href = url;
						a.download = `can-recorder-${Date.now()}.csv`;
						document.body.appendChild(a);
						a.click();
						document.body.removeChild(a);
						URL.revokeObjectURL(url);
						appendLog("Recorder download complete");
						toast("Recorder CSV downloaded");
					})
					.catch(() => toast("Error"));
			}

			function renderWifi() {
				let h = "<b>Mode:</b> " + escapeHtml((W.mode || "?").toUpperCase());
				h += " &nbsp; <b>IP:</b> " + escapeHtml(W.ip || "N/A");
				if (W.mode === "sta") {
					h += "<br><b>SSID:</b> " + escapeHtml(W.ssid || "N/A");
					h += " &nbsp; <b>RSSI:</b> " + escapeHtml(W.rssi || "?") + " dBm";
					h += "<br><b>Status:</b> " + (W.connected ? "Connected" : "Disconnected");
				} else {
					h += "<br><b>SSID:</b> " + escapeHtml(W.ssid || "N/A");
					h += " &nbsp; <b>Clients:</b> " + escapeHtml(W.clients || 0);
					h += "<br><b>Channel:</b> " + escapeHtml(W.channel || "?");
				}
				$("wifiSt").innerHTML = h;
				document
					.querySelectorAll("#wifiModeSeg button")
					.forEach((b) =>
						b.classList.toggle("on", b.textContent.toLowerCase() === W.mode),
					);
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
				if (B.deviceName) h += "<br><b>Device:</b> " + escapeHtml(B.deviceName);
				$("bleSt").innerHTML = h;
				$("bleTg").checked = !!B.enabled;
			}

			function render() {
				let h = "<b>Variant:</b> " + escapeHtml(S.variant || "?");
				h += " &nbsp; <b>CAN:</b> " + (S.chassisOnline ? "Online" : "Offline");
				h += "<br><b>FSD:</b> " + (S.fsd ? "ON" : "OFF");
				h += " &nbsp; <b>Nag:</b> " + escapeHtml(S.nagMode || "off");
				h += " &nbsp; <b>Mirror Auto:</b> " + (S.mirrorAutoFold ? "ON" : "OFF");
				const pn =
					["Chill", "Normal", "Hurry", "Max", "Sloth"][S.profile] ||
					escapeHtml(S.profile);
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
				if ($("nagModeSel")) $("nagModeSel").value = S.nagMode || "off";
				if ($("nagBypassTg")) $("nagBypassTg").checked = !!S.nagOrgBypass;
				$("isaTg").checked = !!S.isaChime;
				$("mirrorAutoTg").checked = !!S.mirrorAutoFold;
				$("apGateTg").checked = !!S.apGateEnabled;
				if ($("dasDriveTg")) {
					$("dasDriveTg").checked = !!S.dasDriveEnabled;
					const lim = $("dasSpeedLimitIn"),
						cap = $("dasSpeedCapIn"),
						lbl = $("dasSpeedCapMaxLbl");
					if (lim && document.activeElement !== lim) lim.value = S.dasSpeedLimitKph ?? "";
					if (cap) {
						if (document.activeElement !== cap) cap.value = S.dasSpeedCapKph ?? "";
						cap.max = S.dasSpeedCapMaxKph ?? 200;
					}
					if (lim) lim.max = S.dasSpeedCapKph ?? 25;
					if (lbl) lbl.textContent = "max " + (S.dasSpeedCapMaxKph ?? 200);
				}
				$("apGateSt").innerHTML =
					"<b>Gate:</b> " +
					(S.apGateOpen
						? '<span style="color:var(--ok)">OPEN</span>'
						: '<span style="color:var(--accent)">CLOSED</span>') +
					" &nbsp; <b>AP:</b> " +
					(S.apGateAp ? "ON" : "off") +
					" &nbsp; <b>Park:</b> " +
					(S.apGatePark ? "ON" : "off") +
					" &nbsp; <b>Summon:</b> " +
					(S.apGateSummon ? "ON" : "off");

				if (hiddenApVisible) {
					$("hiddenApTg").checked = !!S.enhancedAutopilot;
					$("hiddenApSt").innerHTML =
						"<b>Status:</b> " +
						(S.enhancedAutopilot
							? '<span style="color:var(--warn)">Enabled</span>'
							: '<span style="color:var(--ok)">Disabled</span>') +
						"<br><b>Command:</b> " +
						(S.enhancedAutopilot ? "eap:on" : "eap:off");
				}

				document
					.querySelectorAll("#hwSeg button")
					.forEach((b) =>
						b.classList.toggle("on", b.textContent.toLowerCase() === S.variant),
					);
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
						// /api/status now carries ble/recorder/gamepad subsystem
						// snapshots — no need for per-feature polls anymore.
						if (d.ble) {
							B = d.ble;
							renderBle();
						}
						if (d.recorder) {
							R = d.recorder;
							renderRecorder();
						}
						if (d.gamepad) {
							G = d.gamepad;
							if (G.bindings && !$("gpBindBody").dataset.loaded) {
								renderGpBindings(G.bindings);
								$("gpBindBody").dataset.loaded = "1";
							}
							renderGamepad();
						}
					})
					.catch(() => {});

				fetch("/api/wifi/status")
					.then((r) => r.json())
					.then((d) => {
						W = d;
						renderWifi();
					})
					.catch(() => {});
			}

			setInterval(poll, 2000);
			ventPreview();
			document.addEventListener("keydown", (ev) => {
				if (ev.ctrlKey && ev.shiftKey && (ev.key === "A" || ev.key === "a")) {
					setHiddenApVisibility(!hiddenApVisible);
				}
			});
			appendLog("Dashboard booted");
			poll();
		</script>
	</body>
</html>
)HTML";
