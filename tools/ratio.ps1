# Owner: shared tooling, coordinated by Usman Khan.
# Count physical source lines after removing comments; no assembly padding.
param(
    [string]$SourceDirectory = (Join-Path $PSScriptRoot '..\src'),
    [string]$ReportPath = (Join-Path $PSScriptRoot '..\docs\RATIO.md'),
    [string]$TestDirectory = (Join-Path $PSScriptRoot '..\tests')
)
$ErrorActionPreference = 'Stop'
$rows = @()
$directories = @(@{ Path = $SourceDirectory; Scope = 'Production' }, @{ Path = $TestDirectory; Scope = 'Tests' })
foreach ($directory in $directories) {
if (!$directory.Path) { continue }
foreach ($file in (Get-ChildItem -LiteralPath $directory.Path -File | Where-Object { $_.Extension -in '.cpp', '.h' } | Sort-Object Name)) {
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
    $folder = Split-Path -Leaf ([IO.Path]::GetFullPath($directory.Path))
    $rows += [pscustomobject]@{ File = "$folder/$($file.Name)"; Assembly = $asm; Cpp = $cpp; Scope = $directory.Scope }
}
}
$production = @($rows | Where-Object Scope -eq 'Production')
$tests = @($rows | Where-Object Scope -eq 'Tests')
$combined = @($rows)
foreach ($summary in @(@{ Name = 'PRODUCTION (src only)'; Rows = $production }, @{ Name = 'TESTS ONLY'; Rows = $tests }, @{ Name = 'COMBINED (src + tests)'; Rows = $combined })) {
    $rows += [pscustomobject]@{ File = $summary.Name; Assembly = ($summary.Rows | Measure-Object Assembly -Sum).Sum; Cpp = ($summary.Rows | Measure-Object Cpp -Sum).Sum; Scope = 'Summary' }
}
$report = @('# Assembly / C++ ratio', '',
    'Production scope: src/*.cpp and src/*.h, including the standalone proof and guarded source.',
    'Combined scope: production plus tests/*.cpp and tests/*.h. Build scripts and compiler listings are excluded.',
    'The final project assembly requirement uses the PRODUCTION ratio from CODEX_CONTEXT.md section 11. Testing code is disclosed separately.',
    'Counts physical nonblank, noncomment lines. C++ braces and preprocessor lines count as C++.',
    'Assembly source lines (instructions and labels) inside __asm blocks and single-line __asm statements count as assembly.',
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
$report += @('', 'Match milestone: real recursive flood-fill, popping, and floating removal are implemented; preview behavior is unchanged.',
    'The combined percentage includes the two ESP-reading MOV lines in tests/match_tests.cpp as test assembly.',
    'The final minimum is 70% assembly, targeting 72-75%; further gameplay assembly is deferred.',
    'No pointless instructions were added to increase the percentage.')
$parent = Split-Path -Parent $ReportPath
if ($parent -and !(Test-Path -LiteralPath $parent)) { New-Item -ItemType Directory -Path $parent | Out-Null }
# Markdown reports use LF, matching the repository's .gitattributes policy.
[IO.File]::WriteAllText([IO.Path]::GetFullPath($ReportPath), ($report -join "`n") + "`n", [Text.UTF8Encoding]::new($false))
$report | ForEach-Object { Write-Output $_ }
