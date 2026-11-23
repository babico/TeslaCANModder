# Task A4 — Recorder and provenance (report)

Branch: `spike/raspberry-pi-portability`
Commit: `5e01484` — `feat(pi): capture recorder with provenance` (pushed)

## Files changed

- `pi/src/record/recorder.ts` (new, 112 lines) — `createRecorder(dir, options?)`
- `pi/test/record.test.ts` (new, 103 lines) — 3 tests

No other files staged or committed. ~200 pre-existing unstaged files left untouched; staged list verified with `git diff --cached --name-status` before commit.

## TDD evidence

### RED

`npm test -w @teslacanmodder/pi` failed to compile the new suite:

```
FAIL test/record.test.ts
  ● Test suite failed to run
    TS2307: Cannot find module '../src/record/recorder.js' or its corresponding type declarations.
Test Suites: 1 failed, 3 passed, 4 total
Tests:       26 passed, 26 total
```

### GREEN

After implementation (and again after the Husky pre-commit hooks rewrote formatting):

```
PASS test/bridge.test.ts
PASS test/record.test.ts
PASS test/socketcan.test.ts
PASS test/smoke.test.ts
Test Suites: 4 passed, 4 total
Tests:       29 passed, 29 total
```

`npm run lint:all` — ESLint clean, Prettier "All matched files use Prettier code style!", markdownlint 0 issues in 44 files.

## Implementation decisions (provenance shape)

Brief specified header keys `{vehicle,bus,firmware,date}` but not their types/defaults. Resolved as follows — flagging here so the controller can correct if a different shape was intended:

- Header is **one JSON object as the first JSONL line**, with exactly those four keys (test asserts exact key set).
- `vehicle: string` default `"unknown"`; `firmware: string` default `"unknown"` (aligned with tools' `dataset_source.vehicle` / `.firmware` in `tools/lib/canExport.js`).
- `bus: BusId[]` default `[0, 1, 2]` (all three X179 lanes the A2 reader can produce).
- `date: string` ISO-8601, from injected `now()` or `new Date()`.
- Recorder API: `createRecorder(dir, options)` → `{ path, provenance, write(frame), record(frame) [alias], stop() }`. `write` returns a promise that clears backpressure.
- Frame lines reuse the firmware wire shape `{t:"frame",dir:"rx",bus,id,dlc,d}` plus `ts: frame.rxMs`, matching `tools/commands/dump.js` JSONL convention. Hex data via `Buffer.from(data).toString("hex")`.
- Output file: `capture-<date>.jsonl` (date colons/dots sanitized); `dir` created recursively only when using the real stream factory.
- `stream?: Writable` option injects a fake writable for tests, so no hardware/socket/file is needed for those cases.
- Writes are serialized through a promise chain (order preserved); `drain` is awaited via `node:events.once`, which rejects on stream `error`. A persistent `error` listener attached before the first write captures the first error and prevents an uncaught `error` event. `stop()` awaits the whole chain, ends the stream, awaits `finish`, and throws the captured error. `stop()` is idempotent.

## Constraints check

- No CAN TX, no Sentry code, no hardware — tests use injected `CanFrame` literals and a fake `Writable`; one test uses a real temp file + `mkdtempSync`/`rmSync` to prove actual flush-to-disk on `stop()`.
- Frames consumed from A2 `CanFrame` (type-only import), no runtime coupling to the SocketCAN reader.
- Staged only the two new files; never main, no force-push (fast-forward `5d7bc72..5e01484`).

## Self-review

- [x] Header shape exactly `{vehicle,bus,firmware,date}`; no extra keys.
- [x] N frames written after header, in injection order; verified against on-disk bytes.
- [x] `stop()` awaits drain/finish, ends stream, surfaces first stream error; idempotent.
- [x] Persistent `error` listener attached before first write; no uncaught error possible.
- [x] Rejected write tasks keep the chain alive (`tail = task.catch(...)`) and have a rejection handler, so no unhandled promise rejection from the fire-and-forget header write.
- [x] No secrets, no generated artifacts, no unrelated files in commit.
- [x] Lint + format + markdown clean; tests pass post-hook.

## Concerns / minor nits (non-blocking, not fixed)

1. JSDoc on `RecorderOptions.now` says "Defaults to `Date.now`" but the implementation uses `() => new Date()` (required for `.toISOString()`). Cosmetic only.
2. Default `bus: [0, 1, 2]` and frame field `ts` are my choices; if the controller expects a single numeric `bus` or `rxMs` frame key, this is the spot to change.
3. Output filename sanitizes `:` and `.` from `date` but not `/`; a caller-supplied `provenance.date` containing path separators could escape `dir`. Low risk (caller-controlled config, not vehicle data), but can be hardened with an allowlist regex.
4. `stop()` throws on a captured error before calling `stream.end()`; Node normally auto-destroys an errored writable, but an explicit `destroy()` in that path would make cleanup deterministic.
5. `record()` alias is currently unused by any caller; harmless but could be dropped if a tighter API is preferred.
6. `Recorder` is not yet wired into the Pi bridge or a CLI entry point — A4 only asked for the module; integration belongs to a later task.
