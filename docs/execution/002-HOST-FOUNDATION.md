# Entrega: fundação host dos dois firmwares

Data: 2026-11-07. Checkout: `cidade-viva-firmware`.
Remote conferido: <https://github.com/ddouglss/cidade-viva-firmware>.
Autorização deste chat permite implementação de firmware pelo Codex; backend
permanece com Antigravity. SDD anterior à implementação em
`docs/sdd/002-host-foundation/{spec,plan,tasks}.md`.

## Alterações concretas

| Arquivos | Resultado |
|---|---|
| `apps/esp32-water/{host.json,include/PumpSafety.hpp,src/PumpSafety.cpp,tests/main.cpp}` | supervisor puro, boia fail-safe, pulso, cooldown e falha retida |
| `apps/esp32-energy/{host.json,include/EnergySafety.hpp,src/EnergySafety.cpp,tests/main.cpp}` | intenção inicial all-red/OFF e detector de conflito |
| `tools/{test-host.ps1,README.md}` | runner único que compila projetos separados, ferramenta fixada e instruções |
| `.gitignore` | ignora artefatos e arquivos de ambiente |
| `AGENTS.md`, README raiz e READMEs dos apps | atualiza autorização, execução e estado real |
| `docs/sdd/002-host-foundation/*`, este relatório e `002-host-test-output.txt` | escopo, decisões e evidências |

Não há HEAD/commit inicial; documentos anteriores já estavam untracked. Assim,
`git diff` convencional não apresenta o conteúdo desta entrega. Não foi feito
stage, commit, push, PR ou operação destrutiva. A tabela acima descreve o delta
da sessão; arquivos anteriores foram preservados, com apenas as atualizações
documentais explícitas acima.

## Ferramentas e testes realmente executados

- Inspeção: nenhum PlatformIO, CMake, g++, clang++ ou cl no PATH. Visual Studio
  possui VsDevCmd, mas nenhum cl.exe foi encontrado nas instalações consultadas.
  Python no PATH era alias WindowsApps. Docker CLI encontrado, daemon indisponível;
  não foi iniciado nem alterado. Nenhum desses itens foi usado para testar C++.
- Compilador portátil obtido do mantenedor, fora do Git: LLVM-MinGW 20260602,
  Clang 22.1.7; SHA-256 conferido e igual ao digest publicado pela API oficial.
  URL/hash e instalação reproduzível em `tools/README.md`.
- `./tools/test-host.ps1`: **exit 0**, compilação C++17 com warnings como erros,
  **11 verificações energia + 25 água = 36 PASS**. Saída sanitizada preservada
  em `002-host-test-output.txt`. Reexecutado para capturar evidência, novamente exit 0.
- Água: boot/reset lógico OFF, bloqueios LOW/UNKNOWN, interrupções em execução,
  recusa de duração inválida, pedido ocupado sem extensão, fronteiras de pulso e
  cooldown, recuperação sem partida automática, timeout exato/atrasado/retido,
  wrap uint32 e parâmetros inválidos.
- Energia: arranque lógico, matriz 3x3 de cores e recriação sem verde conflitante.
- SHA-256: oito documentos em `docs/shared` idênticos aos respectivos originais
  em `00_COMPARTILHADO` após a entrega.
- `git check-ignore` confirmou ambos os executáveis de teste ignorados.
  `git diff --check` exit 0, mas sem cobertura dos arquivos untracked;
  não equivale a uma revisão completa do delta. `git rev-parse --verify HEAD`
  falhou como esperado por ausência de commit inicial.

## Decisões e limites

Parâmetros de água existentes permanecem laboratoriais: 3000/15000 ms. Duração
igual ao máximo é aceita, mas atingir o limite gera FAULT_TIMEOUT por política
conservadora documentada antes do código. Cooldown inicia na observação de OFF.
Falha de timeout fica retida até reconstrução; recuperação operacional/ACK_FAULT
não foi implementada. Estado UNKNOWN não inventa como o circuito identifica
falha: essa capacidade depende do hardware. A lógica depende de avaliações
regulares com relógio monotônico; não protege sozinha contra CPU/loop travado.

Nenhuma alteração de contrato, schemaVersion/tópicos, energia ou compra. Nenhum
campo de telemetria novo, simulação MQTT ou dependência de outro repositório.
Nenhum percentual de umidade, limiar, fase/tempo semafórico ou medição inventado.

## Cobertura e pendências

HF-001–HF-004 concluídas apenas para a fatia host. E001–E012 e W001–W010
continuam abertas. W004/W005/W009, E008/E010 e requisitos EN/WA relacionados têm
cobertura parcial descrita na SDD 002; não constituem aceite dos requisitos inteiros.

**Não executados:** build PlatformIO/ESP32, serial, upload, GPIO, Wi-Fi/MQTT,
deduplicação/expiração, backend/dashboard, sensores, watchdog físico, ensaio
elétrico/hidráulico, vazão e ciclo semafórico. Estado físico de ambos os apps:
**PENDING_HARDWARE_VALIDATION**. Nenhuma evidência ponta a ponta física existe.

Próxima fatia: especificar recuperação de falha e política temporal de água;
definir tempos/fallbacks de energia antes de implementar controladores completos.
Fechar TTL, deduplicação após reboot e payloads/ACK em revisão coordenada antes
de rede. Board/GPIO/driver só depois de modelo, fotos, datasheets e revisão elétrica.
