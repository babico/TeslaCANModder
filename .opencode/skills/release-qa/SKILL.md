---
name: release-qa
description: Prepare releases and run QA quality gates for TeslaCANModder including version alignment, artifact building, and rollback drills
license: GPL-3.0
compatibility: opencode
metadata:
    area: all
    workflow: release
---

## What I do

Guide agents through preparing releases and running QA checks in TeslaCANModder. I cover:

- Pre-release quality gates and CI pipeline checks
- Version alignment across the monorepo
- Building release artifacts (firmware matrix, Docker image)
- Rollback drills and drive-context validation
- CAN review and deprecation checklists

## When to use me

Use this skill when:

- Preparing a tagged release
- Running pre-release quality gates
- Aligning versions across the monorepo
- Building release artifacts
- Performing rollback drills

## Release checklist

Every tagged release MUST pass all gates. Walk `docs/checklists/release-checklist.md` in full.

### 1. CI Pipeline Gates

| Gate                        | Requirement                      | Command / Evidence         |
| --------------------------- | -------------------------------- | -------------------------- |
| Firmware native tests       | All pass                         | `npm run test:firmware`    |
| Protocol unit tests         | All pass                         | `npm run test:protocol`    |
| E2E / smoke coverage        | All configured smoke checks pass | CI `e2e-smoke` job         |
| Client typecheck            | Zero errors                      | `npm run typecheck:client` |
| Client unit tests           | All pass                         | `npm run test:client`      |
| Client browser export build | Succeeds                         | release workflow           |
| Tools tests                 | All pass                         | `npm run test:tools`       |
| Docker build                | Succeeds                         | CI `docker` job            |
| Docker smoke test           | Heartbeat healthy                | CI `docker-smoke` job      |
| Security audit              | No medium+ vulnerabilities       | `npm run audit:ci`         |
| License check               | All deps in allowlist            | CI `security-audit` job    |
| Markdown lint               | Zero errors                      | CI `markdown-lint` job     |
| Workflow lint               | Zero errors                      | CI `workflow-lint` job     |

### 2. Manual Verification

- Flash `esp32_chassis_8mhz` firmware on test board - boot JSON valid
- Flash `esp32_wifi_ble_chassis_vehicle_body_8mhz` firmware - boot JSON valid
- Browser client connects to device via Web Serial
- Client app BLE scan finds ESP32 device
- Command round-trip: FSD on -> ack received
- Frame streaming: frames appear in dashboard
- EEPROM/NVS persistence: settings survive reboot
- AP gate blocks live mutation paths until open

### 3. Version Alignment

All component versions must be aligned before release:

| Component | Version File                                    |
| --------- | ----------------------------------------------- |
| Root      | `package.json` -> `version`                     |
| Protocol  | `packages/protocol/package.json` -> `version`   |
| Client    | `client/package.json` -> `version`              |
| Tools     | `tools/package.json` -> `version`               |
| Firmware  | `firmware/platformio.ini` comment or build flag |

Use `npm version <type>` at root to bump all workspace versions.

The release automation reads the root `package.json` version and only auto-tags stable `X.Y.Z` releases when `CHANGELOG.md` already contains a matching `## [X.Y.Z]` section.

### 4. Documentation

- CHANGELOG.md updated with release notes
- Breaking changes documented with migration guide
- New commands/features documented in `docs/reference/commands.md`
- Experimental toggles documented in `docs/guides/security.md`
- Hardware wiring changes documented in `docs/guides/hardware-setup.md`
- README.md version badge updated
- THIRD_PARTY_LICENSES updated if deps changed

### 5. Release Artifacts

| Artifact              | Format        | Location                                            |
| --------------------- | ------------- | --------------------------------------------------- |
| ESP32 firmware matrix | `.bin` (x~44) | GitHub Release attachments - one per PlatformIO env |
| Browser client        | Docker image  | ghcr.io or Docker Hub                               |
| Protocol package      | npm (private) | npm workspace                                       |

Firmware naming: `esp32[_wifi][_ble][_chassis][_vehicle][_body][_8mhz|_16mhz].bin`

Default: `esp32_chassis_8mhz.bin`
Full I/O: `esp32_wifi_ble_chassis_vehicle_body_8mhz.bin` and `*_16mhz.bin`
Passive sniffer: `esp32_vehicle_body_8mhz.bin` (no chassis - DAS injection disabled)

### 6. Rollback Plan

- Previous release tag identified: `vX.Y.Z`
- Rollback firmware binaries available in previous release
- Docker image rollback: `docker pull <image>:previous-tag`
- Known breaking changes that block rollback: (list or "none")

Rollback drills required:

- Firmware rollback drill (`vN` -> `vN-1`)
- Unified client rollback drill
- Export compatibility drill
- Container rollback drill

### 7. Go / No-Go Sign-off

| Area                    | Sign-off Owner |
| ----------------------- | -------------- |
| Firmware quality gate   | Firmware owner |
| Protocol + tooling gate | Protocol owner |
| Unified client gate     | Client owner   |
| Docs gate               | Docs owner     |
| Rollback drill gate     | Release owner  |

## Targeted drive context validation

If the release changes live drive-context signals, run strict drive-context capture:

```bash
node tools/debug.js drive-context --port COM5 --drive-duration 60000 --drive-min-samples 10 --drive-expect-full --drive-output artifacts/drive-context-report.json --drive-note-output artifacts/drive-context-note.txt
```

Required gates:

- `closureReadiness.d05 === true` - Turn + blind-spot indicators
- `closureReadiness.d11 === true` - Door / frunk / trunk open-state signals
- `closureReadiness.d13 === true` - Cruise / map / max-speed context
- `closureReadiness.allReady === true`

## Release process

1. Ensure all CI checks pass on `main`
2. Complete manual verification (Section 2)
3. Verify version alignment (Section 3)
4. Update `CHANGELOG.md` and add a matching `## [X.Y.Z]` entry
5. Confirm the root `package.json` version is the target stable version `X.Y.Z`
6. Push the release commit to `main` and let `auto-tag-release.yml` create `vX.Y.Z`
7. If automation was skipped or the tag already exists, run `release.yml` manually against the `vX.Y.Z` tag ref
8. GitHub Actions release workflow builds and attaches firmware artifacts
9. Verify release artifacts are downloadable
10. Post release notes

## CAN review checklist

For any firmware change that can affect CAN frame mutation, routing, checksums, or transport-visible message shape, walk `docs/checklists/can-review-checklist.md` before declaring the task done.

## Testing plan

For comprehensive test planning, walk `docs/checklists/testing-plan.md`.

## Deprecation checklist

For deprecating features or APIs, walk `docs/checklists/deprecation-checklist.md`.

## Key files to reference

- `docs/checklists/release-checklist.md` - full release checklist
- `docs/checklists/can-review-checklist.md` - CAN safety review
- `docs/checklists/testing-plan.md` - test planning
- `docs/checklists/deprecation-checklist.md` - deprecation process
- `CHANGELOG.md` - release notes
- `package.json` (root and workspaces) - versions
