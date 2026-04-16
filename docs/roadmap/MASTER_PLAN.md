# Master Improvement Plan

## Objectives

1. Stabilize CI and eliminate flaky/failing checks.
2. Harden protocol and runtime safety paths.
3. Improve product functionality and operator workflows.
4. Upgrade UI/UX for web and mobile with clearer feedback and faster control.
5. Increase test confidence from unit-level to integration and E2E.

## Delivery Phases

## Phase 1 - CI And Foundations (Week 1) ✅ COMPLETE

- CI test discovery fixes and workflow stabilization.
- Baseline dependency and test execution reliability.
- Tracker and execution governance setup.

Exit Criteria:

- All PR CI jobs pass consistently on clean runners.
- No false-negative failing suites from placeholder/legacy files.
- Protocol prebuild dependency handled in jobs that consume shared artifacts.

## Phase 2 - Protocol And Runtime Reliability (Weeks 2-3) ✅ COMPLETE

- Input and range validation in protocol command builders.
- Parser error surfacing improvements.
- Web/mobile transport error handling and reconnect behavior.
- Firmware command ack timeout feedback loop in clients.

Exit Criteria:

- Invalid command ranges are rejected before send.
- Parser reports malformed lines predictably.
- Disconnect/reconnect behavior is deterministic and user-visible.

## Phase 3 - Feature And Experience (Weeks 3-5)

- Web monitor export/filter presets and command discoverability.
- Mobile BLE picker and reconnect UX improvements.
- Docs onboarding simplification and troubleshooting expansion.

Exit Criteria:

- Operator can export, filter, and replay practical frame sessions.
- Mobile reconnect and permission flows are clear.
- Setup docs reduce first-time setup friction.

## Phase 4 - Scale And Release Governance (Weeks 6-8)

- Integration/E2E coverage introduction.
- Security and dependency gates in CI.
- Firmware size/memory regression checks.
- Release checklist and quality bars.

Exit Criteria:

- E2E smoke path validates end-to-end command lifecycle.
- Security checks run in CI and block critical regressions.
- Releases follow a documented gated checklist.

## KPIs

- CI pass rate >= 98% on default branch.
- Mean time to diagnose failing CI check < 15 minutes.
- Web/mobile command round-trip success >= 99% in smoke tests.
- First-time setup completion time reduced by at least 30%.
- Integration coverage introduced for top 5 critical workflows.

## Dependencies And Risks

- Hardware-in-the-loop constraints for full E2E.
- Browser transport differences (Web Serial support matrix).
- BLE variance by platform/device vendor.

## Mitigations

- Keep deterministic simulator-based tests for core protocol paths.
- Separate hardware-required and hardware-free CI jobs.
- Add explicit feature detection and fallback UX in clients.
