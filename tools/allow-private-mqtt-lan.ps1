$ErrorActionPreference = 'Stop'
$name = 'Cidade Viva MQTT bridge LAN'
if (-not (Get-NetFirewallRule -DisplayName $name -ErrorAction SilentlyContinue)) {
    New-NetFirewallRule -DisplayName $name -Direction Inbound -Action Allow -Protocol TCP `
        -LocalPort 1883 -LocalAddress 192.168.1.5 -RemoteAddress 192.168.1.0/24 `
        -Profile Private -Program "$env:WINDIR\System32\OpenSSH\ssh.exe" | Out-Null
}
Get-NetFirewallRule -DisplayName $name | Select-Object DisplayName,Enabled,Direction,Action
Write-Host 'Regra restrita ao Wi-Fi privado 192.168.1.0/24. Pressione Enter para fechar.'
Read-Host | Out-Null
