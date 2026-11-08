# Primeira gravação e execução física confirmadas

Data: 2026-11-07. Continuação autorizada após usuário reconectar a placa.
Uma unidade CP210x na COM5, ESP32-D0WD-V3; sem atribuição definitiva energy/water.

## Preservação e comunicação

A leitura monolítica pelo stub voltou a não progredir e somente os processos
esptool dessa leitura foram encerrados. A leitura ROM em blocos de 64 KiB foi
concluída com sucesso para os 64 blocos, sem mensagens de retry nessa execução.

Comando: Python do venv PlatformIO, `tools/esp32-smoke/backup_flash.py --port COM5
--output-dir .build/usb-smoke/rom-chunks-new-connection`, exit 0.
Resultado: `BACKUP_COMPLETE bytes=4194304`.
Backup local ignorado: `.build/usb-smoke/rom-chunks-new-connection/flash-original.bin`.
SHA-256: `5E78B43921BC4908D2CA915C42A7A593BD76C70D93739F51C9AF58321E2955A2`.
O conteúdo não foi publicado, impresso nem incluído no Git. O helper novo usa
somente read_flash com tamanho/destino controlados e tentativas limitadas;
não restaura nem faz upload automaticamente.

Isso mostra sucesso dessa estratégia de leitura; não comprova que a leitura
monolítica ou todas as velocidades USB estejam estáveis.

## Upload e execução reais

Build já validado na entrega 007 foi reutilizado. Comando:
`./tools/pio.ps1 run --project-dir tools/esp32-smoke -e usb-smoke --target upload --upload-port COM5`.
Exit 0, `[SUCCESS] Took 26.91 seconds`; firmware 269280 bytes, gravação de app
a partir de 0x10000, hash verificado pelo esptool. Bootloader/partições também
gravados pelo fluxo normal PlatformIO. O programa anterior foi substituído
somente após backup; não foi executado erase_flash integral.

Captura: Python do venv, `tools/esp32-smoke/capture_serial.py --port COM5 --seconds
12 --reset`, exit 0. Reset via RTS, serial 115200, porta fechada ao terminar.
Trechos reais:

```text
CIDADE_VIVA_USB_SMOKE v1
DIAGNOSTIC_ONLY: external actuators and sensors NOT configured
chip=ESP32-D0WD-V3 revision=3 cpuMHz=240 flashBytes=4194304
No WiFi/MQTT; no application GPIO configured.
USB_SMOKE_ALIVE uptimeMs=1000
USB_SMOKE_ALIVE uptimeMs=12000
CAPTURE_RESULT boot_marker=True heartbeats=12
```

Saída serial completa sem identificadores de rede em `008-usb-serial-output.txt`.
Logs completos de upload/backup ficam em `.build/usb-smoke`, ignorados.
Arduino informa revisão principal 3; a consulta esptool anterior informou v3.1.

## Escopo comprovado e limites

USB-001–USB-004 concluídas para diagnóstico dessa unidade: build, backup, upload,
boot, reset e serial. Firmware atual é somente diagnóstico, não apps energy/water.
Não configurou GPIO de aplicação, sensor, carga, Wi-Fi/MQTT ou proteção de bomba.
Nenhuma prova de vazão, medição ou atuação física foi produzida.

E002/W003 continuam compostas/parciais; não se homologa LED/pinout só com serial.
Apps energy/water e pinouts continuam PENDING_HARDWARE_VALIDATION para periféricos.
Sem alterações de contrato, outros repositórios ou pacote original. Sem commit/push.

Próximo passo: identificar as duas unidades e seus papéis, validar componentes,
drivers e mapa de GPIO por sinal antes de conectar qualquer atuador.
