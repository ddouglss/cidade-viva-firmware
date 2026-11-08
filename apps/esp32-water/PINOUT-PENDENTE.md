# ESP32-02 — pinout NÃO aprovado

## Monitor integrado, sem GPIO de bomba

2026-11-07: water-monitor usa os mesmos G27 e G34 dos testes. G27 foi integrado
ao supervisor conforme orientação relatada: CLOSED estável indica PRESENT,
OPEN indica LOW, fechamento indeterminado UNKNOWN. Abertura bloqueia na próxima
avaliação. Driver da bomba continua desabilitado, nenhum GPIO de potência definido.
Montagem definitiva/driver/corrente/proteção continuam pendentes; não representa
aprovação de circuito hidráulico. Ver SDD 010/entrega 015.

## Exceção delimitada: ADC do sensor de solo

2026-11-07: usuário confirmou alimentação compatível com 3,3 V e montagem
sensor GND→GND, VCC→3V3 e AOUT→G34. Informação de compatibilidade é relato,
não identificação inequívoca do fabricante/datasheet ou medição de saída.
Perfil water-soil34-test usa GPIO34 INPUT/ADC1_CH6, 12 bits e ADC_11db.
Captura: oito amostras brutas 3409–3446; percentual/calibração ausentes.
Não homologa umidade, montagem final ou bomba. Ver SDD 009/entrega 014.

## Exceção delimitada: leitura de contato da boia em bancada

2026-11-07: usuário confirmou boia entre G27 (GPIO27, serigrafia fotografada)
e GND. Perfil water-float27-test usa INPUT_PULLUP e reporta OPEN/CLOSED com
estabilidade de 50 ms de diagnóstico. Serial confirmou CLOSED na posição atual.
Não há mapeamento para água presente/baixa, nem diagnóstico de cabo rompido
distinto de contato aberto. Driver da bomba continua desabilitado sem GPIO.
Isso não aprova pinout final, montagem hidráulica ou circuito de segurança.
Ver SDD 008 e entrega 013. Falta usuário movimentar e caracterizar as posições.

**Bloqueante:** identificação da placa, driver MOSFET, bomba e sensores.

| Sinal | Interface pretendida | Regra |
|---|---|---|
| SENSOR_SOIL | ADC1 analógico | no máximo 3,3 V no GPIO, calibrar seco/molhado em substrato real |
| FLOAT_LOW_WATER | Entrada digital | fail-safe: falha de cabo ou nível baixo → bomba desligada |
| PUMP_GATE | GPIO digital → módulo MOSFET | módulo compatível com 3,3 V, bomba alimentada pelo ramo protegido |
| SENSOR_FLOW | Não existe no MVP | sem prova de vazão em telemetria |

Não ligar bomba em GPIO ou via protoboard final. Escolha de diodo de proteção e capacitores depende de modelo real do motor/driver. Não usar entradas ADC2 com Wi-Fi em ESP32 clássico.
