---
title: Release Checklist
title_tr: Sürüm Kontrol Listesi
description: Pre-release quality gate checklist
category: checklists
folder: checklists
tags: [release, checklist, qa]
order: 14
icon: 🏷️
---

# Release Quality Gate Checklist

Pre-release checklist for TeslaCANModder. Every tagged release **must** pass all gates before publishing artifacts.

---

## 1. CI Pipeline Gates

| Gate                        | Requirement                              | Verified By             | Evidence                                                 |
| --------------------------- | ---------------------------------------- | ----------------------- | -------------------------------------------------------- |
| Firmware native tests       | All pass (`pio test -e native`)          | CI `firmware` job       | Pending CI run                                           |
| Firmware size regression    | Within thresholds                        | CI `firmware` job       | Pending CI run                                           |
| Protocol unit tests         | All pass (`npm run test:protocol`)       | CI `protocol` job       | Attach latest CI run or local command output             |
| E2E / smoke coverage        | All configured smoke checks pass         | CI `e2e-smoke` job      | Attach latest CI run or note if covered by another suite |
| Client typecheck            | Zero errors (`npm run typecheck:client`) | CI `client` job         | Attach latest CI run or local command output             |
| Client unit tests           | All pass (`npm run test:client`)         | CI `client` job         | Attach latest CI run or local command output             |
| Client browser export build | Succeeds                                 | release workflow        | Pending CI run                                           |
| Tools tests                 | All pass (`npm run test:tools`)          | CI `tools` job          | Attach latest CI run or local command output             |
| Docker build                | Succeeds                                 | CI `docker` job         | Pending CI run                                           |
| Docker smoke test           | Heartbeat healthy                        | CI `docker-smoke` job   | Pending CI run                                           |
| Security audit              | No medium+ vulnerabilities               | CI `security-audit` job | Pending CI run                                           |
| License check               | All deps in allowlist                    | CI `security-audit` job | Pending CI run                                           |
| Markdown lint               | Zero errors                              | CI `markdown-lint` job  | Pending CI run                                           |
| Workflow lint               | Zero errors                              | CI `workflow-lint` job  | Pending CI run                                           |

## 2. Manual Verification

| Check                                                  | Owner    | Done? |
| ------------------------------------------------------ | -------- | ----- |
| Flash `esp32` firmware on test board — boot JSON valid | Engineer | ☐     |
| Flash `esp32_wifi_ble` firmware — boot JSON valid      | Engineer | ☐     |
| Browser client connects to device via Web Serial       | Engineer | ☐     |
| Client app BLE scan finds ESP32 device                 | Engineer | ☐     |
| Command round-trip: FSD on → ack received              | Engineer | ☐     |
| Frame streaming: frames appear in dashboard            | Engineer | ☐     |
| EEPROM/NVS persistence: settings survive reboot        | Engineer | ☐     |

## 3. Version Alignment

| Component | Version File                                    | Current |
| --------- | ----------------------------------------------- | ------- |
| Root      | `package.json` → `version`                      | —       |
| Protocol  | `packages/protocol/package.json` → `version`    | —       |
| Client    | `client/package.json` → `version`               | —       |
| Tools     | `tools/package.json` → `version`                | —       |
| Firmware  | `firmware/platformio.ini` comment or build flag | —       |

All component versions must be aligned before release. Use `npm version <type>` at root.

The release automation reads the root `package.json` version and only auto-tags stable `X.Y.Z` releases when `CHANGELOG.md` already contains a matching `## [X.Y.Z]` section.

## 4. Documentation

| Check                                                                 | Done? |
| --------------------------------------------------------------------- | ----- |
| CHANGELOG.md updated with release notes                               | ☐     |
| Breaking changes documented with migration guide                      | ☐     |
| New commands/features documented in `docs/reference/commands.md`      | ☐     |
| Hardware wiring changes documented in `docs/guides/hardware-setup.md` | ☐     |
| README.md version badge updated                                       | ☐     |
| THIRD_PARTY_LICENSES updated if deps changed                          | ☐     |

## 5. Release Artifacts

| Artifact                | Format        | Location                                          |
| ----------------------- | ------------- | ------------------------------------------------- |
| ESP32 firmware (serial) | `.bin`        | GitHub Release attachment (`esp32*.bin`)          |
| ESP32 WiFi firmware     | `.bin`        | GitHub Release attachment (`esp32_wifi*.bin`)     |
| ESP32 BLE firmware      | `.bin`        | GitHub Release attachment (`esp32_ble*.bin`)      |
| ESP32 WiFi+BLE firmware | `.bin`        | GitHub Release attachment (`esp32_wifi_ble*.bin`) |
| Browser client          | Docker image  | ghcr.io or Docker Hub                             |
| Protocol package        | npm (private) | npm workspace                                     |

## 6. Rollback Plan

- [ ] Previous release tag identified: `vX.Y.Z`
- [ ] Rollback firmware binaries available in previous release
- [ ] Docker image rollback: `docker pull <image>:previous-tag`
- [ ] Known breaking changes that block rollback: (list or "none")

### 6.1 Rollback Drill Evidence (Required)

| Drill                                    | Expected Result                                                 | Evidence Link / Artifact                                         | Owner            | Done?                        |
| ---------------------------------------- | --------------------------------------------------------------- | ---------------------------------------------------------------- | ---------------- | ---------------------------- |
| Firmware rollback drill (`vN` -> `vN-1`) | Board boots cleanly and reports expected variant/features       | Release note link + console capture                              | Firmware owner   | ☐ Awaiting hardware          |
| Unified client rollback drill            | `client/` starts and core tabs remain functional after rollback | Command output, screenshot set, or QA note for reverted snapshot | Client owner     | ☐                            |
| Export compatibility drill               | New exports remain readable by existing tools/docs expectations | Sample artifacts + verification note                             | Tools/Docs owner | ☐                            |
| Container rollback drill                 | Previous image starts healthy and serves expected API/UI        | Image digest + health output                                     | DevOps owner     | ☐ Awaiting docker deployment |

Rollback is not considered validated until all drills above are checked with evidence.

## 7. Go / No-Go Sign-off

| Area                    | Sign-off Owner | Status | Notes |
| ----------------------- | -------------- | ------ | ----- |
| Firmware quality gate   | Firmware owner | ☐      |       |
| Protocol + tooling gate | Protocol owner | ☐      |       |
| Unified client gate     | Client owner   | ☐      |       |
| Docs gate               | Docs owner     | ☐      |       |
| Rollback drill gate     | Release owner  | ☐      |       |

Release decision:

- [ ] **GO** — all gates and rollback drills complete with evidence
- [ ] **CONDITIONAL GO** — software gates pass; pending hardware/deployment drills are explicitly accepted

## 7.1 Targeted Drive Context Validation (Only for runtime signal releases)

If the release changes live drive-context signals, run strict drive-context capture and archive the report artifact.

Command:

```bash
node tools/debug.js drive-context --port COM5 --drive-duration 60000 --drive-min-samples 10 --drive-expect-full --drive-output artifacts/drive-context-report.json --drive-note-output artifacts/drive-context-note.txt
```

| Task                                    | Required report gate            | Required evidence                            |
| --------------------------------------- | ------------------------------- | -------------------------------------------- |
| Turn + blind-spot indicators            | `closureReadiness.d05 === true` | report JSON + scenario notes for 14.1-14.5   |
| Door / frunk / trunk open-state signals | `closureReadiness.d11 === true` | report JSON + scenario notes for 14.6-14.10  |
| Cruise / map / max-speed context        | `closureReadiness.d13 === true` | report JSON + scenario notes for 14.11-14.14 |

Global gate:

- `closureReadiness.allReady === true`
- strict command exit code is success
- validation notes include artifact path and capture timestamp (can be sourced from `drive-context-note.txt`)

## 8. Release Process

1. Ensure all CI checks pass on `main`
2. Complete manual verification (Section 2)
3. Verify version alignment (Section 3)
4. Update `CHANGELOG.md` and add a matching `## [X.Y.Z]` entry for the target release
5. Confirm the root `package.json` version is the target stable version `X.Y.Z`
6. Push the release commit to `main` and let `auto-tag-release.yml` create `vX.Y.Z`
7. If automation was skipped or the tag already exists, run `release.yml` manually against the `vX.Y.Z` tag ref
8. GitHub Actions release workflow builds and attaches firmware artifacts
9. Verify release artifacts are downloadable
10. Post release notes
