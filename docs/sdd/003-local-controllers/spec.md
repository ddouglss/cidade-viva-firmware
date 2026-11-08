# Controladores locais completos em host

Data: 2026-11-07. Autorização: completar lógica sem hardware, mantendo regras
indefinidas pendentes. Complementa SDD 001/002, sem modificar contratos ou pinouts.

## Requisitos e fronteira

- EN-003/004: iluminação AUTO/MANUAL, histerese e estabilidade temporal.
- EN-006/007/009: sequência da SDD 001, travessia e debounce; sem rede.
- WA-003/004: representação explícita de origem/qualidade, ADC bruto e conversão
  opcional por dois pontos fornecidos, sem afirmar umidade volumétrica absoluta.
- WA-005/006/008/010: irrigação local AUTO/MANUAL sobre PumpSafety.
- EN-001/WA-001: interfaces de hardware e execução testada com fakes; não há GPIO.

Nenhum valor operacional novo: tempos de trânsito/debounce, limiares/estabilidade
de luz, timeout de leitura e calibração devem ser fornecidos explicitamente.
Ausência/invalidade impede iniciar a automação correspondente. Valores numéricos
dos testes são fixtures sintéticas, não sugestão de calibração ou montagem.

## Iluminação

Boot MANUAL/OFF. AUTO requer configuração válida: lux de acendimento menor que
lux de apagamento, ambos finitos/não negativos; tempo de estabilidade positivo
menor que meia faixa uint32; maxAgeMs positivo na mesma faixa; política explícita
para sensor inválido (OFF, ON ou manter última intenção). Não há fallback escolhido
para montagem. Leitura inválida/stale/NaN/infinita aplica a política e zera candidato.
Valores estritamente abaixo/acima dos limiares propõem ON/OFF; igualdade ou faixa
de histerese mantém saída e cancela candidato. Candidato exige observações válidas
contínuas pela janela configurada. MANUAL ignora sensor, mas não habilita GPIO.
Troca de modo cancela candidato; entrar AUTO exige nova janela válida.

## Trânsito e travessia

Boot ALL_RED_START. Configuração exige tempo positivo para cada estado. Sequência:
ALL_RED_START → A_GREEN → A_YELLOW → ALL_RED_1 → B_GREEN → B_YELLOW →
ALL_RED_2 → (PED_WALK se solicitado → PED_CLEARANCE) → A_GREEN.
Saídas de pedestres são intenção local, não campos MQTT. Durante WALK/CLEARANCE
ambos veículos RED. Buzzer depende de política explícita na configuração
(habilitado durante WALK ou desabilitado), sem inventar padrão sonoro.
Uma avaliação avança no máximo uma fase e inicia seu tempo no instante observado,
sem saltar amarelos/all-red para recuperar atraso. Pedido fica coalescido em um
bool; pedidos durante WALK/CLEARANCE são considerados atendidos pelo ciclo atual.
Debounce lógico recebe pressed (polaridade já traduzida pelo futuro adaptador),
exige estabilidade positiva; botão pressionado no boot não gera evento até soltar
e pressionar novamente. Pulso/evento não implementa deduplicação MQTT.

## Umidade e irrigação

Conversão física: raw dentro da faixa ADC fornecida e entre referências dry/wet;
referências distintas, dentro da faixa fornecida. Percentual = 100*(raw-dry)/(wet-dry),
com ambas polaridades possíveis. Fora de calibração retorna INVALID sem percentual;
sem calibração preserva somente raw e origem física. Essa transformação é ferramenta
de calibração relativa, cuja metodologia/valores físicos seguem pendentes.
Simulação usa factory separada e SIMULATED_INPUT; não aceita NaN ou fora de 0..100.
Leituras carregam tempo monotônico de aquisição; stale não autoriza AUTO.

Boot MANUAL/OFF. AUTO exige limiar explícito finito em [0,100], idade máxima e
pulso positivo abaixo do máximo local (evita timeout normal da política SDD 002).
AUTO avalia boia/timeout antes de sensor. Leitura válida abaixo do limiar e segurança
IDLE inicia pulso; leitura inválida/stale interrompe AUTO e inibe novos pulsos.
Pulso válido pode terminar mesmo se umidade subir; novos pulsos exigem seca na
avaliação seguinte, após cooldown. Leitura válida ainda fresca pode sustentar nova
decisão após cooldown; parâmetros de idade devem ser aprovados antes do hardware.
MANUAL só inicia por solicitação explícita e nunca ignora boia/tempo/cooldown.
Troca real de modo cancela pulso e mantém cooldown. Mesma seleção não reinicia nada.
Timeout continua retido; ACK_FAULT/recuperação operacional continuam pendentes.

## Adaptadores e aceite

Interfaces pequenas por app: clock/sensores/driver. Runtime inicia OFF/all-red antes
de ler sensores; cada tick processa segurança e aplica intenção, sem espera/rede.
Fakes comprovam ordem de chamadas, não polaridade/latência/proteção elétrica.
Relógio modular uint32 como SDD 002. Testar fronteiras, wrap, inválidos, modos,
três ciclos sem conflito, pedidos/debounce, interrupção, cooldown e reset.

## Pendências explícitas

EN-002/005/008, WA-007/009: MQTT, JSON/status/ACK completos, allowlist normativa,
autorização, expiração/relógio UTC e deduplicação persistente exigem revisão conjunta;
não criar implementações parciais que aparentem proteger comando remoto.
Sem board/PlatformIO físico, driver real, INA219, cancela, Wi-Fi, upload ou hardware.
Nenhum requisito inteiro EN/WA fica concluído só por teste host.
