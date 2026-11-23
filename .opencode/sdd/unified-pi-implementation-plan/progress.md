# SDD ledger — plan: .opencode/plans/unified-pi-implementation-plan.md

Spec: `.opencode/plans/unified-pi-sentry-design.md` + `.opencode/reports/rpi-spike/ADR-2026-11-22.md`.
Branch: `spike/raspberry-pi-portability` (feature branch, pushed). Base for Task A1: `9926af3`.
Worktree has ~200 pre-existing unstaged files: every implementer stages ONLY its own files with explicit `git add <paths>`.

Ruling: no separate git worktree — current branch is already isolated. Cost if wrong: unrelated user work could be swept into a task commit (visible, revertible).

## Pre-flight conflict scan

| Tasks             | Shared surface                                       | Finding / ruling                                                                                                                                      |
| ----------------- | ---------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------- |
| A1 → A2 → A3 → A4 | `pi/` workspace, `createPiServer()`, `CanFrame` type | Strictly sequential: scaffold defines interfaces, reader produces frames, bridge consumes them, recorder consumes frames. No parallel implementation. |
| A3 ↔ B1           | ports/paths                                          | A3 binds `127.0.0.1:8090`; B1 nginx proxies `/tcm` there and `/` to Sentry USB. Keep distinct; no shared static root.                                 |
| B2 ↔ B3           | wiring vs RT gate                                    | B2 is documentation/bring-up only; B3 writes `pi/gate/rt-result.json`. Both must state TX disabled until gate passes.                                 |
| B3 → C1           | gate artifact                                        | C1 default `PI_CAN_TX=0` and unreachable without valid `rt-result.json`. Do NOT parallelize.                                                          |
| C1 ↔ can-review   | CAN mutation                                         | Checklist + listen-only capture mandatory; no bench hardware here → C1 may only implement gated path, not claim vehicle acceptance.                   |
| C2 ↔ Sentry USB   | `drive-data.json`                                    | Read-only consumption; never write to Sentry storage. No code copy (PolyForm Noncommercial).                                                          |
| All               | docs/superpowers ban                                 | New artifacts only under `.opencode/{plans,analysis,reports}/`.                                                                                       |

## Environment

- Node 22 present, npm workspaces installed, PlatformIO present. No physical Pi, no MCP2515 hardware, no bench capture — all hardware claims UNVERIFIED.
- Free model `opencode/hy4-flash-free#high` for implementers and reviewers (paid reviewer failed on credits).

## Task status

- Task A1: complete (commits 9926af3..01641d8, review clean). Workspace + `createPiServer()` + `/health` + `CAN_TRANSPORT`; 4/4 tests, typecheck/lint clean. Lockfile sync verified legitimate, jest config conventional.
    - Minor (deferred): RED was workspace-resolution failure, not assertion failure.
    - Minor (deferred): standalone `test:pi` needs prebuilt protocol dist; `test:all` builds it first.

- Task A2: complete (commits 01641d8..9e60626, review clean). `createCanReader()` with literal can_frame parse, 11-bit/DLC guards, bus mapping, transport re-creation on error; 11/11 focused, Pi 15/15, lint/typecheck clean.
    - ⚠️ Reviewer caveat: tests use fake transport; `createNetTransport` is a TCP gateway client, NOT a real SocketCAN binding. Real vcan/SocketCAN integration remains a hardware task (B2). All hardware claims UNVERIFIED.
    - ⚠️ `test:all` hangs at pre-existing `test:firmware` pio.ps1 wrapper on Windows; JS/TS suites green. Track as environment issue; do not misreport as green full suite.
    - Minors (deferred): clean EOF terminates reader (disclosed); carry lost on reconnect; lifetime reconnect counter; unvalidated `CAN_GATEWAY_PORT`.

- Task A3: complete (commits 9e60626..5d7bc72, review clean). Auth 401, 415 before body, 413 with drain, registry-derived allowlist (status/ping), TX refusal tested; 26/26 Pi tests, lint/typecheck clean.
    - Minor (deferred, fold into B1): loopback `127.0.0.1` is documented/tested but not code-enforced; a caller binding `0.0.0.0` would expose the token surface. B1 nginx/systemd must pin loopback and add a bind test.
    - Minors (deferred): stale fallback comment in index.ts; unused timeout option; OPTIONS bypass (fine behind same-origin proxy); single-consumer frame pump (design note for B1).

- Task A4: complete (commits 5d7bc72..5e01484, review clean). Provenance JSONL header + frames, awaited drain/finish, persistent error listener, `stop()` surfaces write errors; 29 Pi tests, lint clean.
    - Minors (deferred): path sanitization misses `/`/`\`; no explicit `destroy()` on error (autoDestroy covers it); discarded write rejection in chain; JSDoc Date.now vs new Date; unused `record()` alias.

- Task B1: fix round 1/5 started (reviewer Needs fixes; commits 5e01484..c77e6b1).
    - Ruling: important finding is real — a full standalone nginx config cannot be installed as a `sites-enabled` snippet on Pi OS. Fix: strip `worker_processes`/`events{}`/`http{}` from `pi/deploy/nginx.conf` so it is a valid snippet (keep `map`/`upstream`/`server`), update README install steps, and add a structural test tying install path to config shape. Alternative (full nginx.conf at /etc/nginx/nginx.conf) rejected because it would replace distro config. Cost if wrong: documented install fails `nginx -t` on first boot.
    - Minors folded in: assert no `alias` as well as `root`; exclude `::1` from the wildcard detector.
    - Fix round 1/5 complete (commit 67b7ef1, scoped re-review clean): nginx config is a valid sites-enabled snippet, README install path updated, structural test ties path to shape; focused 7/7, Pi 36/36.
    - Task B1: complete (commits 5e01484..67b7ef1, review clean). Two loopback upstreams, `/` and `/tcm` split, WebSocket proxy, wildcard-bind detector, systemd unit + runbook.
    - ⚠️ Runtime UNVERIFIED: Sentry USB port placeholder, `pi/dist/main.js` entrypoint not built yet, client `/tcm/` base path, no `nginx -t` binary here. Block hardware bring-up on runtime entrypoint.

- Task B2: fix round 1/5 started (reviewer Important: extra-bus overlay recipe not actionable). Ruling: keep RX-only but replace generic `mcp2515-canN` + arbitrary-GPIO CS with concrete per-bus lines — bus0 CE0, bus1 CE1 + `interrupt=24`, bus2 explicitly flagged as requiring a verified `cs=`/custom overlay. Also reconcile module VCC range and tighten `spi-dma` wording. Cost if wrong: multi-bus bring-up stays unverified, but no unsafe TX is enabled.
    - Fix round 1/5 complete (commits 6aa5056 + d581e67, scoped re-review clean): all three findings addressed; bus2 remains DO NOT USE pending verified overlay; lint + serial-contract clean.
    - Task B2: complete (commits 64e4a16..d581e67, review clean). Canonical wiring doc + regenerated bundle; all hardware claims UNVERIFIED by design.
    - Minor (deferred): bus2 CE2=GPIO18 label in the table is not guaranteed by stock overlays; contained by do-not-use caveat.

- Task B3: complete (commits d581e67..5eda9f8, review clean). Pure `decideRtGate()` boundary tests 99/100/101 µs, 1.9/2.0/2.1 ms, 4.9/5.0/5.1 ms; signed v1 artifact behind explicit opt-ins; constant-time HMAC, re-decides from raw numbers, fails closed on malformed/missing; no artifact created; TX still disabled. 55/55 Pi tests, lint/typecheck clean.
    - Minor (deferred, C1 sign-off): v1 artifact schema/HMAC self-defined, localized behind `isGateOpen()`.
    - Minor (deferred): runbook wording about failing runs writing artifacts only true for threshold failures; QA flake in untouched bridge/smoke tests noted, 3 clean reruns.

- Task C1: complete (commits 5eda9f8..3a80a95, review clean). Two-factor gate (`PI_CAN_TX=1` + B3 artifact), no default sender, 0x334 frame ported with independent oracle; 74/74 Pi tests, lint/typecheck clean. No artifact/capture/vehicle acceptance — injection remains release blocker.
    - Minors (deferred): cwd-relative gate path (fails closed, but silent), report's incorrect "capture-derived magic tables" wording, injectable gate predicate (test-only today).

- Task C2: complete (commits 3a80a95..83ef83a, review clean). Read-only tolerant `drive-data.json` connector with bounded walk, graceful missing/invalid degradation, fixture tests, Studio/Drive + encryption docs. 79/79 Pi tests, lint clean. Real path/schema UNVERIFIED; no production caller yet.
    - Minor (deferred): self-link guard (`clip === drive`) not present.

## Final whole-branch review (base 9926af3, head 5da646f)

Ruling: reviewer's two Important findings are accepted and will be FIXED, not re-scoped — the maintainer explicitly asked for the whole stack on the Pi, so "libraries + runbook only" would contradict the directive. Add a runtime entrypoint (`pi/src/main.ts` + emit build) and a real SocketCAN read transport (candump-backed gateway with injected process spawn), keep gated TX unmounted. Cost if wrong: more unmounted code without hardware verification; all hardware claims stay UNVERIFIED.

Ruling: nginx not machine-validated is an acceptable UNVERIFIED deferral (no nginx binary here; structural test + runbook). Cost if wrong: runtime config error discovered on first Pi boot.

One fix wave with the complete findings list; no per-finding fixers.

- Final fix wave complete (commits 9a08256 + dc555b5, scoped re-review clean): all 2 Important + 9 minors ADDRESSED. Runtime entrypoint (`pi/src/main.ts`, emit build, 5 composition tests) and candump-backed SocketCAN transport (7 fake-process tests) shipped; TX remains unmounted, no artifact. Pi 102/102, typecheck/build/lint clean.
    - Ruling: reviewer's new Medium finding (clean `candump` child exit is not reconnectable — `candump.ts:104-112,156` + `socketcan.ts:212`) is PARKED, not fixed in a second wave (skill rule). It fails closed by stopping frame flow, not by injecting; systemd will show a healthy process with a dead stream. Cost if wrong: Pi service silently stops seeing CAN after interface removal/restart until manually restarted. Surface in the final message.
    - Minor (deferred): `createNetTransport()` retained as dead optional path; `fanoutFrames()` per-subscriber queues unbounded; `parseBindConfig()` accepts non-loopback unicast (deploy pins 127.0.0.1). All disclosed in the report.
