# Repo state — 2026-11-22

## Where the repo is now

| Area                  | State                                                                                                                                                                                             |
| --------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Branch                | `chore/legacy-tesla-drug-fork-submodules` (pushed, commits `7d90b9a`, `2205a2e`, `0e2541b`, `103ac5c`)                                                                                            |
| PR #128               | legacy SHA bumps + two dead upstreams preserved as tracked folders — merged to `main`                                                                                                             |
| PR for current branch | not opened yet                                                                                                                                                                                    |
| `legacy/`             | 139 Tesla-drug submodules pinned (4 originals + 135 added), 3 dead 404 + 10 empty repos stay catalog-only                                                                                         |
| Deep-dive docs        | 99 of 104 repo docs use the 17-section contract (17 h2 sections in order, evidence grades, parity matrix, re-verification recipe); ~4.8 MB                                                        |
| Shallow docs          | 5 orphan docs with no repo in tree (`1-v-1-tesla-fsd-can-mod`, `alzza-tesla-open-can-mod`, `tesla-fsd-can-mod-2-main`, `tesla-open-can-mod-main`, `ylovex75-...`); one is deleted by CI           |
| Skills                | `.opencode/skills/legacy-submodule-update`, `.opencode/skills/legacy-repo-deep-dive` (TDD-tested on 10 pilots)                                                                                    |
| Plans/reports         | `.opencode/plans/`, `.opencode/analysis/`, `.opencode/reports/`; nothing new under `docs/superpowers/`                                                                                            |
| Working tree          | 191 pre-existing unstaged changes (firmware, client, tools line-ending/format work) untouched by the legacy sweep                                                                                 |
| CI (last observed)    | protocol/client/tools/lint/markdown/docker/e2e/workflow/security/review green; firmware job red on "Firmware size regression check" — same failure existed on PR #127, so it predates this branch |

## What the 99 deep dives actually produced

The value is not "more docs". It is **cross-repo arbitration**: a claim repeated by independent upstreams, plus a capture where one exists, is a real signal about our firmware.

### A. High-confidence our-side defects (no port needed — our code is wrong)

| Finding                                                                                                                         | Evidence                                         | Status                                 |
| ------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------ | -------------------------------------- |
| `key.h` role constants: ours OWNER=4 / CHARGING_MANAGER=5, SDK proto 2/6                                                        | 3 independent snapshots (teslamotors, tlc, seth) | candidate fix, bench confirm           |
| BLE wire field numbers in `msg.h`: `to_destination` 2 vs 6, `uuid` 14 vs 51, `wake` field 25 is actually `CancelSoftwareUpdate` | official protos, tlc/core                        | candidate fix, bench capture           |
| Local Name is `S+sha1(VIN)[:8]C`; our last-8-VIN scan filter cannot match a real advert                                         | tlc ble_mqtt_core                                | candidate fix                          |
| `0x249` ALC frame: upstream/capture uses DLC-4 CRC; our DLC-3 additive + MAGIC tables fit 0/1048 real frames                    | uhi22 (compiled and ran both tools, 1048/1048)   | safety candidate, can-review-checklist |
| `tlssc.h` rewrites `0x331`; plugin upstream puts TLSSC at `0x3FD` bit38                                                         | ev-open plugins                                  | capture-first                          |
| `apGateParked` treats gear 0/5 as parked (fail-open)                                                                            | ev-open + 3 more repos + old todo P1-008         | candidate fix                          |

### B. Multi-source decode conflicts (capture-first, do not flip on code-only evidence)

- `0x118` gear/pedal: several upstreams read gear at byte2 bits 5:7; ours reads byte0 bits 3:1.
- `0x257` speed: one upstream re-decoded 823,335 real frames — our layout reads nonzero at standstill.
- `0x219` TPMS: 4 independent docs — DBC uses 5-byte muxed frame, ours reads 8-byte grouped; `0x31F` alternative exists.
- `0x3F5` turn signals: 3 docs point at left/right field swap; our `turn:left3/right3` may write the wrong side.
- ECE R79: bit 19 upstream vs our bit 20 clear.
- Endianness: 6+ upstreams decode overlapping signals little-endian; our decoders are big-endian. Note one upstream (LeeGaHyeon) has an _execution-verified byte-swap bug_, so upstream is not automatically right — capture decides.
- `0x399` vs `0x39B` DAS role, `0x3D2` charge/discharge halves, `0x132` current scale, `0x353`-family BMS slots: all disputed.
- `0x370` checksum is **settled**: `nagChecksum` matched 25,288 real frames; the dead `MAGIC_0x370` table can be removed.

### C. Advertised-but-unimplemented firmware surface (11 confirmed by negative greps)

`0x318` OTA TX guard · `smt:*` · `elm327:*` · `gvret:*` · `ha:*` · `espnow:*` · `speedalert:*` · `btnmap:*` · `mqttShouldPublish` (no caller, `mqttConnected` never true) · `0x401` per-cell voltage decode · 0-100 timer. Decide per item: implement, or remove from protocol/docs/client. Do not leave the contract advertised.

### D. Already present — cross-check only, no port

FSD mux bit layouts, profile/follow-distance maps, offset pipeline, organic/feifan nag paths, ring buffer, MCP2515 bus-off ideas, dataset ingestion in the client. Several upstreams are strict subsets of our firmware.

### E. Never port

Untrusted ZIP/cert/key material from the Tesla-drug forks; parking and occupancy-defeat hacks; non-commercial licensed sources (canhackers CC BY-NC, rafal83); Tesla ECU reflash tools (gregjhogan); BLE OTA with size-only integrity. No archive was extracted or executed during analysis.

## Hardware capacity question

CI builds `esp32_chassis_8mhz` with flash ≤ 1,100,000 B (~84% of 1.31 MB) and RAM ≤ 300,000 B (~92% of 320 KB); the size check is the red job. Three tracks run in parallel as **spikes**, not as blockers for correctness work. SoC data below is the maintainer's Turkey/Nov-2026 market snapshot — prices and board configurations must be re-checked at purchase time.

### SoC role map for a heterogeneous cluster

| SoC                      | Relevant specs                                                                                    | Proposed role                                                                                    |
| ------------------------ | ------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------ |
| ESP32 (classic, current) | 2×240 MHz Xtensa, 520 KB SRAM, 4 MB flash, Classic BT + BLE 4.2                                   | Keep as CAN bus worker (MCP2515 SPI) and legacy transport; proven firmware base                  |
| ESP32-S3 N16R8           | 2×240 MHz, 512 KB SRAM, 16 MB flash + 8 MB PSRAM, BLE 5, USB                                      | Coordinator / cache / key material / WiFi-heavy dashboard; natural successor to the classic main |
| ESP32-C3                 | 160 MHz RISC-V, 400 KB SRAM, 4 MB flash, BLE 5, ~₺177                                             | Cheap dedicated sniffer or single-bus worker — good for scaling node count                       |
| ESP32-C6                 | 160 MHz RISC-V, 512 KB SRAM, 4–8 MB flash, WiFi 6 **2.4 GHz only**, BLE 5.3, Zigbee/Thread/Matter | Home-Assistant/Matter bridge node if HA work resumes; do not buy for 5 GHz                       |
| ESP32-C5                 | 240 MHz RISC-V, 384 KB SRAM, 4–8 MB flash, WiFi 6 **2.4/5 GHz**, BLE 5, ~₺977                     | Only if a 5 GHz network or dual-band AP is a hard requirement; expensive                         |
| ESP32-P4                 | 2×400 MHz RISC-V, 768 KB SRAM + PSRAM, no WiFi/BT, display/camera                                 | Local HMI/display node only — needs a separate radio node                                        |
| ESP32-H2                 | 96 MHz, no WiFi, BLE 5.3, Zigbee/Thread                                                           | Low-power sensor/mesh node; not a CAN candidate                                                  |

Recommended lab configuration: **S3 N16R8 as coordinator** (16 MB flash + 8 MB PSRAM removes the current size pressure), classic ESP32/C3 as per-bus CAN workers, C6 only if Matter/HA is in scope. C5 and P4 stay out until a concrete 5 GHz or display requirement exists.

### Raspberry Pi track (new)

The maintainer wants the code also evaluated on Raspberry Pi. Two very different jobs must not be conflated:

1. **Pi as supervisor (low risk, high value).** `packages/protocol`, `tools/` and the Expo web client already run on Linux. A Pi can host the web UI, MQTT broker, capture/log database, OTA staging and fleet/cluster management while ESP32 nodes do real-time CAN I/O. Native firmware tests already compile on the host, which supports this split.
2. **Pi as CAN firmware replacement (high risk).** GPIO/SPI MCP2515 or native CAN overlays exist, and Linux gives far more RAM/CPU, but Linux scheduling is not deterministic enough for 100 Hz `0x370` echo and 25/50 Hz DAS injection without RT-PREEMPT. Boot time, SD-card wear and power consumption are additional costs. Do not port the shipping CAN loop first; spike it and keep the safety gate on a single authority.

**Cluster safety rule:** regardless of node count, AP/park/OTP-style injection gates must have exactly one owner. Distributed gating is not acceptable.

All three tracks need measured flash/RAM numbers, a written inter-node interface and a failure matrix before any shipping decision. Do not migrate on "it feels tight" alone — first profile the current build and make the size check honest for all 30+ envs (old todo P4-009/P4-010/P8-002).

## Web UI state

102 source files, 45 test files. The old review already flagged `AppExperience.tsx` (1238-line god component, duplicated state) as P2-010, plus 7 cross-workspace imports, dead lifecycle dispatch, and several gating bugs. A full rewrite is a real project; do it as a dedicated phase with the state providers and command transport as the boundary, after the safety/correctness wave so the rewrite is not chasing moving wire contracts.

## Scope-creep control

New findings go to `.opencode/analysis/findings-intake.md` with: source doc, evidence grade, our-side counterpart, one-line impact, and a triage label (now / next / never). Re-prioritize the roadmap at each phase gate; never silently add work to an in-flight phase.
