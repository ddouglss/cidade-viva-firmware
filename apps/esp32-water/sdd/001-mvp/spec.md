# SDD 001 — Firmware ESP32 Água e Irrigação

**Estado:** PROPOSTO | **HW:** placa/bomba/boia ainda sem teste | **Contrato:** `esp32-water`.

## Requisitos
- **WA-001:** GPIO da bomba inicia OFF e mantém OFF diante de reset, falha de sensor, falta de água ou timeout.
- **WA-002:** ler boia e distinguir água presente, nível baixo e falha de circuito quando o hardware permitir teste de falha.
- **WA-003:** adquirir umidade por sensor capacitivo V1.2 em cartucho real **ou** aceitar entrada de simulação declarada; nunca confundir origens.
- **WA-004:** calibração seco/molhado do sensor real e publicação somente de percentual após calibração; registrar ADC bruto quando não calibrado.
- **WA-005:** `AUTO`: se leitura válida inferior ao limiar calibrado e boia OK, habilitar pulso de irrigação.
- **WA-006:** `MANUAL`: receber `PUMP_PULSE` somente dentro dos limites de boia, tempo máximo e cooldown.
- **WA-007:** qualquer comando duplicado (QoS 1), expirado ou não autorizado é recusado ou respondido sem repetir pulso.
- **WA-008:** estado local `IDLE / AUTO_ARMED / PUMPING / COOLDOWN / BLOCKED_LOW_WATER / FAULT_SENSOR / FAULT_TIMEOUT`.
- **WA-009:** telemetria/ACK MQTT com `origin`, `quality`, boia, modo, uptime, atuador, motivo de erro.
- **WA-010:** ficar seguro sem Wi-Fi, broker, API ou PC; teste de água real em bandeja estanque.

## Parâmetros de laboratório (não definitivos)
`maxPulseMs=3000`, `cooldownMs=15000`; calibrar em ensaio a vazão, a repetição, o limiar seco/molhado e limite absoluto de funcionamento. Nenhum limite do dashboard pode exceder a proteção local. Se não houver amostra real, `AUTO` só funciona como cenário demonstrativo claramente rotulado.

## Aceite
1. Nível baixo e bomba solicitada: `REJECTED / RESERVOIR_LOW`, motor não parte.
2. Bomba ligada e boia muda para baixo: desliga imediatamente e alerta.
3. Pulso excede máximo local: força OFF e `FAULT_TIMEOUT`.
4. Comando MQTT duplicado: uma única atuação física.
5. Reinício do ESP32 durante bomba: saída começa OFF.
6. Sem broker: proteção continua.
7. Sem amostra de substrato: painel distingue `SIMULATED_INPUT`; não há alegação de medição física.

## Fora de escopo
Sensor de vazão, medição de química do solo, agricultura real, controle por IA, bomba ligada direto ao GPIO.
