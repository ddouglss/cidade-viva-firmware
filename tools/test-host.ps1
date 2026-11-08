param(
    [ValidateSet('all', 'esp32-energy', 'esp32-water')][string]$App = 'all',
    [string]$Compiler = (Join-Path $env:USERPROFILE '.cidade-viva/toolchains/llvm-mingw-20260602-ucrt-x86_64/bin/clang++.exe')
)
$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path $PSScriptRoot -Parent
if (!(Test-Path -LiteralPath $Compiler -PathType Leaf)) { throw "Compiler missing at '$Compiler'. See tools/README.md or supply -Compiler with its full path." }
& $Compiler --version
if ($LASTEXITCODE -ne 0) { throw 'Compiler version failed.' }
$apps = if ($App -eq 'all') { @('esp32-energy', 'esp32-water') } else { @($App) }
foreach ($name in $apps) {
    $appRoot = Join-Path $repoRoot "apps/$name"
    $config = Get-Content -LiteralPath (Join-Path $appRoot 'host.json') -Raw | ConvertFrom-Json
    $buildDir = Join-Path $repoRoot ".build/$name"
    New-Item -ItemType Directory -Force $buildDir | Out-Null
    $binary = Join-Path $buildDir 'tests.exe'
    $sources = @($config.sources | ForEach-Object { Join-Path $appRoot $_ })
    & $Compiler -std=c++17 -Wall -Wextra -Wpedantic -Werror -O2 -static '-I' (Join-Path $appRoot $config.include) @sources -o $binary
    if ($LASTEXITCODE -ne 0) { throw "Compilation failed: $name" }
    Write-Output "TEST APP: $name ($($config.hardwareStatus))"
    & $binary
    if ($LASTEXITCODE -ne 0) { throw "Tests failed: $name" }
}
