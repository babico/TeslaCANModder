# Task B1 — nginx portal + systemd units

Where: first deployment task. Two separate services behind one web entry point.

## Requirement (verbatim from plan)

Files:

- Create: `pi/deploy/nginx.conf`, `pi/deploy/tcm-can.service`, `pi/deploy/README.md`
- Do not modify Sentry USB files.

Interfaces:

- Consumes: A3 server on `127.0.0.1:8090`; Sentry USB on its own port.
- Produces: `/` → Sentry UI, `/tcm` → our client/static bundle; separate service units.

Steps:

1. Write a config test (regex/structural check) asserting two distinct upstreams, no shared root, and that `/tcm` proxies WebSocket/HTTP correctly.
2. RED.
3. Write nginx + systemd files + runbook.
4. GREEN; document `systemctl status` checks.
5. Commit — `feat(pi): nginx portal and systemd units`.

## Additional binding fix (from A3 review)

A3's `127.0.0.1` default is documented/tested but not code-enforced. This task MUST pin loopback in the systemd unit / nginx upstream and add a test that a bind to `0.0.0.0` is rejected (or at minimum that the deployed config never binds a wildcard). Fold into the same commit.

## Global constraints

- No Sentry code copied or modified; no CAN TX; no hardware claims (UNVERIFIED).
- Stage ONLY files you change (~200 unrelated unstaged exist); verify staged list before commit.
- Feature branch `spike/raspberry-pi-portability`; never main, never force-push.
