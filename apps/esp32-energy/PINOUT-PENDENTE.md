# ESP32-01 — pinout NÃO aprovado

## Exceção delimitada: LED avulso de bancada

2026-11-07: GPIO26 (serigrafia G26 da unidade D0WD-V3) reservado somente para
teste isolado de um LED vermelho, resistor 220 ohms em série e retorno GND.
Usuário confirmou montagem e autorizou habilitar; não houve foto conclusiva do
circuito pronto nem medição. Ambiente explícito energy-led26-test foi gravado;
solicitações HIGH/LOW confirmadas por serial, observação luminosa ainda pendente.
Isso não aprova MAIN_LIGHT, semáforos, corrente de outras cargas ou pinout geral.
Ver SDD 006 e entrega 011. Perfil padrão energy-disabled não usa esse GPIO.

**Estado:** bloqueado para fiação final até fotografia, identificação de revisão da placa e teste na bancada.

| Sinal | Interface | Critério |
|---|---|---|
| BH1750 | I²C SDA/SCL | GPIOs reais da placa, alimentação 3,3 V e endereço verificados |
| INA219 | I²C compartilhado | endereço distinto, tensão de referência e ramo medido aprovados |
| MAIN_LIGHT | GPIO → driver 3,3 V | 2 postes principais, nunca carga grande direto no GPIO |
| TRAFFIC_A/B R/Y/G | 6 saídas ou driver/expansor | verificar corrente, lógica ativa e tipo dos dois sinaleiros |
| CROSSING_BUTTON | Entrada digital | pull-up/pull-down e debounce |
| ACCESSIBILITY_BUZZER | Driver ou GPIO conforme consumo | saída definida em estado seguro no boot |
| BARRIER | Reservado P1 | tipo de motor/servo e driver pendentes |

O D1 R32 expõe identificação tipo Arduino; Dn/Ax **não** equivale automaticamente a números GPIO. No ESP32 clássico ADC2 interfere com Wi-Fi; pinos de strapping devem ser evitados no boot se houver alternativa. A pinagem proposta no caderno v1.0 é apenas estudo, não ordem de ligação. Atualizar este documento para `PINOUT-APROVADO.md` após teste por sinal.
