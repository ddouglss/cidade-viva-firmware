param(
    [string]$Port = 'COM5',
    [ValidateSet('esp32-energy', 'esp32-water')][string]$DeviceId = 'esp32-energy',
    [string]$BrokerIp = '192.168.1.5'
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$python = Join-Path $env:USERPROFILE '.platformio/penv/Scripts/python.exe'
$secretPath = Join-Path $env:USERPROFILE "AppData/Roaming/CidadeViva/mqtt-$($DeviceId.Substring(6)).dpapi"
if (-not (Test-Path -LiteralPath $python)) { throw 'Python do PlatformIO nao encontrado.' }

Write-Host "Configure $DeviceId com o Wi-Fi 2.4 GHz usado neste PC."
$env:CV_WIFI_SSID = Read-Host 'Nome exato da rede (SSID)'
$wifiSecure = Read-Host 'Senha do Wi-Fi' -AsSecureString
$mqttSecure = if (Test-Path -LiteralPath $secretPath) {
    Get-Content -LiteralPath $secretPath | ConvertTo-SecureString
} else {
    Read-Host "Senha MQTT de $DeviceId (cole com Ctrl+V)" -AsSecureString
}
$wifiPtr = [Runtime.InteropServices.Marshal]::SecureStringToBSTR($wifiSecure)
$mqttPtr = [Runtime.InteropServices.Marshal]::SecureStringToBSTR($mqttSecure)
try {
    $env:CV_WIFI_PASSWORD = [Runtime.InteropServices.Marshal]::PtrToStringBSTR($wifiPtr)
    $env:CV_MQTT_PASSWORD = [Runtime.InteropServices.Marshal]::PtrToStringBSTR($mqttPtr)
    & $python (Join-Path $root 'tools/configure-private-mqtt.py') --port $Port --device-id $DeviceId --broker-ip $BrokerIp --from-env
    if ($LASTEXITCODE -ne 0) { throw 'A placa nao confirmou a configuracao.' }
} finally {
    [Runtime.InteropServices.Marshal]::ZeroFreeBSTR($wifiPtr)
    [Runtime.InteropServices.Marshal]::ZeroFreeBSTR($mqttPtr)
    Remove-Item Env:CV_WIFI_SSID,Env:CV_WIFI_PASSWORD,Env:CV_MQTT_PASSWORD -ErrorAction SilentlyContinue
}
