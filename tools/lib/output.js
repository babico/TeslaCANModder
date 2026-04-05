/** ANSI colour helpers and formatted output for the CLI. */

export function createColors(noColor) {
  if (noColor) return { green: '', red: '', yellow: '', cyan: '', dim: '', bold: '', reset: '' };
  return {
    green:  '\x1b[32m',
    red:    '\x1b[31m',
    yellow: '\x1b[33m',
    cyan:   '\x1b[36m',
    dim:    '\x1b[2m',
    bold:   '\x1b[1m',
    reset:  '\x1b[0m',
  };
}

export function createOutput(C) {
  let passed = 0, failed = 0, warned = 0;

  return {
    pass(label, detail = '') {
      passed++;
      console.log(`  ${C.green}✓${C.reset} ${label}${detail ? `  ${C.dim}${detail}${C.reset}` : ''}`);
    },
    fail(label, reason = '') {
      failed++;
      console.log(`  ${C.red}✗${C.reset} ${label}${reason ? `  ${C.yellow}${reason}${C.reset}` : ''}`);
    },
    warn(label, reason = '') {
      warned++;
      console.log(`  ${C.yellow}⚠${C.reset} ${label}${reason ? `  ${C.dim}${reason}${C.reset}` : ''}`);
    },
    info(msg) {
      console.log(`    ${C.dim}${msg}${C.reset}`);
    },
    section(title) {
      console.log(`\n${C.bold}${title}${C.reset}`);
    },
    observation(tag, msg) {
      console.log(`${C.dim}[${ts()}]${C.reset} ${C.cyan}[${tag}]${C.reset} ${msg}`);
    },
    counts() { return { passed, failed, warned }; },
    reset() { passed = 0; failed = 0; warned = 0; },
  };
}

export function ts() {
  return new Date().toTimeString().slice(0, 12);
}
