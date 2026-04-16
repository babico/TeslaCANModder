import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';

const currentDir = path.dirname(fileURLToPath(import.meta.url));
const rootDocsDir = path.resolve(currentDir, '../docs');
const hardwareBuildRoot = path.resolve(currentDir, '../hardware/.pio/build');
const defaultAllowedHosts = ['localhost', '127.0.0.1', '::1'];
const allowedHosts = [
  ...new Set([
    ...defaultAllowedHosts,
    ...(globalThis.process?.env?.VITE_ALLOWED_HOSTS || '')
      .split(',')
      .map((host) => host.trim())
      .filter(Boolean),
  ]),
];
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

function serveRootDocs() {
  return {
    name: 'serve-root-docs',
    configureServer(server) {
      server.middlewares.use('/docs', (req, res, next) => {
        const requestPath = decodeURIComponent((req.url || '').split('?')[0]);
        const normalizedPath = path.normalize(requestPath).replace(/^([/\\])+/, '');
        const filePath = path.resolve(rootDocsDir, normalizedPath);

        if (!filePath.startsWith(rootDocsDir) || !filePath.endsWith('.md')) {
          next();
          return;
        }

        if (!fs.existsSync(filePath)) {
          next();
          return;
        }

        res.setHeader('Content-Type', 'text/markdown; charset=utf-8');
        res.setHeader('Cache-Control', 'no-store');
        res.end(fs.readFileSync(filePath, 'utf-8'));
      });
    },
    closeBundle() {
      const outDir = path.resolve(currentDir, 'dist/docs');
      fs.mkdirSync(outDir, { recursive: true });
      for (const file of fs.readdirSync(rootDocsDir)) {
        if (file.endsWith('.md')) {
          fs.copyFileSync(path.join(rootDocsDir, file), path.join(outDir, file));
        }
      }
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
  test: {
    environment: 'jsdom',
    globals: true,
    setupFiles: ['./test/setup.ts'],
    css: true,
    include: ['test/**/*.test.ts', 'test/**/*.test.tsx'],
  },
  server: {
    allowedHosts,
    proxy: {
      '/api/build': {
        target: 'http://localhost:8100',
        changeOrigin: true,
        rewrite: (p) => p.replace(/^\/api\/build/, '/build'),
      },
    },
  },
  plugins: [react(), serveHeartbeat(), serveRootDocs(), serveHardwareBuilds()],
});
