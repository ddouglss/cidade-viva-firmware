# Forward arguments to the isolated PlatformIO installation without changing PATH.
$pioPython = Join-Path $env:USERPROFILE '.platformio/penv/Scripts/python.exe'
if (!(Test-Path -LiteralPath $pioPython -PathType Leaf)) {
    throw 'PlatformIO is missing. See tools/README.md for installation instructions.'
}
& $pioPython -X utf8 -m platformio @args
if ($LASTEXITCODE -ne 0) { throw "PlatformIO exited with code $LASTEXITCODE" }
