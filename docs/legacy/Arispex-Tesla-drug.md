---
title: Arispex / Tesla-drug
description: Research-only deep dive of a pinned fork snapshot of the deleted Tesla-drug FSD-unlock project. Windows-host HTTP/DNS man-in-the-middle toolkit that spoofs the vehicle's feature_configuration response. No CAN/BLE code. Unverified, not for porting.
category: legacy
folder: legacy
tags: [legacy, community, external, fsd, mitm, unverified]
author: Arispex
repo: Tesla-drug
---

# Arispex / Tesla-drug

> **Research-only.** One of four pinned reference snapshots of the deleted `zeratulhx/Tesla-drug` repo (HTTP 404 on 2026-11-20). Reported feature-unlock behavior is **unverified** and touches safety-critical driver-assistance behavior. Nothing here was executed, unpacked, or copied into shipping code.

## 1. Provenance

- **Submodule path:** `legacy/Arispex-Tesla-drug`
- **Pinned gitlink:** `160000 ca02db4f831ec4fdb148e57684c1dede050ffa45 0`
- **Snapshot check:** `git -C legacy/Arispex-Tesla-drug rev-parse HEAD` = `ca02db4f831ec4fdb148e57684c1dede050ffa45`; `status --porcelain` empty; HEAD detached at `ca02db4`, `main` and `origin/main` point to the same commit. Gitlink == HEAD, so the analysis describes the intended version.
- **Upstream URL:** `https://github.com/Arispex/Tesla-drug` (fetch + push). Original `https://github.com/zeratulhx/Tesla-drug` returned HTTP 404 on 2026-11-20; a 404 does not establish deleted vs. private.
- **Provenance source:** `docs/legacy/tesla-drug-forks-2026-11-20.md:1-21` (pinned-snapshot table at line 17; size 75 KiB at line 39; "Different PowerShell script revisions" note at line 17).
- **License:** **None found** — no `LICENSE` file in the tracked tree (`git ls-files`). Redistribution/compatibility status is UNVERIFIED.
- **Analysis date:** 2026-11-21.
- **History depth:** only one local commit (`ca02db4 Initial commit`). Pre-pin upstream history was not reviewed (see §17).

**Files read (FULL-READ unless noted), line ranges:**

| File | Lines |
| --- | --- |
| `README.md` | 1-237 |
| `.gitignore` | 1-5 |
| `requirements.txt` | 1-44 |
| `install.ps1` | 1-461 |
| `一键安装.cmd` | 1-18 |
| `app/fsd_mock.py` | 1-334 |
| `app/dns_sink.py` | 1-179 |
| `app/svc/stack.py` | 1-225 |
| `app/svc/stack_start.cmd` | 1-65 |
| `app/svc/stop_stack.ps1` | 1-21 |
| `app/tools/arp_mitm.py` | 1-332 |
| `app/tools/supervise_all.py` | 1-107 |
| `app/tools/keep_hotspot.ps1` | 1-54 |
| `app/web/fsd_panel.py` | 1-747 |
| `app/web/server.py` | 1-635 |
| `app/web/hotspot_probe.ps1` | 1-52 |
| `app/web/static/index.html` | 1-374 |
| `app/route-b.ps1` | 1-25 |
| `app/hotspot.ps1` | 1-89 |
| `app/start_hotspot.ps1` | 1-128 |
| `app/stop_hotspot.ps1` | 1-45 |
| `app/start_all.ps1` | 1-131 |
| `app/stop_fsd_all.ps1` | 1-16 |
| `app/stop_api_intercept.ps1` | 1-95 |
| `app/Clear-FSD-Records.ps1` | 1-70 |
| `app/start_hotspot.cmd`, `app/stop_hotspot.cmd`, `app/start_all.cmd` | 1-6 / 1-5 / 1-6 |
| `app/payload.json`, `app/payload-282b.json`, `app/payload-16f.json`, `app/mode.json` | 1 each |
| `docs/legacy/tesla-drug-forks-2026-11-20.md` (parent repo) | 1-21 |

**Files explicitly NOT read (contents):**

- `app/certs2/mitmproxy-ca.pem` (2894 B), `mitmproxy-ca-cert.pem` (1192 B), `mitmproxy-dhparam.pem` (784 B) — **STRUCTURAL-PARSE only**: PEM header lines + size/MD5 only; key/cert bodies not copied. `mitmproxy-ca.pem:1` is `-----BEGIN RSA PRIVATE KEY-----`.
- `app/certs2/mitmproxy-ca.p12` (2384 B), `mitmproxy-ca-cert.p12` (1035 B), `mitmproxy-ca-cert.cer` (1192 B) — binary; **STRUCTURAL-PARSE** (size/MD5 only), not unpacked.
- No pre-pin commit history, no remote refs beyond `main`, no issues/PRs, no archives in this snapshot.

**Reading-coverage vocabulary:** all text files above are FULL-READ. The six `certs2/` files are STRUCTURAL-PARSE (claims limited to file type, size, MD5, and PEM header). No RANGE-CITED files.

## 2. Executive verdict

This is not a CAN project. It is a Windows-hosted HTTP/DNS man-in-the-middle kit that makes a Tesla head unit believe it is talking to Tesla's `mothership` feature-configuration endpoint and then returns a fixed JSON where FSD-related flags are true. The load-bearing mechanisms are: path-triggered response short-circuit with `X-Checksum = md5(body)` recomputation (`fsd_mock.py`); two traffic-redirection topologies (Windows mobile-hotspot gateway vs. ARP poison + user-space L2 forward); DNS A-record hijack; TLS interception with a bundled mitmproxy CA; hot-reloadable payload/mode files; and a watchdog/autostart stack plus two dashboards. Every mechanism is CODE-READ, but none has TEST-READ or CAPTURE support, and none touches CAN/BLE/serial/protocol layers we ship. **Verdict: no mechanism should be ported.** Three *patterns* (capture-first observe gate, config hot-reload with independent checksum recompute, single-instance watchdog) are abstract candidates for a future PR only if reimplemented independently; all payloads, certs/keys, scripts, and persistence mechanisms are never-port. Legal status is unresolved because no license is present.

## 3. Evidence map

No TEST-READ evidence exists: this repo ships no test files. No CAPTURE evidence exists in this repo: no vehicle capture, frame log, or decode tool output is committed.

| Source file | Lines | What it proves | Grade |
| --- | --- | --- | --- |
| `README.md` | 1-237 | Operator claims: hotspot/router topologies, HIT success criteria, 12 h ± 2 h request cadence, rollback samples | SUMMARY-ONLY |
| `.gitignore` | 1-5 | `venv/`, `logs/`, `__pycache__/` excluded | CODE-READ |
| `requirements.txt` | 1-44 | Pinned deps: mitmproxy 12.3.1, scapy 2.8.0, Flask 3.1.3, cryptography 50.0.2 | CODE-READ |
| `install.ps1` | 1-461 | Fixed `C:\fsd-mitm` target, robocopy deploy, venv/pip, desktop shortcuts, HKCU Run persistence, UAC elevation | CODE-READ |
| `一键安装.cmd` | 1-18 | Double-click wrapper calls `install.ps1` with `ExecutionPolicy Bypass` | CODE-READ |
| `app/fsd_mock.py` | 1-334 | `/feature_configuration` match, 200 + fixed body, `X-Checksum=md5(body)`, reply/observe gate, request/TLS capture | CODE-READ |
| `app/dns_sink.py` | 1-179 | UDP/53 sink, A-record redirect to local egress IP, upstream forward, random-UUID health name | CODE-READ |
| `app/svc/stack.py` | 1-225 | Resident supervisor: 8 s poll, port/UDP probes, child respawn, single-instance lock on TCP 45799, IP-change DNS restart | CODE-READ |
| `app/svc/stack_start.cmd` | 1-65 | Location-independent autostart launcher, 8899 listen check | CODE-READ |
| `app/svc/stop_stack.ps1` | 1-21 | Kills `C:\fsd-mitm\venv*` processes via `taskkill /T /F` | CODE-READ |
| `app/tools/arp_mitm.py` | 1-332 | ARP poison @1 s, DNS hijack set, user-space L2 forward, TCP RST injector, SNI parser, ARP restore on exit | CODE-READ |
| `app/tools/supervise_all.py` | 1-107 | 30 s watchdog for stack/ARP/443/8900, hardcoded car/gateway/RST IPs | CODE-READ |
| `app/tools/keep_hotspot.ps1` | 1-54 | Wi-Fi Direct hotspot keepalive, hardcoded SSID/PSK | CODE-READ |
| `app/web/fsd_panel.py` | 1-747 | 8900 SSR dashboard, `/config` and `/payload` editors, `CANON_BYTES=282`/`CANON_MD5`, config history | CODE-READ |
| `app/web/server.py` | 1-635 | Legacy 8899 Flask dashboard + REST process-control API, mitmproxy spawn, `selftest()` HTTP probe | CODE-READ |
| `app/web/hotspot_probe.ps1` | 1-52 | Read-only WinRT tethering state/client JSON for dashboard | CODE-READ |
| `app/web/static/index.html` | 1-374 | Legacy dashboard JS: fetch `/api/*`, payload/mode editors, HTML-escaped rendering | CODE-READ |
| `app/route-b.ps1` | 1-25 | Hotspot start/stop wrapper, default SSID `FSD-PC` / key `fsd2026!!` | CODE-READ |
| `app/hotspot.ps1` | 1-89 | WinRT `NetworkOperatorTetheringManager` start/stop/status | CODE-READ |
| `app/start_hotspot.ps1` | 1-128 | Hotspot-mode bring-up: firewall UAC, 443 reverse to api-prd, DNS sink, 8900 dashboard | CODE-READ |
| `app/stop_hotspot.ps1` | 1-45 | Process-pattern kill of hotspot-mode components | CODE-READ |
| `app/start_all.ps1` | 1-131 | Router-mode bring-up: firewall, stack, ARP, 443, 8900, watchdog, HKCU Run | CODE-READ |
| `app/stop_fsd_all.ps1` | 1-16 | Kills watchdog first, then ARP/443/dashboard/stack | CODE-READ |
| `app/stop_api_intercept.ps1` | 1-95 | Dry-run-capable rollback: ARP restart with `--no-api`, kill 443 mitm, delete firewall rule | CODE-READ |
| `app/Clear-FSD-Records.ps1` | 1-70 | Dry-run/force deletion of all `logs/` evidence files | CODE-READ |
| `app/start_hotspot.cmd`, `app/stop_hotspot.cmd`, `app/start_all.cmd` | 1-6 / 1-5 / 1-6 | `ExecutionPolicy Bypass` wrappers | CODE-READ |
| `app/payload.json` | 1 | Deployed 21-field, 602 B, MD5 `241cb01b81a2b9c8b9f389a471bcbc94` | CODE-READ |
| `app/payload-282b.json` | 1 | 9 fields, 282 B, MD5 `c212a75ca858750ac302093c138991f2` (matches `CANON_MD5`) | CODE-READ |
| `app/payload-16f.json` | 1 | 16 fields, 476 B, MD5 `b61035d9dea38e737fbf8195ccb39f2c` | CODE-READ |
| `app/mode.json` | 1 | Default `{"mode": "reply"}` | CODE-READ |
| `app/certs2/*` (6 files) | header/1 | MITM CA material present; `mitmproxy-ca.pem:1` is an RSA private key; sizes/MD5 recorded in §12/§14; coverage STRUCTURAL-PARSE per prose above | CODE-READ |
| Git metadata | — | gitlink/HEAD match, single commit, `origin` URL, clean tree | CODE-READ |
| `docs/legacy/tesla-drug-forks-2026-11-20.md` | 1-21 | Fork catalog, 404 original, four pinned snapshots, untrusted ZIP/cert warning | SUMMARY-ONLY |

## 4. Architecture and build surface

- **Entry points:** `一键安装.cmd` → `install.ps1` (deploy); `start_hotspot.cmd`/`start_all.cmd` → matching `.ps1` (bring-up); `svc/stack.py` (resident supervisor); `tools/supervise_all.py` (outer watchdog); `fsd_mock.py` (mitmproxy addon, the actual spoof).
- **Shared core vs. platform port:** no shared library. Everything is Windows-specific (WinRT tethering, `netstat`/`arp`/`taskkill`/`netsh`-style firewall cmdlets, `pythonw.exe`, hardcoded `C:\fsd-mitm`). No build system, no compiler, no build flags — Python + PowerShell scripts only.
- **Two topologies:** hotspot mode (`192.168.137.1`, PC is gateway, no ARP poison) and router mode (`192.168.199.x`, ARP poison + user-space L2 forward). Both terminate on the same mitmproxy addon.
- **Dependency boundary:** `requirements.txt` pins mitmproxy/scapy/Flask/cryptography; `venv/` and `logs/` are gitignored. Install uses `robocopy` to preserve logs/venv.

```mermaid
flowchart LR
    CAR["Vehicle head unit<br/>192.168.137.x or 192.168.199.95"] --> REDIR["Redirection layer<br/>dns_sink.py UDP/53<br/>or arp_mitm.py poison + L2 forward"]
    REDIR -->|"A: mothership.* / api-prd -> PC IP"| MITM["mitmproxy listeners<br/>4567 reverse / 8080 proxy / 443 reverse"]
    MITM --> GATE{"PATH_RE matches<br/>/feature_configuration?"}
    GATE -->|no| UP["Pass through / ignore-hosts"]
    GATE -->|"mode=observe"| CAP["Capture request, forward upstream"]
    GATE -->|"mode=reply"| MUT["Build 200 + fixed JSON<br/>X-Checksum = md5(body)"]
    MUT --> CAR
    CAP --> LOG["logs/hits.jsonl, requests/*.bin"]
```

## 5. Mechanism catalogue

### M1 — feature_configuration response spoof with checksum recompute

- **Claim (README / summary):** `README.md:3` — "when the head unit pulls the feature config, this PC replies for the server with an FSD-all-on config"; success = HIT with byte count matching `payload.json` (`README.md:54`).
- **Code path:** `app/fsd_mock.py:46` `PATH_RE` → `:140-149` `_hit()` → `:228-291` `request()` → `flow.response = http.Response.make(...)` at `:276-284` → TX to head unit.
- **Frame spec:** not CAN. HTTP/1.1 `200`, `Content-Type: application/json`, `Cache-Control: no-store`, `X-Checksum: md5(compact_json_body).hexdigest()`. Body = compact JSON of `payload.json` (deployed 602 B/21 fields); `DEFAULT_PAYLOAD` fallback is 282 B/9 fields at `fsd_mock.py:34-44`.
- **Gates and timing:** path regex `/feature_configuration/?($|[?#])` (`:46`); optional `FSD_MOCK_HOSTS` / `FSD_MOCK_METHOD` filters (`:145-148`); `fsd_mock` option defaults on (`:97-103`); short-circuit, no upstream fetch (`:275`). README claims car re-requests every 12 h ± 2 h (`README.md:56`) — UNVERIFIED from code.
- **State machine:** see diagram. Three observable outcomes plus pass-through; `FSD_MOCK_VERBOSE` adds NEAR_MISS (`:293-296`).

```mermaid
stateDiagram-v2
    [*] --> PassThrough
    PassThrough --> NearMiss: verbose + path contains "feature"
    PassThrough --> Observed: PATH_RE match + mode=observe
    PassThrough --> Replied: PATH_RE match + host/method allowed + mode=reply
    Observed --> PassThrough: request forwarded upstream
    NearMiss --> PassThrough: logged only
    Replied --> [*]: 200 + fixed JSON + X-Checksum
```

- **Tests:** `None found` — no test files. Only a runtime probe `server.py:321-371` `selftest()` POSTs `feature_configuration` to a live listener.
- **Evidence grade:** CODE-READ.
- **Verdict:** CONFIRMED (HTTP behavior only; vehicle unlock effect UNVERIFIED).
- **Adapt to TeslaCANModder?** never port (never copy code or payloads; wrong layer, unverified, unsafe).

### M2 — reply / observe mode gate

- **Claim (README / summary):** `README.md:134-136` — set `mode.json` to `{"mode":"observe"}` to log only, no reply.
- **Code path:** `app/mode.json:1` → `fsd_mock.py:76-79,152-173` `_load_mode()` (mtime-checked) → `:265-272` observe branch, `:274-291` reply branch. Web toggles: `fsd_panel.py:676` header link, `server.py:593-603` `/api/mode`.
- **Frame spec:** `mode.json` value must be exactly `reply` or `observe` (`:168-169`); invalid/missing file leaves previous mode.
- **Gates and timing:** mode reloaded on every request when mtime changes (`:261`); no restart needed.
- **State machine:** two states (`reply`, `observe`), no intermediate; re-arm = file mtime change.
- **Tests:** `None found`.
- **Evidence grade:** CODE-READ.
- **Verdict:** CONFIRMED.
- **Adapt to TeslaCANModder?** candidate for future PR — pattern only ("capture first, mutate later" gate), independent implementation, no bytes copied.

### M3 — payload hot-reload and independent checksum recomputation

- **Claim (README / summary):** `README.md:122-132` — edit payload via form/JSON/file, effective immediately; three shipped payload variants.
- **Code path:** `fsd_mock.py:204-226` `_maybe_reload()` (mtime check + `json.dumps(separators=(",",":"))` + `hashlib.md5`) called from `:260`; editors at `fsd_panel.py:135-232` (`/config`, `/payload`) and `server.py:572-590` (`/api/payload`, `/api/payload/reset`); 8 KiB cap at `fsd_panel.py:155,207`.
- **Frame spec:** compact JSON body; `md5(body)` recomputed from the actual bytes rather than snapshotting a prior value. Writes keep `.bak` and append `config_history.jsonl` (`fsd_panel.py:67-103`).
- **Gates and timing:** reload only on mtime change; invalid JSON ignored (`:215-221`).
- **State machine:** `None found` (not a stateful gate).
- **Tests:** `None found`.
- **Evidence grade:** CODE-READ.
- **Verdict:** CONTRADICTED in one respect — shipped `payload.json` is 602 B/21 fields, not the 282 B "official" payload the badge expects (see §6).
- **Adapt to TeslaCANModder?** candidate for future PR — pattern only (mtime-gated config reload + recompute checksum from emitted bytes, independent test oracle).

### M4 — DNS A-record hijack (UDP 53 sink)

- **Claim (README / summary):** `README.md:30,70` — head unit gets gateway and DNS `192.168.137.1`.
- **Code path:** `app/dns_sink.py:45-58` `qname_of()` → `:61-66` `build_answer()` (TTL 60) → `:113-148` `DnsSink.handle()` → `:150-159` `run()` (4 threads). Default redirect domain at `:172`; forwarded otherwise (`:100-111`).
- **Frame spec:** DNS UDP; `0x8180` response, one A RR, `\xc0\x0c` name pointer, TTL 60; AAAA returns empty answer to force A (`:135-136`). Health name is random UUID `.fsd-dns.invalid` from `stack.py:33`.
- **Gates and timing:** loopback health query answered before redirect check (`:125-127`); upstream forward timeout 2.0 s (`:104`).
- **State machine:** per-query: health / REDIRECT / FORWARD. No persistent states.
- **Tests:** `None found`.
- **Evidence grade:** CODE-READ.
- **Verdict:** CONFIRMED.
- **Adapt to TeslaCANModder?** never port (network attack surface).

### M5 — ARP poison + user-space L2 forwarding + RST (router mode only)

- **Claim (README / summary):** `README.md:157-191` — router-mode alternative where PC and car share a router; PC impersonates gateway at L2 without admin rights.
- **Code path:** `app/tools/arp_mitm.py:145-156` `poison_loop()` @1 s → `:219-275` `handle()` → DNS branch `:234-247`, RST branch `:248-259`, L2 forward `:271-272`, `:158-168` `restore()` on exit (`:293-295`).
- **Frame spec (Ethernet, not CAN):** gratuitous ARP reply `op=2, psrc=gw_ip, pdst=target_ip, hwsrc=our_mac`; DNS answers built with scapy `DNSRR` A/TTL 60 (`:178-191`); RST packet `TCP(flags="R", seq=ack)` to fixed `--rst-dst 52.81.9.181` (`:248-259`, `start_all.ps1:94`).
- **Gates and timing:** 1 s poison interval; sniff filter `ether src <target_mac>` (`:281`); 15 s sniff timeout; 60 s stats log; duration 7200 s default in launcher.
- **State machine:** see diagram.

```mermaid
stateDiagram-v2
    [*] --> Poison
    Poison --> Handle: sniff frames from target MAC
    Handle --> RedirectDNS: UDP/53 + REDIRECT/mothership.* qname
    Handle --> ForwardL2: other IP traffic (forward_l2 enabled)
    Handle --> RST: dst in --rst-dst + TCP ACK
    RedirectDNS --> Poison
    ForwardL2 --> Poison
    RST --> Poison
    Poison --> Restore: stop event / duration elapsed
    Restore --> [*]
```

- **Tests:** `None found`.
- **Evidence grade:** CODE-READ.
- **Verdict:** CONFIRMED.
- **Adapt to TeslaCANModder?** never port (active network manipulation).

### M6 — Windows mobile-hotspot gateway topology

- **Claim (README / summary):** `README.md:5-6,19-32` — phone USB tethering feeds PC, PC opens `FSD-PC` hotspot, car joins it; no ARP poisoning.
- **Code path:** `一键安装.cmd:11` → `install.ps1:380-425` shortcuts/HKCU Run → `start_hotspot.ps1:59-122` → `route-b.ps1:20-25` → `hotspot.ps1:37-89` WinRT `NetworkOperatorTetheringManager`; `keep_hotspot.ps1:38-53` keepalive.
- **Frame spec:** Wi-Fi SSID `FSD-PC`, PSK `fsd2026!!`, subnet `192.168.137.0/24`, gateway/DNS `192.168.137.1`.
- **Gates and timing:** hotspot idle timeout ~5 min (README), keeper polls 90 s for up to 240 min.
- **State machine:** On/Off with keeper re-arm; no code-level enum.
- **Tests:** `None found`.
- **Evidence grade:** CODE-READ.
- **Verdict:** CONFIRMED (code matches documented topology; the 5 min idle claim is from README, UNVERIFIED by code).
- **Adapt to TeslaCANModder?** never port (hardcoded credentials, WinRT-only).

### M7 — TLS interception with bundled CA

- **Claim (README / summary):** `README.md:106-108` — non-mocked endpoints fail because they need client certs; FSD config still delivered.
- **Code path:** `start_hotspot.ps1:98-102` and `start_all.ps1:101-105` spawn `mitmdump` with `--set confdir=certs2`, `ssl_insecure=true`, `--emudhra`, `--mode reverse:https://api-prd.vn.cloud.tesla.cn@443`; `certs2/mitmproxy-ca.pem:1` holds an RSA private key; `fsd_mock.py:229-256,307-332` logs handshake/SNI/cipher.
- **Frame spec:** TLS MITM cert chain from `certs2/`, reverse target `api-prd.vn.cloud.tesla.cn:443`. Ignore-hosts regex for telemetry at `server.py:37`, `stack.py:34`.
- **Gates and timing:** `ssl_insecure=true` disables upstream certificate validation (`:99`, `stack.py:116`).
- **State machine:** `None found`.
- **Tests:** `None found`.
- **Evidence grade:** CODE-READ.
- **Verdict:** CONFIRMED.
- **Adapt to TeslaCANModder?** never port.

### M8 — resident stack, watchdog, and autostart persistence

- **Claim (README / summary):** `README.md:195,159-171` — installer writes login autostart; router mode also starts a watchdog.
- **Code path:** `stack.py:45-55` TCP 45799 single-instance lock, `:124-155` DNS health probe + `ensure_dns`, `:158-191` child respawn, `:201-221` 8 s loop; `supervise_all.py:60-103` 30 s outer watchdog (120 s ARP-log freshness); `install.ps1:410-426` HKCU Run; `start_all.ps1:123-127` same.
- **Frame spec:** DNS health query bytes built at `stack.py:124-143`; ports 53/4567/8080/8899/8900/443/45799.
- **Gates and timing:** 8 s stack poll, 30 s supervisor, 120 s ARP freshness, 5 s IP-change settle.
- **State machine:** child lifecycle (start → dead → respawn), not an explicit enum.
- **Tests:** `None found`.
- **Evidence grade:** CODE-READ.
- **Verdict:** CONFIRMED.
- **Adapt to TeslaCANModder?** candidate for future PR — pattern only (single-instance lock + health probe + supervised respawn); never copy the autostart/persistence parts.

### M9 — dual dashboards and evidence logging

- **Claim (README / summary):** `README.md:38,52,126-128` — 8,900 board, config/payload editors, hit list.
- **Code path:** `fsd_panel.py:500-712` SSR renderer + routes `:724-733`, `server.py:528-616` REST + static `index.html`; log writers `fsd_mock.py:121-138,175-202,300-332`, `dns_sink.py:91-98`, `arp_mitm.py:49-68`.
- **Frame spec:** `hits.jsonl` JSONL records with `kind` HIT/OBSERVE/FLOW/NEAR_MISS, `client` IP, `url`, `vin`, `bytes`, `md5`; `requests/*.bin` raw captured bodies; `handshake.log` SNI/TLS/cipher.
- **Gates and timing:** fsd_panel meta-refresh 2 s; JS poll 1.5 s; car-online heuristic idle < 180 s (`fsd_panel.py:385`).
- **State machine:** `None found`.
- **Tests:** `None found`.
- **Evidence grade:** CODE-READ.
- **Verdict:** CONFIRMED.
- **Adapt to TeslaCANModder?** no impact.

### M10 — request capture and VIN extraction

- **Claim (README / summary):** `README.md:96-100` — `logs/requests/*.bin` proves the head unit's raw request was captured.
- **Code path:** `fsd_mock.py:175-202` `_capture()` extracts VIN via `/vehicles/([^/?#]+)` (`:180-182`), writes body + first 256 bytes (hex or text), `:265-266` called before replying.
- **Frame spec:** filename `YYYYmmdd-HHMMSS-<md5[:8]>.bin`; info fields `req_bytes`, `req_md5`, `req_ctype`, `vin`, `mode`, `req_file`, `req_head`.
- **Gates and timing:** capture happens in both observe and reply modes; write errors swallowed (`:200-201`).
- **State machine:** `None found`.
- **Tests:** `None found`.
- **Evidence grade:** CODE-READ.
- **Verdict:** CONFIRMED.
- **Adapt to TeslaCANModder?** no impact (privacy-sensitive vehicle data logging).

## 6. Summary vs code contradiction table

No live `docs/legacy/Arispex-Tesla-drug.md` existed before this analysis; pre-replace docs are `README.md` and `docs/legacy/tesla-drug-forks-2026-11-20.md` (the latter read as provenance lead only).

| # | Summary claim (cite) | Code fact (cite) | Resolution |
| --- | --- | --- | --- |
| C1 | README:54 — success iff HIT byte count matches `payload.json`; README:132 calls `payload-282b.json` the "official original" (282 B). | `app/payload.json:1` is 602 B/21 fields, MD5 `241cb01b…`; `fsd_panel.py:46-47` badges "official" only for 282 B / `c212a75c…`. A real HIT therefore reports 602 B and the config page shows "非官方载荷" (non-official). | Shipped default contradicts the "official payload" narrative; treat README's success criterion as SUMMARY-ONLY. |
| C2 | `fsd_panel.py:47` canonical MD5 `c212a75ca858750ac302093c138991f2` (282 B) is presented as "the" 282 B payload. | `fsd_mock.py:34-44` `DEFAULT_PAYLOAD` compacts to 282 B but MD5 `104ccde7d817ab00667f2d98326259ec` (different key order, verified with `json.dumps(separators=(",",":"))`). Both are 282 B; only the `payload-282b.json` file order yields `c212a75c…`. | Internal contradiction: byte count alone does not identify the canonical body; key order is load-bearing for the checksum. |
| C3 | README:3 frames it as an "FSD switches all on" config. | `app/payload.json:1` also sets `disable_cabin_camera:true`, `wifi_disabled_mode:true`, `disable_eng_voice_recording_uploads:true`, `ethtrace_auto_upload:false`, `tcpdump_enabled:false` — it suppresses logging/telemetry and camera, not only FSD flags. | README understates payload scope; whole payload is UNVERIFIED and must not be imported. |
| C4 | `docs/legacy/tesla-drug-forks-2026-11-20.md:17` labels this fork "Different PowerShell script revisions" (implies a narrow delta). | Snapshot carries two complete topologies (hotspot + router/ARP), two dashboards, installer, watchdog, cert chain — not merely script revisions. | Catalog note is shallow relative to code; use this document instead. |

## 7. CAN frame and signal inventory

`None found` — no CAN bus code, CAN IDs, DLCs, counters, or checksums exist in this repo (negative anchor in §16: `MCP2515|0x3FD|0x370|CANFRAME` over `app/` → no output). The only frame-like artifacts are Ethernet ARP/DNS/TCP packets produced by `app/tools/arp_mitm.py` (documented as L2 in M5, not CAN) and HTTP responses in M1.

## 8. Constants and tuning table

| Constant | Value | File:line | Meaning | Grade |
| --- | --- | --- | --- | --- |
| `PATH_RE` | `/feature_configuration/?($|[?#])` | `app/fsd_mock.py:46` | Spoof trigger | CODE-READ |
| Checksum | `md5(body).hexdigest()` | `app/fsd_mock.py:68,225,281` | `X-Checksum` header | CODE-READ |
| `DEFAULT_PAYLOAD` | 9 fields, 282 B, MD5 `104ccde7…` | `app/fsd_mock.py:34-44` | Fallback body when no `FSD_MOCK_JSON` | CODE-READ |
| Deployed payload | 21 fields, 602 B, MD5 `241cb01b…` | `app/payload.json:1` | Actually served body | CODE-READ |
| `CANON_BYTES` / `CANON_MD5` | `282` / `c212a75ca858750ac302093c138991f2` | `app/web/fsd_panel.py:46-47` | "Official" badge test | CODE-READ |
| `CANON_MD5` counterpart | 282 B, MD5 `104ccde7…` | `fsd_mock.py:34-44` (computed) | In-code default, different order | CODE-READ |
| Redirect domains | `mothership.vn.teslamotors.com` | `app/dns_sink.py:172` | A-record hijack target | CODE-READ |
| `REDIRECT` set | `mothership.vn.teslamotors.com`, `mothership.vn.cloud.tesla.cn`, `api-prd.vn.cloud.tesla.cn` | `app/tools/arp_mitm.py:39` | ARP/DNS hijack set | CODE-READ |
| `REDIRECT_PREFIX` | `("mothership.",)` | `app/tools/arp_mitm.py:40` | Wildcard hijack | CODE-READ |
| Upstream DNS | `223.5.5.5,1.1.1.1` | `app/dns_sink.py:173`, `arp_mitm.py:41` | Forward resolver | CODE-READ |
| DNS TTL | `60` s | `dns_sink.py:61`, `arp_mitm.py:181` | A-record TTL | CODE-READ |
| Poison interval | `1.0` s | `app/tools/arp_mitm.py:156` | ARP replay cadence | CODE-READ |
| `LOCK_PORT` | `45799` | `app/svc/stack.py:41` | Single-instance lock | CODE-READ |
| Stack poll | `8` s | `app/svc/stack.py:221` | Supervisor cadence | CODE-READ |
| Supervisor interval | `30` s | `app/tools/supervise_all.py:27` | Outer watchdog cadence | CODE-READ |
| ARP freshness | `120` s | `app/tools/supervise_all.py:71` | "ARP alive" threshold | CODE-READ |
| Car / gateway / our IP | `192.168.199.95` / `.1` / `.148` | `start_all.ps1:92-94`, `supervise_all.py:77-79`, `fsd_panel.py:36-38` | Router-mode fixed addresses | CODE-READ |
| `rst_dst` | `52.81.9.181` | `start_all.ps1:94`, `supervise_all.py:79` | Fixed TCP-RST victim IP | CODE-READ |
| Hotspot SSID / PSK | `FSD-PC` / `fsd2026!!` | `route-b.ps1:14-15`, `hotspot.ps1:18-19`, `keep_hotspot.ps1:47` | Hardcoded Wi-Fi credentials | CODE-READ |
| Listen ports | `53,443,4567,8080,8899,8900` | `start_all.ps1:59-65`, `fsd_panel.py:739`, `server.py:621` | Service surface | CODE-READ |
| `IGNORE_HOSTS` | `^(hermes-prd|hermes-stream-prd|telemetry-prd)\.vn\.cloud\.tesla\.cn(?::\d+)?$` | `server.py:37`, `stack.py:34` | Pass-through telemetry hosts | CODE-READ |
| Payload size cap | `8192` B | `fsd_panel.py:155,207` | Editor limit | CODE-READ |
| `CAR_CAP` | `999` | `fsd_panel.py:34` | Car-request rolling cache cap | CODE-READ |
| mitm flags | `--emudhra`, `ssl_insecure=true`, `connection_strategy=lazy` | `start_hotspot.ps1:99-101`, `stack.py:116-118` | TLS interception config | CODE-READ |
| Request cadence | `12 h ± 2 h` | `README.md:56` | Claimed car interval | SUMMARY-ONLY |

## 9. Failure modes and edge cases

| Failure | Handling code path | Tested? | Grade |
| --- | --- | --- | --- |
| mitmproxy port already bound | `server.py:264-265` returns `already`; `stack.py:180-189` skips spawn if port open | No | CODE-READ |
| UDP 53 occupied by hotspot DNS proxy | `stack.py:84-95,146-155` bind-probe → `port_in_use`; `start_hotspot.ps1:112-114` prints `netstat` hint | No | CODE-READ |
| Local IP changes (WiFi/network switch) | `stack.py:166-176` terminates and restarts DNS sink | No | CODE-READ |
| Duplicate launchers | `stack.py:45-55,194-197` TCP 45799 lock | No | CODE-READ |
| DNS health probe fails | `stack.py:124-143` byte-compare expected reply, logged at `:205-210` | No | CODE-READ |
| Child process exits | `stack.py:158-162` poll + `proc.pop`, respawn at `:180-189` | No | CODE-READ |
| TLS handshake fails | `fsd_mock.py:321-332` `tls_failed_client` logs error | No | CODE-READ |
| Request-body write fails | `fsd_mock.py:200-201` `except OSError: pass` (silent loss) | No | CODE-READ |
| `mode.json` missing/invalid | `fsd_mock.py:156-169` keeps prior mode, defaults `reply` at `:77` | No | CODE-READ |
| `payload.json` invalid JSON / non-dict | `fsd_mock.py:215-221` ignores reload; `server.py:216-224` falls back to `DEFAULT_PAYLOAD` | No | CODE-READ |
| Stale config form submission | `fsd_panel.py:157-163` compares `_rev` md5, logs `config-stale` | No | CODE-READ |
| MAC resolution failure | `arp_mitm.py:102-123,318-322` ping + `getmacbyip` + `arp -a`, exits rc=2 | No | CODE-READ |
| ARP restore on exit | `arp_mitm.py:158-168,293-295` sends 3 restore replies | No | CODE-READ |
| Car offline / idle | `fsd_panel.py:365-393` heuristic from last hit/DNS; `server.py:444-455` ping; 180 s idle window | No | CODE-READ |
| Reboot / power loss | HKCU Run autostart (`install.ps1:421`, `start_all.ps1:126`) + watchdog | No | CODE-READ |
| OTA guard / bus-off / queue overflow / brownout state | `None found` — no CAN layer, no OTA handling in this snapshot | No | CODE-READ |

## 10. Test coverage map

No test files exist in `git ls-files`; no `def test_`, `unittest`, or `pytest` anchors hit (§16).

| Test file | Asserts |
| --- | --- |
| `None found` | No automated tests. |

Critical paths with no test (risk in one line each):

- `X-Checksum = md5(body)` (`fsd_mock.py:68`) — a wrong serialization/order silently fails vehicle validation; only a live car would reveal it.
- `PATH_RE` (`fsd_mock.py:46`) — over- or under-matching would spoof the wrong endpoint or miss the real one.
- reply/observe gate (`fsd_mock.py:265-272`) — an observe-mode regression would mutate traffic when the operator wants capture-only.
- DNS answer bytes (`dns_sink.py:61-78`) — a malformed RR would blackhole the car's DNS.
- ARP poison/restore (`arp_mitm.py:145-168`) — failed restore leaves the car's gateway MAC poisoned after exit.
- RST injector (`arp_mitm.py:248-259`) — incorrect `seq`/direction could terminate unrelated connections.
- Payload hot-reload (`fsd_mock.py:204-226`) — partial/invalid writes could change the served body mid-session.
- Watchdog/autostart (`supervise_all.py:60-103`) — unintended persistence and process kills on the host.
- `selftest()` (`server.py:321-371`) is a live-network probe, not a deterministic test oracle.

## 11. Parity matrix with TeslaCANModder

No shared signal or frame surface exists. Every row below is SUMMARY-ONLY because there is no our-side CODE-READ counterpart (negative anchor in §16: `git grep -nE 'feature_configuration|X-Checksum|mitmproxy|arp_mitm' -- firmware/ client/ packages/ tools/` → no output). No row can drive an adaptation decision.

| Signal / frame | Upstream decode / behavior (cite) | Our decode (cite) | Delta | Action |
| --- | --- | --- | --- | --- |
| `/feature_configuration` HTTP response | `fsd_mock.py:46,276-285` | None found (no our-side CODE-READ) | Different layer entirely | ignore |
| `X-Checksum` (MD5 over body) | `fsd_mock.py:68,281` | None found (no our-side CODE-READ) | HTTP header vs. our CAN CRC work; no shared bytes | ignore |
| DNS A-record hijack | `dns_sink.py:61-66,129-142` | None found | No our-side DNS surface | ignore |
| ARP poison / L2 forward / RST | `arp_mitm.py:145-168,207-259` | None found | No our-side L2 surface | ignore |
| TLS MITM CA | `certs2/*`, `start_hotspot.ps1:98-102` | None found | No our-side TLS interception | ignore |
| Config hot-reload + checksum recompute | `fsd_mock.py:204-226` | None found | Generic pattern only, no shared data | ignore |

## 12. Surface inventory and security notes

| Surface | Endpoint / mechanism | Semantics | Cite |
| --- | --- | --- | --- |
| mitmproxy reverse | TCP 4567 | HTTP reverse to upstream, spoof addon | `server.py:240-243` |
| mitmproxy proxy | TCP 8080 | Regular HTTP/CONNECT proxy | `server.py:243` |
| HTTPS reverse | TCP 443 | `reverse:https://api-prd.vn.cloud.tesla.cn@443` | `start_hotspot.ps1:101` |
| DNS sink | UDP 53 | A-record hijack + forward | `dns_sink.py:150-159` |
| Dashboard (new) | HTTP 8900 `/`, `/api/state` | SSR + JSON state | `fsd_panel.py:724-733` |
| Config editor | 8900 `/config` GET/POST | Field editor, writes `payload.json`, 8 KiB cap | `fsd_panel.py:135-195` |
| Payload editor | 8900 `/payload` GET/POST | Raw JSON editor | `fsd_panel.py:197-232` |
| Dashboard (legacy) | HTTP 8899 `/`, `/api/state` | Flask + static `index.html` | `server.py:528-538` |
| Process control | 8899 `/api/proxy/<mode>/start\|stop`, `/api/hotspot/<action>`, `/api/logs/clear`, `/api/selftest` | Spawns/kills processes, toggles hotspot, clears evidence | `server.py:541-616` |
| Payload/mode API | 8899 `/api/payload`, `/api/payload/reset`, `/api/mode` | Writes `payload.json` / `mode.json` | `server.py:572-603` |
| WinRT tethering | `hotspot.ps1`, `hotspot_probe.ps1` | Starts/stops Wi-Fi Direct hotspot, lists clients | `hotspot.ps1:37-89` |
| Firewall | `New-NetFirewallRule` TCP 443/4567/8080/8899, UDP 53 | Inbound allow, UAC self-elevation | `start_all.ps1:58-79` |
| Persistence | HKCU Run `fsd-mitm-supervisor` | Runs `start_hotspot.ps1` / `supervise_all.py` at logon | `install.ps1:410-426`, `start_all.ps1:123-127` |
| Host process control | `taskkill /T /F`, `Stop-Process -Force`, command-line regex | Kills `C:\fsd-mitm\venv*` or python processes by pattern | `stop_stack.ps1:10-15`, `stop_fsd_all.ps1:5-13` |

**Security / provenance findings:**

- **Untrusted private key material:** `app/certs2/mitmproxy-ca.pem:1` is `-----BEGIN RSA PRIVATE KEY-----` (CA signing key); `mitmproxy-ca.p12` (2384 B) and `mitmproxy-ca-cert.p12` (1035 B) are PKCS#12 bundles; never import or trust. Provenance doc warns ZIP/cert/key material in forks is untrusted (`tesla-drug-forks-2026-11-20.md:10`).
- **Hardcoded credentials:** Wi-Fi PSK `fsd2026!!` in three scripts (`route-b.ps1:15`, `hotspot.ps1:19`, `keep_hotspot.ps1:47`).
- **TLS verification disabled:** `ssl_insecure=true` (`start_hotspot.ps1:99`, `stack.py:116`); `--emudhra` flag passed to mitmdump.
- **Active network manipulation:** ARP poisoning and TCP RST injection to a fixed IP (`arp_mitm.py:145-168,248-259`).
- **Privilege/persistence:** UAC self-elevation (`start_all.ps1:73-77`), HKCU Run autostart, broad process kills by regex (risk of killing unrelated processes).
- **Data capture:** VINs, raw request bodies, SNI, cipher, client IPs written to `logs/` (`fsd_mock.py:175-202,229-256`); `Clear-FSD-Records.ps1` exists to erase that evidence.
- **No license file** in the tree — legal status UNVERIFIED.

## 13. Adaptation assessment

| Mechanism | Value | Cost | Risk | Evidence grade | Decision |
| --- | --- | --- | --- | --- | --- |
| M1 response spoof + MD5 | Low for us (no HTTP surface) | High | Safety-critical, unverified unlock | CODE-READ | never port |
| M2 reply/observe gate | Medium (capture-first pattern) | Low | Gate regression if miscopied | CODE-READ | candidate for future PR (pattern only) |
| M3 hot-reload + checksum recompute | Medium (config pipeline + independent oracle) | Low | Key-order sensitivity | CODE-READ | candidate for future PR (pattern only) |
| M4 DNS hijack | None for us | Medium | Network attack | CODE-READ | never port |
| M5 ARP/L2/RST | None for us | High | Disrupts network | CODE-READ | never port |
| M6 hotspot topology | None for us | Medium | Hardcoded PSK, WinRT-only | CODE-READ | never port |
| M7 TLS MITM + CA | None for us | High | Untrusted keys, disabled verification | CODE-READ | never port |
| M8 watchdog + single-instance lock | Low (robustness pattern) | Low | Persistence/autostart half is unsafe | CODE-READ | candidate for future PR (pattern only) |
| M9 dashboards + logging | None for us | Low | Privacy-sensitive logs | CODE-READ | no impact |
| M10 request capture + VIN | None for us | Low | PII/vehicle data | CODE-READ | no impact |

For any candidate that ever touches CAN frame shape, routing, checksums, or transport-visible messages, `docs/checklists/can-review-checklist.md` plus a listen-only bench capture are required. Port behavior/constants/byte layout only, under our names and style, with an independent test oracle; never import legacy files.

## 14. Do-not-copy list

| Item (file:line) | Reason |
| --- | --- |
| `app/certs2/mitmproxy-ca.pem:1` and full file | RSA private CA key — untrusted, enables TLS impersonation |
| `app/certs2/mitmproxy-ca.p12`, `mitmproxy-ca-cert.p12`, `mitmproxy-ca-cert.cer`, `mitmproxy-ca-cert.pem`, `mitmproxy-dhparam.pem` | Untrusted cert/key/DH material; structural-parsed only, never redistributed |
| `app/payload.json:1`, `app/payload-282b.json:1`, `app/payload-16f.json:1` | Unverified FSD/telemetry flag sets; also alter camera/logging behavior |
| `app/fsd_mock.py:276-285` | Response short-circuit that spoofs a vehicle safety endpoint |
| `app/tools/arp_mitm.py:145-168,207-259` | ARP poisoning, L2 interception, and TCP RST injection |
| `app/start_hotspot.ps1:98-102`, `app/start_all.ps1:101-105` | MITM TLS flags (`ssl_insecure`, `--emudhra`, fixed upstream) |
| `install.ps1:410-426`, `app/start_all.ps1:123-127` | HKCU Run autostart / persistence |
| `app/start_hotspot.ps1:59-81`, `app/start_all.ps1:58-79` | UAC self-elevation and inbound firewall rule creation |
| `app/route-b.ps1:14-15`, `app/hotspot.ps1:18-19`, `app/tools/keep_hotspot.ps1:47` | Hardcoded Wi-Fi PSK |
| `app/Clear-FSD-Records.ps1:61-69` | Evidence-erasure routine |
| `README.md:1-237` and all orchestration scripts | Unverified FSD-unlock claims; no license grants reuse |
| Any ZIP/archive material | None in this snapshot, but the fork family carries uninspected archives (`tesla-drug-forks-2026-11-20.md:10`) — treat any future archive as untrusted |

## 15. History and evolution

Local submodule history contains exactly one commit: `ca02db4 Initial commit` (HEAD, `main`, `origin/main`). No changelog, no tags, no prior revisions, and no alternative branches are present. Consequently **no abandoned approaches or rollbacks can be observed from evidence** — inferences such as "hotspot mode was added after router mode" (two topologies and two dashboards coexist) are UNVERIFIED. The provenance catalog describes this fork as having "different PowerShell script revisions" (`tesla-drug-forks-2026-11-20.md:17`) and records the original `zeratulhx/Tesla-drug` as HTTP 404 on 2026-11-20 (`:7`). Network-only claims (issues, PRs, deletion vs. private) remain UNVERIFIED.

## 16. Re-verification recipe

Pin in canonical form: `160000 ca02db4f831ec4fdb148e57684c1dede050ffa45 0`.

```bash
OLD=ca02db4f831ec4fdb148e57684c1dede050ffa45
git ls-files -s legacy/Arispex-Tesla-drug          # expect 160000 $OLD 0
git -C legacy/Arispex-Tesla-drug rev-parse HEAD    # expect $OLD
git -C legacy/Arispex-Tesla-drug status --porcelain # expect empty
git -C legacy/Arispex-Tesla-drug fetch --tags --prune   # may fail if upstream removed
git -C legacy/Arispex-Tesla-drug log --oneline $OLD..origin/main
git -C legacy/Arispex-Tesla-drug diff --stat $OLD..origin/main
git -C legacy/Arispex-Tesla-drug diff $OLD..origin/main -- app/
```

Demonstrated anchors (run 2026-11-21 with `git grep -nE`; expected hits pasted):

```text
git -C legacy/Arispex-Tesla-drug grep -nE 'X-Checksum|hashlib\.md5\(self\.body\)' -- app/fsd_mock.py
  -> app/fsd_mock.py:5,68,225,281
git -C legacy/Arispex-Tesla-drug grep -nE 'feature_configuration' -- app/fsd_mock.py
  -> app/fsd_mock.py:2,4,46,102
git -C legacy/Arispex-Tesla-drug grep -nE 'mothership\.vn\.teslamotors\.com|api-prd' -- app/dns_sink.py app/tools/arp_mitm.py
  -> dns_sink.py:4,7,172 ; arp_mitm.py:39,310,314,315
git -C legacy/Arispex-Tesla-drug grep -nE 'CANON_BYTES|CANON_MD5' -- app/web/fsd_panel.py
  -> app/web/fsd_panel.py:46,47,177,216
git -C legacy/Arispex-Tesla-drug grep -nE 'api-prd\.vn\.cloud\.tesla\.cn@443|fsd-mitm-supervisor' -- app/start_hotspot.ps1 install.ps1
  -> app/start_hotspot.ps1:101 ; install.ps1:421,422
git -C legacy/Arispex-Tesla-drug grep -nE '_maybe_reload|_load_mode|8192' -- app/fsd_mock.py app/web/fsd_panel.py
  -> fsd_mock.py:76,79,152,204,260,261 ; fsd_panel.py:155,207
git -C legacy/Arispex-Tesla-drug grep -nE 'BEGIN RSA PRIVATE KEY' -- app/certs2/mitmproxy-ca.pem
  -> app/certs2/mitmproxy-ca.pem:1
```

Negative anchors (absence claims — confirmed no output):

```text
git -C legacy/Arispex-Tesla-drug grep -nE 'MCP2515|0x3FD|0x370|CANFRAME|canRingBuffer' -- app/   -> no output
git -C legacy/Arispex-Tesla-drug grep -nE 'def test_|unittest|pytest' -- app/                      -> no output
git grep -nE 'feature_configuration|X-Checksum|mitmproxy|arp_mitm|dns_sink' -- firmware/ client/ packages/ tools/  -> no output
```

If line numbers drift more than ~10, re-read the whole function; do not trust this document's numbers. Pre-pin upstream history was **not** reviewed (local clone has a single commit). `docs/legacy/**` is lint-ignored, so verify hygiene manually.

## 17. Open questions and explicitly not investigated

- **Pre-pin history:** not reviewed. Only the pinned `ca02db4` snapshot exists locally; earlier revisions, if any, are unknown.
- **Network-only claims:** original `zeratulhx/Tesla-drug` 404, deletion vs. private status, issues/PRs, and other forks' content are UNVERIFIED.
- **Vehicle effect:** no CAPTURE evidence; nothing proves any flag actually unlocks FSD or that a car accepted the response. HIT logs only prove an HTTP reply was returned.
- **Request cadence:** README's "12 h ± 2 h" and "5–10 min after reboot" are SUMMARY-ONLY, not derivable from code.
- **Payload provenance:** who captured `payload-282b.json` / `payload-16f.json`, against which vehicle firmware, and whether they are genuine is UNVERIFIED.
- **Binary material:** `.p12`/`.cer` contents were not unpacked or certificate-parsed beyond PEM header/size/MD5; subject/expiry/key usage remain UNVERIFIED.
- **License:** no license file; redistribution rights UNVERIFIED.
- **Domain/IP freshness:** `mothership.vn.teslamotors.com`, `api-prd.vn.cloud.tesla.cn`, and `52.81.9.181` were not checked for current validity (2026-11-21).
- **Other three pinned forks:** not compared in this task (scope was this repo only).
- **Our-side CAN review:** no CAN-relevant behavior was found, so no `can-review-checklist.md` walk was triggered; if a pattern from §13 is ever ported, that checklist and a bench capture become mandatory.
