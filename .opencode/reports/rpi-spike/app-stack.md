# Raspberry Pi portability spike — APP STACK (client / protocol / tools / docs / CI)

Scope: read-only spike. No Pi, no installs, no builds run here. Every claim tagged
`CODE-READ` (source/config inspected) or `UNVERIFIED` (needs hardware/install).
Firmware/CAN safety surface is a separate track; this report covers the app stack only.

## 1. What already runs on Linux today

### 1.1 Shared protocol package

- Pure TypeScript ESM package, no native deps. `packages/protocol/package.json`: `type: module`,
  `main dist/index.js`, scripts `build = tsc -p tsconfig.build.json`,
  `test = node --experimental-vm-modules ../../node_modules/jest/bin/jest.js --config jest.config.js`.
  `CODE-READ`
- Root scripts `build:protocol`, `test:protocol`, `typecheck:protocol` are platform-neutral Node/Jest/tsc.
  Root `package.json:13,25,28`. `CODE-READ`
- CI proof on Linux: `.github/workflows/ci.yml` `protocol` job (`ubuntu-latest`, setup-node 22,
  `npm ci`, `npm test -w packages/protocol`) and `e2e-smoke` job (build + jest e2e-smoke).
  `CODE-READ`
- Actual execution on a Pi was not attempted. `UNVERIFIED`

### 1.2 Tools CLI and serial-HTTP bridge

- `tools/package.json`: `type: module`, `bin tcm-debug -> ./debug.js`, scripts
  `test` (Jest ESM via `node --experimental-vm-modules`) and `bridge = node serial-http-bridge.js`,
  `peerDependencies.serialport ^13.0.0`, own `engines.node >=18`. `CODE-READ`
- `tools/debug.js` registers commands: `smoke, watch, test, scan, dump, replay, benchmark, vehicle,
drive-context, das-drive, flash`, plus portless `export`. All session commands open a serial port
  via `openSerial()` and speak JSON-RPC; `export` is file-only (no port). `CODE-READ`
- Replay/dump exist as first-class CLI paths (`tools/commands/dump.js`, `replay.js`, `export.js`
  imported in `debug.js:18-22,39`). `CODE-READ`
- `tools/serial-http-bridge.js`: Node `node:http` REST server, default bind `127.0.0.1:8080`,
  endpoints `GET /health` (public), `GET /api/status`, `POST /api/command` (bearer token via
  `--token`/`TCM_BRIDGE_TOKEN`, timing-safe compare), explicit-origin CORS (`*` rejected),
  30 s server timeout, 1 MiB body cap. It wraps `BoardSession` over one serial port. `CODE-READ`
- Bridge is designed as a loopback-local translator: serial JSON-RPC in, HTTP REST out.
  `CODE-READ`
- CI proof on Linux: `tools` job (`npm ci`, `npm test -w tools`). `CODE-READ`
- Running serialport on Pi hardware not attempted. `UNVERIFIED`

### 1.3 Expo web client

- `client/package.json` scripts: `web = expo start --web`, `web:open = expo start --web`,
  `web:build = expo export --platform web --output-dir dist-web`, `start = expo start`,
  `preweb`/`generate-docs = node scripts/generate-docs-catalog.mjs ../docs`. `CODE-READ`
- Web stack: expo ~54.0.33, react-native-web 0.21.2, react-dom 19.2.7, react 19.1.0,
  nativewind 4.2.5, tailwindcss 3.4.17. `CODE-READ`
- Docs catalog is pre-generated: `client/scripts/generate-docs-catalog.mjs` walks `docs/**/*.md`
  and writes `client/src/docs/_generated-docs.ts` (bundled markdown so docs work on web).
  The generated file exists; `preweb` regenerates it on `npm run web`. `CODE-READ`
- CI proof on Linux: `client` job (`npm run build -w packages/protocol`,
  `npm run typecheck -w @teslacanmodder/client`, `npm test -w @teslacanmodder/client`).
  `CODE-READ`
- `expo export` on Pi not attempted; success on arm64 Pi OS is `UNVERIFIED`.

### 1.4 Docker / dev servers

- Root `docker-compose.yml`: `client` service (target `dev`, host `0.0.0.0:4173`, command
  `npm run web -w @teslacanmodder/client -- --host 0.0.0.0 --port 4173`, bind-mount repo) and
  `client-prod` service (target `production`, host `8080`, `read_only`, nginx). `CODE-READ`
- `client/Dockerfile`: `FROM node:22-bookworm-slim`; `build` stage runs
  `npm run build -w packages/protocol` then `npm run web:build -w @teslacanmodder/client`;
  `production` stage copies `client/dist-web` into `nginx:1.27-alpine`. `CODE-READ`
- CI `docker` + `docker-smoke` jobs run `docker compose build --no-cache`,
  `docker compose up --build -d client-prod`, and curl `http://127.0.0.1:8080/`. `CODE-READ`
- Multi-arch Docker build on Pi (arm64) not tested; `nginx:1.27-alpine` and `node:22-bookworm-slim`
  arm64 tags assumed available but `UNVERIFIED`.
- `AGENTS.md` forbids agents starting `expo start`/`npm run web`/watchers; dev servers are manual.
  `CODE-READ`

### 1.5 MQTT and other advertised-but-absent claims

- `docs/reference/commands.md` MQTT section states config-only, no publish loop, no broker
  connection; `mqttShouldPublish()` has no caller and `mqttConnected` is never set. `CODE-READ`
- Roadmap C1-13 resolved by removal of firmware MQTT claim; C1-06 resolved by removal of OTA TX
  guard claims; client MQTT UI left as deferred cleanup (`.opencode/plans/roadmap-2026-11-22.md`,
  `.opencode/sdd/roadmap-2026-11-22/progress.md`). `CODE-READ`
- Residual client UI still reads `mqttConnected` (`client/src/components/monitor/sections/ConnectionSection.tsx:211`)
  and docs still list `mqtt:*` config + `mqttConnected` (`vehicle-features.md`, `connections.md`,
  `state-fields.md`). These are display/config surfaces, not a working broker path. `CODE-READ`
- `ha:*`, `espnow:*`, `gvret:*`, `smt:*`, `elm327:*`, `btnmap:*`, `speedalert:*`, `das:arm|disarm|status`
  are marked UNSUPPORTED in `docs/reference/commands.md`. `CODE-READ`

## 2. Runtime prerequisites on a Pi

### 2.1 Node version floor

- Root `package.json:40-42` `engines.node >=22`; root `package-lock.json:32` repeats `>=22`.
  `CODE-READ`
- `tools/package.json:21-23` `engines.node >=18` (looser than root). CI and Docker both pin Node 22
  (`ci.yml` setup-node 22 in protocol/e2e/client/tools/lint/security jobs;
  `client/Dockerfile:1` `node:22-bookworm-slim`). `CODE-READ`
- Practical floor for the workspace is Node 22, not 18. `CODE-READ`
- Node 22 availability for 32-bit `armv7l` Pi OS is `UNVERIFIED`; 64-bit `arm64` is the safer target
  but was not tested. `UNVERIFIED`

### 2.2 Native modules

- `serialport ^13.0.0` is a peer dep of `tools`, a dep of `firmware/package.json`, and resolved in
  root `package-lock.json` as `node_modules/serialport` 13.0.0 plus `@serialport/bindings-cpp`
  13.0.0 and `@serialport/binding-mock` 10.2.2. `CODE-READ`
- `tools/lib/session.js:17-28` dynamically imports `serialport` and emits an actionable
  "npm install -w @teslacanmodder/tools serialport" error when missing. `CODE-READ`
- Native binding must be compiled or prebuilt for the Pi arch. Presence of an arm64 prebuild for
  `@serialport/bindings-cpp` 13.0.0 is `UNVERIFIED`; assume `python3`, `make`, `g++` (build-essential)
  may be needed. `UNVERIFIED`
- `lightningcss` optional native binaries for Linux are in the lock
  (`lightningcss-linux-arm64-gnu` / `-musl` 1.27.0 and 1.32.0); no `linux-armv7` variant found.
  `CODE-READ`
- Effect: Tailwind/NativeWind builds are plausible on 64-bit Pi OS; 32-bit Pi OS may fall back to
  wasm or fail. `UNVERIFIED`

### 2.3 Expo / web build

- Web build is `expo export`, a Node-side static build; no GUI browser required to produce
  `client/dist-web`. `CODE-READ`
- Docs catalog generation requires `docs/` to be present at build time (script walks `../docs`).
  `CODE-READ`
- Browser requirement is limited to Web Serial flashing: `FlashScreen.tsx:179`,
  `webEspFlasher.ts:32`, `runtimeAdapters.ts:66-73` require Chrome/Edge with Web Serial; native
  runtime has no serial transport and falls back to REST. `CODE-READ`
- Serving static assets needs only nginx (docker target) or any static server; `CODE-READ`

### 2.4 Python / PlatformIO / esptool

- Firmware CI uses `actions/setup-python` 3.12, `pip install platformio`, `pio test -e native`,
  and `scripts/check_firmware_size.py`. Python/PlatformIO are needed only to _rebuild_ firmware.
  `CODE-READ`
- CLI flashing needs Python + esptool: `tools/commands/flash.js` resolves esptool from
  `~/.platformio/penv` (`bin` on non-Windows) or falls back to `python -m esptool`, invokes
  `--chip esp32 --port <dev> --baud 460800 write_flash`, optionally `erase_flash`, then verifies boot
  over serial. `CODE-READ`
- Merged release images are flashed as a single segment at `0x0` and do not require `boot_app0.bin`;
  raw `firmware.bin` builds require bootloader/partitions/boot_app0 beside them. `CODE-READ`
- Pi host flashing is feasible if Python + esptool + USB serial exist, but was not executed.
  `UNVERIFIED`

### 2.5 Linux serial specifics

- Docs reference Linux device paths (e.g. `/dev/cu.SLAB_USBtoUART` in generated bundle); bridge
  default host is loopback. `CODE-READ`
- USB permissions (dialout group), stable device naming via udev, and `/dev/ttyUSB*` symlinks on Pi
  are not configured anywhere in the repo. `UNVERIFIED`

## 3. Blocked or absent

- **Browser flasher cannot move to a headless Pi.** Web Serial needs Chrome/Edge GUI; the Pi can
  flash only through the Node CLI + esptool path. `CODE-READ` (`FlashScreen.tsx`, `webEspFlasher.ts`)
- **No "canPi host" concept exists in the repo.** No Raspberry/CanPi reference found in canonical
  docs/config; only arch-specific lockfile binaries and legacy repos matched. `CODE-READ` (absence)
- **OTA staging absent.** C1-06 removed all OTA TX-pause claims; no `otaInProgress`/`txPaused` field
  or handler; `0x318` is defined raw only. `CODE-READ` (`can-protocol-safety/SKILL.md:82`,
  `firmware-development/SKILL.md:140`, roadmap C1-06)
- **No MQTT broker/publish loop.** C1-13 removed the firmware claim; no MQTT client dependency in
  firmware `lib_deps`. A broker on Pi would publish nothing until firmware is implemented.
  `CODE-READ`
- **Home automation / GVRET / ESP-NOW / SPP / ELM327 / button remap / speed camera are UNSUPPORTED**
  per `docs/reference/commands.md`. `CODE-READ`
- **No supervisor, no process manager, no capture DB, no MQTT service** in the repo. Nothing to
  deploy as-is beyond the bridge and static web client. `CODE-READ` (absence)
- **Docs catalog is a generated artifact**, so a Pi build must either run `preweb` (docs present)
  or consume a prebuilt `dist-web`; it is not a runtime blocker if built in CI. `CODE-READ`
- **No ARM CI runner.** All CI jobs are `ubuntu-latest` x86; Docker build is not cross-arch tested.
  `CODE-READ`

## 4. Smallest supervisor architecture on Pi

### 4.1 Processes (systemd units)

1. `tcm-bridge@<n>.service` — one `node tools/serial-http-bridge.js --port /dev/ttyUSB<n> --listen 808<n>
--token <secret>` per ESP32 node. Reuses existing bearer-protected REST bridge verbatim.
   `CODE-READ` (component exists), deployment shape `UNVERIFIED`.
2. `tcm-web.service` (optional) — nginx serving prebuilt `client/dist-web` on LAN port 8080; build
   in CI/Docker, copy artifact to Pi. `CODE-READ` (docker production target), on-Pi unit `UNVERIFIED`.
3. `tcm-capture.service` (optional) — subscribe to bridge `stream:on` frames and poll
   `/api/status`; append JSONL (or SQLite) under `/var/lib/tcm/`. No such service exists today.
   New code — `UNVERIFIED` behaviour until written/tested.
4. `tcm-flash.service` — oneshot invoking `node tools/debug.js flash --port ... --hex <image>`
   (esptool under the hood). Manually triggered, not a daemon. `CODE-READ`
5. `mosquitto.service` — only if/when MQTT publish loop is implemented on firmware. Out of scope now.
   `UNVERIFIED`

### 4.2 Transport boundaries

- ESP32 node ↔ Pi: USB serial, single-line JSON-RPC `{"cmd":"..."}\n` at 115200 baud, via
  `serialport`/`BoardSession`. `CODE-READ`
- Pi bridge ↔ clients: HTTP REST `/api/status`, `/api/command` with bearer token; `/health` for
  supervisor probes. Bind loopback; expose LAN only behind authenticated reverse proxy.
  `CODE-READ` (bridge), topology `UNVERIFIED`.
- One `BoardSession` per serial port — do not multiplex multiple clients onto one bridge instance;
  run one bridge process per node. `CODE-READ` (session is single-port)
- WiFi/BLE transports on the ESP32 connect directly to the client and bypass the Pi entirely;
  do not proxy them. `CODE-READ` (`docs/reference/commands.md` transports)

### 4.3 Persistence and logging

- ESP32 keeps NVS: settings, gamepad bindings, variant, speed limits, etc. Do not mirror to Pi.
  `CODE-READ`
- Pi-local state: bridge token/config in `/etc/tcm`, captures/status in `/var/lib/tcm` (JSONL or
  SQLite), service logs to journald. Locations are a proposal, not present in repo — `UNVERIFIED`.
- Docs source stays `docs/`; generated bundle stays derived. `CODE-READ`

### 4.4 What must NOT move to Pi

- CAN frame mutation, checksums, mux handling, bus routing, AP gate, GTW/Ban Shield, nag modes,
  DAS Drive, drive-mode override, region/ECE R79 — all firmware-side on ESP32. `CODE-READ`
- Tesla BLE key material and pairing. `CODE-READ`
- Firmware compilation — keep PlatformIO in CI; Pi needs esptool only for flashing. `CODE-READ`
- Web Serial flashing GUI — remains in Chrome/Edge on engineer machine. `CODE-READ`
- Safety gating and NVS authority remain on the device. `CODE-READ`

## 5. Effort estimate and risks

### 5.1 Estimates (hours, `UNVERIFIED` until on hardware)

- Node 22 arm64 install + `npm ci` + run `test:protocol` / `test:tools` on Pi: 1–2 h.
- Native module resolution (serialport/bindings-cpp, lightningcss) on Pi: 1–4 h (0 if prebuilds exist).
- Web client `expo export` + nginx static serve on Pi: 2–4 h; 0.5 h if build in CI and copy `dist-web`.
- Bridge-per-node systemd units + health checks + token handling: 3–5 h.
- Capture/log writer service (JSONL/SQLite + status polling): 4–8 h.
- Pi-side flashing via CLI + esptool + udev rules: 2–4 h.
- MQTT publish path (firmware + broker + client cleanup): 8–16 h + bench; out of scope for a spike.
- Rough spike total (run + serve + supervise + capture): 12–24 h. `UNVERIFIED`

### 5.2 Risks

- Node 22 absent on 32-bit `armv7l`; prefer 64-bit Pi OS. `UNVERIFIED`
- serialport / lightningcss native binaries may lack arm64 prebuilds → toolchain needed. `UNVERIFIED`
- USB device renumbering and dialout permissions break `tcm-bridge@<n>` unit stability. `UNVERIFIED`
- Single serial session per bridge; concurrent clients need per-node processes, not one shared port.
  `CODE-READ`
- Token leakage: bridge only safe when bound loopback or behind auth proxy; wildcard CORS rejected but
  token still travels in env/unit file. `CODE-READ`
- No MQTT/OTA to supervise: supervisor cannot deliver telemetry or OTA until those are implemented.
  `CODE-READ`
- Docs bundle must be regenerated after doc edits or served from a CI-built artifact. `CODE-READ`
- No ARM CI job means regressions on Pi packaging are not caught upstream. `CODE-READ`

## 6. One-line verdict

App stack is largely Linux-portable already (protocol/tools/web/bridge all run on Node 22 in x86 CI);
smallest Pi supervisor is "systemd bridge-per-serial-node + nginx static client + optional capture
writer", with flasher as an on-demand esptool oneshot — but MQTT, OTA, home-automation, and the
browser Web Serial flasher are absent/unmovable, and every Pi-specific runtime assumption here is
`UNVERIFIED`.
