# Energia: gravação e boot da versão desabilitada

Data: 2026-11-07. Usuário informou unidade reconectada e autorizou testar/gravar.
CP210x com status OK na COM5; consulta esptool confirmou mesma identidade física
da unidade usada no diagnóstico anterior (MAC omitido), D0WD-V3 rev. 3.1.

## Executado

`./tools/pio.ps1 run --project-dir apps/esp32-energy -e energy-disabled --target
upload --upload-port COM5`: exit 0, SUCCESS em 18.54 s. Imagem de 271088 bytes,
programa gravado a partir de 0x10000 e hash verificado pelo esptool. Bootloader e
partições pelo fluxo normal PlatformIO; sem erase_flash integral. Substituiu o
diagnóstico CIDADE_VIVA_USB_SMOKE. Backup anterior da unidade continua preservado
em `.build/usb-smoke/rom-chunks-new-connection/flash-original.bin`, ignorado.

Helper de captura estendido com perfil explícito `energy-disabled`, mantendo
perfil `smoke` como padrão. Verifica marcador próprio e flags DISABLED/NONE em
cada heartbeat. Python do venv PlatformIO:

```text
tools/esp32-smoke/capture_serial.py --port COM5 --profile energy-disabled --seconds 12 --reset
```

Exit 0, porta fechada ao terminar. Resultado real:

```text
CIDADE_VIVA_ENERGY v0.1.0-disabled
deviceId=esp32-energy externalOutputs=DISABLED
hardware=PENDING_HARDWARE_VALIDATION sensors=NOT_CONFIGURED
chip=ESP32-D0WD-V3 cpuMHz=240 flashBytes=4194304
ENERGY_ALIVE uptimeMs=1000 externalOutputs=DISABLED lightMode=MANUAL lightIntent=OFF trafficIntent=ALL_RED measurements=NONE
CAPTURE_RESULT profile=energy-disabled boot_marker=True heartbeats=12 disabled_status_ok=True
```

Captura completa em `010-energy-serial-output.txt`; upload local em
`.build/energy-disabled/upload.log`. Não foi necessário repetir os 128 testes
host da entrega 009: lógica e entrada Arduino não mudaram nesta gravação.

## Limites

Validado reset/boot/execução serial do app energia com adaptador desabilitado,
não GPIO ou circuitos externos. ALL_RED/OFF são intenções lógicas. Sem sensores,
medições, rede/MQTT ou controle de cargas. A porta foi liberada para monitor do
usuário. Sem commit/push, alterações de água/contratos/outros repositórios.
ED-005 concluída; E/W compostas e periféricos PENDING_HARDWARE_VALIDATION.
