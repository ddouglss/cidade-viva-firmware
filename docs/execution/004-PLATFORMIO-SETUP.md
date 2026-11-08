# Preparação das ferramentas ESP32

Data: 2026-11-07. Pedido explícito: instalar ferramentas agora, sem ESP32 disponível;
testes/gravação físicos serão feitos quando as placas chegarem.

## Instalado e verificado

Instalação isolada em `%USERPROFILE%/.platformio/penv`, baseada no Python 3.12.14
existente no runtime local do Codex. Pacotes de desenvolvimento em
`%USERPROFILE%/.platformio/{platforms,packages}`. Sem alteração do PATH global.
Se o Python base do Codex for removido, recriar o venv com um Python da máquina.

| Componente | Versão efetiva |
|---|---|
| PlatformIO Core | 6.2.0 |
| plataforma Espressif32 | 6.13.0 |
| compilador Xtensa ESP32 | GCC 8.4.0 / 2021r2-patch5 |
| pacote tool-esptoolpy | 2.41100.260830 |
| esptool.py | 4.11.0 |
| framework-arduinoespressif32 | 3.20017.241212+sha.dcc1105b |

PlatformIO veio do PyPI oficial com versão explícita; a plataforma e os pacotes
vieram do registro oficial. O framework foi resolvido pela faixa `~3.20017.0`
indicada no manifesto da plataforma instalada e sua versão exata foi registrada.
Não foi escolhido o pacote Arduino mais recente sem verificar compatibilidade.
As versões finais por app serão homologadas no primeiro build do hardware real.

## Comandos executados e resultados

- `python-base -m venv .../.platformio/penv`: exit 0.
- `python-do-venv -m pip install --index-url https://pypi.org/simple platformio==6.2.0`: exit 0.
- `platformio pkg install --global --platform platformio/espressif32@6.13.0`: exit 0;
  baixou plataforma, compilador e ferramenta de gravação com suas dependências.
- `./tools/pio.ps1 pkg install --global --tool platformio/framework-arduinoespressif32@~3.20017.0`:
  exit 0, framework instalado.
- `./tools/pio.ps1 --version`: exit 0, PlatformIO Core 6.2.0.
- `./tools/pio.ps1 pkg list --global`: exit 0, versões da tabela confirmadas.
- `python-do-venv -m pip check`: exit 0, `No broken requirements found.`
- `xtensa-esp32-elf-g++ --version`: exit 0, GCC 8.4.0.
- `python-do-venv .../tool-esptoolpy/esptool.py version`: exit 0, esptool 4.11.0;
  sem conexão serial ou tentativa de gravação.

A primeira listagem direta pelo executável PlatformIO encontrou erro de encoding
CP1252 ao imprimir a árvore Unicode no terminal capturado. O wrapper usa
`python -X utf8 -m platformio`, restrito ao processo; a mesma listagem passou
após a correção. Nenhuma configuração global de encoding foi alterada.

## Arquivos e limites

- `tools/pio.ps1`: acesso à CLI isolada com UTF-8, sem exigir PATH global.
- `tools/platformio-requirements.txt`: versões Python instaladas, incluindo
  dependências da ferramenta de gravação, registradas com `pip freeze`.
- `tools/README.md`: reprodução e comandos de verificação.

Não houve alteração da lógica de firmware, contratos, pinouts, backend/frontend
ou pacote SDD original. Não foi instalada extensão de IDE nem driver USB; driver
depende da interface USB identificada na placa. Sem board/porta/GPIO selecionados,
PlatformIO.ini físico, build ESP32, upload ou energização. Instalar os pacotes não
homologa compatibilidade com uma placa ainda não identificada.

Os testes host da lógica continuam registrados na entrega 003 (124 PASS); não
foram repetidos nesta instalação, pois a lógica e seu runner não mudaram.
E002/W003 continuam parciais; nenhuma tarefa composta de hardware foi concluída.
Estado de ambos os apps: **PENDING_HARDWARE_VALIDATION**. Sem commit/push/PR.

Fontes primárias consultadas:
- [Instalação PlatformIO em ambiente Python](https://docs.platformio.org/en/latest/core/installation/methods/pypi.html).
- [Gerenciamento de pacotes](https://docs.platformio.org/en/latest/core/userguide/pkg/cmd_install.html).
- [Releases oficiais Espressif32](https://github.com/platformio/platform-espressif32/releases).
