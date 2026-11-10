param(
    [string]$LanAddress = '192.168.1.5',
    [string]$Server = '159.195.240.96'
)

$ErrorActionPreference = 'Stop'
$key = Join-Path $env:USERPROFILE '.ssh/cidade_viva_mqtt_bridge_ed25519'
if (-not (Test-Path -LiteralPath $key)) { throw 'Chave SSH da ponte nao encontrada.' }
if (-not (Get-NetIPAddress -AddressFamily IPv4 -IPAddress $LanAddress -ErrorAction SilentlyContinue)) {
    throw "O endereco $LanAddress nao pertence a este PC. Confira o Wi-Fi antes de iniciar a ponte."
}

while ($true) {
    $arguments = @(
        '-i', $key, '-o', 'BatchMode=yes', '-o', 'ExitOnForwardFailure=yes',
        '-o', 'ServerAliveInterval=30', '-o', 'ServerAliveCountMax=3',
        '-o', 'ConnectTimeout=8', '-N', '-L',
        "${LanAddress}:1883:127.0.0.1:1883", "cv_mqtt_bridge@$Server"
    )
    $child = Start-Process -FilePath 'ssh.exe' -ArgumentList $arguments -WindowStyle Hidden -PassThru
    try {
        while (-not $child.HasExited) {
            Start-Sleep -Seconds 10
            $listener = Get-NetTCPConnection -OwningProcess $child.Id -LocalAddress $LanAddress `
                -LocalPort 1883 -ErrorAction SilentlyContinue
            if (-not $listener -or -not (Test-NetConnection $LanAddress -Port 1883 -InformationLevel Quiet)) {
                break
            }
            $child.Refresh()
        }
    } finally {
        if (-not $child.HasExited) { Stop-Process -Id $child.Id -Force }
    }
    Start-Sleep -Seconds 5
}
