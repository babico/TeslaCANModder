"""
PlatformIO pre-build script: converts dashboard.html → dashboard.h (PROGMEM).
Run automatically before each build — edit dashboard.html, not dashboard.h.
"""

import os
from typing import Any

try:
    from SCons.Script import Import  # type: ignore[import-not-found]
except ModuleNotFoundError:
    def Import(*_args: str) -> None:
        return None


env: Any = globals().get("env")
Import("env")
env = globals().get("env", env)


def embed_html(source, target, env):
    src_dir = env.subst("$PROJECT_DIR")
    html_path = os.path.join(src_dir, "lib", "io", "wifi", "dashboard", "esp32", "dashboard.html")
    header_path = os.path.join(src_dir, "lib", "io", "wifi", "dashboard", "esp32", "dashboard.h")

    if not os.path.exists(html_path):
        print("embed_html: dashboard.html not found, skipping")
        return

    with open(html_path, "r", encoding="utf-8") as f:
        html = f.read()

    header = "#pragma once\n"
    header += "// AUTO-GENERATED from dashboard.html — do not edit directly.\n"
    header += "#include <pgmspace.h>\n\n"
    header += 'static const char DASH_HTML[] PROGMEM = R"HTML('
    header += html
    header += ')HTML";\n'

    # Only write if content changed (avoid unnecessary rebuilds)
    if os.path.exists(header_path):
        with open(header_path, "r", encoding="utf-8") as f:
            if f.read() == header:
                return

    with open(header_path, "w", encoding="utf-8") as f:
        f.write(header)
    print("embed_html: dashboard.h updated from dashboard.html")


if env is not None:
    env.AddPreAction("buildprog", embed_html)
    # Ensure header exists even when pre-action ordering does not run before compile.
    embed_html(None, None, env)
