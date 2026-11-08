# Entrega — controladores locais dos dois dispositivos

Data: 2026-11-07. Repositório: `cidade-viva-firmware`.
Escopo confirmado pelo usuário: completar a lógica sem hardware, mantendo GPIO,
gravação e regras ainda indefinidas pendentes. SDD escrita antes da implementação:
`docs/sdd/003-local-controllers/{spec,plan,tasks}.md`.

## Resultado e arquivos desta sessão

| App/arquivos | Implementação |
|---|---|
| energia: `include/LightController.hpp`, `src/LightController.cpp` | modos AUTO/MANUAL, histerese, estabilidade, stale e fallback configurável |
| energia: `include/TrafficController.hpp`, `src/TrafficController.cpp` | fases da SDD 001, travessia protegida, buzzer condicional e CrossingButton com debounce |
| energia: `include/EnergyRuntime.hpp`, `src/EnergyRuntime.cpp` | interfaces clock/hardware e aplicação de estado inicial antes das entradas |
| água: `include/SoilReading.hpp`, `src/SoilReading.cpp` | ADC bruto, origem explícita, conversão relativa opcional e validade temporal |
| água: `include/IrrigationController.hpp`, `src/IrrigationController.cpp` | AUTO/MANUAL, sensor inválido/stale, prioridade de boia/timeout/cooldown |
| água: `include/WaterRuntime.hpp`, `src/WaterRuntime.cpp` | interfaces, boot OFF, segurança antes da leitura do solo e nova leitura de boia antes de acionar |
| água: `include/PumpSafety.hpp`, `src/PumpSafety.cpp` | cancelamento local seguro, novo resultado interno WrongMode e estado não reconhecido tratado como falha |
| ambos: `tests/controllers.cpp`, `tests/main.cpp`, `host.json` | cenários novos e inclusão das fontes nos projetos independentes |
| README raiz, READMEs dos apps, `tools/README.md` | execução, configuração e fronteira do que está implementado |
| SDD 003, este relatório, `003-local-test-output.txt` | decisões, rastreabilidade e evidência sanitizada |

O checkout continua sem commit inicial e com documentação anterior untracked;
`git diff` convencional não representa o delta. Não houve stage/commit/push/PR.
Nenhum arquivo do backend/frontend ou pacote original foi editado.

## Validação realmente executada

Comando na raiz: `./tools/test-host.ps1` — **exit 0**.
LLVM-MinGW 20260602 / Clang 22.1.7, mesma ferramenta fixada na entrega 002;
`-std=c++17 -Wall -Wextra -Wpedantic -Werror -O2 -static`.

**124 verificações PASS: 53 energia e 71 água**, incluindo as 36 verificações
anteriores. A primeira execução da ampliação passou; após reforçar os cenários
de reboot, leitura stale em execução e boia inválida, a execução final também
passou. Saída final sanitizada em `003-local-test-output.txt`.

- Energia: limiares/igualdade/histerese, estabilidade interrompida, modos,
  leitura infinita/stale, recuperação, fallback explícito e wrap uint32.
- Trânsito: sequência exata, três ciclos completos, pedestres com ambos sinais
  vermelhos, pedidos coalescidos, atraso sem saltar fases, buzzer configurável,
  reboot nas fases, wrap e invariante em 20.000 atualizações determinísticas
  com atrasos e solicitações variadas (não contam como 20.000 testes separados).
- Botão: boot pressionado, bounce, pressão prolongada, rearmar e wrap.
- Água: calibração nas duas polaridades e endpoints, ADC sem calibração,
  entradas inválidas/NaN/infinitas, origem de simulação e idade exata/wrap.
- Irrigação: AUTO/MANUAL, limiar seco estrito, boia inválida/baixa, cooldown,
  leitura inválida/stale durante pulso, troca de modo, timeout retido e wrap.
- Runtimes com fakes: estado seguro antes das entradas, aplicação de saída,
  segurança antes de leitura de solo, boia que muda durante aquisição e reboot.
- Revisão: cancelamento tardio passou a detectar timeout antes de retornar
  cooldown, evitando que troca de modo esconda o limite local ultrapassado.
- Integridade SHA-256: **16 arquivos idênticos à origem** — oito documentos
  compartilhados, dois pinouts pendentes e os seis arquivos SDD 001 dos apps.
- `git diff --check` exit 0; como arquivos são untracked, isso não é evidência
  de uma revisão de whitespace/diff de todo o conteúdo novo.

## Cobertura parcial dos requisitos e tarefas originais

| IDs | Disponível em host | Ainda necessário para aceite integral |
|---|---|---|
| EN-001/009; E002/E010 | boot lógico e runtime sem dependência de rede | GPIO, PlatformIO, serial, ensaio e perda de rede real |
| EN-003/004; E005/E006 | LightController completo e parametrizado | BH1750, driver e perfil aprovado de limiares/fallback |
| EN-006/007; E007/E008/E010 | trânsito, travessia, botão e intenção de buzzer | sinaleiros, identificação elétrica e tempos aprovados |
| WA-001/002/010; W003/W004/W009 | supervisor e ordem segura no runtime | GPIO/driver, circuito de boia, watchdog e ensaios |
| WA-003/004; W006 | representação/conversão relativa e origens separadas | calibração física, aquisição ADC e publicação normativa |
| WA-005/006/008; W005 | IrrigationController completo local | perfil aprovado, comando remoto validado e hardware |
| EN-002/005/008; E003/E004/E009 | não implementados nesta fatia | rede, payloads, ACK/deduplicação/expiração e INA219 |
| WA-007/009; W007/W010 | não implementados nesta fatia | contrato coordenado, MQTT, backend/dashboard e evidências |

LC-001–LC-007 concluídas para escopo host. Nenhuma tarefa composta E/W recebeu
`[x]` e nenhum requisito integral foi declarado concluído só com testes host.

## Decisões, impactos e pendências

Nenhum novo limiar, GPIO ou tempo operacional foi escolhido. Parâmetros novos
devem ser fornecidos explicitamente; sem configuração, automações dependentes
não iniciam. Valores dos testes são sintéticos. Conversão de ADC é relativa a
dois pontos fornecidos, sem equivaler a teor volumétrico real de água no solo.

Não há alteração de schemaVersion=1.0, tópicos ou documentos compartilhados.
Nenhuma dependência de biblioteca/repositório, simulador MQTT, compra, mudança
de alimentação, serviço externo ou instalação global foi adicionada.

**PENDING_HARDWARE_VALIDATION:** sem board, ambiente PlatformIO físico, GPIO,
upload, sensores reais, circulação/vazão, medição elétrica/solar ou E2E remoto.
Runtimes dependem de adaptadores não bloqueantes e de avaliações regulares;
não substituem watchdog/proteção de CPU travada. Boot all-red e OFF são intenções
lógicas que exigem aplicação elétrica correta e revisão de polaridade.

Antes de integração física: identificar peças/pinagem, homologar parâmetros,
fallback de lux, buzzer, fonte/driver e proteção local contra travamentos.
Antes de rede: fechar payloads, TTL/relógio confiável, autorização, deduplicação
persistente e semântica de ACK/recuperação FAULT_TIMEOUT em revisão coordenada.
Métodos locais do runtime não podem ser expostos diretamente a pacotes MQTT.
