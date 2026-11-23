# Task B3 — RT measurement gate

Where: third deployment task. Produces the go/no-go artifact that alone may enable CAN TX in C1.

## Requirement (verbatim from plan)

Files:

- Create: `pi/src/rt/measure.ts`, `pi/test/rt.test.ts`, `pi/deploy/rt-gate.md`
- Produces: a command that runs `cyclictest` and an RX→TX probe, writes a signed gate artifact `pi/gate/rt-result.json` with measured numbers; TX backend reads that artifact.

Steps:

1. Unit-test the gate decision function against boundary values (99/100/101 µs, 1.9/2.0/2.1 ms, 4.9/5.0/5.1 ms) — pure, no hardware.
2. RED / implement / GREEN.
3. Document the bench procedure (isolated core, PREEMPT_RT, load generator) and exact go/no-go thresholds.
4. Commit — `feat(pi): RT measurement gate (TX still disabled)`.

Thresholds (binding): `cyclictest <100 µs`, RX→TX p99.99 <2 ms, worst <5 ms under load. All must pass; no hardware here so the artifact does NOT exist yet and TX stays disabled.

## Global constraints

- No CAN TX implementation here; gate decision + harness only.
- No Sentry code; no hardware claims — bench procedure is a runbook, mark UNVERIFIED.
- Stage ONLY files you change (~200 unrelated unstaged exist); verify staged list before commit.
- Feature branch `spike/raspberry-pi-portability`; never main, never force-push.
