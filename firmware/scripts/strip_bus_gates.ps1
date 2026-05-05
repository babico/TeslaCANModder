#!/usr/bin/env pwsh
# Strip BUS_*_ACTIVE preprocessor gates from a file, keeping the bus-active branch.
# Patterns:
#   (1) #if BUS_X_ACTIVE   ... #endif           → keep body, drop #if/#endif
#   (2) #if !BUS_X_ACTIVE  <stub> #else <real> #endif  → keep <real> only
#   (3) #if !BUS_X_ACTIVE  <stub> #endif        → drop entire block
# Other #if/#ifdef/#ifndef blocks (BOARD_ENABLE_*, etc.) are preserved verbatim.
param([Parameter(Mandatory = $true)] [string[]] $Files)

$busPosRegex  = '^\s*#\s*if\s+(?:BUS_[A-Z_]+_ACTIVE(?:\s*(?:\|\||&&)\s*BUS_[A-Z_]+_ACTIVE)*)\s*$'
$busNegRegex  = '^\s*#\s*if\s+!\s*BUS_[A-Z_]+_ACTIVE\s*$'
$ifRegex      = '^\s*#\s*(?:if|ifdef|ifndef)\b'
$endifRegex   = '^\s*#\s*endif\b'
$elseRegex    = '^\s*#\s*else\b'
$elifRegex    = '^\s*#\s*elif\b'

function Skipping($stk) {
	foreach ($e in $stk) { if ($e.State -eq 'skip') { return $true } }
	return $false
}

foreach ($file in $Files) {
	if (!(Test-Path $file)) { Write-Warning "Missing: $file"; continue }
	$lines = Get-Content -LiteralPath $file
	$out = New-Object System.Collections.Generic.List[string]
	$stack = New-Object System.Collections.Generic.Stack[hashtable]
	$removed = 0

	for ($i = 0; $i -lt $lines.Count; $i++) {
		$line = $lines[$i]

		if ($line -match $busPosRegex) {
			$stack.Push(@{ Kind = 'pos'; State = 'keep' })
			$removed++; continue
		}
		if ($line -match $busNegRegex) {
			$stack.Push(@{ Kind = 'neg'; State = 'skip' })
			$removed++; continue
		}
		if ($line -match $elseRegex) {
			if ($stack.Count -gt 0 -and $stack.Peek().Kind -eq 'neg') {
				$stack.Peek().State = 'keep'
				$removed++; continue
			}
			if (-not (Skipping $stack)) { $out.Add($line) }
			continue
		}
		if ($line -match $elifRegex) {
			if ($stack.Count -gt 0 -and $stack.Peek().Kind -eq 'neg') {
				throw "Unsupported #elif inside !BUS gate at line $($i+1) of $file"
			}
			if (-not (Skipping $stack)) { $out.Add($line) }
			continue
		}
		if ($line -match $ifRegex) {
			$state = if (Skipping $stack) { 'skip' } else { 'keep' }
			$stack.Push(@{ Kind = 'other'; State = $state })
			if ($state -eq 'keep') { $out.Add($line) }
			continue
		}
		if ($line -match $endifRegex) {
			if ($stack.Count -eq 0) { $out.Add($line); continue }
			$top = $stack.Pop()
			if ($top.Kind -eq 'pos' -or $top.Kind -eq 'neg') { $removed++; continue }
			if (-not (Skipping $stack)) { $out.Add($line) }
			continue
		}
		if (-not (Skipping $stack)) { $out.Add($line) }
	}

	if ($stack.Count -ne 0) { throw "Unbalanced preprocessor stack in $file" }
	Set-Content -LiteralPath $file -Value $out
	Write-Host "Stripped $removed gate line(s) from $file"
}
