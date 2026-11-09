# Firmware ESP32-02 — hidráulica e irrigação

**Tecnologia:** ESP32 D1 R32 Wi-Fi/Bluetooth a confirmar, C++/PlatformIO, MQTT v1.  
**SDD:** `sdd/001-mvp/{spec,plan,tasks}.md`.

Atuação: bomba AM174 (se homologada), boia de nível mínimo e eventual sensor capacitivo de umidade em cartucho real. A horta visual é **artificial** com circulação de água real e retorno estanque. É permitido modo demonstrativo `SIMULATED_INPUT` rotulado, mas não afirmar irrigação comandada por umidade física quando não houver amostra real.

Da raiz: `./tools/test-host.ps1 -App esp32-water`.
Ver `../../docs/sdd/003-local-controllers/spec.md` e `../../tools/README.md`.

| Componente | Responsabilidade |
|---|---|
| `PumpSafety` | boia fail-safe, limite local, cooldown e timeout retido |
| `SoilReading` | ADC bruto, conversão relativa opcional, origem e validade temporal |
| `IrrigationController` | AUTO/MANUAL e prioridade das proteções locais |
| `WaterRuntime` | boot OFF, segurança antes da leitura de solo e aplicação das intenções |

Fornecer `AutoConfig` e calibração aprovados; somente `LabLimits` possui os
parâmetros provisórios já especificados de 3000/15000 ms. Conversão relativa por
dois pontos não comprova umidade volumétrica: valores e método físico requerem
ensaio. A factory `SoilReading::simulated` mantém a origem separada.
`IClock`, `IFloatSwitch`, `ISoilSensor` e `IPumpDriver` são exercitados por fakes.
Valores das fixtures em `tests/controllers.cpp` são sintéticos.
Hardware **PENDING_HARDWARE_VALIDATION**. Sem MQTT/deduplicação, GPIO ou vazão.
Recuperação de `FAULT_TIMEOUT`/`ACK_FAULT` continua pendente de regra operacional.

## Primeira imagem física: water-disabled

Segunda unidade destinada à água: D0WD-V3 rev. 3.1, flash 4 MB, CP210x, identificada
como diferente da unidade energia mesmo reutilizando COM5. Ambiente PlatformIO
independente `water-disabled`, perfil genérico esp32dev só para boot/USB.
Entrada `firmware/main.cpp` integra WaterRuntime com adaptadores sem GPIO:
boia UNKNOWN, sensor de solo inválido e driver de bomba desabilitado.

Gravado e testado por serial em 2026-11-07: `CIDADE_VIVA_WATER v0.1.0-disabled`,
WATER_ALIVE, pumpRequested=OFF, pumpOutput=DISABLED, state=FAULT_SENSOR.
**FAULT_SENSOR é esperado neste perfil**, pois não existe boia configurada;
não significa defeito identificado na placa. Não conectar bomba diretamente.

Da raiz firmware:

```powershell
./tools/pio.ps1 run --project-dir apps/esp32-water -e water-disabled
./tools/pio.ps1 run --project-dir apps/esp32-water -e water-disabled -t compiledb
./tools/pio.ps1 run --project-dir apps/esp32-water -e water-disabled -t upload --upload-port COM5
./tools/pio.ps1 device monitor --port COM5 --baud 115200
```

Confirmar unidade/porta antes de upload: COM5 também foi usada pela energia.
Porta COM sozinha não identifica a placa. Programação é independente; instalar
water-disabled em uma unidade substitui o programa anterior daquela unidade.
Nenhum GPIO de bomba/boia/solo foi escolhido e nenhum ensaio hidráulico foi feito.

## Teste atual da boia (somente contato)

Perfil explícito `water-float27-test` gravado na unidade água: boia entre G27 e
GND, leitura INPUT_PULLUP. HIGH=OPEN, LOW=CLOSED; SETTLING durante 50 ms de
estabilidade diagnóstica. O domínio continua reservoir=UNKNOWN e a bomba fica
desabilitada independentemente do contato. Ainda não foi definido qual posição
significa água suficiente. Cabo rompido não é distinguível de contato aberto.

Na raiz firmware:

```powershell
./tools/pio.ps1 run --project-dir apps/esp32-water -e water-float27-test -t upload --upload-port COM5
./tools/pio.ps1 device monitor --port COM5 --baud 115200
```

Movimentar somente a peça flutuante, sem alterar fios energizados, e observar
OPEN/CLOSED. Bomba deve permanecer desconectada. Desligar USB antes de modificar
a montagem. Perfil padrão water-disabled permanece sem GPIO de aplicação.
Ver entrega 013 para estado realmente capturado.

## Teste atual: sensor capacitivo G34 + contato da boia G27

Perfil explícito `water-soil34-test` gravado após confirmação do usuário de
compatibilidade com 3,3 V e montagem GND/3V3/G34. Lê ADC1 GPIO34 em 12 bits,
ADC_11db, uma amostra bruta por segundo; não converte para tensão ou percentual.
Reutiliza SoilReading::physical sem calibração: somente raw, sem relativePct.
Entrada flutuante/desconexão não é diagnosticada apenas pelo valor ADC.

Preserva leitura OPEN/CLOSED do G27, mas nível do reservatório continua UNKNOWN;
sem GPIO de bomba, rede ou AUTO. Capture inicial: raw 3409–3446, boia OPEN,
bomba DISABLED. Falta observar resposta em substrato e calibrar método/valores.

Na raiz do firmware:

```powershell
./tools/pio.ps1 run --project-dir apps/esp32-water -e water-soil34-test -t upload --upload-port COM5
./tools/pio.ps1 device monitor --port COM5 --baud 115200
```

Mensagens SOIL34 mostram raw em ADC_COUNT, calibration=NONE e
humidityPct=UNAVAILABLE. Só a sonda comprida entra no substrato; componentes,
conector, ESP32, USB e emendas permanecem secos. Bomba segue desconectada.
Não alterar fios com USB conectado nem alimentar o GPIO em 5 V.

## Versão integrada atual: water-monitor

`CIDADE_VIVA_WATER v0.2.0-pump-disabled` gravado na unidade água (entrega 015).
Integra boia G27 ao supervisor: abertura bloqueia imediatamente na próxima
avaliação; fechamento exige 50 ms estáveis para indicar PRESENT, transição indica
UNKNOWN. Orientação é a relatada pelo usuário; instalação definitiva ainda
pendente. Cabo aberto bloqueia; curto não é distinguível de água presente.

Serial WATER_STATUS reúne indicação de reservatório, estado do controlador,
pedido de bomba e ADC bruto G34. Na captura real: LOW/BLOCKED_LOW_WATER,
pumpRequested=OFF, pumpOutput=DISABLED. Não há saída elétrica de bomba nem console
para acioná-la. AUTO não foi habilitado sem calibração/limiares; Wi-Fi/MQTT
também não estão implementados/configurados. Os limites locais existentes são
laboratoriais (3000/15000 ms), não ensaio físico da bomba.

Na raiz firmware:

```powershell
./tools/pio.ps1 run --project-dir apps/esp32-water -e water-monitor -t upload --upload-port COM5
./tools/pio.ps1 device monitor --port COM5 --baud 115200
```

Perfil padrão ainda é water-disabled. Habilitar potência exige especificar e
validar fonte/corrente, acionamento/proteção do motor e GPIO real antes de trocar
o DisabledPumpDriver. Nenhuma confirmação de vazão é inferida do software.

## Teste de bancada Wi-Fi

Conecte somente a unidade da água por USB e confirme a porta. Os comandos
abaixo partem da raiz do firmware e usam COM5 como exemplo. Feche o monitor
serial antes de rodar o teste.

```powershell
./tools/pio.ps1 run --project-dir apps/esp32-water -e water-wifi-test -t upload --upload-port COM5
& "$env:USERPROFILE/.platformio/penv/Scripts/python.exe" ./tools/test-water-wifi.py --port COM5
```

Informe o SSID de 2,4 GHz e digite a senha no prompt oculto. O script envia os
dados pelo USB. A placa guarda a configuração Wi-Fi em RAM. O reset inicia uma
nova tentativa com novo envio das credenciais.

O resultado esperado é `ESP32 conectado ao Wi-Fi. IP: ...`. Execute o comando
`ping` mostrado pelo script para testar o alcance a partir do notebook. O
perfil continua monitorando boia e ADC bruto com bomba desabilitada.

Veja o escopo e o estado da validação em
[SDD 011](../../docs/sdd/011-water-wifi-test/spec.md).
