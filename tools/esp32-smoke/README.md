# Diagnóstico USB da primeira placa

Somente serial, sem sensores/cargas/GPIO de aplicação ou rede. Não é o firmware
energy/water. Unidade observada: ESP32-D0WD-V3 rev. 3.1, 4 MB, CP2102, COM5 no
primeiro ensaio. Perfil genérico esp32dev serve apenas para este diagnóstico;
pinout e identidade final do fabricante permanecem pendentes.

Da raiz do repositório:

```powershell
./tools/pio.ps1 run --project-dir tools/esp32-smoke -e usb-smoke
./tools/pio.ps1 run --project-dir tools/esp32-smoke -e usb-smoke -t upload --upload-port COM5
./tools/pio.ps1 device monitor --port COM5 --baud 115200
```

Upload substitui o programa anterior: usar somente na placa separada, sem
componentes externos, após preservar backup quando necessário. Não repetir
upload para tentar resolver erro de leitura sem investigar comunicação.
Monitor: Ctrl+C fecha a porta. Só um programa pode usar a COM por vez.

Esperado: `CIDADE_VIVA_USB_SMOKE v1` no boot e `USB_SMOKE_ALIVE uptimeMs=...`
a cada segundo. Pressionar RST/EN repete o marcador. Não pisca LED nem aciona
qualquer equipamento. A configuração não aprova GPIO ou conexão elétrica.

Fonte do perfil: https://docs.platformio.org/en/stable/boards/espressif32/esp32dev.html
SDD: `../../docs/sdd/004-usb-smoke/`.

## Preservação antes de gravar

Se a leitura longa pelo stub falhar, o helper abaixo usa somente leitura ROM,
em blocos de 64 KiB e com tentativas limitadas. Destino deve ser uma pasta nova
sob `.build`, ignorada pelo Git. Arquivos podem conter dados do programa anterior;
não publicar. Não modifica a flash, mas reinicia a placa ao consultar cada bloco.

```powershell
& "$env:USERPROFILE/.platformio/penv/Scripts/python.exe" tools/esp32-smoke/backup_flash.py --port COM5 --output-dir .build/usb-smoke/novo-backup
```

Somente `BACKUP_COMPLETE` e `flash-original.bin` com 4194304 bytes indicam que
todos os blocos foram lidos. `flash-incomplete.tmp` não é backup válido. O helper
não grava, restaura, apaga flash nem inicia upload automaticamente.
