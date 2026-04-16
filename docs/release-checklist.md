# Release Quality Gate Checklist

Pre-release checklist for TeslaCANModder. Every tagged release **must** pass all gates before publishing artifacts.

---

## 1. CI Pipeline Gates

| Gate | Requirement | Verified By |
| ----- | ------------ | ------------ |
| Firmware native tests | All pass (`pio test -e native`) | CI `firmware` job |
| Firmware size regression | Within thresholds | CI `firmware` job |
| Protocol unit tests | All 158+ tests pass | CI `protocol` job |
| E2E smoke tests | All pass | CI `e2e-smoke` job |
| Web lint + typecheck | Zero errors | CI `web` job |
| Web unit tests | All pass | CI `web` job |
| Web production build | Succeeds | CI `web` job |
| Mobile lint + typecheck | Zero errors | CI `mobile` job |
| Mobile unit tests | All pass | CI `mobile` job |
| Tools tests | All pass | CI `tools` job |
| Docker build | Succeeds | CI `docker` job |
| Docker smoke test | Heartbeat healthy | CI `docker-smoke` job |
| Security audit | No medium+ vulnerabilities | CI `security-audit` job |
| License check | All deps in allowlist | CI `security-audit` job |
| Markdown lint | Zero errors | CI `markdown-lint` job |
| Workflow lint | Zero errors | CI `workflow-lint` job |

## 2. Manual Verification

| Check | Owner | Done? |
| ------ | ------ | ------ |
| Flash `uno` firmware on test board — boot JSON valid | Engineer | ☐ |
| Flash `esp32_wifi_ble` firmware — boot JSON valid | Engineer | ☐ |
| Web dashboard connects to device via Web Serial | Engineer | ☐ |
| Mobile app BLE scan finds ESP32 device | Engineer | ☐ |
| Command round-trip: FSD on → ack received | Engineer | ☐ |
| Frame streaming: frames appear in dashboard | Engineer | ☐ |
| EEPROM/NVS persistence: settings survive reboot | Engineer | ☐ |

## 3. Version Alignment

| Component | Version File | Current |
| ---------- | ------------ | -------- |
| Root | `package.json` → `version` | — |
| Protocol | `packages/protocol/package.json` → `version` | — |
| Web | `web/package.json` → `version` | — |
| Mobile | `mobile/package.json` → `version` | — |
| Tools | `tools/package.json` → `version` | — |
| Firmware | `hardware/platformio.ini` comment or build flag | — |

All component versions must be aligned before release. Use `npm version <type>` at root.

## 4. Documentation

| Check | Done? |
| ------ | ------ |
| CHANGELOG.md updated with release notes | ☐ |
| Breaking changes documented with migration guide | ☐ |
| New commands/features documented in `docs/commands.md` | ☐ |
| Hardware wiring changes documented in `docs/hardware-setup.md` | ☐ |
| README.md version badge updated | ☐ |
| THIRD_PARTY_LICENSES updated if deps changed | ☐ |

## 5. Release Artifacts

| Artifact | Format | Location |
| --------- | ------- | --------- |
| Arduino Uno firmware | `.hex` | GitHub Release attachment |
| Arduino Uno + BT firmware | `.hex` | GitHub Release attachment |
| ESP32 firmware (serial) | `.bin` | GitHub Release attachment |
| ESP32 WiFi firmware | `.bin` | GitHub Release attachment |
| ESP32 BLE firmware | `.bin` | GitHub Release attachment |
| ESP32 WiFi+BLE firmware | `.bin` | GitHub Release attachment |
| Web dashboard | Docker image | ghcr.io or Docker Hub |
| Protocol package | npm (private) | npm workspace |

## 6. Rollback Plan

- [ ] Previous release tag identified: `vX.Y.Z`
- [ ] Rollback firmware binaries available in previous release
- [ ] Docker image rollback: `docker pull <image>:previous-tag`
- [ ] Known breaking changes that block rollback: (list or "none")

## 7. Release Process

1. Ensure all CI checks pass on `main`
2. Complete manual verification (Section 2)
3. Verify version alignment (Section 3)
4. Update CHANGELOG.md
5. Create git tag: `git tag -a vX.Y.Z -m "Release vX.Y.Z"`
6. Push tag: `git push origin vX.Y.Z`
7. GitHub Actions release workflow builds and attaches firmware artifacts
8. Verify release artifacts are downloadable
9. Post release notes
