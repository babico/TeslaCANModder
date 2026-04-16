# Site Mirror Rebuild Skill

Purpose
- Recreate the local mirror and extracted artifacts after each site update.
- Keep outputs reproducible, complete, and easy to diff between runs.

Minimal setup
- This workflow is PowerShell-only.
- No .cmd launcher is used or required.
- Single entrypoint: recreate_site_mirror.ps1

What this skill does
- Downloads the latest https://tesla-fsd.netlify.app/ into index.html.
- Writes _scrape_report.txt with size/hash/headers metadata.
- Clears old generated outputs in this folder.
- Rebuilds:
  - boards/*.html and *.txt (per-board raw exports)
  - codes/*.ino (decoded from embedded INO_B64 blobs)
  - markdown/*.md (rich markdown converted from board HTML)
  - board-code-map.csv
  - README.md

Markdown extraction behavior
- Converts board HTML to structured markdown with:
  - Headings (h2-h5)
  - Tables (markdown table format)
  - Lists and paragraphs
  - Note and warning blocks (blockquote style)
  - Code and bit boxes (fenced code blocks)
  - Links converted to markdown links
- Replaces very large embedded base64 image payloads with readable placeholders.
- Preserves technical code operators like << and >> in extracted code samples.

Run
```powershell
pwsh -ExecutionPolicy Bypass -File .\recreate_site_mirror.ps1
```

Run from repository root
```powershell
pwsh -ExecutionPolicy Bypass -File .\legacy\tesla-fsd.netlify.app\recreate_site_mirror.ps1
```

Quick validation checklist
- Confirm script prints:
  - [OK] Site mirror rebuilt successfully.
  - [OK] Output: <path>
- Confirm these exist:
  - index.html
  - _scrape_report.txt
  - boards/
  - codes/
  - markdown/
  - board-code-map.csv
- Spot-check one markdown file to verify it contains:
  - summary section (board id/categories/code ids)
  - extracted content section with real tables and fenced code blocks

Notes
- Script is idempotent: safe to run repeatedly.
- Output always reflects the current live site snapshot.
- Existing generated artifacts are intentionally replaced on each run.
- Legacy full-extract/ layout is automatically removed during rebuild.

Troubleshooting
- If output looks stale, verify network access and rerun.
- If markdown quality regresses, inspect recreate_site_mirror.ps1 HTML-to-markdown converter functions first.
- If a board is missing, check whether the source page still contains the tab id pattern:
  - <div id="tab-..." class="tab-content">
