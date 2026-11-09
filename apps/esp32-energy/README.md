# Firmware ESP32-01 — energia, iluminação e mobilidade

**Tecnologia:** ESP32 D1 R32 Wi-Fi/Bluetooth (se confirmado), C++/PlatformIO, framework Arduino, MQTT v1.  
**SDD:** `sdd/001-mvp/{spec,plan,tasks}.md`.

Atuação: dois postes 5 V principais, BH1750, dois sinaleiros, botão acessível e buzzer. INA219 mede **somente o ramo onde for instalado**. Cancela fica P1 e condicionada ao motor/driver. Quatro postes solares funcionam em circuito autônomo e não são parte da comutação principal deste firmware no MVP.

Da raiz: `./tools/test-host.ps1 -App esp32-energy`.
Ver `../../docs/sdd/003-local-controllers/spec.md` e `../../tools/README.md`.

| Componente | Responsabilidade |
|---|---|
| `EnergySafety` | intenção inicial e detector de verdes conflitantes |
| `LightController` | AUTO/MANUAL, histerese, estabilidade e qualidade temporal |
| `TrafficController` | fases seguras parametrizadas e travessia |
| `CrossingButton` | debounce e evento único por pressão |
| `EnergyRuntime` | aplica boot antes de entradas e integra controladores às interfaces |

Fornecer `LightConfig`, `TrafficConfig` e debounce aprovados ao runtime; não há
perfil de montagem com limiares/tempos inventados. `IEnergyHardware` e `IClock`
devem ser implementados para a placa futura; hoje são exercitados por fakes.
Valores das fixtures em `tests/controllers.cpp` são sintéticos.
Hardware **PENDING_HARDWARE_VALIDATION**. Sem drivers de sensores, GPIO, rede,
INA219 ou cancela; o ciclo validado é exclusivamente lógico.

## Primeira imagem ESP32: saídas externas desabilitadas

`platformio.ini`, ambiente `energy-disabled`, compila `firmware/main.cpp` com o
núcleo real deste app. `DisabledEnergyHardware` não configura nem aciona GPIO;
não produz medições físicas. Perfil genérico esp32dev serve apenas para boot/USB
na unidade D0WD-V3 com 4 MB já consultada, não para aprovar o pinout.

Da raiz do firmware:

```powershell
./tools/pio.ps1 run --project-dir apps/esp32-energy -e energy-disabled
./tools/pio.ps1 run --project-dir apps/esp32-energy -e energy-disabled -t compiledb
```

O segundo comando gera os includes Arduino reais para o IntelliSense, sem
adicioná-los manualmente ao perfil host. O banco gerado é ignorado pelo Git.
Testes host seguem em `host.json`, sem compilar a entrada Arduino.

Quando a unidade estiver reconectada e a COM for confirmada, gravação explícita
(substitui o programa atual):

```powershell
./tools/pio.ps1 run --project-dir apps/esp32-energy -e energy-disabled -t upload --upload-port COM5
./tools/pio.ps1 device monitor --port COM5 --baud 115200
```

Marcador esperado: `CIDADE_VIVA_ENERGY v0.1.0-disabled`; depois `ENERGY_ALIVE`
com `externalOutputs=DISABLED`. `trafficIntent=ALL_RED` é intenção lógica, sem
LED vermelho físico. Não há console que habilite saídas ou simulação de sensor.
A imagem foi compilada, gravada e teve boot/reset/serial verificados na unidade
destinada à energia, COM5, em 2026-11-07. Captura real: entrega 010.
Nenhum parâmetro de produto ou GPIO novo foi escolhido.

## LED vermelho avulso de bancada (perfil separado)

`energy-led26-test` configura somente GPIO26: inicia LOW e alterna a cada 500 ms.
Montagem confirmada pelo usuário: G26 → resistor 220 Ω → anodo do LED vermelho;
catodo → GND. Esse teste não aciona postes/sinaleiros e não aprova pinout geral.
Nenhuma fonte externa, motor ou outro periférico faz parte dele. Desconectar USB
antes de mudar qualquer fio. Não usar esse perfil sem o circuito correto.

Gravado e serial HIGH/LOW verificada na COM5 (entrega 011); confirmação visual
do LED depende do usuário. Perfil padrão continua energy-disabled. Comandos
explícitos, na raiz firmware:

```powershell
./tools/pio.ps1 run --project-dir apps/esp32-energy -e energy-led26-test -t upload --upload-port COM5
./tools/pio.ps1 device monitor --port COM5 --baud 115200
```

Para parar o teste, desconectar USB; para voltar à imagem sem GPIO, gravar o
ambiente energy-disabled conforme comando anterior. A serial reporta pedido de
nível, não medição de luz. Arquivo de entrada: firmware/led26_main.cpp.

## Teste de bancada Wi-Fi

O perfil `energy-wifi-test` mantém as saídas de aplicação desabilitadas e
aguarda o SSID e a senha pelo USB. Na raiz do repositório, com a unidade
de energia na COM5 e o monitor serial fechado, execute:

```powershell
./tools/pio.ps1 run --project-dir apps/esp32-energy -e energy-wifi-test -t upload --upload-port COM5
& "$env:USERPROFILE/.platformio/penv/Scripts/python.exe" ./tools/test-water-wifi.py --port COM5
```

O script também atende a unidade de energia. Digite a senha no prompt
oculto e confira o IP mostrado após a associação. Reiniciar a placa
descarta as credenciais e permite um novo teste. Detalhes e evidências na
[SDD de bancada](../../docs/sdd/011-water-wifi-test/spec.md).
