# SDD 001 — Firmware ESP32 Energia e Mobilidade

**Estado:** PROPOSTO | **HW:** modelo/revisão D1 R32 pendente | **Contrato:** `esp32-energy`.

## Requisitos
- **EN-001:** inicializar GPIOs de atuadores em estado seguro antes de Wi-Fi/MQTT.
- **EN-002:** conectar Wi-Fi/broker local com reconexão não bloqueante e LWT.
- **EN-003:** ler BH1750 em lux validado e aplicar histerese: acender dois postes principais com baixa luz, apagar após nível alto estável.
- **EN-004:** aceitar modo manual de iluminação, sempre dentro da interface elétrica segura.
- **EN-005:** publicar telemetria de luminosidade, status de LEDs e, se instalado/calibrado, corrente/tensão INA219 do ramo medido.
- **EN-006:** gerenciar dois sinaleiros por máquina de estados, nunca verde conflitante.
- **EN-007:** solicitação de travessia física/digital com ciclo seguro e indicação sonora quando houver hardware aprovado.
- **EN-008:** receber comandos com allowlist/expiração/deduplicação de ID, retornar ACK apropriado.
- **EN-009:** manter iluminação e semáforos locais seguros sem MQTT e após reboot.
- **EN-010 (P1):** cancela com motor identificado, driver aprovado, limites de posição, testes de falha.

## Máquinas de estado
Iluminação: `AUTO / MANUAL / FAULT_SENSOR`; luminosidade com filtro, limiar de acendimento e apagamento separados e tempo de estabilidade calibrável. Sensor inválido → política de estado seguro documentada, alerta, sem oscilação.

Semáforos: `A_GREEN → A_YELLOW → ALL_RED_1 → B_GREEN → B_YELLOW → ALL_RED_2 → (PED_WALK quando pedido) → PED_CLEARANCE → ciclo`; todos os tempos parametrizados e estados de arranque seguros. A presença de LEDs já montados não garante resistores/driver.

## Aceite
1. Sem broker, duas regras locais continuam (luz automática e semáforo); telemetria volta após reconexão.
2. Três repetições de ciclo de trânsito sem verdes conflitantes, inclusive reinício durante transição.
3. Comando duplicado não repete transição que já foi consumida; ACK reenvia resultado.
4. Medição elétrica não afirma potência solar se INA219 estiver apenas no circuito principal.
5. Escuridão/luz em sensor BH1750 tem histerese estável sem pisca-pisca.

## Dependências e limitações
GPIO e corrente ainda não aprovados; cancela não entra em MVP antes de identificar motor; sinais de 5 V não entram diretamente no ESP32.
