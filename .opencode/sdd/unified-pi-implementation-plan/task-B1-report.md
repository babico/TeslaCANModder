# Task B1 report — nginx portal + systemd units

Branch: `spike/raspberry-pi-portability`
Base: `5e01484` (A4 tip).
Commit: `c77e6b1 feat(pi): nginx portal and systemd units` — pushed to `origin/spike/raspberry-pi-portability` (`5e01484..c77e6b1`).
Status: DONE — TDD RED/GREEN confirmed, 4 files committed with explicit staging, pushed, self-review clean, post-commit gates green.

## What was built

- `pi/deploy/nginx.conf` — full standalone nginx template: `events`/`http`, two loopback-only
  upstreams, public `:80` portal. `/` proxies to `sentry_usb`; `location = /tcm` redirects to
  `/tcm/`; `/tcm/` proxies to `tcm_can` with the trailing slash stripped so A3 sees `/health`,
  `/status`, `/api/status`, `POST /api/command`. WebSocket handled via `map $http_upgrade
$connection_upgrade` + `proxy_http_version 1.1` + `Upgrade`/`Connection` headers. No `root`
  directive anywhere — no shared static root.
- `pi/deploy/tcm-can.service` — `Type=simple`, dedicated `tcmcan` user, `EnvironmentFile` for the
  A3 bearer token, `TCM_BIND_HOST=127.0.0.1` / `TCM_BIND_PORT=8090`, hardening directives,
  `Restart=on-failure`. Does not reference or modify any Sentry USB unit/file.
- `pi/deploy/README.md` — UNVERIFIED input register, loopback rationale, install steps
  (`envsubst` render with restricted variable list, `nginx -t`, `daemon-reload`, `enable --now`),
  `systemctl status` checks, `ss`/`curl` probes, token file permissions, safety/no-TX section.
- `pi/test/deploy.test.ts` — 6 structural tests: two distinct loopback upstreams with the
  Sentry port marked UNVERIFIED, route split + no shared root, WebSocket/HTTP proxy headers,
  systemd loopback pin, wildcard-bind detector (clean on real files, positive on 4 bad
  fixtures), and runbook completeness.

## Sentry USB port — asked/UNVERIFIED decision

Brief: "If Sentry USB's actual local port/asset paths are unknown, ASK NOW — do not guess a port;
use a documented configurable variable with a default clearly marked UNVERIFIED."

Searched `docs/`, `.opencode/plans/`, `.opencode/sdd/`, and the full grep corpus: the only
statement is "Sentry USB on its own port" / "Consumes: A3 server on `127.0.0.1:8090`; Sentry USB
on its own port". No port or asset path is documented. As a spawned subagent I cannot block on an
interactive answer, so I applied the brief's fallback: `SENTRY_USB_PORT` is an `envsubst` variable
with **no guessed numeric default** — the template keeps the literal placeholder and the README
requires the maintainer to supply the real value. Every occurrence is labelled UNVERIFIED. This
is the top concern for the controller to resolve before bench deployment.

## A3 residual (loopback enforcement)

A3 documented/tested `127.0.0.1` but did not enforce it at deploy time. B1 now pins loopback in
both deploy surfaces (nginx upstream `server 127.0.0.1:8090`, systemd `TCM_BIND_HOST`/
`TCM_BIND_PORT`), and the test scans both files for wildcard bind tokens. The detector is proven
against synthetic wildcard fixtures, so a regression that rewrites either file to a wildcard bind
fails the suite. No source-code change to A3 was needed (its listener binds only where the caller
asks; the caller is now the systemd unit contract).

## TDD evidence

### RED — test written before any `pi/deploy/` file existed

Command: `npm test -w @teslacanmodder/pi -- test/deploy.test.ts`

```text
FAIL test/deploy.test.ts
  Pi deployment portal
    x defines two distinct loopback upstreams
    x routes / to Sentry USB and /tcm to our client upstream without a shared static root
    x proxies HTTP and WebSocket headers on /tcm
    x pins the systemd service to loopback port 8090
    v rejects wildcard binds in deployed config and detects them in bad fixtures
    x documents rendering, UNVERIFIED Sentry port, and systemctl checks
Test Suites: 1 failed, 1 total
Tests:       5 failed, 1 passed, 6 total
```

Missing files read as empty strings, so RED was assertion failures, not ENOENT.

Note: two intermediate RED runs after writing the files exposed a test-harness bug (the upstream
regex stopped at the `}` inside `${SENTRY_USB_PORT}` and assumed column-zero braces). Fixed the
regex to accept indented upstreams and newline-delimited closing braces, and moved the
UNVERIFIED comment inside the upstream block. No production config was weakened to make the test
pass.

### GREEN — after writing nginx.conf, tcm-can.service, README.md

```text
PASS test/deploy.test.ts
  √ defines two distinct loopback upstreams (2 ms)
  √ routes / to Sentry USB and /tcm to our client upstream without a shared static root
  √ proxies HTTP and WebSocket headers on /tcm
  √ pins the systemd service to loopback port 8090
  √ rejects wildcard binds in deployed config and detects them in bad fixtures
  √ documents rendering, UNVERIFIED Sentry port, and systemctl checks
Test Suites: 1 passed, 1 total
Tests:       6 passed, 6 total
```

## Verification

| Command                                                         | Result                                                                                                       |
| --------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------ |
| `npm test -w @teslacanmodder/pi -- test/deploy.test.ts` (RED)   | FAIL, 5/6 assertions failed                                                                                  |
| `npm test -w @teslacanmodder/pi -- test/deploy.test.ts` (GREEN) | PASS, 6/6                                                                                                    |
| `npm test -w @teslacanmodder/pi`                                | PASS, 35/35 in 5 suites (pre-commit and post-commit)                                                         |
| `npm run typecheck:pi`                                          | clean, no diagnostics                                                                                        |
| `npm run lint:all`                                              | ESLint clean; Prettier all files clean; markdownlint 0 issues in 45 files (pre- and post-commit)             |
| `git diff --cached --name-only` (pre-commit)                    | exactly `pi/deploy/README.md`, `pi/deploy/nginx.conf`, `pi/deploy/tcm-can.service`, `pi/test/deploy.test.ts` |
| `git status --short -- pi` (post-commit)                        | empty — no collateral Pi changes                                                                             |
| `git push origin spike/raspberry-pi-portability`                | `5e01484..c77e6b1`, no force                                                                                 |

`npm run test:all` not run: prior A2/A3 reports record the pre-existing Windows hang at
`test:firmware` (`pio.ps1`). No firmware files touched; full-suite status remains UNVERIFIED for
that environment reason. The brief's exact gates (Pi tests + `lint:all`) are green.

## Files

| File                        | Change                                                                                                       |
| --------------------------- | ------------------------------------------------------------------------------------------------------------ |
| `pi/deploy/nginx.conf`      | New: full nginx template, two loopback upstreams, `/`→Sentry, `/tcm`→A3, WS upgrade headers, no shared root. |
| `pi/deploy/tcm-can.service` | New: hardened systemd unit with loopback bind env + token env file.                                          |
| `pi/deploy/README.md`       | New: runbook, UNVERIFIED register, install/verify/safety steps.                                              |
| `pi/test/deploy.test.ts`    | New: 6 structural deployment tests including wildcard-bind rejection.                                        |

No Sentry USB files created, copied, or modified. Report file intentionally NOT committed —
`.opencode/sdd/**` is shared, untracked scratch for other task briefs/reports.

## Self-review

- Test-first ordering verified with recorded RED output before any deploy file existed.
- Staging used explicit paths only; staged list verified before commit; commit stat = exactly
  4 files / 309 insertions. ~200 pre-existing unstaged files untouched. Husky lint-staged
  reformatted only the staged test + README; post-commit tests and lint re-verified green.
- Upstreams are genuinely distinct: Sentry placeholder port vs fixed 8090, both loopback; route
  blocks assert non-crossing `proxy_pass` targets.
- Wildcard detector rejects `0.0.0.0`, `[::]`, bare `::`, and port-glob tokens; verified it sees
  none in the shipped config. It is a text check, not an nginx parser (no nginx binary here).
- Redirect `/tcm` → `/tcm/` avoids a prefix-less request landing in Sentry's `/` location.
- `proxy_pass http://tcm_can/;` strips the prefix, matching A3's exact URL routing.
- No `console.*`, no `any`, strict TS clean, ESM/tabs/double quotes/trailing commas/LF per repo
  config. Markdownlint clean.
- No long-running server or watcher started; no Sentry storage or CAN bus touched.
- Pre-existing lint-staged backup stash refs left intact; none dropped.

## Concerns / notes for controller

1. **Sentry USB local port and asset paths remain UNVERIFIED.** Maintainer input required. The
   template deliberately ships no guessed numeric default; deployment is blocked until
   `SENTRY_USB_PORT` is set to a real loopback port. Ask the user directly if possible.
2. **systemd `ExecStart` points at `pi/dist/main.js`, which does not exist yet.** A1–A4 produce
   libraries only; no runtime entrypoint composes A2 reader + A3 bridge + A4 recorder and binds
   `TCM_BIND_HOST`/`TCM_BIND_PORT`. The unit is a deployment contract and is documented as not
   runnable until that entrypoint lands (likely a follow-up task). This is the main functional
   gap for real bring-up.
3. **Client asset base path UNVERIFIED.** If the Expo web bundle emits absolute `/assets/...`
   URLs, those requests hit the Sentry upstream. The client must be built with `/tcm/` as its
   public path; confirm on hardware.
4. **Structural test only.** `nginx -t` / `systemd-analyze verify` cannot run in this Windows
   environment. Config syntax is human-reviewed but not machine-parsed here.
5. **WebSocket headers are future-proofing.** A3's listener is HTTP-only today; the headers are
   harmless and satisfy the brief's WebSocket requirement.
6. **No token value shipped.** README generates one; A3 fails closed on empty token. The env file
   must be root:tcmcan 0640.

---

## Fix round 1/5 — valid sites-enabled snippet

Finding (Important): `nginx.conf` had `worker_processes`, `events{}`, and `http{}` at top level
while the runbook installed it under `sites-available/` and symlinked into `sites-enabled/`.
On Debian/Raspberry Pi OS those includes sit inside the distro `http{}` block, so `nginx -t`
would fail with duplicate/not-allowed directives. Controller ruling: strip the wrapper, keep
`map`/`upstream`/`server`, keep the sites-available/sites-enabled install path, add a structural
assertion tying path to shape; also reject `alias` alongside `root` and stop the wildcard
detector from flagging IPv6 loopback.

### What changed

| File                     | Change                                                                                                                                                                                                                                                                                                         |
| ------------------------ | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `pi/deploy/nginx.conf`   | Removed `worker_processes auto;`, `events{}`, `http{}` and the extra closing brace; de-indented `map`/`upstream`/`server` to snippet top level. Header now states it is a sites-enabled snippet and why no wrapper exists. Routes/upstreams/headers unchanged.                                                 |
| `pi/deploy/README.md`    | Step 4 now says "Render and install the portal snippet" and explains that the distro main nginx config supplies the wrapper; explicitly says do not overwrite it. sites-available/sites-enabled commands kept.                                                                                                 |
| `pi/test/deploy.test.ts` | New test `is a sites-enabled snippet and documents the matching install path` (no wrapper directives, top-level `map`/`upstream`/`server`, balanced braces, README uses both sites-available and sites-enabled and does not redirect into the main nginx config). Changed shared-root assertion to `/\b(?:root | alias)\s+/`. Wildcard detector now uses `::(?!1\b)`so`::1`and`[::1]:8090`are allowed; positive fixtures still include`0.0.0.0`, `[::]`, and port-glob binds. Extended `rejects wildcard binds in deployed config and detects them in bad fixtures` with two IPv6-loopback assertions. |

Test names after fix (7 total):

1. `is a sites-enabled snippet and documents the matching install path`
2. `defines two distinct loopback upstreams`
3. `routes / to Sentry USB and /tcm to our client upstream without a shared static root`
4. `proxies HTTP and WebSocket headers on /tcm`
5. `pins the systemd service to loopback port 8090`
6. `rejects wildcard binds in deployed config and detects them in bad fixtures`
7. `documents rendering, UNVERIFIED Sentry port, and systemctl checks`

### RED — new test added before config edits

Command: `npm test -w @teslacanmodder/pi -- test/deploy.test.ts`

```text
FAIL test/deploy.test.ts
  Pi deployment portal
    x is a sites-enabled snippet and documents the matching install path (2 ms)
    v defines two distinct loopback upstreams (1 ms)
    v routes / to Sentry USB and /tcm to our client upstream without a shared static root (1 ms)
    v proxies HTTP and WebSocket headers on /tcm
    v pins the systemd service to loopback port 8090
    v rejects wildcard binds in deployed config and detects them in bad fixtures
    v documents rendering, UNVERIFIED Sentry port, and systemctl checks
Test Suites: 1 failed, 1 total
Tests:       1 failed, 6 passed, 7 total

  ● Pi deployment portal › is a sites-enabled snippet and documents the matching install path
    expect(received).not.toMatch(expected)
    Expected pattern: not /^\s*worker_processes\b/m
    Received string: "# TeslaCANModder Pi portal — nginx template. ... worker_processes auto; ..."
```

### GREEN — after stripping the wrapper and updating the runbook

Same command, exit 0:

```text
PASS test/deploy.test.ts
  Pi deployment portal
    v is a sites-enabled snippet and documents the matching install path (2 ms)
    v defines two distinct loopback upstreams
    v routes / to Sentry USB and /tcm to our client upstream without a shared static root (1 ms)
    v proxies HTTP and WebSocket headers on /tcm
    v pins the systemd service to loopback port 8090
    v rejects wildcard binds in deployed config and detects them in bad fixtures
    v documents rendering, UNVERIFIED Sentry port, and systemctl checks
Test Suites: 1 passed, 1 total
Tests:       7 passed, 7 total
```

### Re-verification

| Command                                                         | Output                                                                                          |
| --------------------------------------------------------------- | ----------------------------------------------------------------------------------------------- |
| `npm test -w @teslacanmodder/pi -- test/deploy.test.ts` (RED)   | 1 failed / 7 — wrapper still present                                                            |
| `npm test -w @teslacanmodder/pi -- test/deploy.test.ts` (GREEN) | 7 passed / 7                                                                                    |
| `npm test -w @teslacanmodder/pi`                                | `Test Suites: 5 passed, 5 total`; `Tests: 36 passed, 36 total`                                  |
| `npm run lint:all`                                              | ESLint clean; `All matched files use Prettier code style!`; markdownlint `0 issues in 45 files` |

Commit for this round: `fix(pi): make nginx portal config a valid sites-enabled snippet`.
Staged explicitly: `pi/deploy/nginx.conf`, `pi/deploy/README.md`, `pi/test/deploy.test.ts`.
No Sentry files touched; main branch untouched; no force-push.

Remaining concerns unchanged from round 0 (Sentry port UNVERIFIED, missing runtime entrypoint,
client base path UNVERIFIED, no nginx binary in this environment to run `nginx -t`). The wrapper
bug is now covered by a test that fails if a future edit reintroduces a top-level directive.
