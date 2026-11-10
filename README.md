# Projeto 02 — Firmware IoT e integração elétrica (`cidade-viva-firmware`)

Dois firmwares independentes em C++/PlatformIO para ESP32 D1 R32 (placas a confirmar), com contratos MQTT v1 compartilhados (`docs/shared/02-CONTRATOS-MQTT-V1.md`). O circuito solar e a distribuição de energia exigem validação de engenharia própria (`docs/eletrica_e_solar/SEGURANCA-E-DIMENSIONAMENTO.md`).

## Troca de Wi-Fi após instalar na maquete
Os perfis `energy-telemetry` e `water-telemetry` preservam Wi-Fi/MQTT em NVS. Se o Wi-Fi ficar indisponível por 30 segundos, a placa abre `CidadeViva-Energy` ou `CidadeViva-Water` com senha WPA2 individual gerada no primeiro boot. Registre essa senha **antes** da instalação; ela é exibida somente pela serial USB como `CV_AP_PASSWORD` e não deve ser enviada por capturas de tela ou commit.

Conecte o notebook/celular à rede da placa, abra `http://192.168.4.1` e informe SSID, senha Wi-Fi de 2,4 GHz e IPv4 privado do notebook que executa a ponte MQTT. A credencial MQTT permanece gravada. O portal fecha quando a placa volta a conectar. A página hospedada em produção não acessa as portas COM locais. Redes com portal cativo ou isolamento entre clientes podem impedir a conexão; para demonstração, prefira um hotspot 2,4 GHz controlado por você.

## Fluxo de trabalho
1. Simular os dois firmwares e construir clientes MQTT antes de receber as placas.
2. Identificar serigrafia do modelo e mapear **GPIO real**, não inferir de pinos `D2/D3`.
3. Energizar um ESP32 por USB de dados; testar piscar LED com resistor e sensor separadamente.
4. Adicionar motor/bomba apenas depois de driver/proteção/fonte aprovados.
5. Testar desconexão, reinício, timeout, água baixa, bomba e semáforos; somente então integrar na base.

Não incluir tensão AC no interior da maquete. Circuitos da água e da eletrônica devem ter segregação física.

## Responsabilidade deste repositório
- Concentrar as especificações, pinouts e futuras implementações em C++/PlatformIO dos dois controladores físicos da maquete:
  - `apps/esp32-energy` (`deviceId: esp32-energy`): iluminação principal (2 postes 5 V via driver) com modo manual e automático por BH1750 com histerese, máquina de estados segura para 2 sinaleiros (sem verdes conflitantes), botão de travessia acessível com buzzer, medição INA219 no ramo instrumentado (P1) e cancela condicional (P1).
  - `apps/esp32-water` (`deviceId: esp32-water`): controle de bomba d'água via driver MOSFET com intertravamento físico obrigatório por boia de nível mínimo, timeout máximo de pulso, cooldown, leitura de umidade calibrada em cartucho de substrato real (`PHYSICAL_SENSOR`) ou modo demonstrativo rotulado (`SIMULATED_INPUT`).
- Manter a documentação de segurança elétrica e dimensionamento do ramo principal 5 V CC e do módulo solar autônomo (4 postes solares isolados eletricamente da fonte principal) em `docs/eletrica_e_solar/SEGURANCA-E-DIMENSIONAMENTO.md`.
- **Autoridade física:** os firmwares detêm a palavra final de segurança física; comandos manuais ou remotos nunca anulam boia baixa, timeout de bomba ou transições seguras de semáforo, e as proteções locais continuam operando mesmo sem Wi-Fi ou broker MQTT.

## Relação com os outros dois repositórios
O projeto Cidade Viva está organizado em três repositórios públicos independentes (sem submodules, subtrees ou Git aninhado):
1. **`cidade-viva-firmware` (este repositório — `https://github.com/ddouglss/cidade-viva-firmware`):** firmwares `esp32-energy` e `esp32-water`, controle de pinouts e regras elétricas/solares.
2. **`cidade-viva-backend` (`https://github.com/ddouglss/cidade-viva-backend`, antigo `cidade-viva-platform`):** API ASP.NET Core, PostgreSQL e broker MQTT local com o qual os dois ESP32 trocam telemetria, status/LWT, comandos e ACKs idempotentes.
3. **`cidade-viva-frontend` (`https://github.com/ddouglss/cidade-viva-frontend`):** dashboard Angular + TypeScript que exibe o estado reportado pelos dispositivos (via backend) e solicita comandos sujeitos à validação local do firmware.

Os documentos em `docs/shared/` são cópias sincronizadas de referência nos três repositórios; qualquer alteração de contrato exige revisão coordenada e sincronização antes da integração.

## Ordem de leitura recomendada
1. `AGENTS.md` — regras obrigatórias de segurança elétrica, hardware e SDD, divisão atual de papéis e critérios de PR.
2. `docs/shared/01-VISAO-ESCOPO.md`, `docs/shared/02-CONTRATOS-MQTT-V1.md` e `docs/shared/03-REGRAS-INTEGRACAO.md` — escopo P0/P1, contratos MQTT v1 e matriz de proteções locais.
3. `docs/shared/04-INVENTARIO-E-DECISOES-PENDENTES.md`, `docs/shared/05-ADRS.md`, `docs/shared/06-PLANO-VALIDACAO-END-TO-END.md` e `docs/eletrica_e_solar/SEGURANCA-E-DIMENSIONAMENTO.md` — bloqueios `BL-01` a `BL-09`, isolamento solar e requisitos de bancada (`docs/shared/08-ESPECIFICACAO-INTEGRADA-REFERENCIA-v1.0.md` é referência histórica v1.0).
4. `apps/esp32-energy/README.md`, `apps/esp32-energy/PINOUT-PENDENTE.md` e `apps/esp32-energy/sdd/001-mvp/{spec.md,plan.md,tasks.md}` — requisitos `EN-001` a `EN-010` e tarefas `E001` a `E012`.
5. `apps/esp32-water/README.md`, `apps/esp32-water/PINOUT-PENDENTE.md` e `apps/esp32-water/sdd/001-mvp/{spec.md,plan.md,tasks.md}` — requisitos `WA-001` a `WA-010` e tarefas `W001` a `W010`.
6. `docs/execution/001-PACOTE-ANTIGRAVITY.md` e `docs/decisions/001-ORGANIZACAO-REPOSITORIOS-E-MAPA-DOCUMENTAL.md` — plano de entregas e registro de organização/rastreabilidade.

## Estado real de implementação

- **Imagem atual da água:** `water-monitor`, `v0.2.0-pump-disabled`, gravada com boia G27 no supervisor e ADC bruto G34. Captura confirmou LOW/BLOCKED_LOW_WATER e bomba desabilitada; calibração/AUTO, potência e rede ainda pendentes (entrega 015). Perfis de teste anteriores abaixo são histórico/disponíveis, não a imagem atual.

- **Perfil atual na unidade água:** `water-soil34-test` gravado e capturado. ADC bruto G34 (sem calibração/percentual) e contato da boia G27, mantendo bomba sem GPIO/desabilitada e nível do domínio UNKNOWN. Ver entrega 014; perfis anteriores permanecem disponíveis, mas não são a imagem atual.

- **Teste atual da unidade água (COM5):** perfil explícito `water-float27-test` lê somente contato G27/GND. CLOSED observado por serial; posição e alternância OPEN/CLOSED aguardam usuário. Sem atribuir nível de água e sem GPIO de bomba. Perfil padrão water-disabled continua disponível. Ver SDD 008/entrega 013.

- **Teste atual da unidade de energia (COM5):** perfil explícito `energy-led26-test` gravado; somente GPIO26 alterna para um LED avulso com resistor 220 Ω, conforme montagem relatada pelo usuário. Captura serial confirmou HIGH/LOW; luz física ainda aguarda observação. Não é MAIN_LIGHT definitivo. O perfil padrão `energy-disabled` continua disponível para retornar ao estado sem GPIO. Ver SDD 006/entrega 011.
- **Núcleo local em host:** dois projetos C++17 independentes com configuração própria (`host.json`). Energia possui iluminação AUTO/MANUAL com histerese, trânsito/travessia e debounce; água possui calibração relativa/origem de leitura e irrigação AUTO/MANUAL sobre supervisor de bomba. Ambos possuem runtimes com interfaces de sensores/saídas testadas por fakes. Escopo atual: `docs/sdd/003-local-controllers/`; fundação histórica: `docs/sdd/002-host-foundation/`. Execução: `tools/README.md`; resultados atuais: `docs/execution/003-LOCAL-CONTROLLERS.md`.
- **Papéis atuais:** Codex implementa firmware neste chat; Antigravity mantém exclusivamente a fundação backend. Os registros antigos de divisão são históricos.
- **Energia no ESP32:** ambiente PlatformIO `energy-disabled` integra o núcleo à entrada Arduino com todas as saídas externas desabilitadas. Build, gravação e boot/reset/serial passaram na unidade D0WD-V3, 4 MB, COM5. Ela executa `CIDADE_VIVA_ENERGY v0.1.0-disabled`, substituindo o diagnóstico USB inicial. Ver `apps/esp32-energy/README.md`, SDD 005 e entregas 009/010.
- **Água no ESP32:** water-disabled compilado/gravado na segunda unidade, identidade distinta, com boot/serial confirmados (entrega 012). Bomba desabilitada e boia UNKNOWN bloqueando o controlador. Nenhum GPIO de água configurado.
- **Hardware e integração remota:** `PENDING_HARDWARE_VALIDATION` para sensores/cargas/pinout geral em ambos os apps. Perfis genéricos validam boot/USB das unidades, sem homologar o circuito final; energia tem apenas teste delimitado de LED G26. Não há controle MQTT/JSON, persistência de deduplicação ou ACK remoto implementados. Tempos/limiares novos não possuem valores operacionais padrão. Todas as tarefas compostas `E001` a `E012` e `W001` a `W010` permanecem pendentes (`[ ]`); cobertura parcial nas SDDs incrementais.
