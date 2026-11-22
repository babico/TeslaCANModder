---
title: jdgbv2 / Tesla-drug
description: Windows PC-side HTTP/TLS MITM kit that spoofs Tesla feature_configuration responses; research-only, no CAN code
category: legacy
folder: legacy
tags: [legacy, community, external, unverified, mitm]
author: jdgbv2
repo: Tesla-drug
---

# jdgbv2 / Tesla-drug

> Research-only, unverified FSD-unlock material. One of four pinned reference snapshots of a deleted upstream (`zeratulhx/Tesla-drug`, HTTP 404). Do not run against a vehicle. No payload, archive, certificate, key, or script in this snapshot was executed or unpacked; all binary/PKCS#12 material was only structurally parsed. No code was copied into shipping firmware, client, protocol, or tools.

## 1. Provenance

- Submodule path: `legacy/jdgbv2-Tesla-drug`.
- Pinned gitlink: `160000 05d2b265a372d43f2eaf0e00bf7ce8be7d5c05d8 0`. `git -C legacy/jdgbv2-Tesla-drug rev-parse HEAD` returns the same SHA; `git -C legacy/jdgbv2-Tesla-drug status --porcelain` is empty. Analyst did not mutate the submodule.
- Upstream: `https://github.com/jdgbv2/Tesla-drug` (origin fetch/push). Original `zeratulhx/Tesla-drug` returned HTTP 404 on 2026-11-20; a 404 does not establish deletion vs. private — `docs/legacy/tesla-drug-forks-2026-11-20.md:7`.
- Provenance record: `docs/legacy/tesla-drug-forks-2026-11-20.md:8,10,16` — pinned reference snapshot, "PowerShell/Python intercept scripts; original installer variant"; ZIP/certificate/key material in forks is untrusted and not executed or validated (`:10`).
- License: **None found** in snapshot — no `LICENSE`/`COPYING` in `git ls-files`. Rights are unknown; treat as all-rights-reserved. This is a hard blocker on any code reuse.
- Version at pin: HEAD `05d2b26 Fix PowerShell 5.1 ANSI parsing: ensure UTF-8 BOM on all .ps1` (2026-11-20 09:56:52 +0800). Four local commits total; no tags.
- Analysis date: 2026-11-21.
- Document mode: **replace**, but no pre-existing `docs/legacy/jdgbv2-Tesla-drug.md` existed (`Test-Path` = False). No YAML frontmatter and no `## Upstream (...)` section to preserve; frontmatter was created new per contract. Pre-replace summary claims therefore come from upstream `README.md`, cited directly below — not from `git show`.

Files read in full (FULL-READ): `README.md:1-237`, `fsd_mock.py:1-334`, `dns_sink.py:1-179`, `svc/stack.py:1-225`, `tools/arp_mitm.py:1-332`, `tools/supervise_all.py:1-107`, `web/fsd_panel.py:1-747`, `web/server.py:1-635`, `install.ps1:1-480`, `start_hotspot.ps1:1-128`, `start_all.ps1:1-131`, `hotspot.ps1:1-89`, `route-b.ps1:1-25`, `stop_api_intercept.ps1:1-95`, `tools/keep_hotspot.ps1:1-54`, `Clear-FSD-Records.ps1:1-70`, `payload.json:1-1` (single line, 22 fields), `payload-16f.json:1-1` (16 fields), `payload-282b.json:1-1` (9 fields), `mode.json:1-1`, `requirements.txt:1-44`, `.gitignore:1-5`.

Files STRUCTURAL-PARSE only: `app/certs2/mitmproxy-ca.pem` (line 1 `-----BEGIN RSA PRIVATE KEY-----`, line 28 `-----BEGIN CERTIFICATE-----`), `app/certs2/mitmproxy-ca-cert.pem` / `.cer` (`BEGIN CERTIFICATE`), `app/certs2/mitmproxy-dhparam.pem` (line 2 `BEGIN DH PARAMETERS`), `app/certs2/mitmproxy-ca.p12` and `mitmproxy-ca-cert.p12` (binary PKCS#12, header bytes only, not opened). No key material was imported or used.

Files NOT read: `app/web/static/index.html` (18,614-byte static asset served by `web/server.py:528-530`); `app/svc/stack_start.cmd`, `app/svc/stop_stack.ps1`, `app/start_all.cmd`, `app/start_hotspot.cmd`, `app/stop_hotspot.cmd`, `app/stop_fsd_all.ps1`, `app/web/hotspot_probe.ps1`, `app/stop_hotspot.ps1`, `一键安装.cmd` (keys/ports/paths for these were collected by targeted grep only). No test files exist in the snapshot.

Evidence vocabulary: no TEST-READ evidence exists — this repo ships no tests. No CAPTURE evidence exists in this repo — no captured frames or decode-tool output is committed; all "success" evidence is the project's own log schema.

## 2. Executive verdict

This is a Windows-only Python/PowerShell **network MITM toolkit**, not a CAN or firmware project. It makes a PC answer Tesla's `/feature_configuration` config fetch with a canned 200 JSON whose `full_self_driving`-style flags are all enabled and whose `X-Checksum` is recomputed as `md5(body)` (`fsd_mock.py:33-44,50-51,68,276-285`). Two mutually exclusive topologies deliver it: hotspot mode, where the PC is the car's gateway/DNS and a UDP-53 sink answers `api-prd.vn.cloud.tesla.cn` locally (`dns_sink.py`, `start_hotspot.ps1`); and router mode, where Scapy ARP-poisoning redirects the car's traffic through the PC, with L2 forwarding, DNS answers, and TCP RST to a pinned upstream (`arp_mitm.py`, `start_all.ps1`). Supporting machinery — watchdogs, single-instance lock, HKCU Run persistence, hidden processes, config hot-reload, and an evidence dashboard — is operationally mature but security-hostile. **No CAN frame is decoded, mutated, or injected anywhere in this repo**, so there is essentially no mechanism overlap with TeslaCANModder's shipping code; the only transferable *ideas* are checksum-recompute-on-mutate, an observe/reply gate, hot-reload with a stale-write guard, and watchdog self-heal — all of which we already implement differently. Must never be ported: the MITM scripts, the MITM CA private key and PKCS#12 bundles, the hardcoded Wi-Fi key, the startup-persistence and UAC/firewall plumbing, and the payload flags themselves. Treat every feature-unlock claim as UNVERIFIED.

## 3. Evidence map

| Source file | Lines | What it proves | Grade |
| --- | --- | --- | --- |
| `README.md` | 1-58, 120-153 | Intended operation: PC answers car config fetch; hotspot vs. router link; 12 h cadence claim | SUMMARY-ONLY |
| `README.md` | 155-237 | Rollback payload labels, port map, hardcoded-path admission (~24 sites) | SUMMARY-ONLY |
| `fsd_mock.py` | 33-51 | 9-field canned payload, compact JSON, `md5(body)` checksum | CODE-READ |
| `fsd_mock.py` | 46, 140-149 | `PATH_RE` match plus host/method gates for interception | CODE-READ |
| `fsd_mock.py` | 152-173, 260-272 | `mode.json` hot-reload; `observe` forwards upstream, `reply` short-circuits | CODE-READ |
| `fsd_mock.py` | 175-202, 300-332 | Request capture (VIN, md5, raw body to disk) and TLS handshake/SNI logging | CODE-READ |
| `fsd_mock.py` | 204-226, 276-290 | External payload hot-reload; 200 response with `X-Checksum`, `no-store` | CODE-READ |
| `dns_sink.py` | 45-78, 113-148 | DNS query parse; A→local IP, AAAA→empty, TTL 60; non-redirect forwarded | CODE-READ |
| `dns_sink.py` | 162-175 | Default redirect `mothership.vn.teslamotors.com`, upstream `223.5.5.5,1.1.1.1` | CODE-READ |
| `svc/stack.py` | 24-34, 98-118 | Pinned paths, IGNORE regex, mitmproxy flags (`ssl_insecure`, `--emudhra`) | CODE-READ |
| `svc/stack.py` | 39-55, 121-191, 194-221 | Lock port 45799, child supervision, IP-change DNS restart, 8 s loop | CODE-READ |
| `tools/arp_mitm.py` | 32-46, 145-168 | Scapy imports, redirect set, 1 s ARP poison + restore | CODE-READ |
| `tools/arp_mitm.py` | 178-216, 219-275 | DNS A answer, L2 forward, RST injection, SNI read-only extraction | CODE-READ |
| `tools/supervise_all.py` | 19-27, 60-103 | 30 s watchdog; liveness by ports 4567/443/8900 and ARP-log mtime <120 s | CODE-READ |
| `web/fsd_panel.py` | 27-47, 67-89, 135-232, 724-733 | Dashboards, config write + `.bak` + history, CANON badge, `/api/state` | CODE-READ |
| `web/server.py` | 25-49, 230-371, 528-616 | Legacy 8899 dashboard, process control, live `selftest()` probe, control APIs | CODE-READ |
| `install.ps1` | 14-29, 183-209, 267-291, 399-445 | C:\fsd-mitm hardcode, ports, robocopy, UTF-8 BOM fix, shortcuts, HKCU Run | CODE-READ |
| `start_hotspot.ps1` | 15, 59-122 | Hotspot topology: no ARP poison, 443/DNS/8900 launch, firewall rules | CODE-READ |
| `start_all.ps1` | 59-127 | Router topology: 5 firewall rules, hardcoded IPs/RST target, watchdog, autostart | CODE-READ |
| `route-b.ps1` / `hotspot.ps1` / `tools/keep_hotspot.ps1` | 14-24 / 16-20, 54-80 / 30-53 | SSID `FSD-PC`, key `fsd2026!!`, WinRT tethering, keepalive | CODE-READ |
| `app/certs2/*` | structural | CA private key + cert + DH params + 2 PKCS#12 bundles | STRUCTURAL-PARSE |
| `payload-282b.json` | 1-1 | 9 fields, 282 bytes, md5 `c212a75ca858750ac302093c138991f2` (verified by hash) | CODE-READ |
| `payload-16f.json` / `payload.json` | 1-1 | 16 / 22 fields; both still set `full_self_driving:true` | CODE-READ |
| `docs/legacy/tesla-drug-forks-2026-11-20.md` | 7,10,16 | Fork provenance, 404 upstream, untrusted material warning | SUMMARY-ONLY |
| Our-side absence check | negative grep | No `feature_configuration`/`mothership`/`api-prd`/`X-Checksum`/`mitmproxy` in `firmware/`, `client/src/`, `packages/`, `tools/` | CODE-READ |

## 4. Architecture and build surface

- Entry points: `一键安装.cmd` → `install.ps1` (7 steps: env → target → copy → venv → health → shortcuts → autostart → launch). Operators then use `start_hotspot.cmd`/`start_hotspot.ps1` (hotspot mode) or `start_all.cmd`/`start_all.ps1` (router mode); both end in `fsd_panel.py:8900`.
- Two process trees: hotspot mode = 443 mitmdump reverse + `dns_sink.py` + `fsd_panel.py` + `keep_hotspot.ps1`; router mode = `svc/stack.py` (DNS/reverse/proxy/web children) + `arp_mitm.py` + 443 mitmdump + `fsd_panel.py` + `supervise_all.py`. `stack.py:180-189` uses ports 4567/8080/8899; `supervise_all.py:64-100` uses 4567/443/8900 plus ARP-log freshness.
- Shared core vs. platform port: none. Everything is Windows-specific — `C:\fsd-mitm` hardcodes (`install.ps1:14,183`; `supervise_all.py:19`), WinRT tethering (`hotspot.ps1:24-43`), `New-NetFirewallRule` (`start_hotspot.ps1:71`), `powershell.exe -ExecutionPolicy Bypass` (`route-b.ps1:24`), `CREATE_NO_WINDOW` flags (`stack.py:109-110`).
- Build flags: none. Deployment is `python -m venv` + `pip install -r requirements.txt` (`install.ps1:339-368`); pinned stack includes `mitmproxy==12.3.1`, `scapy==2.8.0`, `Flask==3.1.3`, `cryptography==50.0.2`, `pydivert==3.1.3` (`requirements.txt:13,14,24,30,36`). No compiler, no firmware, no CAN toolchain.
- Dependency boundaries: mitmproxy addon (`fsd_mock.py`) ↔ filesystem (`payload.json`, `mode.json`, `logs/`); Flask dashboards read the same logs and write the same config; Scapy ARP tool is independent of mitmproxy and only shares DNS log schema.

**None found: no CAN frame pipeline in this repo.** The following is the *network* request/response flow, not a CAN frame flow:

```mermaid
flowchart LR
    CAR["Tesla car head unit"] -->|"HTTPS GET/POST /feature_configuration"| LAYER{"Delivery layer"}
    LAYER -->|"hotspot mode: PC is gateway + DNS"| DNS["dns_sink.py<br/>UDP 53 A -> local IP"]
    LAYER -->|"router mode: no PC gateway"| ARP["arp_mitm.py<br/>ARP poison + L2 forward + RST"]
    DNS --> MITM["mitmdump :443 / :4567<br/>fsd_mock.py addon"]
    ARP --> MITM
    MITM -->|"PATH_RE hit + mode=reply"| FORGE["200 JSON body<br/>X-Checksum = md5(body)"]
    MITM -->|"mode=observe"| UP["Forward to real upstream"]
    FORGE --> CAR
    MITM --> LOGS["logs/*.jsonl + requests/*.bin"]
    LOGS --> PANEL["fsd_panel.py :8900<br/>server.py :8899"]
    classDef net fill:#1e3a5f,stroke:#4a7fb5,color:#fff
    class DNS,ARP,MITM net
```

## 5. Mechanism catalogue

### M1 — Canned `/feature_configuration` response with recomputed checksum

- **Claim (README / summary):** "车机拉功能配置时，这台电脑替服务器回一份「FSD 开关全开」的配置" — `README.md:3`.
- **Code path:** `fsd_mock.py:228-291` — `request()` → `_hit()` (`140-149`) → `_capture()` (`175-202`) → `http.Response.make(200, self.body, {...})` → TX to car. Body precomputed at `:67-68` via `_compact()` → `hashlib.md5().hexdigest()`.
- **Frame spec:** None found (no CAN frame). Wire artifact: HTTP 200, `Content-Type: application/json`, `X-Checksum: <md5 of exact body bytes>`, `Cache-Control: no-store`; body = compact JSON, 9/16/22 fields depending on `payload.json`; trigger regex `/feature_configuration/?($|[?#])` (`:46`).
- **Gates and timing:** `FSD_MOCK` option (`:97-103`); optional `FSD_MOCK_HOSTS` / `FSD_MOCK_METHOD` allowlists (`:69-70`, `:145-148`); optional `FSD_MOCK_JSON` override (`:57-65`); no interval logic — purely request-driven.
- **State machine:** Single request path (hit / observe / no-hit), no multi-state FSM.
- **Tests:** None found.
- **Evidence grade:** CODE-READ.
- **Verdict:** CONFIRMED (implementation); UNVERIFIED (that any car actually unlocks FSD from it).
- **Adapt to TeslaCANModder?** no impact — no CAN frame, no shared transport. Idea "recompute checksum over exact body after mutation" is already present for CAN at `firmware/lib/vehicle/can/feature/fsd/nag.h:485-486` (`f.data[7] = nagChecksum(f.data)`).

### M2 — reply/observe gate via hot-reloaded `mode.json`

- **Claim (README / summary):** "只看不回包 … `{"mode":"observe"}` … 只记录、不回包" — `README.md:134-136`.
- **Code path:** `fsd_mock.py:152-173` — `_load_mode()` → `mode.json` mtime compare → `self.mode`; `:265-272` — on hit, `observe` logs and returns without `flow.response`, so mitmproxy forwards upstream.
- **Frame spec:** None found (no CAN frame). Gate file: `mode.json:1` = `{"mode":"reply"}`; valid values `reply` / `observe` only.
- **Gates and timing:** File mtime polled on every request (`:261`); invalid JSON / unknown mode silently ignored (`:166-169`). No persistence of mode changes beyond the file.
- **State machine:** None found (two values, no transitions beyond file rewrite).
- **Tests:** None found.
- **Evidence grade:** CODE-READ.
- **Verdict:** CONFIRMED.
- **Adapt to TeslaCANModder?** no impact. Analogous safety gate exists in our code via `State::apGateOpen()` (`firmware/lib/core/types.h:837`), consulted before FSD frame edits (`firmware/lib/vehicle/can/handler/variant/hw4.h:67,136`).

### M3 — Payload hot-reload and "official 282B" badge

- **Claim (README / summary):** "保存即生效，不用重启" and "回滚：`payload-282b.json`（官方原版）、`payload-16f.json`（16 字段版）" — `README.md:130,132`.
- **Code path:** `fsd_mock.py:204-226` — `_maybe_reload()` on payload mtime change → re-`_compact()` → new md5. `web/fsd_panel.py:63-89` — `_save_payload()` writes `payload.json.bak` + `logs/config_history.jsonl`; `:135-232` — `/config` and `/payload` write routes; `:157-163` — stale `_rev` guard.
- **Frame spec:** None found (no CAN frame). Config file: compact JSON object, ≤8192 B enforced (`fsd_panel.py:155-156,207-208`).
- **Gates and timing:** Hot reload only when `FSD_MOCK_JSON` is set (`:206-207`); invalid/non-dict JSON silently ignored (`:215-221`); canonical badge compares exact bytes+md5 (`fsd_panel.py:46-47,177-179,216-218`).
- **State machine:** None found.
- **Tests:** None found.
- **Evidence grade:** CODE-READ (hot reload, badge); UNVERIFIED (key-order/md5 mismatch consequence — see section 6 C1, hash computed at write time, not a runtime test).
- **Verdict:** PARTIAL — reload works, but "official" badge and in-code default disagree.
- **Adapt to TeslaCANModder?** no impact. Our config persistence is NVS-based (`firmware/lib/core/persist/nvs.h:14`); different layer, no shared contract.

### M4 — UDP-53 DNS sink redirect

- **Claim (README / summary):** "DNS 劫持 … 网关和 DNS 都是 `192.168.137.1`" — `README.md:30,38`.
- **Code path:** `dns_sink.py:113-148` — `handle()` → `qname_of()` (`:45-58`) → `build_answer()` (`:61-66`) / `build_empty()` / `build_nxdomain()` → `sock.sendto`; 4 daemon threads (`:156-157`).
- **Frame spec:** None found (no CAN frame). Wire artifact: DNS UDP reply, `QR=1 AA=1`, A TTL 60 to `local_ip_for(peer)` (`:27-42,132-134`); AAAA/other → empty answer forcing A (`:135-138`).
- **Gates and timing:** Redirect set from `--redirect` (default `mothership.vn.teslamotors.com`, `:172`); special loopback health name `FSD_DNS_HEALTH_NAME` (`:85,125-127`); `SO_EXCLUSIVEADDRUSE` on Windows (`:152-153`); non-redirect forwarded to upstream with 2 s timeout (`:100-111`).
- **State machine:** None found.
- **Tests:** None found — no DNS wire test; the project's own "status" check is a hand-built query in `stack.py:124-143`, not a test suite.
- **Evidence grade:** CODE-READ.
- **Verdict:** CONFIRMED.
- **Adapt to TeslaCANModder?** no impact — our firmware does not run a DNS server.

### M5 — Hotspot topology (PC as car gateway, no ARP poisoning)

- **Claim (README / summary):** "手机 USB 给电脑供网 → 电脑开热点 → 车机连电脑热点" — `README.md:5`; `start_hotspot.ps1:5-6` explicitly says no ARP poisoning because PC is gateway.
- **Code path:** `route-b.ps1:20-25` → `hotspot.ps1:54-80` (WinRT `NetworkOperatorTetheringManager`) → `start_hotspot.ps1:83-122` (443/DNS/8900 launch) → `dns_sink.py`.
- **Frame spec:** None found (no CAN frame). Network: SSID `FSD-PC`, key `fsd2026!!` (`route-b.ps1:14-15`), subnet `192.168.137.0/24`, gateway `192.168.137.1` (`start_hotspot.ps1:15`).
- **Gates and timing:** Firewall rules `fsd-mitm-api443` (TCP 443) and `fsd-mitm-dns-53` (UDP 53) created via UAC elevation (`start_hotspot.ps1:63-79`); hotspot auto-off after ~5 min idle counters by `keep_hotspot.ps1:9-10,40-53` (90 s poll, 240 min default deadline).
- **State machine:** Hotspot On/Off with keepalive re-arm — see diagram below.
- **Tests:** None found (`start_hotspot.ps1 -Status` at `:44-53` is a runtime probe, not a test).
- **Evidence grade:** CODE-READ.
- **Verdict:** CONFIRMED.
- **Adapt to TeslaCANModder?** no impact.

```mermaid
stateDiagram-v2
    [*] --> ApOff
    ApOff --> ApOn: hotspot.ps1 -Action start
    ApOn --> ApOff: Windows idle timeout / -Action stop
    ApOff --> ApOn: keep_hotspot.ps1 (every 90 s)
    ApOn --> ApOn: keepalive no-op
```

### M6 — Router topology: ARP poison + L2 forward + DNS answer + RST

- **Claim (README / summary):** "ARP 引流 … 常驻栈 … 把车机流量引到本机" — `README.md:159-191`; `start_all.ps1:8-15`.
- **Code path:** `arp_mitm.py:145-168` `poison_loop()`/`restore()` → `:219-275` `handle()` → `:178-216` `_answer_a`/`_answer_empty`/`_forward_dns`/`_forward_l2` → `run()` `:277-296`.
- **Frame spec:** None found (no CAN frame). Ethernet/IP artifacts: ARP reply `psrc=gw_ip, hwsrc=our_mac` every 1 s (`:146-156`); DNS A answer to `--answer`; L2 frames re-written `src=our_mac, dst=gw_mac` (`:207-212`); TCP RST with `seq=t.ack` to `--rst-dst` on ACK (`:248-259`); restore sends 3 genuine-MAC ARP replies on exit (`:158-168`).
- **Gates and timing:** Poison interval 1 s (`:156`); duration 7200 s from `start_all.ps1:94`; watchdog restarts if `arp_mitm.log` mtime older than 120 s (`supervise_all.py:69-80`); MAC resolution 12 tries with ping/`arp -a` fallback (`:102-123`); `--no-api` drops `api-prd.vn.cloud.tesla.cn` from redirect set (`:313-315`).
- **State machine:** Poison → handle (DNS/L2/RST paths) → restore on exit; see diagram.
- **Tests:** None found.
- **Evidence grade:** CODE-READ.
- **Verdict:** CONFIRMED (code), UNVERIFIED (real vehicle behavior and network topology assumptions).
- **Adapt to TeslaCANModder?** never port — active network-layer MITM, no relation to our CAN bus mission.

```mermaid
stateDiagram-v2
    [*] --> ResolvingMAC
    ResolvingMAC --> Failed: target or gateway MAC unresolved
    ResolvingMAC --> Poisoning: resolve_mac OK
    Poisoning --> Handling: sniff car frames
    Handling --> Handling: DNS A/empty / L2 forward / RST / SNI log
    Handling --> Restoring: duration elapsed or KeyboardInterrupt
    Restoring --> [*]: 3x genuine-MAC ARP replies
```

### M7 — Process supervision, self-heal, single-instance lock, autostart

- **Claim (README / summary):** "看门狗（挂了自动拉起）… 登录自启" — `README.md:166,195`; `start_all.ps1:14-15`.
- **Code path:** `stack.py:45-55` `acquire_single_instance()` (bind `127.0.0.1:45799`), `:98-112` `spawn()` with `CREATE_NEW_PROCESS_GROUP|CREATE_NO_WINDOW`, `:158-191` `ensure()`, `:194-221` `main()` 8 s loop; `supervise_all.py:60-103` 30 s loop; `install.ps1:429-445` HKCU Run `fsd-mitm-supervisor`.
- **Frame spec:** None found (no CAN frame). Watch artifacts: `logs/stack_state.json` (`stack.py:211-218`), `logs/supervise.log`, `logs/*.log`.
- **Gates and timing:** Lock port 45799; restart interval 8 s / 30 s; ARP log freshness 120 s; IP change restarts DNS sink (`stack.py:165-176`); watchdog liveness by port 4567/443/8900.
- **State machine:** `dns_status` ∈ `listening` / `not_responding` / `port_in_use` / `starting` (`:146-156`) plus child-exit reaping — ≥3 states, diagram below.
- **Tests:** None found.
- **Evidence grade:** CODE-READ.
- **Verdict:** CONFIRMED.
- **Adapt to TeslaCANModder?** no impact (host-OS service supervision, not embedded). Idea "freshness gate before trusting a cached value" exists in our tree as CAN TX freshness checks, but no code will be copied from here.

```mermaid
stateDiagram-v2
    [*] --> Starting
    Starting --> Listening: dns_responding() true
    Starting --> NotResponding: spawned but no answer
    Starting --> PortInUse: udp_open(53) false
    NotResponding --> Listening: child re-spawned
    PortInUse --> PortInUse: log once, wait for port
    Listening --> Starting: local IP changed -> DNS restarted
```

### M8 — Evidence logging, VIN capture, dashboards

- **Claim (README / summary):** "命中列表出现一条 `HIT`，客户端是车机 IP … 字节数与 `payload.json` 一致" — `README.md:54`; "看板" throughout.
- **Code path:** `fsd_mock.py:121-138` `_log()` → `logs/hits.jsonl`; `:175-202` `_capture()` → `logs/requests/*.bin` + VIN regex `/vehicles/([^/?#]+)`; `:299-332` handshake/SNI logs; `web/fsd_panel.py:235-456` aggregation → `/` and `/api/state` (`:724-733`); `web/server.py:377-522` legacy 8899 aggregation.
- **Frame spec:** None found (no CAN frame). Log schema fields: `kind`, `client`, `url`, `host_header`, `user_agent`, `vin`, `req_bytes`, `req_md5`, `bytes`, `md5`, `mode` (`:122-136,183-202`).
- **Gates and timing:** Dashboard auto-refresh 2 s (`fsd_panel.py:3-7`, meta refresh at `:673`); `car_recent` capped at 999 (`:33-34`); `_CONN_CACHE` 3 s (`:44,263-286`); ping cache 10 s (`:311,314-329`).
- **State machine:** None found.
- **Tests:** None found.
- **Evidence grade:** CODE-READ.
- **Verdict:** CONFIRMED (logging code), UNVERIFIED (that any log represents a real car — `127.0.0.1` self-tests are distinguishable but not automatically rejected in all views).
- **Adapt to TeslaCANModder?** no impact; our recorder/dashboard is CAN-native (`firmware/lib/core/can/` recorder, client monitor screens).

### M9 — Installer, hidden persistence, stealth and privilege plumbing

- **Claim (README / summary):** "双击安装 … 装完位置固定在 `C:\fsd-mitm` … 登录自启" — `README.md:12-17,195`.
- **Code path:** `install.ps1:258-291` robocopy + UTF-8 BOM fix, `:293-369` venv/pip, `:399-427` desktop shortcuts, `:429-445` HKCU Run, `:447-458` launch; `start_hotspot.ps1:63-79` UAC firewall elevation; `start_all.ps1:58-79` five firewall rules; `stack.py:109-110` hidden child processes; `server.py:555-569` `powershell.exe -ExecutionPolicy Bypass` hotspot control.
- **Frame spec:** None found (no CAN frame). Persistence keys: `HKCU:\Software\Microsoft\Windows\CurrentVersion\Run\fsd-mitm-supervisor` (`install.ps1:435-441`, `start_all.ps1:124-127`); firewall rules `fsd-mitm-reverse-4567`, `fsd-mitm-proxy-8080`, `fsd-mitm-dns-53`, `fsd-mitm-web-8899`, `fsd-mitm-api443` (`start_all.ps1:59-65`).
- **Gates and timing:** `#requires -Version 5.1`; x64 only; target must equal `C:\fsd-mitm` (`install.ps1:164-189`); ports 443/4567/8080/8899/8900 pre-flight (`:199-209`).
- **State machine:** None found.
- **Tests:** None found (`-Check` at `:211-256` is a pre-flight probe, not a test suite).
- **Evidence grade:** CODE-READ.
- **Verdict:** CONFIRMED.
- **Adapt to TeslaCANModder?** never port — this is exactly the host-compromise shape our project must not acquire.

### M10 — Read-only TLS/SNI observation

- **Claim (README / summary):** Not claimed in README; present as diagnostic capability.
- **Code path:** `fsd_mock.py:228-258` (handshake_ok from `client_conn` cert list, ALPN, cipher) and `:307-332` (`tls_clienthello`, `tls_failed_client`); `arp_mitm.py:71-100` `tls_sni()` parses ClientHello extension 0 without decrypting; `:260-270` logs SNI and flags `mothership.` hits.
- **Frame spec:** None found (no CAN frame). Observed TLS fields: SNI, ALPN, version, cipher, issuer/subject of client cert; no key material derived.
- **Gates and timing:** Best-effort `try/except` everywhere; first request per connection only for handshake_ok (`fsd_mock.py:232-234`).
- **State machine:** None found.
- **Tests:** None found.
- **Evidence grade:** CODE-READ.
- **Verdict:** CONFIRMED (read-only parsing).
- **Adapt to TeslaCANModder?** no impact — no TLS server in shipping firmware.

## 6. Summary vs code contradiction table

| # | Summary claim | Code fact | Verdict | Grade |
| --- | --- | --- | --- | --- |
| C1 | `README.md:132` calls `payload-282b.json` "官方原版" (official original); `web/fsd_panel.py:46-47,177-179` badge calls its md5 `c212a75c…` "与官方 282B 一致" | That file contains `full_self_driving:true`, `enable_eu_fsd_behavior:true`, `eu_fsd_allow_region_override:true` and `lab_diagnostic_marker:"cn-region-http-probe"` — the project's own unlock payload, not an upstream Tesla artifact. Its md5 is `c212a75ca858750ac302093c138991f2` (verified by `certutil -hashfile`), while `fsd_mock.py:34-44` `DEFAULT_PAYLOAD` has the same 9 fields in **different key order** → same 282 B but md5 `104ccde7d817ab00667f2d98326259ec` (computed at write time). The panel badge would mark the built-in default "非官方载荷". | CONTRADICTED | CODE-READ |
| C2 | `README.md:132` frames `payload-16f.json` as a benign "16 字段版" rollback | `payload-16f.json:1-1` still sets `full_self_driving:true`, `fsd_navigation_supported:true`, `fsd_stops_maps:true`, `drive_on_nav_maps:true`; it is not a restore-to-stock payload. | CONTRADICTED | CODE-READ |
| C3 | `web/fsd_panel.py:8` docstring: "只读 `logs\` 下的证据文件，不改任何配置" | Same file writes `payload.json`, `payload.json.bak` and `logs/config_history.jsonl` in `_save_payload()` (`:67-89`) via routes `/config` and `/payload` (`:135-232`); `web/server.py:572-603` also writes payload/mode via POST APIs. | CONTRADICTED | CODE-READ |
| C4 | `README.md:56` "之后每 **12 小时 ± 2 小时**请求一次。命中一次后短时间内不会再来" | No code in this repo implements or measures that cadence; it is a claim about Tesla client behavior. | UNVERIFIED (would need a multi-day car-side capture) | SUMMARY-ONLY |
| C5 | `README.md:106-108` Q6: non-mock APIs return 400 because they need a client certificate, and "FSD 配置下发不受影响" | Only supporting code is `--ignore-hosts` for hermes/telemetry (`stack.py:34`) and `ssl_insecure=true`; no cert-present/400 handling exists. | UNVERIFIED (needs a live car capture) | SUMMARY-ONLY |
| C6 | `README.md:173` says hardcoded router-mode IPs are at `start_all.ps1` "第 ~91 行" | Actual `--target/--gateway/--our-ip/--answer` block is `start_all.ps1:92-94`; duplicate in `stop_api_intercept.ps1:56-58` and `supervise_all.py:77-79`. Minor drift, not a protocol contradiction — recorded as agreement-with-drift. | AGREEMENT (drift) | CODE-READ |

## 7. CAN frame and signal inventory

**None found — no CAN frames are decoded, mutated, or injected in this repo.** The only network wire artifacts are below; they are *not* CAN frames and are listed so a future agent does not mistake the dashboard's "链路计数" for bus traffic. No vendored DBC/JSON frame catalog exists (`payload*.json` are HTTP config bodies, STRUCTURAL-PARSE only — one-line objects, 9/16/22 keys).

| Wire artifact (non-CAN) | Layer | Period | DLC/size | Byte/bit layout | Scale/units | Counter | Checksum | Read/write | Grade |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `/feature_configuration` HTTP 200 | TCP/443 or 4567 | Per car request | 282 / 16-field / 22-field compact JSON | Flat JSON object; `full_self_driving`, `fsd_navigation_supported`, `fsd_stops_maps`, `drive_on_nav_maps`, `enable_eu_fsd_behavior`, `lab_diagnostic_marker` | booleans/strings | None | `X-Checksum = md5(body)` (`fsd_mock.py:68,281`) | Write (inject response) | CODE-READ |
| DNS A answer | UDP/53 | Per query | 1 RR | `0xC00C` name pointer, type A, TTL 60, ipv4 | IP address | DNS txid | None | Write (inject answer) | CODE-READ |
| DNS AAAA / other answer | UDP/53 | Per query | 0 RR | header `0x8180`, ancount 0 | — | DNS txid | None | Write (force A) | CODE-READ |
| ARP reply (poison) | Ethernet | 1 s | 28 B + header | `ARP(op=2, psrc=gw_ip, hwsrc=our_mac)` | MAC/IP | None | None | Write (inject) | CODE-READ |
| ARP reply (restore) | Ethernet | 3× on exit | 28 B + header | `hwsrc=gw_mac` | MAC/IP | None | None | Write (restore) | CODE-READ |
| TCP RST | Ethernet/IP | Per ACK to `--rst-dst` | 20 B TCP | `flags="R", seq=t.ack` | — | TCP seq/ack | None | Write (inject) | CODE-READ |
| L2 forwarded frame | Ethernet | Per car frame | original | `src=our_mac, dst=gw_mac` | — | — | None | Write (relay) | CODE-READ |
| `hits.jsonl` / `requests/*.bin` | Filesystem | Per hit | variable | JSONL / raw body | — | — | md5 recorded | Read+write (evidence) | CODE-READ |

## 8. Constants and tuning table

| Constant | Value | File:line | Meaning | Grade |
| --- | --- | --- | --- | --- |
| `DEFAULT_PAYLOAD` | 9 fields, 282 B, md5 `104ccde7d817ab00667f2d98326259ec` | `fsd_mock.py:34-44` | In-code canned FSD-unlock config | CODE-READ |
| `CANON_BYTES` / `CANON_MD5` | `282` / `c212a75ca858750ac302093c138991f2` | `web/fsd_panel.py:46-47` | "official 282B" badge target (= payload-282b.json) | CODE-READ |
| `PATH_RE` | `/feature_configuration/?($|[?#])` | `fsd_mock.py:46` | Intercept trigger | CODE-READ |
| `NEAR_MISS_RE` | `feature` (case-insensitive) | `fsd_mock.py:47` | Verbose near-miss logging | CODE-READ |
| DNS TTL | `60` s | `dns_sink.py:61`, `arp_mitm.py:181` | A-record answer TTL | CODE-READ |
| DNS upstreams | `223.5.5.5,1.1.1.1` / `223.5.5.5` | `dns_sink.py:173`, `arp_mitm.py:41` | Non-hijacked query forwarders | CODE-READ |
| Redirect domains | `mothership.vn.teslamotors.com`; + `mothership.vn.cloud.tesla.cn`, `api-prd.vn.cloud.tesla.cn`; prefix `mothership.` | `dns_sink.py:172`, `arp_mitm.py:39-40`, `stack.py:153` | Hosts to hijack | CODE-READ |
| `LOCK_PORT` | `45799` | `svc/stack.py:41` | Single-instance guard | CODE-READ |
| Stack loop | `8` s | `svc/stack.py:221` | Child health-check period | CODE-READ |
| Watchdog interval / ARP freshness | `30` s / `120` s | `supervise_all.py:27,71` | Restart thresholds | CODE-READ |
| ARP poison interval | `1.0` s | `arp_mitm.py:156` | Gratuitous ARP rate | CODE-READ |
| `duration` / `--rst-dst` | `7200` s / `52.81.9.181` | `start_all.ps1:94` | Router-mode session length; RST target | CODE-READ |
| Hardcoded IPs | `192.168.199.95` (car), `.1` (gw), `.148` (PC); `192.168.137.1` (hotspot) | `start_all.ps1:92-94`, `start_hotspot.ps1:15` | Network topology | CODE-READ |
| Wi-Fi SSID / key | `FSD-PC` / `fsd2026!!` | `route-b.ps1:14-15`, `hotspot.ps1:18-19`, `keep_hotspot.ps1:47` | Hardcoded hotspot credential | CODE-READ |
| Service ports | `443, 4567, 8080, 8899, 8900`, UDP `53` | `install.ps1:199-209`, `start_all.ps1:59-65` | Intercept/proxy/dashboard ports | CODE-READ |
| mitmproxy flags | `ssl_insecure=true`, `block_global=false`, `--emudhra`, `confdir=certs2` | `start_hotspot.ps1:99-101`, `stack.py:115-118` | TLS verification disabled; CA dir | CODE-READ |
| `IGNORE` regex | `^(hermes-prd\|hermes-stream-prd\|telemetry-prd)\.vn\.cloud\.tesla\.cn(?::\d+)?$` | `svc/stack.py:34` | Hosts excluded from MITM | CODE-READ |
| Payload size cap | `8192` B | `fsd_panel.py:155-156,207-208` | Web config write guard | CODE-READ |
| `CAR_CAP` | `999` | `fsd_panel.py:34` | Rolling car-request cache cap | CODE-READ |
| Install root | `C:\fsd-mitm` | `install.ps1:28,183` | Hardcoded deployment path | CODE-READ |
| Config request cadence | `12 h ± 2 h` | `README.md:56` | Claimed car-side re-fetch | SUMMARY-ONLY / UNVERIFIED |

## 9. Failure modes and edge cases

| Failure | Handling code path | Tested? | Grade |
| --- | --- | --- | --- |
| UDP 53 occupied by Windows hotspot DNS proxy | `dns_sink.py:152-154` bind may fail; `stack.py:146-155` reports `port_in_use`; `start_hotspot.ps1:112-114` prints `netstat` hint | No | CODE-READ |
| mitmdump child exits | `stack.py:158-163` reaps and re-spawns; `supervise_all.py:82-95` restarts 443 | No | CODE-READ |
| Local IP / subnet changes | `stack.py:165-176` terminates DNS child and restarts with new IP | No | CODE-READ |
| Duplicate launchers (Startup + task + manual) | TCP bind lock on `127.0.0.1:45799` (`stack.py:39-55,194-197`) | No | CODE-READ |
| Target/gateway MAC unresolvable | `arp_mitm.py:102-123` 12 tries, then `main()` returns 2 (`:318-322`) | No | CODE-READ |
| Force-kill leaves ARP table poisoned | `restore()` only runs in `finally` on clean exit (`arp_mitm.py:293-296`); external `Stop-Process` skips it (`stop_api_intercept.ps1:50-52`) | No | CODE-READ |
| Non-mock upstream APIs fail | Documented as 400 in `README.md:106-108`; only mitigation is `--ignore-hosts` / `ssl_insecure` | No | UNVERIFIED |
| Observe mode leaks real request upstream | Intentional (`fsd_mock.py:267-272`); request body already written to disk by `_capture()` first (`:266`) | No | CODE-READ |
| Invalid/hot-reloaded payload JSON | Silently ignored (`fsd_mock.py:215-221`); stale web page guarded by `_rev` (`fsd_panel.py:157-163`) | No | CODE-READ |
| TLS verification disabled | `--set ssl_insecure=true` in all launchers; `server.py:351` uses `_create_unverified_context()` | No | CODE-READ |
| Bus-off / watchdog / OTA / brownout / queue overflow / missing-frame timeout | None found — no CAN bus, no embedded state machine, no frame queue | N/A | CODE-READ (absence) |

## 10. Test coverage map

**None found.** No `test/`, no `*_test.py`, no CI config, and no committed test framework in `git ls-files`. Nothing is asserted by a test oracle.

Critical paths with NO test:
- HTTP hit path and `X-Checksum == md5(body)` — `fsd_mock.py:50-51,276-285`. Risk: a body/key-order change silently breaks the badge and any car-side checksum check (see C1).
- reply/observe gate — `fsd_mock.py:265-272`. Risk: observe mode could still mutate traffic if `_hit()` changed.
- DNS query parse and A/AAAA answers — `dns_sink.py:45-78`. Risk: compressed qname raises and is silently dropped (`:53-54,120-122`).
- ARP poison/restore and RST injection — `arp_mitm.py:145-168,248-259`. Risk: a crash mid-run leaves the car's ARP cache poisoned.
- Watchdog liveness and single-instance lock — `stack.py:39-55,158-191`, `supervise_all.py:60-103`. Risk: duplicate stacks fighting over ports 53/443.
- Installer path guard and BOM rewrite — `install.ps1:183-189,274-291`. Risk: PowerShell 5.1 misparses Chinese comments if BOM step regresses (this was the pinned commit's fix).
- `server.py:321-371` `selftest()` and `start_hotspot.ps1 -Status` / `install.ps1 -Check` are **runtime probes**, not tests — they verify the current machine state, not correctness.

## 11. Parity matrix with TeslaCANModder

No shared CAN signal or frame exists. Every row below has a our-side CODE-READ citation; where our mechanism lives at a different layer, the action is `no impact`, never a port. Negative absence anchors are in section 16.

| Signal/frame or idea | Upstream decode/behavior (cite) | Our decode/behavior (cite) | Delta | Action |
| --- | --- | --- | --- | --- |
| FSD enable state | HTTP flag `full_self_driving` injected in response body (`fsd_mock.py:38`) | CAN mux edit gated by `fsdAllowed && apGateOpen` (`firmware/lib/vehicle/can/handler/variant/hw4.h:136`; `filters.h:55-63`) | Cloud config spoof vs. in-frame CAN mutation; different attack surface | ignore — no shared wire |
| Checksum recompute after mutation | `X-Checksum = md5(body)` (`fsd_mock.py:68,281`) | `nagChecksum(f.data)` written to `data[7]` after EPAS echo mutation (`firmware/lib/vehicle/can/feature/fsd/nag.h:485-486`; `vehicle/can/checksum.h` include at `:38`) | MD5 over HTTP body vs. in-frame CAN checksum; both are "recompute over exact bytes" | ignore — idea already present independently |
| Gate before mutation | `mode.json` reply/observe (`fsd_mock.py:152-173,265-272`) | `State::apGateOpen()` (`firmware/lib/core/types.h:837`), used before FSD edits (`hw4.h:67,136`) and injection bursts (`firmware/lib/vehicle/can/burst.h:27`) | File-driven host gate vs. CAN/AP-state gate | ignore — already present, different safety basis |
| Config persistence / reload | `payload.json` mtime hot-reload (`fsd_mock.py:204-226`) | NVS `Preferences` persistence (`firmware/lib/core/persist/nvs.h:14`), command-driven toggles (`firmware/lib/client/command/dispatch.h:729-734`) | Filesystem mtime vs. NVS key/value | ignore — different storage layer |
| Watchdog / self-heal | 8 s + 30 s process supervisors (`stack.py:221`, `supervise_all.py:27`) | None found in embedded firmware; host-side only in this repo | PC process model has no embedded analogue | no impact |
| Evidence logging | `hits.jsonl`, `requests/*.bin`, handshake log (`fsd_mock.py:121-138,175-202,299-332`) | CAN recorder/ring buffer under `firmware/lib/core/can/`; client monitor docs | HTTP evidence vs. CAN frame capture | no impact |

## 12. Surface inventory and security notes

Network/web surfaces:

| Surface | Semantics | Security note | Grade |
| --- | --- | --- | --- |
| `mitmdump :443` reverse `https://api-prd.vn.cloud.tesla.cn@443` | TLS-intercepts car API, short-circuits `/feature_configuration` | `ssl_insecure=true`, `--emudhra`, uses bundled CA private key (`start_hotspot.ps1:98-102`) | CODE-READ |
| `mitmdump :4567` reverse + `:8080` regular proxy | HTTP/HTTPS forward path with addon (`stack.py:180-186`) | `block_global=false`; binds `0.0.0.0` | CODE-READ |
| UDP 53 DNS sink | Redirects Tesla domains to local IP; forwards others | Spoofing/inspection capability; `SO_EXCLUSIVEADDRUSE` (`dns_sink.py:152-154`) | CODE-READ |
| `fsd_panel.py :8900` | `/`, `/config` GET+POST, `/payload` GET+POST, `/api/state` | Default bind `127.0.0.1` (`fsd_panel.py:738-739`); writes `payload.json` + `.bak`; no auth (localhost-only assumption) | CODE-READ |
| `server.py :8899` | `/`, `/api/state`, `/api/proxy/<mode>/start\|stop`, `/api/hotspot/<action>`, `/api/payload`, `/api/payload/reset`, `/api/mode`, `/api/selftest`, `/api/logs/clear` | Control APIs can spawn/kill processes via `powershell.exe -ExecutionPolicy Bypass` (`server.py:555-569`) and `taskkill /F` (`:155-159`); no auth | CODE-READ |
| Scapy `arp_mitm.py` | ARP poison, DNS answer, L2 relay, TCP RST, SNI read | Active network disruption; `--rst-dst 52.81.9.181` pinned (`start_all.ps1:94`) | CODE-READ |
| WinRT hotspot | SSID/key control via `NetworkOperatorTetheringManager` (`hotspot.ps1:43-80`) | Hardcoded key `fsd2026!!` printed to console (`:80`) | CODE-READ |

Host/provenance findings:

- **Live MITM CA private key committed**: `app/certs2/mitmproxy-ca.pem:1` `BEGIN RSA PRIVATE KEY` + `:28` cert; `.p12` bundles (`mitmproxy-ca.p12`, `mitmproxy-ca-cert.p12`) are PKCS#12 containers not opened. STRUCTURAL-PARSE only; never import.
- Hardcoded Wi-Fi PSK `fsd2026!!` in at least 4 files (`route-b.ps1:15`, `hotspot.ps1:19`, `keep_hotspot.ps1:47`, `README.md:27,30`).
- Persistence: HKCU Run `fsd-mitm-supervisor` (`install.ps1:435-441`, `start_all.ps1:124-127`); desktop shortcuts (`install.ps1:399-423`).
- Privilege: UAC self-elevation for firewall rules (`start_hotspot.ps1:74-79`, `start_all.ps1:73-78`, `stop_api_intercept.ps1:79-86`); `-ExecutionPolicy Bypass` throughout.
- Stealth: `CREATE_NO_WINDOW`/`pythonw`/`WindowStyle Hidden` (`stack.py:109-110`, `supervise_all.py:21,26`, `start_all.ps1:84-119`).
- No license file; provenance is a 404 original + 152-fork search snapshot (`docs/legacy/tesla-drug-forks-2026-11-20.md:5-10`) — distribution/use rights unknown.

## 13. Adaptation assessment

| Mechanism | Value | Cost | Risk | Evidence grade | Decision |
| --- | --- | --- | --- | --- | --- |
| M1 canned 200 + checksum recompute | Low for us (no HTTP surface) | Low | High if misread as a CAN frame recipe | CODE-READ | no impact |
| M2 reply/observe gate | Medium *as idea* (observe-before-inject discipline) | Low | Low | CODE-READ | already present via `apGateOpen()`; no code port |
| M3 hot-reload + stale-write guard | Medium *as idea* | Medium | Medium (md5/key-order ambiguity) | CODE-READ | no impact; NVS is our config layer |
| M4 DNS sink | None | Medium | High (traffic redirection) | CODE-READ | never port |
| M5 hotspot topology | None | Medium | Medium (credential handling) | CODE-READ | never port |
| M6 ARP poison / L2 forward / RST | None | High | Critical (network disruption, MITM) | CODE-READ | never port |
| M7 supervision/self-heal | Low (host-only) | Medium | Medium (persistence) | CODE-READ | no impact |
| M8 logging/dashboard | Low (we have CAN recorder/client monitor) | Medium | Low | CODE-READ | no impact |
| M9 installer/persistence/privilege | Negative | High | Critical | CODE-READ | never port |
| M10 TLS/SNI observation | None for CAN firmware | Medium | Medium (privacy) | CODE-READ | never port |

Top three *derived ideas* worth keeping in our backlog (no code copied): (1) recompute the checksum over the exact mutated bytes with an independent test oracle — already done in `nag.h`; (2) an observe-first mode before any injection — analogous to our AP gate; (3) stale-write guard on concurrent config editors — relevant only if our dashboard ever gains web-editable config.

## 14. Do-not-copy list

| Item | File:line | Reason |
| --- | --- | --- |
| MITM CA RSA private key | `app/certs2/mitmproxy-ca.pem:1` | Live private key material; committed untrusted; importing enables TLS impersonation |
| PKCS#12 bundles | `app/certs2/mitmproxy-ca.p12`, `app/certs2/mitmproxy-ca-cert.p12` | Binary key/cert containers; untrusted, not opened; may contain private keys |
| Other cert/DH files | `app/certs2/mitmproxy-ca-cert.cer`, `-cert.pem`, `mitmproxy-dhparam.pem` | Part of the untrusted trust chain; no legitimate use in our project |
| Untrusted ZIP/archive material | Provenance warning `docs/legacy/tesla-drug-forks-2026-11-20.md:10` and sibling fork `cos003` `CLEAN-2026.zip` | User mandate: ZIP/cert/key material in forks is untrusted; never execute or unpack in shipping workflows |
| `/feature_configuration` intercept + payloads | `fsd_mock.py:33-51,276-285`, `app/payload*.json` | Unverified FSD-unlock claim; safety-critical; no CAN counterpart |
| Hardcoded Wi-Fi PSK | `route-b.ps1:14-15`, `hotspot.ps1:18-19` | Credential; would weaken our security posture |
| HKCU Run autostart and UAC/firewall self-elevation | `install.ps1:429-445`, `start_hotspot.ps1:63-79`, `start_all.ps1:58-79` | Host persistence/privilege escalation pattern our repo must not acquire |
| Hidden-process flags / `-ExecutionPolicy Bypass` | `stack.py:109-110`, `route-b.ps1:24`, `server.py:555-569` | Stealth execution pattern |
| ARP poison / RST injection / DNS sink | `arp_mitm.py:145-168,248-259`, `dns_sink.py:113-148` | Active network attack; no research value for a CAN firmware project |
| Upstream identifiers/verbatim text | all files | No license grant; reimplement ideas only, with independent tests |

## 15. History and evolution

Local submodule history is short (4 commits, no tags), all on 2026-11-20:

- `ca02db4` 04:19 — Initial commit.
- `f91529c` 09:44 — "Add AU adaptation probe tool (probe_au)": +562 lines across `app/probe_au.ps1`, `app/tools/probe_au.py`, and 2-line changes to `start_hotspot.ps1`/`stop_hotspot.ps1`.
- `f0c41c6` 09:46 — "Keep AU probe tool local only": −560 lines, deleting both probe files two minutes after adding them. This is the only explicit rollback in the snapshot; the commit message says it was kept local-only, so it is **not** part of the pinned tree. What the probe actually did is UNVERIFIED (the deleted file contents were not read).
- `05d2b26` 09:56 (pinned HEAD) — "Fix PowerShell 5.1 ANSI parsing: ensure UTF-8 BOM on all .ps1"; the corresponding live code is `install.ps1:274-291`, which adds EF BB BF to copied `.ps1` files.

Pre-pin upstream history (before `ca02db4`) was **not** reviewed; only the pinned snapshot and its four local commits were examined. All network/issue/PR claims stay UNVERIFIED — no network access was used beyond reading the local provenance doc.

## 16. Re-verification recipe

```bash
OLD=05d2b265a372d43f2eaf0e00bf7ce8be7d5c05d8
git ls-files -s legacy/jdgbv2-Tesla-drug          # expect: 160000 <OLD> 0
git -C legacy/jdgbv2-Tesla-drug rev-parse HEAD    # expect: <OLD>
git -C legacy/jdgbv2-Tesla-drug status --porcelain # expect: empty
git -C legacy/jdgbv2-Tesla-drug fetch --tags --prune
git -C legacy/jdgbv2-Tesla-drug log --oneline $OLD..origin/main
git -C legacy/jdgbv2-Tesla-drug diff --stat $OLD..origin/main
git -C legacy/jdgbv2-Tesla-drug diff $OLD..origin/main -- app/
```

Demonstrated anchors — all run at write time with ripgrep (`grep -nE` equivalent; on this Windows host GNU `grep` is not installed, so the same extended regexes were run with the repo's ripgrep-backed search tool and the exact expected hits pasted).

```text
grep -nE 'PATH_RE = re.compile|X-Checksum|DEFAULT_PAYLOAD' app/fsd_mock.py
  -> expect lines 34 (DEFAULT_PAYLOAD), 46 (PATH_RE), 281 (X-Checksum header)

grep -nE 'REDIRECT = \{|REDIRECT_PREFIX|UPSTREAM = ' app/tools/arp_mitm.py
  -> expect lines 39 (3 domains), 40 ("mothership."), 41 ("223.5.5.5")

grep -nE 'CANON_BYTES|CANON_MD5' app/web/fsd_panel.py
  -> expect lines 46 (282), 47 (c212a75ca858750ac302093c138991f2)

grep -nE 'ssl_insecure=true|--emudhra|api-prd\.vn\.cloud\.tesla\.cn' app/start_hotspot.ps1
  -> expect lines 99, 100, 101, 110

grep -nE 'BEGIN RSA PRIVATE KEY|BEGIN CERTIFICATE' app/certs2/mitmproxy-ca.pem
  -> expect line 1 (RSA PRIVATE KEY), line 28 (CERTIFICATE)

grep -nE '\bCAN\b|0x3FD|0x229|MCP2515|isoTp|CANFRAME' app/
  -> expect NO OUTPUT (negative anchor: no CAN code in this repo)

grep -nE 'feature_configuration|mothership\.vn|api-prd|X-Checksum|mitmproxy' firmware/ client/src/ packages/ tools/
  -> expect NO OUTPUT (negative anchor: no MITM surface in shipping code)

grep -nE 'fsdEnabled|nagMode' firmware/lib/vehicle/can/handler/filters.h
  -> expect lines 55-78 (our-side drift anchor: FSD is CAN-mux based)

grep -nE 'nagChecksum' firmware/lib/vehicle/can/feature/fsd/nag.h
  -> expect line 486 (our-side checksum recompute anchor)
```

Hash cross-check (demonstrated): `certutil -hashfile app/payload-282b.json MD5` → `c212a75ca858750ac302093c138991f2`; compacting the in-code 9-field `DEFAULT_PAYLOAD` (`fsd_mock.py:34-44`) yields 282 B with md5 `104ccde7d817ab00667f2d98326259ec` — the order-sensitivity behind C1.

If line numbers drift more than ~10, re-read the whole function; do not trust this document's numbers. Upstream history before the pinned SHA was not reviewed — only the pinned snapshot and its four local commits.

## 17. Open questions and explicitly not investigated

- Does any real Tesla vehicle actually enable FSD from this HTTP response? UNVERIFIED — needs a licensed, isolated bench capture with a real car; no such capture exists in the repo (no CAPTURE evidence).
- What did the removed `probe_au` tool (commits `f91529c` → `f0c41c6`) actually do? Deleted file contents were not read; name suggests an Australia-region adaptation probe — UNVERIFIED.
- Is the "12 h ± 2 h" re-fetch cadence and the 400-on-other-APIs behavior accurate? README-only (C4, C5) — UNVERIFIED.
- Which upstream fork (if any) contains the untrusted ZIP referenced by the provenance doc? Not inspected — out of scope for this single-repo deep dive.
- `app/web/static/index.html` and the small `.cmd`/`stop_*.ps1` launchers were not read in full; only grep-level constants were collected.
- PKCS#12 bundles were not opened; whether `mitmproxy-ca.p12` duplicates the committed RSA key is UNVERIFIED (structural parse only, by mandate).
- License/copyright status of the original `zeratulhx/Tesla-drug` and of this fork: no license file in snapshot; UNVERIFIED.
- Pre-pin upstream history and network-only issues/PRs: not reviewed (default).
- No test framework, no CI config, and no captured evidence exist here, so every "CONFIRMED" verdict above means "code does what the code says", not "behavior validated against a vehicle".
