#!/usr/bin/env node
/**
 * Cross-platform PlatformIO launcher for npm scripts.
 *
 * Windows: `cmd.exe` associates `.ps1` with Notepad, so `.\pio.ps1 test` from
 * an npm script hangs by opening the wrapper in a GUI editor. Spawning
 * `powershell -File` explicitly keeps the wrapper in a real shell; the wrapper
 * itself is still required because it sets up an ASCII `subst` workspace for
 * non-ASCII repo paths and points PlatformIO at the project-local core dir.
 *
 * Everywhere else: call `pio` directly from `firmware/`.
 *
 * Exit status is forwarded so `npm run test:firmware` / `test:all` fail or pass
 * with the underlying PlatformIO result. This launcher never kills processes.
 */

import { spawn } from "node:child_process";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const repoRoot = resolve(dirname(fileURLToPath(import.meta.url)), "..");
const firmwareDir = resolve(repoRoot, "firmware");
const args = process.argv.slice(2);

const win32 = process.platform === "win32";
const command = win32 ? "powershell" : "pio";
const commandArgs = win32
  ? ["-NoProfile", "-ExecutionPolicy", "Bypass", "-File", resolve(firmwareDir, "pio.ps1"), ...args]
  : args;

const child = spawn(command, commandArgs, {
  cwd: firmwareDir,
  stdio: "inherit",
  windowsHide: true,
});

child.on("error", (error) => {
  console.error(`run-pio: failed to start ${command}:`, error);
  process.exitCode = 1;
});

child.on("close", (code, signal) => {
  process.exitCode = signal ? 1 : (code ?? 1);
});
