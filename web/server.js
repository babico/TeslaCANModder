/**
 * TeslaCANModder — Node.js production server
 *
 * Usage:
 *   npm run build   # build React app into dist/
 *   node server.js  # serve on http://localhost:5173
 *
 * Environment variables:
 *   PORT                  — TCP port (default: 5173)
 *   VITE_ALLOWED_HOSTS    — comma-separated extra allowed Host headers
 */

import http from 'node:http';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const currentDir = path.dirname(fileURLToPath(import.meta.url));
const distRoot = path.resolve(currentDir, 'dist');
const hardwareBuildRoot = path.resolve(currentDir, '../hardware/.pio/build');
const port = Number(process.env.PORT) || 5173;

const defaultAllowedHosts = new Set(['localhost', '127.0.0.1', '::1', `localhost:${port}`]);
const extraHosts = (process.env.VITE_ALLOWED_HOSTS || '')
  .split(',')
  .map((h) => h.trim())
  .filter(Boolean);
const allowedHosts = new Set([...defaultAllowedHosts, ...extraHosts]);

// ── MIME type map ────────────────────────────────────────────────────────────

const MIME = {
  '.html': 'text/html; charset=utf-8',
  '.js':   'application/javascript; charset=utf-8',
  '.mjs':  'application/javascript; charset=utf-8',
  '.css':  'text/css; charset=utf-8',
  '.json': 'application/json; charset=utf-8',
  '.svg':  'image/svg+xml',
  '.png':  'image/png',
  '.jpg':  'image/jpeg',
  '.jpeg': 'image/jpeg',
  '.ico':  'image/x-icon',
  '.woff': 'font/woff',
  '.woff2':'font/woff2',
  '.ttf':  'font/ttf',
  '.txt':  'text/plain; charset=utf-8',
  '.hex':  'text/plain; charset=utf-8',
  '.map':  'application/json; charset=utf-8',
};

function mimeFor(filePath) {
  return MIME[path.extname(filePath).toLowerCase()] || 'application/octet-stream';
}

// ── Heartbeat payload ────────────────────────────────────────────────────────

const heartbeatPayload = JSON.stringify({
  ok: true,
  service: 'TeslaCANModder-web',
  mode: 'production',
});

// ── Route handlers ───────────────────────────────────────────────────────────

function serveHeartbeat(res) {
  res.writeHead(200, {
    'Content-Type': 'application/json; charset=utf-8',
    'Cache-Control': 'no-store',
  });
  res.end(heartbeatPayload);
}

/**
 * Serve a .hex file from the hardware PlatformIO build tree.
 * Only exposes *.hex files and guards against path traversal.
 */
function serveFirmwareBuild(requestPath, res) {
  const normalizedPath = path.normalize(requestPath).replace(/^([/\\])+/, '');
  const filePath = path.resolve(hardwareBuildRoot, normalizedPath);

  if (!filePath.startsWith(hardwareBuildRoot) || !filePath.endsWith('.hex')) {
    res.writeHead(404, { 'Content-Type': 'text/plain' });
    res.end('Not found');
    return;
  }

  if (!fs.existsSync(filePath)) {
    res.writeHead(404, { 'Content-Type': 'text/plain' });
    res.end('Firmware build not found');
    return;
  }

  res.writeHead(200, {
    'Content-Type': 'text/plain; charset=utf-8',
    'Cache-Control': 'no-store',
  });
  res.end(fs.readFileSync(filePath));
}

/**
 * Serve a static file from dist/.
 * Falls back to dist/index.html for SPA routing.
 */
function serveStatic(urlPath, res) {
  // Strip query string
  const cleanPath = urlPath.split('?')[0];
  const safePath = path.normalize(cleanPath).replace(/^([/\\])+/, '');
  let filePath = path.resolve(distRoot, safePath);

  // Guard against path traversal out of dist/
  if (!filePath.startsWith(distRoot)) {
    res.writeHead(403, { 'Content-Type': 'text/plain' });
    res.end('Forbidden');
    return;
  }

  // Directory → try index.html inside it
  if (fs.existsSync(filePath) && fs.statSync(filePath).isDirectory()) {
    filePath = path.join(filePath, 'index.html');
  }

  if (!fs.existsSync(filePath)) {
    // SPA fallback
    filePath = path.join(distRoot, 'index.html');
  }

  if (!fs.existsSync(filePath)) {
    res.writeHead(404, { 'Content-Type': 'text/plain' });
    res.end('Not found — run "npm run build" first');
    return;
  }

  const content = fs.readFileSync(filePath);
  res.writeHead(200, {
    'Content-Type': mimeFor(filePath),
    'Cache-Control': filePath.endsWith('index.html') ? 'no-store' : 'public, max-age=31536000, immutable',
  });
  res.end(content);
}

// ── Request handler ──────────────────────────────────────────────────────────

function onRequest(req, res) {
  // Host header check (mirrors Vite allowedHosts behaviour)
  const host = (req.headers.host || '').split(':')[0].toLowerCase();
  const hostWithPort = (req.headers.host || '').toLowerCase();
  if (!allowedHosts.has(host) && !allowedHosts.has(hostWithPort)) {
    res.writeHead(403, { 'Content-Type': 'text/plain' });
    res.end(`Host "${req.headers.host}" not allowed. Set VITE_ALLOWED_HOSTS env var to permit it.`);
    return;
  }

  // CORS for localhost tooling
  res.setHeader('Access-Control-Allow-Origin', '*');

  const url = req.url || '/';

  if (url === '/heartbeat.json') {
    serveHeartbeat(res);
    return;
  }

  if (url.startsWith('/firmware-builds/')) {
    const buildPath = url.slice('/firmware-builds'.length);
    serveFirmwareBuild(buildPath, res);
    return;
  }

  serveStatic(url, res);
}

// ── Start ────────────────────────────────────────────────────────────────────

const server = http.createServer(onRequest);

server.listen(port, () => {
  console.log(`TeslaCANModder web server running at http://localhost:${port}`);
  console.log(`  Static files : ${distRoot}`);
  console.log(`  Firmware     : ${hardwareBuildRoot}`);
  console.log(`  Heartbeat    : http://localhost:${port}/heartbeat.json`);
});

server.on('error', (err) => {
  if (err.code === 'EADDRINUSE') {
    console.error(`Port ${port} is already in use. Set PORT env var to use a different port.`);
  } else {
    console.error('Server error:', err);
  }
  process.exit(1);
});
