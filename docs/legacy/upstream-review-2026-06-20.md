---
title: Legacy upstream review — 2026-06-20
description: Consolidated review of upstream activity across the 95+ legacy submodules as of 2026-06-20. Companion to the per-repo analysis docs in docs/legacy/.
category: legacy
folder: legacy
tags: [legacy, review, upstream]
---

# Legacy upstream review — 2026-06-20

Scope: fetched all 95+ legacy submodules and compared each to its default branch. Companion document to the per-repo analysis docs in `docs/legacy/` — the per-repo docs are not edited by this review.

## Repos with new upstream commits

| Submodule                                          | Default | New commits | Headline                                                                  |
| -------------------------------------------------- | ------- | ----------- | ------------------------------------------------------------------------- |
| `teslamotors-vehicle-command`                        | main    | 4           | HSM/TEE external integration (`Session` / `ECDHPrivateKey` re-export)     |
| `yoziru-esphome-tesla-ble`                           | main    | 8           | v2026.6.7 → v2026.6.8: `tesla-ble` 5.0.6→5.1.1, `command_builder.h` removed, `send_command_result` phase+outcome, `charger_phases` sensor |
| `hypery11-flipper-tesla-fsd`                         | main    | 12          | v2.16-beta.6 → beta.8: TWAI bus-off auto-recovery, EPAS-faithful nag as demand-state (Mode-C), HW4 Highland `0x39B` byte0 fallback, `feifan_0x370.py` decode tool |
| `ev-open-can-tools-ev-open-can-tools`                | main    | 33          | v2.5.0-beta.9 → v3.0.1: ESP-IDF 6.0.1 migration, FreeRTOS tasks, TWAI/MCP2515 diagnostics, MCP2515 bus-off auto-recovery with exp backoff, plugin byte-mask matching, multi-SSID WiFi, RGB LED |
| `commaai-openpilot`                                  | master  | 107         | WebRTC/athena, modeld ONNX, lateral-curvature `LatControl`, DM escalation, cruise-fault visibility |
| `sunnypilot`                                         | master  | 235         | Sync fork: upstream + sunnypilot-only PRs (NNLC, sunnylink, Mici). One Tesla change: route-spam fix |

## Repos with unreachable upstream

| Submodule                          | Status                                                  | Action                                                                                                                                                                                                                                                                |
| ---------------------------------- | ------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `mikegapinski-tesla-can-explorer`  | GitHub returns **404** (deleted or renamed)            | Submodule entry **commented out** in `.gitmodules`; `[submodule]` section removed from `.git/config`; entry removed from `.git/modules/`. Working tree files preserved for local reference. The decoded CAN datasets from firmware 2026.2 remain useful even without the live repo. |

## Impact on TeslaCANModder roadmap

Grouped by where each upstream change could land.

### Firmware (`firmware/lib/`)

- **MCP2515 bus-off auto-recovery with exponential backoff** — `ev-open-can-tools v3.0.0` (`c449e24`, `4d273ba`) and `hypery11 515e25a` (TWAI). Our `core/can/` health monitor currently logs errors but does not auto-recover. The upstream pattern (detect bus-off, abort pending TX, reset, reconfigure, exp-backoff 1s→2s→4s→8s→max 30s) is a direct candidate.
- **TWAI / MCP2515 diagnostics JSON** — `ev-open-can-tools v3.0.0`. The schema (bus state, RX/TX error counters, arbitration lost, bus-off count, last error code, driver uptime, per-frame-ID TX stats) can be lifted verbatim into our `candiag` output.
- **EPAS-faithful nag Mode-C** — `hypery11 2bc2f33` + `b9bb6c2`. Our `nag:mode:organic` already implements an organic state machine; Mode-C is a *demand-state* variant (driven by DAS, not a fixed schedule). Cross-read `fsd_handler.c` for the state-graph encoding as a potential refinement.
- **HW4 Highland `0x39B` byte0 `DAS_autopilotState` fallback** — `hypery11 b78acd2`. Our `dasApState` decoder reads the documented byte1; Highland trims use byte0. Candidate for a HW4-Highland fallback.
- **Party CAN (bus 2/3) tap for nag killer** — `hypery11 8b9e286`. Confirms our `BUS_BODY_ACTIVE` / `BUS_VEHICLE_ACTIVE` build-flag direction is correct — the nag-killer echo on `0x370` is more reliable when the destination bus matches the source.
- **HSM/TEE external integration** — `teslamotors-vehicle-command 3c31a68` + `92b5dec`. Not needed today, but the `Session` / `ECDHPrivateKey` interfaces are the integration points if we ever move key material into a hardware enclave.

### Architecture (future migration)

- **ESP-IDF 6.0.1 migration** — `ev-open-can-tools v3.0.0` (`9d6536a`). 223 KB flash savings, FreeRTOS task model with pinned-to-core CAN RX / CAN TX / web server / watchdog tasks. We are still on Arduino; this is the reference for a future migration.
- **NNLC + Enforce Torque Control** — `sunnypilot 01a843e0ac`. Lateral-control safety patterns. Our DAS Drive gate could grow a "torque envelope" check that refuses injection if the underlying lateral torque exceeds a configured threshold.
- **Cruise-fault visibility** — `commaai/openpilot 8a80bd70e`. Treat cruise faults as gate-closing events, not just AP state.

### Client / docs

- **sunnylink route metadata schema** — `sunnypilot 097dd9b5f2`. Useful pattern for a future `routes:*` summary if we want a cloud dashboard.
- **`yoziru/tesla-ble` 5.1.1 library bump** — `yoziru 1de8ac2`. Not a direct dependency for us, but informs VCSEC parity work.
- **`feifan_0x370.py` decode tool** — `hypery11 cf631e5`. Useful as a reference for validating our torque-zero + counter+1 echo against captured traffic.

## What was NOT done

- **Submodule SHAs were not updated.** `legacy/` is read-only research; the recorded commits stay pinned to their last-fetched values. The upstream activity is recorded here for reference, not auto-merged. Bumping submodules is a separate decision per repo and should follow the project's "do not copy legacy code into shipping code" rule.
- **No code was copied from upstream.** The project policy is to mine for ideas, not to port code.
- **`commaai-openpilot` and `sunnypilot`** were reviewed but no per-repo doc was created or updated. These submodules were subsequently removed in PR #100 (2026-06-20) along with their README entries; the review notes above remain as a historical record of the upstream activity at the time of review.
- **Existing per-repo analysis docs were not edited.** This document is the single source of truth for the 2026-06-20 review. The per-repo docs in `docs/legacy/<repo>.md` continue to describe the repos themselves; they are not amended with upstream-activity sections.

## Reproduction

```bash
# Fetch every legacy submodule and prune dead refs.
git submodule foreach 'git fetch --tags --prune 2>&1 | tail -5'

# For each submodule, compare HEAD to the default remote branch.
git submodule foreach '
    default=$(git symbolic-ref --short refs/remotes/origin/HEAD 2>/dev/null | sed "s|origin/||")
    [ -z "$default" ] && default=main
    ahead=$(git rev-list --count HEAD..origin/$default 2>/dev/null)
    echo "$name $default +$ahead"
' > /tmp/legacy-compare.txt
```

Any submodule with `+$N` and `N > 0` on the line is a candidate for review.