# Sensor capacitivo: ADC bruto G34 com bomba desabilitada

2026-11-07. Usuário confirmou sensor aceita 3,3 V e montagem GND/3V3/G34
conforme instrução, autorizando execução dos testes. Compatibilidade/alimentação
é confirmação do usuário, não datasheet de fabricante identificado ou medição.
Foto corrigida não foi inspecionada nesta etapa. Entrada G34 é ADC1_CH6.

Perfil explícito water-soil34-test independente das imagens anteriores: G34 INPUT
analógico, resolução 12 bits, atenuação ADC_11db da versão Arduino instalada.
Não usar INPUT_PULLUP no ADC, não converter para tensão/percentual, nem inventar
calibração. Amostra diagnóstica a cada 1000 ms. Implementar ISoilSensor físico
retornando SoilReading::physical(raw, now) sem pontos de calibração.
Reportar ADC_COUNT, origin=PHYSICAL_SENSOR (aquisição no ADC), calibration=NONE,
humidityPct=UNAVAILABLE; não implica sensor saudável ou umidade física calibrada.
Saturação/entrada flutuante não permite diagnosticar sensor desconectado.

Preservar diagnóstico de contato da boia G27 INPUT_PULLUP com filtro de 50 ms,
sem mapear para nível. Domínio continua reservoir=UNKNOWN e DisabledPumpDriver;
não há GPIO de bomba, Wi-Fi/MQTT, AUTO ou atuação mesmo com leitura seca.
Boia levantada=CLOSED/abaixada=OPEN foi relatado pelo usuário; mapeamento de
segurança operacional e instalação hidráulica não entram neste teste de solo.

Aceite: suíte host existente de leitura bruta sem percentual, compilação dos três
perfis água, identidade confirmada antes do upload autorizado, boot e captura de
amostras brutas. Variação com substrato/qualidade/calibração dependem de ensaio
posterior. Nenhum requisito integral de sensor/AUTO/segurança de bomba concluído.
