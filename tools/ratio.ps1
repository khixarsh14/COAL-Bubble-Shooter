# Owner: shared tooling, coordinated by Usman Khan.
# Count physical source lines after removing comments; no assembly padding.
param(
    [string]$SourceDirectory = (Join-Path $PSScriptRoot '..\src'),
    [string]$ReportPath = (Join-Path $PSScriptRoot '..\docs\RATIO.md')
)
$ErrorActionPreference = 'Stop'
$rows = @()
foreach ($file in (Get-ChildItem -LiteralPath $SourceDirectory -File | Where-Object { $_.Extension -in '.cpp', '.h' } | Sort-Object Name)) {
    $asm = 0; $cpp = 0; $depth = 0; $pending = $false; $blockComment = $false
    foreach ($line in [IO.File]::ReadAllLines($file.FullName)) {
        # Strip C/C++ comments while keeping quoted strings from becoming tokens.
        $clean = [Text.StringBuilder]::new()
        $quote = [char]0
        for ($i = 0; $i -lt $line.Length; $i++) {
            $c = $line[$i]
            $next = if ($i + 1 -lt $line.Length) { $line[$i + 1] } else { [char]0 }
            if ($blockComment) {
                if ($c -eq '*' -and $next -eq '/') { $blockComment = $false; $i++ }
                continue
            }
            if ($quote -ne [char]0) {
                if ($c -eq '\') { $i++; continue }
                if ($c -eq $quote) { $quote = [char]0 }
                continue
            }
            if ($c -eq '/' -and $next -eq '/') { break }
            if ($c -eq '/' -and $next -eq '*') {
                $blockComment = $true; $i++; [void]$clean.Append(' '); continue
            }
            if ($c -eq '"' -or $c -eq "'") {
                $quote = $c; [void]$clean.Append('LITERAL'); continue
            }
            # MASM semicolons start comments only in assembly code.
            if ($c -eq ';' -and ($depth -gt 0 -or $clean.ToString() -match '\b__asm\b')) { break }
            [void]$clean.Append($c)
        }
        $code = $clean.ToString().Trim()
        if (!$code) { continue }
        $isAssembly = $false
        if ($depth -gt 0) {
            $isAssembly = ($code -replace '[{}]', '').Trim().Length -gt 0
            $depth += ([regex]::Matches($code, '\{').Count - [regex]::Matches($code, '\}').Count)
        } elseif ($pending -or $code -match '\b__asm\b') {
            $body = ($code -replace '\b__asm\b', '').Trim()
            if (!$body) { $pending = $true }
            elseif ($body.StartsWith('{')) {
                $pending = $false
                $depth = [regex]::Matches($body, '\{').Count - [regex]::Matches($body, '\}').Count
                $isAssembly = ($body -replace '[{}]', '').Trim().Length -gt 0
            } else {
                $pending = $false
                $isAssembly = $true
            }
        }
        if ($isAssembly) { $asm++ } else { $cpp++ }
    }
    if ($depth -ne 0 -or $pending -or $blockComment) { throw "Unclosed assembly block or comment in $($file.Name)" }
    $rows += [pscustomobject]@{ File = $file.Name; Assembly = $asm; Cpp = $cpp }
}
$totalAsm = ($rows | Measure-Object Assembly -Sum).Sum
$totalCpp = ($rows | Measure-Object Cpp -Sum).Sum
$rows += [pscustomobject]@{ File = 'TOTAL'; Assembly = $totalAsm; Cpp = $totalCpp }
$report = @('# Assembly / C++ ratio', '',
    'Scope: all src/*.cpp and src/*.h, including the separate assembly proof (even code behind preprocessor guards).',
    'Counts physical nonblank, noncomment lines. C++ braces and preprocessor lines count as C++.',
    'Assembly instructions inside __asm blocks and single-line __asm statements count as assembly.',
    'The __asm block wrapper and brace-only lines count as C++ syntax, never as instructions.',
    'A line containing an assembly instruction counts once as assembly. Use one instruction per line.',
    'C/C++ comments, assembly semicolon comments, and tokens inside string/character literals are ignored.',
    'This scanner supports ordinary C++ quoted literals, not raw or multiline string literals; the project uses neither.', '',
    '| File | Assembly lines | C++ lines | Assembly % | C++ % |',
    '| --- | ---: | ---: | ---: | ---: |')
foreach ($row in $rows) {
    $total = $row.Assembly + $row.Cpp
    $a = 0.0; $c = 0.0
    if ($total -gt 0) { $a = 100.0 * $row.Assembly / $total; $c = 100.0 * $row.Cpp / $total }
    $report += ('| {0} | {1} | {2} | {3:F2} | {4:F2} |' -f $row.File, $row.Assembly, $row.Cpp, $a, $c)
}
$report += @('', 'Milestone 1 is a C++ foundation with only three real assembly instructions.',
    'This is below the final minimum of 70% and target of 72-75%. Gameplay assembly is deferred.',
    'No pointless instructions were added to increase the percentage.')
$parent = Split-Path -Parent $ReportPath
if ($parent -and !(Test-Path -LiteralPath $parent)) { New-Item -ItemType Directory -Path $parent | Out-Null }
$report | Set-Content -LiteralPath $ReportPath -Encoding UTF8
$report | ForEach-Object { Write-Output $_ }
