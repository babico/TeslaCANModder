# Task A4 — Recorder and provenance

Where: fourth task of the unified Pi plan. Writes CAN captures from A2 frames with provenance metadata.

## Requirement (verbatim from plan)

Files:

- Create: `pi/src/record/recorder.ts`, `pi/test/record.test.ts`

Interfaces:

- Consumes: `CanFrame` (A2).
- Produces: `createRecorder(dir)` → JSONL files with `{vehicle,bus,firmware,date}` provenance header + frames; `stop()` flushes and closes streams (no fire-and-forget).

Steps:

1. Test that a flushed file has a provenance header and N frames, and that a stream error rejects.
2. RED.
3. Implement with awaited drain + error listener.
4. GREEN + lint.
5. Commit — `feat(pi): capture recorder with provenance`.

## Global constraints

- No CAN TX, no Sentry code, no hardware; use injected frames/writable stream in tests.
- Reuse the capture provenance conventions from `tools/commands/dump.js` / `tools/lib/canExport.js` conceptually; do not copy Sentry code.
- Stage ONLY files you change (~200 unrelated unstaged exist); verify staged list before commit.
- Feature branch `spike/raspberry-pi-portability`; never main, never force-push.
