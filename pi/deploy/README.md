# Pi portal deployment

One nginx entry point, two independent services:

- `/` proxies to the existing Sentry USB UI.
- `/tcm/` proxies to our client bundle and the A3 command bridge.

No Sentry USB code is copied or modified here. Sentry USB keeps its own systemd unit and its own
storage; this repo only ships the portal config and the `tcm-can` unit.

## Before you begin — UNVERIFIED inputs

- **Sentry USB local port is UNVERIFIED.** No port is documented in this repo. Ask the maintainer
  for the loopback port Sentry USB actually listens on, then set `SENTRY_USB_PORT`. Do not guess.
- **Sentry USB asset paths are UNVERIFIED.** If its UI uses absolute asset URLs, the `/` proxy
  preserves them; verify on the bench.
- Our client must be built with a `/tcm/` base path so its assets resolve under the prefix
  (UNVERIFIED until built on a real Pi). The A3 bridge routes `/health`, `/status`,
  `/api/status`, and `POST /api/command`; it does not yet serve static assets.
- **Runtime entrypoint is not implemented yet.** A1–A4 expose libraries only; the unit's
  `ExecStart` target `pi/dist/main.js` is a deployment contract, not a runnable binary. Do not
  enable the unit on hardware until that entrypoint exists and honours `TCM_BIND_HOST` /
  `TCM_BIND_PORT`.
- No Pi, CAN hardware, or bench capture was available: all runtime claims here are UNVERIFIED.

## Loopback enforcement

The token-bearing A3 bridge must never be reachable from the LAN:

- nginx upstreams pin `127.0.0.1` for both services; the Sentry port is injected at render time.
- `tcm-can.service` pins `TCM_BIND_HOST=127.0.0.1` and `TCM_BIND_PORT=8090`.
- `pi/test/deploy.test.ts` fails if either file contains a wildcard bind token, and proves the
  detector rejects bad fixtures.
- nginx itself listens on port 80 as the single public entry point. Use the host firewall to
  restrict access to that port; do not add a second public listener for our service.

## Install

Run these on the Pi as root, from a checkout at `/opt/teslacanmodder`.

1. Install prerequisites: `nginx`, Node.js 22+, and `gettext-base` (for `envsubst`).
2. Build the client with base path `/tcm/` and prepare the Pi workspace:

    ```bash
    npm ci
    npm run build -w @teslacanmodder/client
    ```

3. Create the service user and a root-owned token file (A3 fails closed when the token is empty):

    ```bash
    useradd --system --home-dir /nonexistent --shell /usr/sbin/nologin tcmcan
    install -d -o root -g root -m 0750 /etc/teslacanmodder
    umask 0027
    printf 'TCM_BRIDGE_TOKEN=%s\n' '<generate-a-long-random-token>' \
      > /etc/teslacanmodder/tcm-can.env
    chown root:tcmcan /etc/teslacanmodder/tcm-can.env
    chmod 0640 /etc/teslacanmodder/tcm-can.env
    ```

4. Render and install the portal config. Use the real Sentry USB loopback port:

    ```bash
    export SENTRY_USB_PORT='<UNVERIFIED: actual Sentry USB local port>'
    envsubst '${SENTRY_USB_PORT}' < pi/deploy/nginx.conf \
      > /etc/nginx/sites-available/tcm-portal
    ln -sf /etc/nginx/sites-available/tcm-portal /etc/nginx/sites-enabled/tcm-portal
    nginx -t
    systemctl reload nginx
    ```

5. Install and start our service:

    ```bash
    install -m 0644 pi/deploy/tcm-can.service /etc/systemd/system/tcm-can.service
    systemctl daemon-reload
    systemctl enable --now tcm-can
    ```

Sentry USB's own unit is managed by its installer — do not edit its files from this repo.

## Verification

```bash
systemctl status tcm-can
systemctl status <sentry-usb-unit>
systemctl is-active nginx
ss -ltnp | grep -E ':(80|8090)\b'   # 8090 must show 127.0.0.1 only
curl -fsS http://127.0.0.1:8090/health
curl -fsS http://127.0.0.1/tcm/health
curl -fsS -o /dev/null -w '%{http_code}\n' http://127.0.0.1/   # Sentry UI
```

Expected: `tcm-can` active, nginx active, `/health` returns `{"ok":true,...}`, `/` returns the
Sentry UI. A tokenless request to `/tcm/api/command` must return HTTP 401 from the A3 bridge.

## Safety

CAN TX stays disabled. This deployment only reaches the A3 read-only listener (`status`, `ping`);
no injection command is exposed. Enabling TX requires the B3 RT measurement gate and the CAN
review checklist — neither is satisfied here.
