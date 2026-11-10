param([string]$Port = 'COM5', [string]$BrokerIp = '192.168.1.5')
& (Join-Path $PSScriptRoot 'configure-energy-this-pc.ps1') -Port $Port -DeviceId esp32-water -BrokerIp $BrokerIp
