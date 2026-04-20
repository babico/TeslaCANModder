"""
PlatformIO Build Server — HTTP API for on-demand firmware builds.

Accepts POST /build with JSON body:
    { "env": "esp32_wifi", "chassis": 1, "vehicle": 1, "body": 0 }

Bus flags (0=off, 1=on):
    chassis — Chassis / Autopilot CAN (X179 pins 13-14), default 1
  vehicle — Vehicle Control CAN (X179 pins 9-10),  default 0
  body    — Body Control CAN (X179 pins 2-3),      default 0

Returns the built firmware binary (.hex or .bin) as application/octet-stream.
"""

import json
import os
import subprocess
import re
from http.server import HTTPServer, BaseHTTPRequestHandler
from pathlib import Path

PROJECT_DIR = Path(__file__).resolve().parent.parent
BUILD_DIR = PROJECT_DIR / ".pio" / "build"

# Valid environment names (must match platformio.ini)
VALID_ENVS = {
    "uno", "uno_bt",
    "esp32", "esp32_wifi", "esp32_ble", "esp32_wifi_ble",
}

ENV_RE = re.compile(r"^[a-z0-9_]+$")


def build_firmware(env, chassis=1, vehicle=0, body=0):
    """Run PlatformIO build and return (firmware_bytes, extension) or raise."""
    if env not in VALID_ENVS:
        raise ValueError(f"Unknown env: {env}")
    for name, val in [("chassis", chassis), ("vehicle", vehicle), ("body", body)]:
        if val not in (0, 1):
            raise ValueError(f"Invalid {name} value: {val}")

    bus_flags = f"-DBUS_CHASSIS_ACTIVE={chassis} -DBUS_VEHICLE_ACTIVE={vehicle} -DBUS_BODY_ACTIVE={body}"

    cmd = ["platformio", "run", "-e", env]

    extra_env = os.environ.copy()
    existing = extra_env.get("PLATFORMIO_BUILD_FLAGS", "")
    extra_env["PLATFORMIO_BUILD_FLAGS"] = f"{existing} {bus_flags}".strip()

    result = subprocess.run(
        cmd,
        cwd=str(PROJECT_DIR),
        capture_output=True,
        text=True,
        timeout=300,
        env=extra_env,
    )

    if result.returncode != 0:
        raise RuntimeError(f"Build failed:\n{result.stderr[-2000:]}")

    # Find the output file
    if env.startswith("esp32"):
        fw_path = BUILD_DIR / env / "firmware.bin"
        ext = ".bin"
    else:
        fw_path = BUILD_DIR / env / "firmware.hex"
        ext = ".hex"

    if not fw_path.exists():
        raise FileNotFoundError(f"Firmware not found: {fw_path}")

    return fw_path.read_bytes(), ext


class BuildHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == "/health":
            self._json_response(200, {"status": "ok"})
            return
        if self.path == "/envs":
            self._json_response(200, {"envs": sorted(VALID_ENVS)})
            return
        self._json_response(404, {"error": "Not found"})

    def do_POST(self):
        if self.path != "/build":
            self._json_response(404, {"error": "Not found"})
            return

        content_len = int(self.headers.get("Content-Length", 0))
        if content_len > 1024:
            self._json_response(400, {"error": "Request too large"})
            return

        try:
            body = json.loads(self.rfile.read(content_len))
        except (json.JSONDecodeError, UnicodeDecodeError):
            self._json_response(400, {"error": "Invalid JSON"})
            return

        env = body.get("env", "")
        chassis = body.get("chassis", 1)
        vehicle = body.get("vehicle", 0)
        body_bus = body.get("body", 0)

        # Validate env name format
        if not isinstance(env, str) or not ENV_RE.match(env):
            self._json_response(400, {"error": "Invalid env name"})
            return
        for name, val in [("chassis", chassis), ("vehicle", vehicle), ("body", body_bus)]:
            if not isinstance(val, int) or val not in (0, 1):
                self._json_response(400, {"error": f"Invalid {name} value"})
                return

        try:
            fw_bytes, ext = build_firmware(env, chassis, vehicle, body_bus)
        except ValueError as e:
            self._json_response(400, {"error": str(e)})
            return
        except (RuntimeError, FileNotFoundError) as e:
            self._json_response(500, {"error": str(e)})
            return

        parts = [env]
        if vehicle:
            parts.append("vehicle")
        if body_bus:
            parts.append("body")
        filename = f"{'_'.join(parts)}{ext}"
        self.send_response(200)
        self.send_header("Content-Type", "application/octet-stream")
        self.send_header("Content-Disposition", f'attachment; filename="{filename}"')
        self.send_header("Content-Length", str(len(fw_bytes)))
        self.send_header("Access-Control-Allow-Origin", "*")
        self.end_headers()
        self.wfile.write(fw_bytes)

    def do_OPTIONS(self):
        self.send_response(204)
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        self.end_headers()

    def _json_response(self, status, data):
        body = json.dumps(data).encode()
        self.send_response(status)
        self.send_header("Content-Type", "application/json")
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def log_message(self, fmt, *args):
        print(f"[build-server] {fmt % args}")


if __name__ == "__main__":
    port = int(os.environ.get("BUILD_SERVER_PORT", "8100"))
    server = HTTPServer(("0.0.0.0", port), BuildHandler)
    print(f"[build-server] Listening on :{port}")
    print(f"[build-server] Project dir: {PROJECT_DIR}")
    print(f"[build-server] Valid envs: {len(VALID_ENVS)}")
    server.serve_forever()
