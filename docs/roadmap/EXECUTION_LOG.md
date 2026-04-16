# Execution Log

## 2026-04-16

- Established roadmap structure in `docs/roadmap`.
- Added phase documents and task tracker.
- Fixed CI-related web test discovery issue by restricting vitest include patterns to TS/TSX test suites.
- Updated CI workflow to prebuild protocol package before web and mobile jobs.
- Reproduced prior web CI failure mode locally (`No test suite found` in legacy `.js` test files).

## 2026-04-16 (Update 3)

- Implemented all Phase 02 tasks (P2-001 through P2-004).
- P2-001: Added `assertRange()` and `assertVariant()` guards to `commands.ts`. Validates profile(0–4), offset(-10–10), seat(0–3), display(0–100), variant(hw3/hw4/legacy). Exported `COMMAND_RANGES`, `VALID_VARIANTS`, `Variant` type.
- P2-002: Extended `ParsedEvent` type with `'parse-error'` and `reason` field. Parser now distinguishes malformed JSON (parse-error) from non-JSON noise (ignore).
- P2-003: `useSerial` now tracks `pendingCommand` and `lastError` state. 5-second ack timeout auto-surfaces as error. `ConnectionBar` shows pending command indicator and error banner with dismiss. `App.tsx` wires ack messages to clear pending state.
- P2-004: `BleTransport` auto-reconnects up to 3× with increasing backoff (1s, 2s, 3s) on unexpected disconnect. Added `checkBlePermissions()` and `requestBlePermissions()` for Android adapter state and permission diagnostics. `useTransport` hook surfaces `lastError` and `clearError`.
- Files Updated: `packages/protocol/src/commands.ts`, `packages/protocol/src/types.ts`, `packages/protocol/src/parser.ts`, `packages/protocol/src/index.ts`, `packages/protocol/test/commands.test.ts`, `packages/protocol/test/parser.test.ts`, `packages/protocol/test/parser-edge.test.ts`, `web/src/hooks/useSerial.ts`, `web/src/components/ConnectionBar.tsx`, `web/src/components/ConnectionBar.css`, `web/src/pages/DashboardPage.tsx`, `web/src/App.tsx`, `mobile/lib/transport/ble.ts`, `mobile/hooks/useTransport.ts`
- Validation Performed: 118 protocol tests pass. 63 web tests pass. Protocol typecheck clean.
- Follow-up Actions: Begin Phase 03.

## 2026-04-16 (Update 2)

- Full project audit performed to reconcile roadmap tracker against actual repo state.
- Phase 01 declared complete — all 4 original tasks done, plus 3 newly discovered CI jobs (workflow-lint, markdown-lint, docker-smoke) added as P1-005, P1-006, P1-007.
- Task tracker upgraded: added phase summary table, detailed notes per task, and next-actions section.
- P2-002 partial finding: parser has catch block but returns `{ type: 'ignore' }` — no explicit error type surfaced.
- P4-002 partial finding: `security-audit` job exists (`npm audit --audit-level=high`) but lacks Dependabot, license scanning, or medium-level gating.
- Files Updated: `docs/roadmap/TASK_TRACKER.md`, `docs/roadmap/EXECUTION_LOG.md`
- Validation Performed: Cross-referenced ci.yml, vite.config.js, commands.ts, parser.ts, ble.ts, ConnectionBar.tsx, FrameTable.tsx, DashboardPage.tsx, troubleshooting.md, e2e-test-plan.md against tracker.
- Follow-up Actions: Begin Phase 02 starting with P2-001 and P2-002.

## Next Entries Template

- Date:
- Change Summary:
- Files Updated:
- Validation Performed:
- Follow-up Actions:
