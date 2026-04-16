# Task Tracker

## Current Snapshot

- Updated: 2026-04-16
- Program State: Phase 02 complete — Phase 03 next

## Phase Summary

| Phase | Total | Done | In Progress | Not Started | Progress |
|-------|-------|------|-------------|-------------|----------|
| 01 | 7 | 7 | 0 | 0 | 100% |
| 02 | 4 | 4 | 0 | 0 | 100% |
| 03 | 4 | 0 | 0 | 4 | 0% |
| 04 | 4 | 0 | 0 | 4 | 0% |
| **Total** | **19** | **11** | **0** | **8** | **58%** |

## Work Items

| ID | Phase | Area | Task | Priority | Status | Owner | Acceptance Criteria | Notes |
|---|---|---|---|---|---|---|---|---|
| P1-001 | 01 | CI/Web | Restrict vitest include to TS/TSX active suites | High | done | Engineering | Web CI passes without empty JS suite failures | `web/vite.config.js` include restricted to `test/**/*.test.ts(x)` |
| P1-002 | 01 | CI | Build protocol before web job test/build | High | done | Engineering | Web job can consume protocol artifacts deterministically | `needs: protocol` in CI workflow |
| P1-003 | 01 | CI | Build protocol before mobile job test | High | done | Engineering | Mobile job prebuild dependency explicit | `needs: protocol` in CI workflow |
| P1-004 | 01 | Planning | Create detailed roadmap markdown pack | High | done | Engineering | Phase docs + tracker + execution log committed | Under `docs/roadmap/` |
| P1-005 | 01 | CI | Add workflow-lint job (actionlint) | Medium | done | Engineering | CI validates workflow syntax | `workflow-lint` job in `ci.yml` |
| P1-006 | 01 | CI | Add markdown-lint job | Medium | done | Engineering | CI validates markdown files | `markdown-lint` job in `ci.yml` |
| P1-007 | 01 | CI | Add docker-smoke health check job | Medium | done | Engineering | Docker image health verified in CI | `docker-smoke` job depends on `docker` build |
| P2-001 | 02 | Protocol | Add range validation in command builders | High | done | Engineering | Invalid values rejected/tested | `assertRange`/`assertVariant` guards added; RangeError on invalid profile(0–4), offset(-10–10), seat(0–3), display(0–100), variant(hw3/hw4/legacy). 118 protocol tests pass. |
| P2-002 | 02 | Protocol | Surface parse errors explicitly | High | done | Engineering | Parse error path covered by tests | `ParsedEvent.type` extended with `'parse-error'`; `reason` field added. JSON parse failures now return `{ type: 'parse-error', reason }` instead of silent `ignore`. |
| P2-003 | 02 | Web | Add ack timeout and transport error UI | High | done | Engineering | User sees timeout/disconnect states | `useSerial` tracks `pendingCommand` + `lastError`; 5s ack timeout auto-clears. ConnectionBar shows pending indicator + error banner with dismiss. 63 web tests pass. |
| P2-004 | 02 | Mobile | Add BLE reconnect and permission diagnostics | High | done | Engineering | Reconnect tested; clear error prompts | `BleTransport` auto-reconnects 3× with backoff on unexpected disconnect. `checkBlePermissions()` + `requestBlePermissions()` exported. `useTransport` surfaces `lastError`. |
| P3-001 | 03 | Web UX | Add frame export and filter presets | Medium | not-started | TBD | Export works and tested | FrameTable displays live frames; missing CSV/JSON export, filter presets, pause/resume |
| P3-002 | 03 | Web UX | Add command palette and recent actions | Medium | not-started | TBD | Top commands are keyboard and click discoverable | Commands exist as buttons in DashboardPage; no palette or keyboard shortcuts |
| P3-003 | 03 | Mobile UX | Improve BLE picker metadata and recovery flow | Medium | not-started | TBD | Device selection and reconnect success improves | Basic DeviceScanner; missing RSSI labels, service hints |
| P3-004 | 03 | Docs UX | Add quickstart checklist + troubleshooting tree | Medium | not-started | TBD | Fewer setup blockers in test run | `troubleshooting.md` decision tree exists; quickstart checklist missing |
| P4-001 | 04 | QA | Add E2E smoke for core command lifecycle | High | not-started | TBD | PRs execute e2e smoke in CI | `docs/e2e-test-plan.md` has scenarios documented; no test code yet |
| P4-002 | 04 | Security | Harden dependency and security checks | High | not-started | TBD | CI surfaces and enforces security findings | `security-audit` job exists (`npm audit --audit-level=high`); needs Dependabot, license scanning, medium-level gating |
| P4-003 | 04 | Firmware | Add resource regression thresholds in CI | Medium | not-started | TBD | Build fails on threshold overrun | Firmware job runs tests only; no binary size tracking or RAM thresholds |
| P4-004 | 04 | Release | Add release quality gate checklist | Medium | not-started | TBD | Release process documented and used | Governance outline in phase doc; not enforced via CI or checklist file |

## Status Rules

- `not-started`: no implementation work has begun.
- `in-progress`: active implementation or validation.
- `blocked`: waiting on dependency or decision.
- `done`: merged and validated.

## Next Actions

1. Begin Phase 03 — start with P3-001 (frame export/filter) and P3-002 (command palette) as web UX improvements.
2. P3-003 (BLE picker) can parallel with P3-004 (quickstart docs).
