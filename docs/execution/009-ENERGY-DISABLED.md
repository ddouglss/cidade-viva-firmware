# Firmware energia: primeira imagem com saídas desabilitadas

Data: 2026-11-07. Usuário confirmou placa sem conexões externas e autorizou
implementar primeira versão de energia para gravar após reconectar USB.
SDD anterior ao código em `docs/sdd/005-energy-disabled/`.

## Resultado

- `apps/esp32-energy/include/DisabledEnergyHardware.hpp`: adaptador sem I/O;
  aplicação de intenção é no-op, lux inválido e botão ausente.
- `apps/esp32-energy/firmware/main.cpp`: entrada Arduino, clock millis, runtime
  real do app, identificação esp32-energy e diagnóstico serial a cada segundo.
- `apps/esp32-energy/platformio.ini`: ambiente independente energy-disabled,
  perfil genérico esp32dev já usado em boot/USB; nenhuma homologação de GPIO.
- `tests/controllers.cpp`: quatro verificações novas do adaptador/runtime;
  não simula GPIO físico nem afirma leitura de sensor.
- READMEs, SDD 005 e este registro: limites, comandos e evidências.
- Configurações IntelliSense da raiz firmware, app energia e pasta agregadora:
  banco gerado do PlatformIO para descobrir headers Arduino; fallback host para
  testes sem entrada. Banco local ignorado por `.gitignore`.

Sem configuração de parâmetros, iluminação MANUAL/OFF, trânsito ALL_RED lógico,
nenhum sensor/rede nem console de acionamento. Serial declara DISABLED/sem
medições: não representa intenção como saída física confirmada.

## Verificações executadas

- Windows PowerShell: `-NoProfile -ExecutionPolicy RemoteSigned -File tools/test-host.ps1`:
  exit 0, **128 PASS**, incluindo as 124 anteriores e quatro novas de energia.
  Saída sanitizada em `009-energy-host-tests.txt`.
- `./tools/pio.ps1 run --project-dir apps/esp32-energy -e energy-disabled`:
  exit 0, SUCCESS em 14.21 s, com o núcleo do app e entrada Arduino.
  RAM 21608/327680 bytes, flash reportada 270717/1310720 bytes.
  Artefato: `apps/esp32-energy/.pio/build/energy-disabled/firmware.bin`, 271088 bytes.
- `./tools/pio.ps1 run --project-dir apps/esp32-energy -e energy-disabled -t compiledb`:
  exit 0; inclui firmware/main.cpp e núcleo, zero fontes tests/ no build ESP32.
- Versões mantidas: PlatformIO 6.2.0, espressif32 6.13.0, Arduino package
  3.20017.241212+sha.dcc1105b, Xtensa GCC 8.4.0+2021r2-patch5, C++17,
  esptool package 2.41100.260830. Imagem DIO/40 MHz/4 MB como diagnóstico anterior.
- `git check-ignore` confirma firmware.bin e compile_commands.json ignorados.

## Estado físico e próximos passos

**Essa nova imagem não foi gravada nem executada na placa nesta entrega.**
Último programa gravado conhecido continua CIDADE_VIVA_USB_SMOKE v1. Depois de
reconectar a unidade destinada a energia e confirmar COM, gravar explicitamente
energy-disabled e capturar o marcador CIDADE_VIVA_ENERGY v0.1.0-disabled.

ED-001–ED-004 completas apenas para implementação/testes/build. EN-001/009 e E002
têm cobertura parcial. Tarefas compostas E/W permanecem abertas. Sem alteração de
água, GPIO, contratos compartilhados, backend/frontend ou pacote original;
sem requisito novo de energia/compra. Nenhum commit/push. Periféricos permanecem
PENDING_HARDWARE_VALIDATION; faltam mapa de sinais, drivers e parâmetros aprovados.
