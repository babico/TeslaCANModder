param(
  [string]$BaseUrl = 'https://tesla-fsd.netlify.app/'
)

$ErrorActionPreference = 'Stop'

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$IndexPath = Join-Path $Root 'index.html'
$ReportPath = Join-Path $Root '_scrape_report.txt'
$BoardsDir = Join-Path $Root 'boards'
$CodesDir = Join-Path $Root 'codes'
$MarkdownDir = Join-Path $Root 'markdown'
$MapCsvPath = Join-Path $Root 'board-code-map.csv'
$ExtractReadmePath = Join-Path $Root 'README.md'
$LegacyOutRoot = Join-Path $Root 'full-extract'

function HtmlDecode([string]$Text) {
  [System.Net.WebUtility]::HtmlDecode($Text)
}

function StripHtml([string]$Text) {
  if ([string]::IsNullOrWhiteSpace($Text)) { return '' }
  $t = $Text -replace '(?is)<script.*?</script>', ' '
  $t = $t -replace '(?is)<style.*?</style>', ' '
  $t = $t -replace '(?is)<br\s*/?>', "`n"
  $t = $t -replace '(?is)</(h1|h2|h3|h4|p|li|tr|div|section|table|ul|ol)>', "`n"
  $t = $t -replace '(?is)<[^>]+>', ' '
  $t = HtmlDecode $t
  $t = $t -replace '[\t ]+', ' '
  $t = $t -replace '(\r?\n){3,}', "`n`n"
  $t.Trim()
}

function ConvertHtmlTableToMarkdown([string]$TableHtml) {
  $rows = [regex]::Matches($TableHtml, '(?is)<tr[^>]*>(.*?)</tr>')
  if ($rows.Count -eq 0) { return '' }

  $lineRows = @()
  foreach ($r in $rows) {
    $cells = [regex]::Matches($r.Groups[1].Value, '(?is)<t[dh][^>]*>(.*?)</t[dh]>')
    if ($cells.Count -eq 0) { continue }
    $vals = @()
    foreach ($c in $cells) {
      $cell = $c.Groups[1].Value
      $cell = $cell -replace '(?is)<br\s*/?>', ' / '
      $cell = StripHtml $cell
      $cell = $cell -replace '\|', '\\|'
      if (-not $cell) { $cell = ' ' }
      $vals += $cell
    }
    if ($vals.Count -gt 0) { $lineRows += ,$vals }
  }

  if ($lineRows.Count -eq 0) { return '' }

  $header = $lineRows[0]
  $width = $header.Count
  $md = @()
  $md += '| ' + ($header -join ' | ') + ' |'
  $md += '| ' + ((1..$width | ForEach-Object { '---' }) -join ' | ') + ' |'

  for ($i = 1; $i -lt $lineRows.Count; $i++) {
    $row = $lineRows[$i]
    if ($row.Count -lt $width) {
      $row = @($row + ((1..($width - $row.Count) | ForEach-Object { ' ' })))
    }
    if ($row.Count -gt $width) {
      $row = $row[0..($width - 1)]
    }
    $md += '| ' + ($row -join ' | ') + ' |'
  }

  ($md -join "`n")
}

function ConvertBoardHtmlToMarkdown([string]$BoardHtml, [string]$BoardTitle, [string[]]$Categories) {
  if ([string]::IsNullOrWhiteSpace($BoardHtml)) { return '' }

  $h = $BoardHtml
  $h = $h -replace '(?is)<script.*?</script>', ''
  $h = $h -replace '(?is)<style.*?</style>', ''
  $h = $h -replace '(?is)<!--.*?-->', ''
  $h = $h -replace '(?is)<svg.*?</svg>', ''

  # Remove giant embedded image payloads but keep alt as readable note.
  $h = [regex]::Replace(
    $h,
    '(?is)<img[^>]*src="data:[^"]+"[^>]*alt="([^"]*)"[^>]*>',
    {
      param($m)
      $alt = StripHtml $m.Groups[1].Value
      if ($alt) { "<p>[Image: $alt]</p>" } else { '<p>[Image]</p>' }
    }
  )

  $h = [regex]::Replace(
    $h,
    '(?is)<a[^>]*href="([^"]+)"[^>]*>(.*?)</a>',
    {
      param($m)
      $url = $m.Groups[1].Value
      $txt = StripHtml $m.Groups[2].Value
      if (-not $txt) { $txt = $url }
      "[$txt]($url)"
    }
  )

  $h = [regex]::Replace(
    $h,
    '(?is)<div[^>]*class="[^"]*(bit-box|code)[^"]*"[^>]*>(.*?)</div>',
    {
      param($m)
      $code = $m.Groups[2].Value
      $code = $code -replace '(?is)<br\s*/?>', "`n"
      $code = $code -replace '(?is)</?(span|code|pre|b|strong|em|i)[^>]*>', ''
      $code = HtmlDecode $code
      # Prevent later HTML-tag stripping from eating operators like << and >>.
      $code = $code -replace '<', '&lt;'
      $code = $code -replace '>', '&gt;'
      $code = $code -replace '[\t ]+$', ''
      $code = $code.Trim()
      if (-not $code) { return '' }
      $fence = '```'
      "`n$fence" + "text`n$code`n$fence`n"
    }
  )

  $h = [regex]::Replace(
    $h,
    '(?is)<table[^>]*>.*?</table>',
    {
      param($m)
      $tableMd = ConvertHtmlTableToMarkdown $m.Value
      if ($tableMd) { "`n$tableMd`n" } else { '' }
    }
  )

  $h = [regex]::Replace(
    $h,
    '(?is)<h2[^>]*>(.*?)</h2>',
    {
      param($m)
      "`n## " + (StripHtml $m.Groups[1].Value) + "`n"
    }
  )
  $h = [regex]::Replace(
    $h,
    '(?is)<h3[^>]*>(.*?)</h3>',
    {
      param($m)
      "`n### " + (StripHtml $m.Groups[1].Value) + "`n"
    }
  )
  $h = [regex]::Replace(
    $h,
    '(?is)<h4[^>]*>(.*?)</h4>',
    {
      param($m)
      "`n#### " + (StripHtml $m.Groups[1].Value) + "`n"
    }
  )
  $h = [regex]::Replace(
    $h,
    '(?is)<h5[^>]*>(.*?)</h5>',
    {
      param($m)
      "`n##### " + (StripHtml $m.Groups[1].Value) + "`n"
    }
  )

  $h = [regex]::Replace(
    $h,
    '(?is)<li[^>]*>(.*?)</li>',
    {
      param($m)
      "`n- " + (StripHtml $m.Groups[1].Value)
    }
  )

  $h = [regex]::Replace(
    $h,
    '(?is)<div[^>]*class="[^"]*(note|summary-box|warn)[^"]*"[^>]*>(.*?)</div>',
    {
      param($m)
      $note = StripHtml $m.Groups[2].Value
      if (-not $note) { return '' }
      $lines = $note -split "`r?`n" | ForEach-Object { $_.Trim() } | Where-Object { $_ }
      if ($lines.Count -eq 0) { return '' }
      (($lines | ForEach-Object { "> $_" }) -join "`n") + "`n"
    }
  )

  $h = [regex]::Replace(
    $h,
    '(?is)<p[^>]*>(.*?)</p>',
    {
      param($m)
      "`n" + (StripHtml $m.Groups[1].Value) + "`n"
    }
  )

  $h = $h -replace '(?is)<br\s*/?>', "`n"
  $h = $h -replace '(?is)</?(div|section|article|header|footer|main|tbody|thead|tr|td|th|ul|ol|span)[^>]*>', "`n"
  $h = $h -replace '(?is)<[^>]+>', ' '
  $h = HtmlDecode $h
  $h = $h -replace '[\t ]+', ' '
  $h = $h -replace ' ?\r?\n ?', "`n"
  $h = $h -replace '(\r?\n){3,}', "`n`n"
  $h = $h.Trim()

  $md = @()
  $md += "# $BoardTitle"
  if ($Categories -and $Categories.Count) {
    $md += ''
    $md += '## Categories'
    foreach ($c in $Categories) {
      if ($c) { $md += "- $c" }
    }
  }
  $md += ''
  $md += '## Extracted Content'
  $md += ''
  $md += $h
  ($md -join "`n")
}

# 1) Pull latest homepage snapshot
$headers = @{ 'User-Agent' = 'Mozilla/5.0 (compatible; SiteMirrorSkill/1.0)' }
$response = Invoke-WebRequest -Uri $BaseUrl -Headers $headers -MaximumRedirection 10
[IO.File]::WriteAllText($IndexPath, $response.Content, [Text.Encoding]::UTF8)

# 2) Write scrape report
$bytes = (Get-Item $IndexPath).Length
$lines = (Get-Content $IndexPath | Measure-Object -Line).Lines
$hash = (Get-FileHash $IndexPath -Algorithm SHA256).Hash
$head = & curl.exe -I -L -s $BaseUrl
$report = @(
  "File: $IndexPath"
  "Bytes: $bytes"
  "Lines: $lines"
  "SHA256: $hash"
  ''
  'Response headers:'
  $head
)
Set-Content -Path $ReportPath -Value $report -Encoding UTF8

# 3) Clear old generated outputs
if (Test-Path $LegacyOutRoot) { Remove-Item $LegacyOutRoot -Recurse -Force }
if (Test-Path $BoardsDir) { Remove-Item $BoardsDir -Recurse -Force }
if (Test-Path $CodesDir) { Remove-Item $CodesDir -Recurse -Force }
if (Test-Path $MarkdownDir) { Remove-Item $MarkdownDir -Recurse -Force }
if (Test-Path $MapCsvPath) { Remove-Item $MapCsvPath -Force }
if (Test-Path $ExtractReadmePath) { Remove-Item $ExtractReadmePath -Force }

New-Item -ItemType Directory -Path $BoardsDir | Out-Null
New-Item -ItemType Directory -Path $CodesDir | Out-Null
New-Item -ItemType Directory -Path $MarkdownDir | Out-Null

$html = Get-Content $IndexPath -Raw

# 4) Extract board tabs to boards/*.html and boards/*.txt
$tabRegex = '<div id="tab-([a-z0-9]+)" class="tab-content(?: active)?">'
$tabMatches = [regex]::Matches($html, $tabRegex, 'IgnoreCase')
$boardSummaries = @()

for ($i = 0; $i -lt $tabMatches.Count; $i++) {
  $id = $tabMatches[$i].Groups[1].Value
  $start = $tabMatches[$i].Index
  $end = if ($i -lt $tabMatches.Count - 1) { $tabMatches[$i + 1].Index } else { $html.Length }
  $chunk = $html.Substring($start, $end - $start)

  $titleMatch = [regex]::Match($chunk, '(?is)<h2>(.*?)</h2>')
  $title = if ($titleMatch.Success) { StripHtml $titleMatch.Groups[1].Value } else { $id }

  Set-Content -Path (Join-Path $BoardsDir "$id.html") -Value $chunk -Encoding UTF8
  Set-Content -Path (Join-Path $BoardsDir "$id.txt") -Value (StripHtml $chunk) -Encoding UTF8

  $catMatches = [regex]::Matches($chunk, '(?is)<button class="cat-btn(?: active)?"[^>]*>(.*?)</button>')
  $cats = @()
  foreach ($m in $catMatches) {
    $c = StripHtml $m.Groups[1].Value
    if ($c -and -not $cats.Contains($c)) { $cats += $c }
  }

  $h3Matches = [regex]::Matches($chunk, '(?is)<h3[^>]*>(.*?)</h3>')
  $sections = @()
  foreach ($m in $h3Matches) {
    $s = StripHtml $m.Groups[1].Value
    if ($s -and -not $sections.Contains($s)) { $sections += $s }
  }

  $codeIds = [regex]::Matches($chunk, "copyCode\('([^']+)'", 'IgnoreCase') | ForEach-Object { $_.Groups[1].Value } | Select-Object -Unique

  $boardSummaries += [pscustomobject]@{
    id = $id
    title = $title
    categories = $cats
    sections = $sections
    codeIds = @($codeIds)
  }
}

# 5) Decode INO_B64 blobs into codes/*.ino
$inoRegex = "INO_B64\['([^']+)'\]\s*=\s*'([^']*)';"
$inoMatches = [regex]::Matches($html, $inoRegex, 'IgnoreCase')
$decoded = @()

foreach ($m in $inoMatches) {
  $codeId = $m.Groups[1].Value
  $b64 = $m.Groups[2].Value
  $text = ''
  try {
    $text = [Text.Encoding]::UTF8.GetString([Convert]::FromBase64String($b64))
  } catch {
    $text = ''
  }

  $name = switch ($codeId) {
    'code-esp8266' { 'CanFeather_ESP8266_WiFi.ino' }
    'code-esp32' { 'CanFeather_ESP32_WiFi.ino' }
    'code-esp32s3' { 'CanFeather_ESP32S3_TWAI.ino' }
    'code-feather' { 'CanFeather_RP2040.ino' }
    'code-esp32c3' { 'ESP32C3_WiFiBridge.ino' }
    'code-uno' { 'CanFeather_ArduinoUno.ino' }
    'code-esp32lite' { 'CanFeather_ESP32_LITE.ino' }
    default { "$codeId.ino" }
  }

  Set-Content -Path (Join-Path $CodesDir $name) -Value $text -Encoding UTF8
  $lineCount = if ($text) { ($text -split "`n").Count } else { 0 }
  $decoded += [pscustomobject]@{ codeId = $codeId; file = $name; lines = $lineCount }
}

# 6) Write mapping CSV
$csvLines = @('board_id,title,code_ids')
foreach ($b in $boardSummaries) {
  $csvLines += ('"{0}","{1}","{2}"' -f $b.id, ($b.title -replace '"','""'), (($b.codeIds -join ' | ') -replace '"','""'))
}
Set-Content -Path $MapCsvPath -Value $csvLines -Encoding UTF8

# 7) Markdown package
foreach ($b in $boardSummaries) {
  $boardHtmlPath = Join-Path $BoardsDir "$($b.id).html"
  $boardHtml = if (Test-Path $boardHtmlPath) { Get-Content $boardHtmlPath -Raw } else { '' }
  $richMd = ConvertBoardHtmlToMarkdown -BoardHtml $boardHtml -BoardTitle $b.title -Categories $b.categories

  $md = @()
  $md += "# $($b.title)"
  $md += ''
  $md += "Board ID: $($b.id)"
  $md += ''
  $md += '## Categories'
  if ($b.categories.Count) { $b.categories | ForEach-Object { $md += "- $_" } } else { $md += '- (none)' }
  $md += ''
  $md += '## Main Sections'
  if ($b.sections.Count) { $b.sections | ForEach-Object { $md += "- $_" } } else { $md += '- (none)' }
  $md += ''
  $md += '## Code IDs'
  if ($b.codeIds.Count) { $b.codeIds | ForEach-Object { $md += "- $_" } } else { $md += '- (none)' }

  if ($richMd) {
    $md += ''
    $md += '---'
    $md += ''
    $md += $richMd
  }

  Set-Content -Path (Join-Path $MarkdownDir "$($b.id).md") -Value $md -Encoding UTF8
}

$mdIndex = @('# Markdown Package', '', 'Board summaries:')
$mdIndex += ($boardSummaries | ForEach-Object { "- $($_.id).md" })
Set-Content -Path (Join-Path $MarkdownDir 'README.md') -Value $mdIndex -Encoding UTF8

# 8) Main README
$readme = @()
$readme += '# Full Board and Code Extract'
$readme += ''
$readme += "Generated from: $BaseUrl"
$readme += ''
$readme += '## Artifacts'
$readme += '- codes/: decoded firmware files'
$readme += '- markdown/: board summaries'
$readme += '- board-code-map.csv: board-to-code mapping'
$readme += '- boards/: full board html/txt exports'
$readme += ''
$readme += '## Decoded Codes'
$readme += ($decoded | ForEach-Object { "- codes/$($_.file) ($($_.lines) lines)" })
Set-Content -Path $ExtractReadmePath -Value $readme -Encoding UTF8

Write-Host '[OK] Site mirror rebuilt successfully.'
Write-Host "[OK] Output: $Root"
