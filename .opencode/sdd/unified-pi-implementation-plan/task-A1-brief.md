# Task A1 — `pi/` workspace scaffold

Where: first task of the unified Pi plan. Creates the new Node workspace and HTTP server skeleton.

## Requirement (verbatim from plan)

Files:

- Create: `pi/package.json`, `pi/tsconfig.json`, `pi/src/index.ts`, `pi/test/smoke.test.ts`
- Modify: root `package.json` (workspace + scripts `test:pi`, `typecheck:pi`)

Interfaces:

- Produces: `@teslacanmodder/pi` workspace; `createPiServer()` signature used by A3; `CAN_TRANSPORT` env switch (`vcan|socketcan`).

Steps:

1. Write the failing smoke test asserting the workspace resolves `@teslacanmodder/protocol` and `createPiServer()` exists.
2. Run `npm test -w @teslacanmodder/pi` — expect FAIL (module missing).
3. Add workspace, scripts and minimal `createPiServer()` returning an HTTP server with `/health`.
4. Run test + `npm run typecheck:pi` — expect PASS.
5. Commit — `feat(pi): add Pi service workspace scaffold`.

## Global constraints

- No Sentry code copied. No CAN TX code here.
- Stage ONLY files you change; ~200 unrelated unstaged files exist. Verify `git diff --cached --name-only` before commit.
- Commit/push on feature branch `spike/raspberry-pi-portability`; never main, never force-push.
- Follow existing monorepo conventions (ESM, `"type": "module"`, Jest config style from `tools/` or `packages/protocol/`, Prettier).
