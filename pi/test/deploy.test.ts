import { describe, expect, it } from "@jest/globals";
import { readFileSync } from "node:fs";
import { fileURLToPath } from "node:url";

const DEPLOY_DIR = new URL("../deploy/", import.meta.url);

/** Missing files read as empty text so RED fails on assertions, not on ENOENT. */
function readDeploy(name: string): string {
	try {
		return readFileSync(fileURLToPath(new URL(name, DEPLOY_DIR)), "utf8");
	} catch {
		return "";
	}
}

const nginx = readDeploy("nginx.conf");
const unit = readDeploy("tcm-can.service");
const readme = readDeploy("README.md");

/** Extract one location block body by its header; no nested braces in our config. */
function locationBody(source: string, header: RegExp): string {
	const start = source.search(header);
	if (start < 0) return "";
	const open = source.indexOf("{", start);
	const close = source.indexOf("}", open);
	return open >= 0 && close > open ? source.slice(open + 1, close) : "";
}

/** Any token that would expose a listening service beyond loopback. */
function wildcardBinds(text: string): string[] {
	return text.match(/(?:0\.0\.0\.0|\[::\]|::|\*\s*:\s*\d+)/g) ?? [];
}

describe("Pi deployment portal", () => {
	it("defines two distinct loopback upstreams", () => {
		// Closing brace sits on its own line; envsubst placeholders contain `}`.
		const blocks = [
			...nginx.matchAll(/(?:^|\n)[ \t]*upstream\s+(\w+)\s*\{([\s\S]*?)\n[ \t]*\}/g),
		];
		expect(blocks.map((block) => block[1])).toEqual(["sentry_usb", "tcm_can"]);

		const sentry = blocks[0]![2]!;
		const tcm = blocks[1]![2]!;
		expect(sentry).toMatch(/server\s+127\.0\.0\.1:\$\{SENTRY_USB_PORT\};/);
		expect(sentry).toMatch(/UNVERIFIED/);
		expect(tcm).toMatch(/server\s+127\.0\.0\.1:8090;/);
		expect(sentry.trim()).not.toBe(tcm.trim());
	});

	it("routes / to Sentry USB and /tcm to our client upstream without a shared static root", () => {
		const root = locationBody(nginx, /location\s+\/\s*\{/);
		const tcm = locationBody(nginx, /location\s+\/tcm\/\s*\{/);

		expect(root).toMatch(/proxy_pass\s+http:\/\/sentry_usb;/);
		expect(root).not.toMatch(/proxy_pass\s+http:\/\/tcm_can/);
		expect(tcm).toMatch(/proxy_pass\s+http:\/\/tcm_can\//);
		expect(tcm).not.toMatch(/proxy_pass\s+http:\/\/sentry_usb/);
		expect(nginx).toMatch(/location\s+=\s+\/tcm\s*\{[\s\S]*?return\s+301\s+\/tcm\/;/);
		expect(nginx).not.toMatch(/\broot\s+/);
	});

	it("proxies HTTP and WebSocket headers on /tcm", () => {
		const tcm = locationBody(nginx, /location\s+\/tcm\/\s*\{/);

		expect(nginx).toMatch(/map\s+\$http_upgrade\s+\$connection_upgrade\s*\{/);
		expect(tcm).toMatch(/proxy_http_version\s+1\.1;/);
		expect(tcm).toMatch(/proxy_set_header\s+Upgrade\s+\$http_upgrade;/);
		expect(tcm).toMatch(/proxy_set_header\s+Connection\s+\$connection_upgrade;/);
		expect(tcm).toMatch(/proxy_set_header\s+Host\s+\$host;/);
		expect(tcm).toMatch(/proxy_set_header\s+X-Forwarded-For\s+\$proxy_add_x_forwarded_for;/);
		expect(tcm).toMatch(/proxy_set_header\s+X-Forwarded-Proto\s+\$scheme;/);
	});

	it("pins the systemd service to loopback port 8090", () => {
		expect(unit).toMatch(/Environment\s*=\s*TCM_BIND_HOST=127\.0\.0\.1/m);
		expect(unit).toMatch(/Environment\s*=\s*TCM_BIND_PORT=8090/m);
		expect(unit).toMatch(/ExecStart=.*main\.js/m);
		expect(unit).toMatch(/EnvironmentFile=.*tcm-can\.env/m);
	});

	it("rejects wildcard binds in deployed config and detects them in bad fixtures", () => {
		expect(wildcardBinds(nginx)).toEqual([]);
		expect(wildcardBinds(unit)).toEqual([]);

		const bad = [
			"upstream tcm_can { server 0.0.0.0:8090; }",
			"ExecStart=/usr/bin/node main.js --host 0.0.0.0",
			"listen [::]:80;",
			"listen *:80;",
		];
		for (const fixture of bad) {
			expect(wildcardBinds(fixture).length).toBeGreaterThan(0);
		}
	});

	it("documents rendering, UNVERIFIED Sentry port, and systemctl checks", () => {
		expect(readme).toMatch(/SENTRY_USB_PORT/);
		expect(readme).toMatch(/envsubst/);
		expect(readme).toMatch(/UNVERIFIED/);
		expect(readme).toMatch(/systemctl status tcm-can/);
	});
});
