$ErrorActionPreference = 'Stop'
$name = 'Cidade Viva MQTT bridge LAN'
$wifi = Get-NetIPAddress -AddressFamily IPv4 -InterfaceAlias 'Wi-Fi' |
    Where-Object { $_.IPAddress -match '^(10\.|192\.168\.|172\.(1[6-9]|2[0-9]|3[01])\.)' } |
    Select-Object -First 1
if (-not $wifi) { throw 'Wi-Fi privado nao encontrado neste PC.' }
$address = $wifi.IPAddress
$bytes = [System.Net.IPAddress]::Parse($address).GetAddressBytes()
$bits = [int]$wifi.PrefixLength
for ($i = 0; $i -lt 4; $i++) {
    $maskBits = [Math]::Min(8, [Math]::Max(0, $bits - 8 * $i))
    $mask = if ($maskBits -eq 0) { 0 } else { (256 - [Math]::Pow(2, 8 - $maskBits)) }
    $bytes[$i] = [byte]($bytes[$i] -band [int]$mask)
}
$subnet = "$([System.Net.IPAddress]::new($bytes))/$bits"
$rule = Get-NetFirewallRule -DisplayName $name -ErrorAction SilentlyContinue
if (-not $rule) {
    New-NetFirewallRule -DisplayName $name -Direction Inbound -Action Allow -Protocol TCP `
        -LocalPort 1883 -LocalAddress $address -RemoteAddress $subnet `
        -Profile Any -Program "$env:WINDIR\System32\OpenSSH\ssh.exe" | Out-Null
} else {
    $rule | Get-NetFirewallAddressFilter | Set-NetFirewallAddressFilter -LocalAddress $address -RemoteAddress $subnet | Out-Null
    $rule | Set-NetFirewallRule -Profile Any | Out-Null
}
Get-NetFirewallRule -DisplayName $name | Select-Object DisplayName,Enabled,Direction,Action
Write-Host "Regra restrita ao Wi-Fi privado $subnet em $address. Pressione Enter para fechar."
Read-Host | Out-Null
