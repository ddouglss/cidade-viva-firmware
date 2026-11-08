# Testes C++ em host Windows

## VS Code e IntelliSense

Abra a pasta `cidade-viva-firmware` no VS Code (ou um app diretamente). A
configuração `.vscode/c_cpp_properties.json` existe nos três pontos de abertura
e seleciona **Windows host C++17**, Clang x64 e os includes locais apropriados.
`compilerPath` usa o mesmo caminho de `tools/test-host.ps1`, via USERPROFILE.
Os headers padrão (`cstdint`, `initializer_list`, `optional`) são descobertos
pelo compilador; não copie paths internos da biblioteca C++ para includePath.

Na raiz, `Ctrl+Shift+B` compila e testa ambos os apps. Em **Terminal → Run Task**
também há tarefas separadas para energia e água. Elas executam Windows PowerShell
com `-ExecutionPolicy RemoteSigned` apenas no processo da tarefa, sem mudar
políticas persistentes da máquina. Alternativa equivalente no terminal:

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File ./tools/test-host.ps1
```

A extensão Microsoft C/C++ já está instalada nesta máquina. Se diagnósticos
antigos persistirem após a configuração, use **Developer: Reload Window**;
se necessário, **C/C++: Reset IntelliSense Database**. Confirme a configuração
**Windows host C++17**. Uma pasta pai diferente não carrega automaticamente a
configuração de um filho: abra a raiz do firmware ou o app como pasta de workspace.
Na pasta agregadora local `cidade-viva`, também foi adicionada configuração que
encaminha as tarefas ao firmware e aponta o mesmo compilador no USERPROFILE.
Isso configura análise host, sem escolher board ou fingir validação ESP32.

Para a entrada Arduino do app energia (`energy-disabled`), existe agora um
banco de compilação gerado pelo PlatformIO. Gere com
`./tools/pio.ps1 run --project-dir apps/esp32-energy -e energy-disabled -t compiledb`.
O IntelliSense da raiz, do app energia e da pasta agregadora consulta esse banco
para arquivos ESP32; arquivos sem entrada (como testes host) mantêm o perfil
Clang host. A geração não seleciona GPIO nem grava a placa. Ver README do app.

O caminho atual é `%USERPROFILE%/.cidade-viva/toolchains/llvm-mingw-20260602-ucrt-x86_64`.
Ele substitui o antigo LOCALAPPDATA usado na primeira instalação: dados de apps
Windows podem ser redirecionados e ficar diferentes entre Codex e VS Code.

Referência: [configuração oficial do IntelliSense](https://code.visualstudio.com/docs/cpp/configure-intellisense).

## PlatformIO instalado (preparação para futuras placas)

PlatformIO Core 6.2.0 está isolado em `%USERPROFILE%/.platformio/penv`, sem
alterar PATH global. Da raiz, use o wrapper com saída UTF-8:

```powershell
./tools/pio.ps1 --version
./tools/pio.ps1 pkg list --global
```

Para reproduzir em uma máquina com Python 3.12 instalado (substitua `python`
pelo caminho do executável real quando houver apenas alias do Windows Store):

```powershell
python -m venv "$env:USERPROFILE/.platformio/penv"
& "$env:USERPROFILE/.platformio/penv/Scripts/python.exe" -m pip install --index-url https://pypi.org/simple -r ./tools/platformio-requirements.txt
./tools/pio.ps1 pkg install --global --platform 'platformio/espressif32@6.13.0'
./tools/pio.ps1 pkg install --global --tool 'platformio/framework-arduinoespressif32@3.20017.241212+sha.dcc1105b'
```

O ambiente instalado nesta máquina usa Python 3.12.14 existente no runtime local
do Codex como base. Se esse Python for removido, recrie o venv com Python próprio
da máquina; não é necessário mantê-lo no repositório.

As ferramentas são armazenadas em `%USERPROFILE%/.platformio`. Não foi escolhida
uma board, porta USB, pinagem ou configuração final de framework por app.
O pacote Arduino é pré-carregado na versão compatível com a plataforma instalada,
sem homologar essa combinação como build final do projeto. Versões e verificações
efetivas: `docs/execution/004-PLATFORMIO-SETUP.md`.
Não há build/upload físico autorizado por esta instalação; os dois apps continuam
**PENDING_HARDWARE_VALIDATION** e usam `host.json` para testes no computador.

Fontes oficiais: [instalação](https://docs.platformio.org/en/latest/core/installation/methods/pypi.html)
e [pacotes](https://docs.platformio.org/en/latest/core/userguide/pkg/cmd_install.html).

Cada app usa seu próprio `host.json`, fontes, headers e testes. Não há bibliotecas
externas nem dependência de um app no outro. Configuração física de PlatformIO,
framework, board, serial e upload continuam PENDING_HARDWARE_VALIDATION.

## Ferramenta fixada

LLVM-MinGW **20260602 UCRT x86_64**, Clang **22.1.7**, C++17.
Fonte primária: <https://github.com/mstorsjo/llvm-mingw/releases/tag/20260602>.
Arquivo: `llvm-mingw-20260602-ucrt-x86_64.zip`.
SHA-256 publicado pela API de assets do mantenedor e conferido localmente:
`3de3eda9377bbaf35f8c9001f190380f63b8ee981fa55d3ae9d7cce7c6ad7c70`.

Baixar esse asset da release, conferir com `Get-FileHash -Algorithm SHA256` e
extrair em `$env:USERPROFILE/.cidade-viva/toolchains` (não dentro do Git).
O ZIP contém a pasta `llvm-mingw-20260602-ucrt-x86_64`. Não alterar PATH global.
Alternativa: passar caminho absoluto de `clang++.exe` com `-Compiler`.
Outro compilador/versão requer registrar nova validação; não é prova da versão fixada.

Da raiz do repositório, em PowerShell:

```powershell
./tools/test-host.ps1
./tools/test-host.ps1 -App esp32-water
./tools/test-host.ps1 -App esp32-energy
```

O runner não baixa nada: falha se compilador faltar; compila cada app isoladamente
com `-std=c++17 -Wall -Wextra -Wpedantic -Werror -O2 -static` e executa testes
sem sleeps. Qualquer falha de compilação/teste encerra com erro. Artefatos em
`.build/<app>` são ignorados pelo Git. Ver evidências em
`docs/execution/003-LOCAL-CONTROLLERS.md`; a entrega anterior permanece registrada
em `docs/execution/002-HOST-FOUNDATION.md`.

## Fronteiras para o futuro hardware

`water::PumpSafety` recebe estado de boia e relógio; retorna intenção lógica.
O adaptador futuro deve iniciar driver OFF antes de qualquer rede, avaliar
proteções antes das rotinas não bloqueantes e aplicar intenção ao driver.
Não existe watchdog físico nesta fatia: se a avaliação parar, esta lógica pura
não executa; latência máxima do loop e proteção contra travamento exigem revisão
e testes em hardware. Polaridade/falha de cabo dependem do circuito aprovado.

`energy::OutputIntent` representa intenções. `TrafficController` agora executa o
ciclo lógico parametrizado, mantendo veículos vermelhos durante travessia.
`EnergyRuntime` e `WaterRuntime` integram controladores com interfaces de hardware;
testes com fakes verificam arranque e ordem de aplicação, sem validar GPIO real.
Boot lógico all-red precisa de homologação elétrica.

## Configuração e integração local

Não usar fixtures dos testes como configuração operacional. Fornecer parâmetros
aprovados às estruturas `LightConfig`, `TrafficConfig`, `AutoConfig` e
`SoilCalibration`. Configuração ausente impede a automação dependente. Iluminação
e irrigação iniciam em MANUAL/OFF; trânsito sem configuração fica all-red.
Política para lux inválido e buzzer é escolha explícita do perfil futuro.

Os runtimes recebem interfaces por referência, sem conhecer Arduino, GPIO, Wi-Fi
ou MQTT. `begin()` aplica estado inicial antes de sensores; `tick()` é chamado
pelo futuro loop não bloqueante. Métodos de controle são **locais**, sem
autenticação/expiração/deduplicação; não conectar diretamente a um comando MQTT.
O futuro dispatcher deve validar contrato/segurança e persistir deduplicação
antes de poder acionar esses métodos.

A lógica depende de clock monotônico uint32, com intervalos menores que meia
faixa, e de adaptadores não bloqueantes. Um `tick()` parado não é watchdog físico.
Não há tentativa de recuperar falha retida via operação remota não especificada.

