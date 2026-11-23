# Task B2 — Wiring guide + listen-only bring-up — report

Branch: `spike/raspberry-pi-portability` (never main, no force-push).
Worktree: `C:\Users\boran\Masaüstü\KL\Kodlama\TeslaCANModder`.

## Status

DONE. Guide committed first, generated bundle regenerated in a separate second
commit, validations run once before the final commit, pushed, self-reviewed.

## VCC conflict check (asked-for precondition)

**No conflict found — no question asked.** Both cited guides put MCP2515 `VCC`
at 3.3 V:

- 38-3D: `VCC → 3.3V (Pin 1)`.
- tolgakarakurt: module `Vcc 3.3V`, with a separate TJA1050 `Vdd 5V` added by
  trace-cut/jumper because TJA1050 is specified 4.75–5.25 V.

Resolution matches the brief's expectation: MCP2515 at 3.3 V; TJA1050 either
fed 5 V via the documented mod or replaced by a 3.3 V-rated transceiver. The
guides only disagree on crystal value (16 MHz vs 8 MHz), handled via a
`<module crystal Hz>` placeholder.

## Files changed (only these two)

1. `docs/guides/pi-can-bringup.md` (new, 285 lines) — canonical runbook.
2. `client/src/docs/_generated-docs.ts` (286 inserted lines, 0 deletions) —
   regenerated bundle containing exactly one new entry,
   `guides/pi-can-bringup.md`.

No other file staged or committed. ~200 pre-existing unrelated unstaged files
left untouched; staged lists verified before each commit; working tree has no
staged files at end.

## Commits

| Commit    | Subject                                                          | Files |
| --------- | ---------------------------------------------------------------- | ----- |
| `fc9b528` | `docs(pi): add listen-only MCP2515 bring-up guide`               | 1     |
| `64e4a16` | `chore(client): regenerate docs bundle for pi-can-bringup guide` | 1     |

Push result: `67b7ef1..64e4a16 spike/raspberry-pi-portability -> spike/raspberry-pi-portability`.

## Commands run

- Read brief; read `docs/guides/hardware-setup.md`, `docs/reference/can-protocol.md`.
- Fetched both cited guides (read-only; no code copied).
- Inspected `client/scripts/generate-docs-catalog.mjs`, `package.json`,
  `.markdownlint-cli2.jsonc`, `.markdownlint.json`, `.prettierignore`,
  `eslint.config.mjs`, `client/src/docs/catalog.ts`.
- `npx prettier --write docs/guides/pi-can-bringup.md` (formed table; applied typo
  fix for `tolgakarakurt`).
- `npx --yes markdownlint-cli2 "docs/guides/pi-can-bringup.md"` → 0 issues.
- `npx prettier --check docs/guides/pi-can-bringup.md` → clean.
- `git add docs/guides/pi-can-bringup.md` → staged list verified → `git commit`.
- `node scripts/generate-docs-catalog.mjs ../docs` (run from `client/`) →
  "Generated ... with 145 docs".
- `npm run lint:all` → ESLint clean; Prettier `All matched files use Prettier code style!`;
  markdownlint 46 files, 0 issues.
- `npm run validate:serial-contract` → "Validated unified io.schema.json contract."
- `git add client/src/docs/_generated-docs.ts` → staged list verified → `git commit`.
- `git push`.
- Self-review via `git log`, `git status`, `git show --stat`, and full doc re-read.

Both `npm run lint:all` and `npm run validate:serial-contract` were run exactly
once, after bundle generation and before the final (second) commit, as required.

## Doc content checklist (all present)

- [x] UNVERIFIED banner: no hardware available; runbook, not success claim.
- [x] Pin table: VCC 3.3 V to MCP2515; TJA1050 5 V mod **or** 3.3 V transceiver;
      common GND; SCK GPIO11, MOSI GPIO10, MISO GPIO9, CS GPIO8 (CE0) / GPIO7
      (CE1) / free GPIO for bus 3; INT GPIO25 + distinct INT for extra buses;
      level shifters where any line is 5 V.
- [x] TJA1050 5 V trap section with both approaches and 4.75–5.25 V spec.
- [x] Device-tree: `dtparam=spi=on`,
      `dtoverlay=mcp2515-can0,oscillator=<module crystal Hz>,interrupt=25`,
      `dtoverlay=spi-bcm2835`, `spi-dma` only on old kernels; extra buses use
      distinct CS/INT; `/sys/bus/spi/devices/spi0.0/net/can0` check;
      `ip link set can0 up type can bitrate 500000` plus `listen-only on` variant.
- [x] Acceptance: `ip -details link show can0` 500 kbps; `candump can0` expected
      X179 frames per variant (git-derived from `can-protocol.md`); 120 Ω
      termination guidance; hardware provenance record template (Pi model,
      crystal, transceiver variant, wiring photo, etc.).
- [x] Explicit **DO NOT ENABLE TX** section gated on Task B3 — no `cansend`,
      no write commands, no ACK-capable "just to try" mode, stop-and-escalate.
- [x] Both URLs cited; no code copied; 403 forum thread explicitly unused.
- [x] X179 mapping git-derived from `hardware-setup.md`: bus 0 13/14, bus 1 9/10,
      bus 2 2/3.

## Self-review

- Each commit contains exactly one file; `git show --stat` confirms.
- Bundle diff is 286 insertions / 0 deletions — only the new doc entry, no
  accidental regeneration drift from other docs.
- The generated file is prettier-ignored and eslint-ignored; hook tasks did not
  alter its content (commit stat unchanged from pre-commit staged stat).
- Internal doc links (`./hardware-setup.md`, `#do-not-enable-tx`) resolve to an
  existing file / existing heading.
- ALL hardware specifics consistently marked UNVERIFIED; no claim of tested
  success; no TX procedure anywhere.
- Pushed to the feature branch; branch tracks origin; no force.

## Concerns

1. All pin/overlay/frame content is UNVERIFIED by design (no hardware here).
   Controller should treat this as a runbook requiring real-Pi execution before
   any B3 decision. In particular, extra-bus `cs=`/`interrupt=` overlay parameter
   names and `listen-only on` driver support are kernel-dependent and must be
   confirmed on target.
2. Listen-only framing: the baseline command from the guides does not include
   `listen-only on`; the doc prefers the flag but records that even with it the
   controller might still ACK depending on driver (explicit UNVERIFIED caveat).
   If the controller cannot guarantee no-ACK, B3 must decide whether that is
   acceptable on the vehicle bus.
3. The TJA1050 mod is irreversible (trace cut); doc warns to use a scrap module
   first and offers the 3.3 V-transceiver alternative.
4. `candump` expected IDs are the "test expected frames" list from
   `can-protocol.md`; they are a guide, not a guaranteed complete set, and a
   quiet bus is called out as a non-failure condition.
5. Working tree retains ~200 pre-existing unrelated modifications (not touched).
6. No new unit test applies — this task is documentation + generated bundle
   only; validation was lint/format/markdownlint + serial-contract check.

## Fix round 1/5 — review findings addressed

Branch/state at start: `spike/raspberry-pi-portability`, HEAD `64e4a16`, clean
staged area, ~200 pre-existing unrelated unstaged files left untouched.

### Changes (only `docs/guides/pi-can-bringup.md` edited)

1. **Important — incorrect overlay/CS model (fixed).** Replaced the generic
   `dtoverlay=mcp2515-canN` substitution and the "free GPIO (e.g. 16) as CS"
   proposal. The doc now states stock RPi overlays are `mcp2515-can0/1/2`,
   hard-bound to SPI0 CE0/CE1/CE2, and that arbitrary-GPIO CS needs a
   kernel-dependent `cs=` parameter or custom DT fragment. Concrete per-bus
   lines are given: bus 0 = stock `mcp2515-can0` on CE0 with no `cs=`; bus 1 =
   stock `mcp2515-can1` on CE1 with `interrupt=24`; bus 2 = commented out and
   explicitly flagged **DO NOT USE** until overlay presence and CS/INT are
   verified on the target kernel. An explanatory table replaces the old CS/INT
   proposal table. RX-only/listen-only and UNVERIFIED labels preserved; no
   working bus2 overlay invented.
2. **Minor — VCC input range (fixed).** Added a VCC-input-range note: the repo's
   `hardware-setup.md` feeds module VCC at 5 V and many boards have a
   5 V→3.3 V regulator that will not start from 3.3 V VCC; verify the module's
   regulator/silkscreen and confirm 3.3 V VCC operation before applying the
   Pi-safe wiring. Marked UNVERIFIED.
3. **Minor — `spi-dma` wording (fixed).** Tightened both the config comment and
   the explanatory bullet: tolgakarakurt ran kernel 4.19.42-v7+, where
   `spi-dma` was a workaround; do not assume it is required, accepted, or
   available on a current kernel — omit unless that kernel era is confirmed.

### Commands run and output

- `npx prettier --write docs/guides/pi-can-bringup.md` → `(unchanged)`.
- `npx --yes markdownlint-cli2 "docs/guides/pi-can-bringup.md"` → `Summary: 0 issues in 0 files`.
- `git add docs/guides/pi-can-bringup.md`; staged list verified = single file
  (`39 insertions(+), 19 deletions(-)`).
- `git commit -m "docs(pi): correct MCP2515 overlay CS bindings and VCC caveats"`
  → `6aa5056` (1 file changed).
- `node scripts/generate-docs-catalog.mjs ../docs` (from `client/`) →
  `Generated ... _generated-docs.ts with 145 docs`.
- `git diff --numstat -- client/src/docs/_generated-docs.ts` → `39 19`,
  matching only the embedded `guides/pi-can-bringup.md` entry (no drift).
- `npm run lint:all` → ESLint clean; Prettier `All matched files use Prettier code style!`;
  markdownlint 46 files, `0 issues in 0 files`.
- `npm run validate:serial-contract` → `Validated unified io.schema.json contract.`
- `git add client/src/docs/_generated-docs.ts`; staged list verified = single file
  (`39 insertions(+), 19 deletions(-)`).
- `git commit -m "chore(client): regenerate docs bundle for overlay CS fix"`
  → `d581e67` (1 file changed).
- `git push` → `64e4a16..d581e67 spike/raspberry-pi-portability -> spike/raspberry-pi-portability`.

Both validation commands were run once, after bundle regeneration and before the
final commit, as required. No subagents dispatched.

### Self-review (fix round)

- Each commit contains exactly one file; staged lists verified before each.
- Bundle diff matches the guide diff exactly (39/19), no unrelated doc drift.
- All three findings closed; no new claims of tested hardware success; bus 2
  remains explicitly unverified and disabled; TX ban section untouched.
- Pushed to the feature branch; no force-push; nothing staged at end.
