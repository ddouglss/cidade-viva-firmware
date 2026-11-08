# Instruções para Codex / agentes de implementação

Projeto Cidade Viva — Repositório `cidade-viva-firmware`. **Ler primeiro:** `README.md`, `docs/shared/01-VISAO-ESCOPO.md` e `docs/eletrica_e_solar/SEGURANCA-E-DIMENSIONAMENTO.md`; depois `apps/esp32-energy/{README.md,PINOUT-PENDENTE.md,sdd/001-mvp/{spec.md,plan.md,tasks.md}}`, `apps/esp32-water/{README.md,PINOUT-PENDENTE.md,sdd/001-mvp/{spec.md,plan.md,tasks.md}}`, `docs/execution/001-PACOTE-ANTIGRAVITY.md` e `docs/decisions/001-ORGANIZACAO-REPOSITORIOS-E-MAPA-DOCUMENTAL.md`.

## Organização de repositórios e divisão de papéis vigente
- **Papéis atualizados em 07/11/2026 por autorização explícita no chat de firmware:** Codex analisa, especifica, implementa e testa os dois firmwares neste repositório. Antigravity executa exclusivamente a fundação do backend em outro repositório. A divisão antiga nos documentos históricos foi superada para esta entrega; ver `docs/sdd/002-host-foundation/spec.md`. Não alterar backend/frontend nem contratos compartilhados sem revisão coordenada.
- **Três repositórios públicos independentes** (sem submodules, subtrees ou Git aninhado na pasta pai):
  - `https://github.com/ddouglss/cidade-viva-backend` (antigo `cidade-viva-platform` renomeado)
  - `https://github.com/ddouglss/cidade-viva-frontend`
  - `https://github.com/ddouglss/cidade-viva-firmware` (este repositório)
- Os arquivos em `docs/shared/` são cópias sincronizadas de referência de `00_COMPARTILHADO/` do pacote SDD v1.1. Qualquer alteração futura de contrato exige revisão coordenada e sincronização nos três repositórios antes de integrar.

## Regras obrigatórias
- SDD antes do código: não inventar escopo, endpoints, nomes de campos ou pinos fora das especificações.
- Stack: Angular/TypeScript; ASP.NET Core/C# em monólito modular e Clean Architecture; PostgreSQL; MQTT e SignalR; C++/PlatformIO nos ESP32.
- Preservar `schemaVersion=1.0` e tópicos em `docs/shared/02-CONTRATOS-MQTT-V1.md`.
- Tratar MQTT como transporte potencialmente duplicado; comandos críticos idempotentes; `202 Accepted` ≠ execução.
- Regra de segurança para bomba SEMPRE no firmware; manual não anula boia nem timeout.
- Separar medição real, simulação e cálculo; nunca representar comando GPIO como vazão hidráulica comprovada.
- Não inferir GPIO do nome Dn/Ax; consultar `apps/esp32-energy/PINOUT-PENDENTE.md`, `apps/esp32-water/PINOUT-PENDENTE.md` e a placa física.
- Não alimentar carga pelo GPIO; 3,3 V no sinal; alimentação de motor separada via driver; não conectar diretamente dois 5 V independentes.
- Não propor conexão direta de bateria ao painel solar, nem de 127/220 V expostos na maquete.
- Não requerer solda na instalação final sem registrar bloqueio e alternativa de bornes/conectores.
- Segredos só em ambiente, nunca commit; rede MQTT não exposta publicamente.
- Nunca marcar tarefas como concluídas sem código executado, testes e evidências.
- Se o usuário pedir só documentação, não modificar repositórios de código nem o documento-mestre sem autorização específica.

## Entrega para cada PR
Escopo e IDs de RF; arquivos alterados; testes executados; evidências do fluxo ponta a ponta; impactos em contrato, energia e compra; lista de pendências; tarefas marcadas.
