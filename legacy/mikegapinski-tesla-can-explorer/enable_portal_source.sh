#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APP_JS="${SCRIPT_DIR}/app.js"
DATA_DIR="${SCRIPT_DIR}/data"

usage() {
  cat <<'EOF'
Patch Tesla CAN Explorer portal to enable a new data source.

Usage:
  enable_portal_source.sh \
    --source-key <key> \
    --label "<UI label>" \
    [--json-url "./data/can_frames_decoded_all_values_<key>.json"] \
    [--set-default]

Example:
  ./can_re/enable_portal_source.sh \
    --source-key modelsx_amd \
    --label "Model S/X MCU3 (AMD)"

  ./can_re/enable_portal_source.sh \
    --source-key modelsx_amd \
    --label "Model S/X MCU3 (AMD)" \
    --set-default
EOF
}

SOURCE_KEY=""
LABEL=""
JSON_URL=""
SET_DEFAULT="0"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --source-key)
      SOURCE_KEY="${2:-}"
      shift 2
      ;;
    --label)
      LABEL="${2:-}"
      shift 2
      ;;
    --json-url)
      JSON_URL="${2:-}"
      shift 2
      ;;
    --set-default)
      SET_DEFAULT="1"
      shift 1
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown argument: $1" >&2
      usage
      exit 1
      ;;
  esac
done

if [[ -z "${SOURCE_KEY}" || -z "${LABEL}" ]]; then
  echo "Missing required arguments." >&2
  usage
  exit 1
fi

if [[ -z "${JSON_URL}" ]]; then
  JSON_URL="./data/can_frames_decoded_all_values_${SOURCE_KEY}.json"
fi

if [[ ! -f "${APP_JS}" ]]; then
  echo "Portal app.js not found: ${APP_JS}" >&2
  exit 1
fi

# If URL points at local ./data, verify file exists to avoid broken selector entries.
if [[ "${JSON_URL}" == ./data/* ]]; then
  REL_PATH="${JSON_URL#./data/}"
  if [[ ! -f "${DATA_DIR}/${REL_PATH}" ]]; then
    echo "Warning: expected dataset file not found: ${DATA_DIR}/${REL_PATH}" >&2
  fi
fi

python3 - "${APP_JS}" "${SOURCE_KEY}" "${LABEL}" "${JSON_URL}" "${SET_DEFAULT}" <<'PY'
import json
import pathlib
import re
import sys

app_js = pathlib.Path(sys.argv[1])
source_key = sys.argv[2]
label = sys.argv[3]
json_url = sys.argv[4]
set_default = sys.argv[5] == "1"

text = app_js.read_text(encoding="utf-8")

m = re.search(r"const DATA_SOURCES = \{\n(?P<body>.*?)\n\};", text, flags=re.S)
if not m:
    raise SystemExit("Could not find DATA_SOURCES block in app.js")

body = m.group("body")
lines = body.splitlines()

entries = {}
i = 0
while i < len(lines):
    line = lines[i]
    key_match = re.match(r"  ([A-Za-z0-9_]+): \{$", line)
    if not key_match:
        i += 1
        continue
    key = key_match.group(1)
    start = i
    i += 1
    while i < len(lines) and lines[i] != "  },":
        i += 1
    if i >= len(lines):
        raise SystemExit(f"Malformed DATA_SOURCES entry for key: {key}")
    end = i
    entries[key] = (start, end)
    i += 1

entry_lines = [
    f"  {source_key}: {{",
    f"    label: {json.dumps(label, ensure_ascii=False)},",
    f"    url: {json.dumps(json_url, ensure_ascii=False)},",
    "  },",
]

if source_key in entries:
    start, end = entries[source_key]
    lines[start : end + 1] = entry_lines
    action = "updated"
else:
    lines.extend(entry_lines)
    action = "added"

new_body = "\n".join(lines)
text = text[: m.start("body")] + new_body + text[m.end("body") :]

if set_default:
    text, count = re.subn(
        r'const DEFAULT_SOURCE_KEY = "[^"]+";',
        f'const DEFAULT_SOURCE_KEY = "{source_key}";',
        text,
        count=1,
    )
    if count == 0:
        raise SystemExit("Could not find DEFAULT_SOURCE_KEY in app.js")

app_js.write_text(text, encoding="utf-8")
print(f"{action} DATA_SOURCES entry: {source_key}")
if set_default:
    print(f"set DEFAULT_SOURCE_KEY: {source_key}")
PY

echo "Patched portal source in ${APP_JS}"
