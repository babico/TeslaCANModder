# RT measurement gate — bench runbook

> **UNVERIFIED — no measurements were taken for this document.** There is no
> Raspberry Pi, MCP2515/SPI CAN adapter, PREEMPT_RT kernel or `cyclictest` binary
> in the development environment. Every board command below is a procedure to
> run on the exact target board, not a recorded result. This task writes no
> `pi/gate/rt-result.json` artifact and leaves CAN TX disabled.

## What the gate decides

This is the only artifact that may unlock CAN TX in Task C1. All three limits
must hold **for one hour, under 100 % CPU load and a saturated CAN bus, on the
exact board that will run the service**. Each comparison is strictly less-than;
a value exactly on the bound is a **fail**.

| Metric                       | Bound   | Fail at and above |
| ---------------------------- | ------- | ----------------- |
| `cyclictest` maximum latency | <100 µs | 100 µs            |
| RX→TX p99.99 latency         | <2 ms   | 2000 µs           |
| RX→TX worst-case latency     | <5 ms   | 5000 µs           |

Thresholds are binding from ADR §6 (`.opencode/reports/rpi-spike/ADR-2026-11-22.md`)
and Task B3. They are a proposed rule, **UNVERIFIED as a standard**. The pure
decision is `decideRtGate()` in `pi/src/rt/measure.ts`; its unit tests cover both
sides of all three bounds (99/100/101 µs, 1.9/2.0/2.1 ms, 4.9/5.0/5.1 ms).

## Preconditions — all UNVERIFIED on the target

- Listen-only bring-up from `docs/guides/pi-can-bringup.md` is complete: the
  CAN interface is up at 500 kbps and `candump` sees the expected X179 frames
  with 120 Ω termination. The ESP32 remains the independent bench oracle.
- PREEMPT_RT is installed (`uname -r` contains `-rt`) on the exact board.
- One core is isolated and the measurement/bridge threads run `SCHED_FIFO`
  with `mlockall`. `cyclictest` args are built by `buildCyclictestArgs()`.
- Tools present on the board: `rt-tests` (`cyclictest`), `rt-can-test`,
  `stress-ng`, `candump` (verify each with `--version`; UNVERIFIED here).
- No Sentry Six code is used, copied or modified by this procedure.

## Procedure (run on the board, not here)

**Step 1 — confirm the kernel and isolated core.** Record `uname -a`,
`cat /proc/cmdline` and the pinned CPU list in the run notes. `isolcpus` /
`nohz_full` / `rcu_nocbs` must be set at boot; a runtime-only `taskset` pin is
not proof of isolation.

**Step 2 — start the load generator** so the gate is measured under stress:

```bash
stress-ng --cpu "$(nproc)" --cpu-load 100 --timeout 1h &
```

**Step 3 — run `cyclictest`** pinned to the isolated core for one hour. The
skeleton uses `--mlockall`, `--smp`, `--priority=80`, `--interval=200`,
`--distance=0` and `--duration=60m`; record the full summary line, especially
`Max:` (µs).

```bash
taskset -c <isolated-core> cyclictest --mlockall --priority=80 \
  --interval=200 --distance=0 --duration=60m
```

**Step 4 — run the RX→TX probe** with the CAN adapter looped back to a second
channel, as described in `can-hardware.md` §3.3. Capture a single-line JSON
result with `p9999Us` and `worstUs` in microseconds:

```json
{ "p9999Us": 1800, "worstUs": 4200 }
```

**Step 5 — feed both outputs** to the command skeleton with an injected
hardware runner and writer. The skeleton refuses to run without all three
explicit opt-ins (`enable: true`, `PI_RT_MEASURE=1`, `PI_RT_GATE_KEY`), so an
accidental developer-machine invocation produces no artifact:

```bash
export PI_RT_MEASURE=1
export PI_RT_GATE_KEY='<board-local secret, never committed>'
export PI_RT_BOARD='Pi 5 8GB'
export PI_RT_KERNEL="$(uname -r)"
export PI_RT_LOAD='stress-ng --cpu N --cpu-load 100'
# invoke runRtMeasurement() with a node:child_process runner and an fs writer
```

**Step 6 — compare a listen-only CAN capture** against the ESP32 oracle and
walk `docs/checklists/can-review-checklist.md`. A timing pass alone is not
authorisation to inject; the signed review checklist is also required.

## Artifact contract (v1) — for Task C1

Writing the artifact is the job of the hardware-bound runner in step 5. The
file is `pi/gate/rt-result.json`. **It does not exist in this repo** — the
object below is a schema example, not a real result:

<!-- markdownlint-disable MD010 -->

```json
{
    "schemaVersion": 1,
    "generatedAt": "2026-11-23T01:02:03.000Z",
    "board": "Pi 5 8GB",
    "kernel": "6.12-rt",
    "method": {
        "cyclictestArgs": "--mlockall --smp --priority=80 --interval=200 --distance=0 --duration=60m",
        "loadGenerator": "stress-ng --cpu N --cpu-load 100",
        "durationMin": 60,
        "probe": "rt-can-test"
    },
    "measurements": {
        "cyclictestMaxUs": 95,
        "rxtxP9999Us": 1800,
        "rxtxWorstUs": 4200
    },
    "thresholds": {
        "cyclictestMaxUs": 100,
        "rxtxP9999Us": 2000,
        "rxtxWorstUs": 5000
    },
    "verdict": { "pass": true, "failures": [] },
    "signature": {
        "alg": "HMAC-SHA256",
        "keyId": "env:PI_RT_GATE_KEY",
        "value": "<hex HMAC>"
    }
}
```

<!-- markdownlint-enable MD010 -->

- `signature.value` is HMAC-SHA256 over the canonical JSON (recursively sorted
  keys) of every field **except** `signature`, keyed by `PI_RT_GATE_KEY`.
  `keyId` names the key source; the secret itself is never stored.
- C1 must call `isGateOpen(artifact, key)` before enabling TX. It verifies the
  signature, schema version and binding thresholds, requires `verdict.pass`,
  and re-decides from the raw measurements so a claimed pass cannot bypass the
  limits. `PI_CAN_TX` stays `0` by default.
- Missing artifact, wrong key, schema mismatch, tampered numbers or any failed
  metric all mean **TX stays off**. A failing run still writes an artifact with
  `verdict.pass: false`; that is a record, not an unlock.
- The key must be provisioned out of band on the board and never committed.

## Unverified / out of scope

- No board, CAN adapter, kernel or capture was available: no timing number in
  this document is a measurement. All hardware claims are **UNVERIFIED**.
- This task implements the decision function, signing/verification and the
  command skeleton only. It does not implement TX, a real `child_process`
  runner, or an on-board entrypoint.
- `cyclictest` passing on `vcan` is not bench proof. Only the exact board with
  the real adapter counts.
