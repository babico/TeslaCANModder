function Get-ShortPath([string]$path) {
  $escaped = $path.Replace('"', '\"')
  $shortPath = cmd /c "for %I in (""$escaped"") do @echo %~sI"
  if ([string]::IsNullOrWhiteSpace($shortPath)) {
    return $path
  }
  return $shortPath.Trim()
}

function Use-AsciiWorkspace([string]$path) {
  if ($path -cmatch '^[\x00-\x7F]+$') {
    return @{
      Path = $path
      Cleanup = $null
    }
  }

  foreach ($driveLetter in @("T", "U", "V", "W", "X", "Y", "Z")) {
    $drive = "${driveLetter}:"
    $existing = Get-PSDrive -Name $driveLetter -ErrorAction SilentlyContinue

    if ($existing) {
      if ($existing.Root -ieq $path) {
        return @{
          Path = $drive
          Cleanup = $null
        }
      }

      continue
    }

    cmd /c "subst $drive ""$path""" | Out-Null
    if (Test-Path "$drive\") {
      return @{
        Path = $drive
        Cleanup = { cmd /c "subst $drive /d" | Out-Null }
      }
    }
  }

  return @{
    Path = $path
    Cleanup = $null
  }
}

function Normalize-PlatformIOArgs([string[]]$rawArgs) {
  if ($rawArgs.Count -eq 0) {
    return ,$rawArgs
  }

  if ($rawArgs[0] -ne "test") {
    return ,$rawArgs
  }

  $normalized = New-Object System.Collections.Generic.List[string]
  $normalized.AddRange($rawArgs)

  $envIndex = -1
  for ($i = 0; $i -lt $normalized.Count; $i++) {
    if ($normalized[$i] -eq "-e" -or $normalized[$i] -eq "--environment") {
      $envIndex = $i
      break
    }
  }

  if ($envIndex -ge 0 -and ($envIndex + 1) -lt $normalized.Count) {
    return ,$normalized.ToArray()
  }

  $normalized.Add("-e")
  $normalized.Add("native")
  return ,$normalized.ToArray()
}

$projectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$asciiWorkspace = Use-AsciiWorkspace $projectRoot
$shortProjectRoot = Get-ShortPath $asciiWorkspace.Path
$platformioArgs = Normalize-PlatformIOArgs $Args
$isTestCommand = $platformioArgs.Count -gt 0 -and $platformioArgs[0] -eq "test"
$platformioExe = Join-Path $env:USERPROFILE ".platformio\penv\Scripts\platformio.exe"

if (-not (Test-Path $platformioExe)) {
  throw "PlatformIO Core executable not found at $platformioExe"
}

# Keep this project independent from any broken global PlatformIO package cache.
# The wrapper also prefers an ASCII workspace path because the AVR toolchain can
# fail under Unicode Windows paths during archive creation.
$env:PLATFORMIO_CORE_DIR = Join-Path $shortProjectRoot ".pio-home"
$env:PLATFORMIO_SETTING_ENABLE_TELEMETRY = "No"

Push-Location $shortProjectRoot
$exitCode = 0
try {
  if ($isTestCommand) {
    $output = & $platformioExe @platformioArgs 2>&1
    $exitCode = $LASTEXITCODE
    foreach ($line in $output) {
      $text = $line.ToString()
      if ($text -match '\bSKIPPED\b') {
        continue
      }
      Write-Host $text
    }
  } else {
    & $platformioExe @platformioArgs
    $exitCode = $LASTEXITCODE
  }
}
finally {
  Pop-Location
  if ($null -ne $asciiWorkspace.Cleanup) {
    & $asciiWorkspace.Cleanup
  }
}
exit $exitCode
