import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';

const currentDir = path.dirname(fileURLToPath(import.meta.url));
const hardwareBuildRoot = path.resolve(currentDir, '../hardware/.pio/build');
const heartbeatPayload = JSON.stringify({
  ok: true,
  service: 'TeslaCANModder-web',
  mode: 'dev',
});

function serveHeartbeat() {
  return {
    name: 'serve-heartbeat',
    configureServer(server) {
      server.middlewares.use('/heartbeat.json', (_req, res) => {
        res.statusCode = 200;
        res.setHeader('Content-Type', 'application/json; charset=utf-8');
        res.setHeader('Cache-Control', 'no-store');
        res.end(heartbeatPayload);
      });
    },
  };
}

function serveHardwareBuilds() {
  return {
    name: 'serve-hardware-builds',
    configureServer(server) {
      server.middlewares.use('/firmware-builds', (req, res, next) => {
        const requestPath = decodeURIComponent((req.url || '').split('?')[0]);
        const normalizedPath = path.normalize(requestPath).replace(/^([/\\])+/, '');
        const filePath = path.resolve(hardwareBuildRoot, normalizedPath);

        // Only expose generated HEX artifacts from the local PlatformIO build tree.
        if (!filePath.startsWith(hardwareBuildRoot) || !filePath.endsWith('.hex')) {
          next();
          return;
        }

        if (!fs.existsSync(filePath)) {
          next();
          return;
        }

        res.setHeader('Content-Type', 'text/plain; charset=utf-8');
        res.end(fs.readFileSync(filePath));
      });
    },
  };
}

export default defineConfig({
  plugins: [react(), serveHeartbeat(), serveHardwareBuilds()],
});
