param(
    [string]$Port = "COM3",
    [int]$Baud = 115200,
    [string]$Variant = "hw4",
    [int]$WarmupMs = 2500,
    [int]$CommandWaitMs = 700,
    [int]$StreamWatchMs = 3500,
    [switch]$TestPossibilities = $true,
    [switch]$TestFsd,
    [switch]$RestoreAfterTest
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function New-BoardPort {
    param(
        [string]$Name,
        [int]$Speed
    )

    $serial = [System.IO.Ports.SerialPort]::new(
        $Name,
        $Speed,
        [System.IO.Ports.Parity]::None,
        8,
        [System.IO.Ports.StopBits]::One
    )
    $serial.ReadTimeout = 150
    $serial.WriteTimeout = 1000
    $serial.NewLine = "`n"
    $serial.DtrEnable = $true
    $serial.RtsEnable = $true
    return $serial
}

function Read-BoardMessages {
    param(
        [System.IO.Ports.SerialPort]$Serial,
        [int]$DurationMs,
        [string]$Phase,
        [ref]$Buffer
    )

    $messages = New-Object System.Collections.Generic.List[object]
    $deadline = [DateTime]::UtcNow.AddMilliseconds($DurationMs)

    while ([DateTime]::UtcNow -lt $deadline) {
        $chunk = $Serial.ReadExisting()
        if ($chunk) {
            $Buffer.Value += $chunk
            $normalized = $Buffer.Value -replace "`r", ""
            $lines = $normalized -split "`n"
            $Buffer.Value = $lines[-1]

            for ($index = 0; $index -lt ($lines.Count - 1); $index++) {
                $line = $lines[$index].Trim()
                if ([string]::IsNullOrWhiteSpace($line)) {
                    continue
                }

                $entry = [ordered]@{
                    phase = $Phase
                    raw = $line
                    parsed = $null
                    type = "raw"
                }

                $jsonCandidate = $line
                if (-not ($jsonCandidate.StartsWith("{") -and $jsonCandidate.EndsWith("}"))) {
                    $startIndex = $jsonCandidate.IndexOf("{")
                    $endIndex = $jsonCandidate.LastIndexOf("}")
                    if ($startIndex -ge 0 -and $endIndex -gt $startIndex) {
                        $jsonCandidate = $jsonCandidate.Substring($startIndex, $endIndex - $startIndex + 1)
                    }
                }

                if ($jsonCandidate.StartsWith("{") -and $jsonCandidate.EndsWith("}")) {
                    try {
                        $parsed = $jsonCandidate | ConvertFrom-Json -ErrorAction Stop
                        $entry.parsed = $parsed
                        $entry.type = [string]$parsed.t
                    }
                    catch {
                        $entry.type = "parse-error"
                    }
                }

                $messages.Add([pscustomobject]$entry)
            }
        }

        Start-Sleep -Milliseconds 40
    }

    return $messages
}

function Send-BoardCommand {
    param(
        [System.IO.Ports.SerialPort]$Serial,
        [string]$Command
    )

    if ([string]::IsNullOrWhiteSpace($Command)) {
        return
    }

    $Serial.WriteLine($Command)
}

function Add-PhaseMessages {
    param(
        [System.Collections.Generic.List[object]]$AllMessages,
        [System.IO.Ports.SerialPort]$Serial,
        [string]$Phase,
        [int]$WaitMs,
        [string]$Command,
        [ref]$Buffer
    )

    if ($Command) {
        Send-BoardCommand -Serial $Serial -Command $Command
    }

    $messages = Read-BoardMessages -Serial $Serial -DurationMs $WaitMs -Phase $Phase -Buffer ([ref]$Buffer.Value)
    foreach ($message in $messages) {
        $AllMessages.Add($message)
    }
}

function Get-LastStatusValue {
    param(
        $Messages
    )

    $statusMessages = @($Messages | Where-Object { $_.type -eq "status" -and $_.parsed })
    if ($statusMessages.Count -eq 0) {
        return $null
    }

    return $statusMessages[-1].parsed
}

function Get-FrameCountByPhasePrefix {
    param(
        $Messages,
        [string]$Prefix
    )

    return @(
        $Messages |
            Where-Object { $_.type -eq "frame" -and $_.phase.StartsWith($Prefix) }
    ).Count
}

function Get-StatusByPhasePrefix {
    param(
        $Messages,
        [string]$Prefix
    )

    return @(
        $Messages |
            Where-Object { $_.type -eq "status" -and $_.phase.StartsWith($Prefix) }
    )
}

function Get-UnknownCommandCount {
    param(
        $Messages
    )

    return @(
        $Messages |
            Where-Object { $_.type -eq "error" -and [string]$_.parsed.msg -eq "Unknown command" }
    ).Count
}

function Get-PossibilityMatrix {
    param(
        $Messages,
        [string]$VariantRequested
    )

    $parsedMessages = @($Messages | Where-Object { $_.parsed -ne $null })
    $bootSeen = (@($parsedMessages | Where-Object { $_.type -eq "boot" }).Count -gt 0) -or (@($Messages | Where-Object { $_.raw -like '*"t":"boot"*' }).Count -gt 0)
    $pongSeen = @($parsedMessages | Where-Object { $_.type -eq "pong" }).Count -gt 0
    $latestStatus = Get-LastStatusValue -Messages $parsedMessages

    $filteredFrameCount = Get-FrameCountByPhasePrefix -Messages $parsedMessages -Prefix "filtered"
    $rawFrameCount = Get-FrameCountByPhasePrefix -Messages $parsedMessages -Prefix "raw"

    $rawCanAck = @($parsedMessages | Where-Object { $_.type -eq "ack" -and [string]$_.parsed.cmd -eq "can:raw:on" }).Count -gt 0
    $rawCanStatusSeen = @(
        $parsedMessages |
            Where-Object {
                $_.type -eq "status" -and
                $_.parsed.PSObject.Properties.Name -contains "rawCan"
            }
    ).Count -gt 0
    $rawCanSupported = $rawCanAck -or $rawCanStatusSeen

    $rawStatuses = Get-StatusByPhasePrefix -Messages $parsedMessages -Prefix "raw"
    $rawCanEnabled = @(
        $rawStatuses |
            Where-Object { [int]($_.parsed.rawCan) -eq 1 }
    ).Count -gt 0

    $variantMatches = $true
    if ($VariantRequested) {
        $variantMatches = $latestStatus -ne $null -and [string]$latestStatus.variant -eq $VariantRequested
    }

    $unknownCommandCount = Get-UnknownCommandCount -Messages $parsedMessages
    $parseErrorCount = @($Messages | Where-Object { $_.type -eq "parse-error" }).Count
    $noiseLikely = $unknownCommandCount -gt 0 -or $parseErrorCount -gt 0

    $filterMismatchLikely = $rawCanSupported -and $rawCanEnabled -and $rawFrameCount -gt 0 -and $filteredFrameCount -eq 0
    $physicalCanIssueLikely = $rawCanSupported -and $rawCanEnabled -and $rawFrameCount -eq 0
    $firmwareNeedsRawModeLikely = (-not $rawCanSupported)

    return [ordered]@{
        protocolOk = ($bootSeen -and $pongSeen)
        variantMatches = $variantMatches
        filteredFramesSeen = ($filteredFrameCount -gt 0)
        rawCanSupported = $rawCanSupported
        rawCanEnabled = $rawCanEnabled
        rawFramesSeen = ($rawFrameCount -gt 0)
        parserNoiseLikely = $noiseLikely
        likelyFilterMismatch = $filterMismatchLikely
        likelyPhysicalCanIssue = $physicalCanIssueLikely
        likelyNeedsUpdatedFirmware = $firmwareNeedsRawModeLikely
        unknownCommandCount = $unknownCommandCount
        parseErrorCount = $parseErrorCount
    }
}

function Get-Diagnosis {
    param(
        [hashtable]$Matrix,
        [pscustomobject]$LatestStatus
    )

    if (-not $Matrix.protocolOk) {
        return "Seri protokol akisi saglikli degil. COM port, kablo veya satir senkronu once netlesmeli."
    }

    if ($Matrix.likelyNeedsUpdatedFirmware) {
        return "Board can:raw komutunu tanimiyor. Ham CAN testi icin yeni firmware gerekli."
    }

    if ($Matrix.likelyFilterMismatch) {
        return "Ham modda frame var ama filtreli modda yok. Sorun buyuk ihtimalle variant veya filtre ID uyumsuzlugu."
    }

    if ($Matrix.likelyPhysicalCanIssue) {
        return "Ham CAN modunda bile hic frame yok. Sorun buyuk olasilikla kablolama, termination, guc veya aracin CAN hattina fiziksel erisim."
    }

    if ($Matrix.filteredFramesSeen) {
        return "Filtreli modda CAN frame goruluyor. Temel CAN erisimi calisiyor."
    }

    if ($LatestStatus -and [string]$LatestStatus.ready -eq "runtime-ready") {
        return "Board daha once trafik gormus ama bu pencerede frame dusmedi. Kisa pencere, aracin durumu veya aralik etkiliyor olabilir."
    }

    return "Net bir sonuca varilamadi; ham mesajlari incelemek iyi olur."
}

$buffer = ""
$allMessages = New-Object System.Collections.Generic.List[object]
$serial = $null
$initialStatus = $null
$initialFsd = $null

try {
    $serial = New-BoardPort -Name $Port -Speed $Baud
    try {
        $serial.Open()
    }
    catch {
        [pscustomobject]@{
            summary = [ordered]@{
                port = $Port
                baud = $Baud
                variantRequested = $(if ($Variant) { $Variant } else { $null })
                testPossibilitiesRequested = [bool]$TestPossibilities
                fsdTestRequested = [bool]$TestFsd
            }
            diagnosis = "Port acilamadi. COM3 baska bir uygulama tarafindan kullaniliyor olabilir."
            error = $_.Exception.Message
        } | ConvertTo-Json -Depth 6
        return
    }

    Add-PhaseMessages -AllMessages $allMessages -Serial $serial -Phase "warmup" -WaitMs $WarmupMs -Command $null -Buffer ([ref]$buffer)
    Add-PhaseMessages -AllMessages $allMessages -Serial $serial -Phase "variant" -WaitMs $CommandWaitMs -Command $(if ($Variant) { "variant:$Variant" } else { $null }) -Buffer ([ref]$buffer)
    Add-PhaseMessages -AllMessages $allMessages -Serial $serial -Phase "ping" -WaitMs $CommandWaitMs -Command "ping" -Buffer ([ref]$buffer)
    Add-PhaseMessages -AllMessages $allMessages -Serial $serial -Phase "filtered-stream-on" -WaitMs $StreamWatchMs -Command "stream:on" -Buffer ([ref]$buffer)
    Add-PhaseMessages -AllMessages $allMessages -Serial $serial -Phase "filtered-stream-off" -WaitMs $CommandWaitMs -Command "stream:off" -Buffer ([ref]$buffer)

    if ($TestPossibilities) {
        Add-PhaseMessages -AllMessages $allMessages -Serial $serial -Phase "raw-can-on" -WaitMs $CommandWaitMs -Command "can:raw:on" -Buffer ([ref]$buffer)
        Add-PhaseMessages -AllMessages $allMessages -Serial $serial -Phase "raw-stream-on" -WaitMs $StreamWatchMs -Command "stream:on" -Buffer ([ref]$buffer)
        Add-PhaseMessages -AllMessages $allMessages -Serial $serial -Phase "raw-stream-off" -WaitMs $CommandWaitMs -Command "stream:off" -Buffer ([ref]$buffer)
        Add-PhaseMessages -AllMessages $allMessages -Serial $serial -Phase "raw-can-off" -WaitMs $CommandWaitMs -Command "can:raw:off" -Buffer ([ref]$buffer)
    }

    $latestObservedStatus = Get-LastStatusValue -Messages $allMessages.ToArray()
    if ($latestObservedStatus) {
        $initialStatus = $latestObservedStatus
        $initialFsd = [int]$initialStatus.fsd
    }

    if ($TestFsd) {
        Add-PhaseMessages -AllMessages $allMessages -Serial $serial -Phase "fsd-on" -WaitMs $CommandWaitMs -Command "fsd:on" -Buffer ([ref]$buffer)
        Add-PhaseMessages -AllMessages $allMessages -Serial $serial -Phase "fsd-observe" -WaitMs ([Math]::Max($CommandWaitMs, 1200)) -Command $null -Buffer ([ref]$buffer)

        if ($RestoreAfterTest -and $initialFsd -ne $null) {
            $restoreCommand = if ($initialFsd -eq 1) { "fsd:on" } else { "fsd:off" }
            Add-PhaseMessages -AllMessages $allMessages -Serial $serial -Phase "restore" -WaitMs ([Math]::Max($CommandWaitMs, 1200)) -Command $restoreCommand -Buffer ([ref]$buffer)
        }
    }
}
finally {
    if ($serial -and $serial.IsOpen) {
        $serial.Close()
    }
    if ($serial) {
        $serial.Dispose()
    }
}

$parsedMessages = @($allMessages | Where-Object { $_.parsed -ne $null })
$bootMessages = @($parsedMessages | Where-Object { $_.type -eq "boot" })
$statusMessages = @($parsedMessages | Where-Object { $_.type -eq "status" })
$pongMessages = @($parsedMessages | Where-Object { $_.type -eq "pong" })
$ackMessages = @($parsedMessages | Where-Object { $_.type -eq "ack" })
$errorMessages = @($parsedMessages | Where-Object { $_.type -eq "error" })
$frameMessages = @($parsedMessages | Where-Object { $_.type -eq "frame" })
$parseErrors = @($allMessages | Where-Object { $_.type -eq "parse-error" })
$rawOnly = @($allMessages | Where-Object { $_.type -eq "raw" })

$bootSeen = ($bootMessages.Count -gt 0) -or (@($allMessages | Where-Object { $_.raw -like '*"t":"boot"*' }).Count -gt 0)
$latestStatus = Get-LastStatusValue -Messages $parsedMessages

$sampleFrameIds = @(
    $frameMessages |
        Select-Object -First 10 |
        ForEach-Object { [int]$_.parsed.id }
)

$fsdOnAck = @($ackMessages | Where-Object { [string]$_.parsed.cmd -eq "fsd:on" }).Count -gt 0
$fsdOffAck = @($ackMessages | Where-Object { [string]$_.parsed.cmd -eq "fsd:off" }).Count -gt 0
$fsdAfterOn = $null
if ($fsdOnAck) {
    $fsdOnAckMessage = @($ackMessages | Where-Object { [string]$_.parsed.cmd -eq "fsd:on" })[0]
    $messageArray = @($allMessages)
    $fsdOnAckIndex = [array]::IndexOf($messageArray, $fsdOnAckMessage)
    if ($fsdOnAckIndex -ge 0) {
        for ($index = $fsdOnAckIndex + 1; $index -lt $messageArray.Count; $index++) {
            $candidate = $messageArray[$index]
            if ($candidate.type -eq "status") {
                $fsdAfterOn = [int]$candidate.parsed.fsd
                break
            }
        }
    }
}

$fsdAfterRestore = $null
$restoreStatuses = @($statusMessages | Where-Object { $_.phase -eq "restore" })
if ($restoreStatuses.Count -gt 0) {
    $fsdAfterRestore = [int]$restoreStatuses[-1].parsed.fsd
}

$messageArray = $allMessages.ToArray()
$matrix = Get-PossibilityMatrix -Messages $messageArray -VariantRequested $Variant
$diagnosis = Get-Diagnosis -Matrix $matrix -LatestStatus $latestStatus

$summary = [ordered]@{
    port = $Port
    baud = $Baud
    variantRequested = $(if ($Variant) { $Variant } else { $null })
    testPossibilitiesRequested = [bool]$TestPossibilities
    totalMessages = $allMessages.Count
    bootSeen = $bootSeen
    pongSeen = ($pongMessages.Count -gt 0)
    ackCommands = @($ackMessages | ForEach-Object { [string]$_.parsed.cmd })
    errorMessages = @($errorMessages | ForEach-Object { [string]$_.parsed.msg })
    parseErrorCount = $parseErrors.Count
    rawLineCount = $rawOnly.Count
    frameCount = $frameMessages.Count
    filteredFrameCount = Get-FrameCountByPhasePrefix -Messages $parsedMessages -Prefix "filtered"
    rawFrameCount = Get-FrameCountByPhasePrefix -Messages $parsedMessages -Prefix "raw"
    sampleFrameIds = $sampleFrameIds
    initialFsd = $initialFsd
    fsdTest = [ordered]@{
        requested = [bool]$TestFsd
        restoreRequested = [bool]$RestoreAfterTest
        fsdOnAck = $fsdOnAck
        fsdAfterOn = $fsdAfterOn
        fsdOffAck = $fsdOffAck
        fsdAfterRestore = $fsdAfterRestore
    }
    possibilities = $matrix
    latestStatus = $latestStatus
}

[pscustomobject]@{
    summary = $summary
    diagnosis = $diagnosis
    messages = $allMessages
} | ConvertTo-Json -Depth 8
