param(
    [string]$LanAddress = '',
    [string]$Server = '159.195.240.96'
)

$ErrorActionPreference = 'Stop'
$key = Join-Path $env:USERPROFILE '.ssh/cidade_viva_mqtt_bridge_ed25519'
if (-not (Test-Path -LiteralPath $key)) { throw 'Chave SSH da ponte nao encontrada.' }
while ($true) {
    $current = Get-NetIPAddress -AddressFamily IPv4 -InterfaceAlias 'Wi-Fi' -ErrorAction SilentlyContinue |
        Where-Object { $_.IPAddress -match '^(10\.|192\.168\.|172\.(1[6-9]|2[0-9]|3[01])\.)' } |
        Select-Object -First 1
    if (-not $current -or ($LanAddress -and $current.IPAddress -ne $LanAddress)) {
        Start-Sleep -Seconds 5
        continue
    }
    $bindAddress = $current.IPAddress
    $arguments = @(
        '-i', $key, '-o', 'BatchMode=yes', '-o', 'ExitOnForwardFailure=yes',
        '-o', 'ServerAliveInterval=30', '-o', 'ServerAliveCountMax=3',
        '-o', 'ConnectTimeout=8', '-N', '-L',
        "${bindAddress}:1883:127.0.0.1:1883", "cv_mqtt_bridge@$Server"
    )
    $child = Start-Process -FilePath 'ssh.exe' -ArgumentList $arguments -WindowStyle Hidden -PassThru
    try {
        while (-not $child.HasExited) {
            Start-Sleep -Seconds 10
            $activeAddress = (Get-NetIPAddress -AddressFamily IPv4 -InterfaceAlias 'Wi-Fi' -ErrorAction SilentlyContinue |
                Select-Object -First 1).IPAddress
            $listener = Get-NetTCPConnection -OwningProcess $child.Id -LocalAddress $bindAddress `
                -LocalPort 1883 -ErrorAction SilentlyContinue
            if ($activeAddress -ne $bindAddress -or -not $listener) {
                break
            }
            $child.Refresh()
        }
    } finally {
        if (-not $child.HasExited) { Stop-Process -Id $child.Id -Force }
    }
    Start-Sleep -Seconds 5
}
