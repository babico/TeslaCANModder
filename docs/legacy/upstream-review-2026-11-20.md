---
title: Legacy upstream review — 2026-11-20
description: Consolidated review of upstream activity across the legacy submodules as of 2026-11-20, including dead upstreams converted to normal folders.
category: legacy
folder: legacy
tags: [legacy, review, upstream]
---

# Legacy upstream review — 2026-11-20

Scope: fetched all 91 active legacy submodules, compared each to its default branch, re-read the changed code (not just commit messages), and bumped the 10 repos with new upstream commits. Companion to the per-repo analysis docs in `docs/legacy/`.

## Repos with new upstream commits

| Submodule                                         | Default | New commits | Headline                                                                                                                      |
| ------------------------------------------------- | ------- | ----------- | ----------------------------------------------------------------------------------------------------------------------------- |
| `hypery11-flipper-tesla-fsd`                       | main    | 304         | v2.16-beta.6 → v2.17-beta.6: RX ID whitelist, second-core web server, cross-core stream ring fix, stale-frame TX gate, nag fault/driver-override guard |
| `wimaha-TeslaBleHttpProxy`                         | main    | 297         | 2.3.0 → 2.5.0: Go 1.27, dashboard status/refresh endpoints, per-vehicle connection tracking, rejected async command reporting     |
| `ev-open-can-tools-ev-open-can-tools`               | main    | 107         | v2.5.0-beta.9 → v3.1.1: HW4 AP status decode moved to `0x39B` byte0 nibble, HW4 nag fail-closed, T-2CAN Party CAN interface, LILYGO builds |
| `yoziru-esphome-tesla-ble`                          | main    | 90          | v2026.6.7 → v2026.10.4: bounded awake probes (tesla-ble 6.4.0), sleep backoff preserved across reconnects, shared charging-power estimates |
| `teslamotors-vehicle-command`                       | main    | 29          | PIN-to-Drive command, detached AES-GCM definitions, nonce-length and response-metadata validation hardening                      |
| `enstw-tesla-can-mod-guide`                         | main    | 4           | Field notes: vehicle on 2026.20.6.1, `0x370` observed on Party CAN 2/3, region unlock treated as blocked, BLE is not an OTA path  |
| `alzza-tesla-can-moniter`                           | main    | 2           | T-2CAN WiFi monitor UI with light/dark theme                                                                                    |
| `tuncasoftbildik-tesla-can-mod`                     | main    | 1           | Motor telemetry and passive 0–100 km/h timer (v0.3.0)                                                                          |
| `ColinM-sys-tesla-can-boost`                        | master  | 1           | Docs: credit/link to Tesla Mod (Flipper Zero and ESP32)                                                                        |
| `talas9-tesla_can_signals`                          | main    | 1           | Separate derived HW2.5 APE research profile                                                                                    |

## Repos with unreachable upstream

| Submodule                            | Status                        | Action                                                                                                                                                                                                                                          |
| ------------------------------------ | ----------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `linesoft2-tesla-fsd-can-mod-fork`   | GitHub returns **404**        | `[submodule]` block commented out in `.gitmodules`; section removed from `.git/config`; dangling `.git` gitfile and `.git/modules/<path>` removed; gitlink dropped with `git rm --cached`. All 2 source files now tracked as a normal folder. |
| `bobmorane83-TeslaCam`               | GitHub returns **404**        | Same seven-step conversion. All 41 source files (≈2.7 MB, including `Bridge/` and `Camera/` trees) preserved and tracked as a normal folder.                                                                                            |

Rationale: these are read-only research copies. Losing the working-tree files would destroy research data we may still need; keeping them as ordinary tracked folders preserves the code without leaving broken submodule entries that make `git submodule foreach` fail in CI.

## Impact on TeslaCANModder

Assessed against the code we ship in `firmware/lib/`. No code was copied; all items are research candidates.

### Firmware — CAN frame mutation and routing

- **Transmit-freshness gate** — `hypery11 f82d031` adds `fsd_freshness.h`: modified read-modify-retransmit frames are only sent when the source frame is ≤25 ms old. Our `0x370` echo and `0x3FD`/`0x334` rewrites have no age check, so a receive-backlog drain can emit stale counters/payloads. **Candidate for future PR** in `lib/vehicle/can/`.
- **Nag fault / driver-override guard** — `hypery11 f6d41cf` suppresses the `0x370` echo when `das_ap_state` is 8/9 (ABORTING/ABORTED) or `|EPAS torsion| >= 2.5 Nm`. Our `nag.h` has a driver-bypass toggle but no AP-abort guard and no torque threshold. **Candidate for future PR** — strengthens the existing `nag:mode:*` safety model.
- **HW4 AP-state byte changed** — `ev-open-can-tools 2179c92` replaces the latched byte0 fallback with `readHW4DASAutopilotStatus()` reading `0x39B` **byte0 low nibble** (`data[0] & 0x0F`), dropping the old byte1 high-nibble decode. Our `readDASAutopilotState()` in `lib/vehicle/can/handler/frame_readers.h` still reads byte1 bits[7:4]. On 2026.20 Highland cars byte1 can be pinned at 1. **Candidate for future PR**, but validate against a real capture before changing the gate used by AP-First and organic nag.
- **HW4 nag fails closed** — `ev-open-can-tools 4583224` blocks Mode C on HW4 entirely (only Off is allowed). Relevant to `nag:mode:organic` / `feifan` on HW4. **Candidate** — treat as a signal to re-check the HW4 nag safety policy, not as an automatic port.
- **Nag frame bus placement** — `ev-open-can-tools 447049d` plus `enstw` field notes place `0x370` on **Party CAN (X179 pins 2/3)**, not Vehicle CAN. Our docs currently list `0x370` on the Vehicle bus and our echo targets that bus. **Candidate for bench validation** — our `BUS_BODY_ACTIVE` lane is pins 2/3; confirm by listen-only capture before changing routing.
- **MCP2515 dual-buffer reordering** — `hypery11 f82d031` documents stale frames coming from the two hardware RX buffers and the task queue. Corroborates the freshness-gate candidate above.

### Firmware — BLE / VCSEC

- **Detached AES-GCM + nonce checks** — `teslamotors-vehicle-command b583561`, `777b18e`, `1381b44`, `50bb383`. Relevant if our `lib/vehicle/ble/` ever moves beyond the current session model. **No impact today**.
- **PIN to Drive** — `teslamotors-vehicle-command 0a582da`. Fleet-API command, not VCSEC/BLE. **No impact**.
- **Bounded awake probes** — `yoziru 8e8af84` limits awake-state polling to bounded windows (180 s probe, 11 min repeat) and `a3fe35b` preserves sleep backoff across reconnects. Our BLE session/key code has no equivalent power-aware polling. **Candidate for future PR** if we add persistent BLE presence detection.
- **Async command rejection reporting** — `wimaha f9c9c3b` surfaces rejected async commands and lets Stop cancel connection attempts. Pattern worth mirroring in our command dispatch acks. **Candidate for future PR** (small).

### Client / docs

- **Second-core web server so CAN never stalls** — `hypery11 e4d4778` and `ae2e8e2` (cross-core ring safety). Our WiFi dashboard shares the CAN loop; this is the reference pattern if we ever split transports. **Architecture reference only**.
- **Dashboard status endpoints / per-vehicle connection state** — `wimaha 6840095`, `ab52a6f`, `bf35b40`. Minor UX pattern for our REST status split commands. **No impact today**.
- **Shared charging-power estimates** — `yoziru 271d8f7`. Reference for BMS telemetry consistency. **No impact**.

## What was NOT done

- **Commit / push** — all changes are local only, at the user's request.
- **No new per-repo docs** — the two skip-list repos (`commaai-openpilot`, `sunnypilot`) are not present in the tree; the other eight small repos got an `## Upstream (2026-11-20)` note in their existing docs only.
- **No code copied from upstream** — project policy is to mine for ideas, not port code.
- **No aggressive safety change** — byte0 AP-state decode, HW4 nag gating and bus routing need capture-based validation before touching shipping firmware.

## Reproduction

```bash
# Fetch every active legacy submodule, pruning dead refs. One repo
# (ev-open-can-tools) rejects a moved tag; re-run with --force for tags if needed.
git submodule foreach 'git fetch --tags --prune || true'

# Compare HEAD to the default remote branch.
git submodule foreach '
    default=$(git symbolic-ref --short refs/remotes/origin/HEAD 2>/dev/null | sed "s|origin/||")
    [ -z "$default" ] && default=main
    git rev-parse --verify -q origin/$default || default=master
    echo "$name $default +$(git rev-list --count HEAD..origin/$default)"
'

# Bump only the repos that were ahead, on their default branch.
git -C legacy/<repo> pull --ff-only origin <default>

# Convert a dead (404) submodule into a normal tracked folder.
git config --remove-section submodule.<path>
git rm --cached <path>
rm -f <path>/.git
rm -rf .git/modules/<path>
git add -A -- <path>
```

`git submodule status` must not print `fatal: no submodule mapping found in .gitmodules` after the conversion; if it does, the gitlink is still in the index and `git rm --cached` was missed.
