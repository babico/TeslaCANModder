"""
PlatformIO pre-build script: converts dashboard.html → dashboard.h (PROGMEM).
Run automatically before each build — edit dashboard.html, not dashboard.h.
"""
Import("env")
import os

def embed_html(source, target, env):
    src_dir = env.subst("$PROJECT_DIR")
    html_path = os.path.join(src_dir, "lib", "io", "wifi", "dashboard.html")
    header_path = os.path.join(src_dir, "lib", "io", "wifi", "dashboard.h")

    if not os.path.exists(html_path):
        print("embed_html: dashboard.html not found, skipping")
        return

    with open(html_path, "r", encoding="utf-8") as f:
        html = f.read()

    header = '#pragma once\n'
    header += '// AUTO-GENERATED from dashboard.html — do not edit directly.\n'
    header += '#include <pgmspace.h>\n\n'
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

env.AddPreAction("buildprog", embed_html)
