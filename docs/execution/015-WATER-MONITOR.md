# Integração do controlador e sensores água

2026-11-07. Usuário autorizou implementar/gravar restante. A parte independente
do circuito de potência foi executada; não foi tratado como autorização para
inventar calibração, corrente, fonte/driver/proteção ou payloads de rede ausentes.
SDD 010 escrita antes do código. Unidade água novamente identificada na COM5,
mesma identidade conhecida (MAC omitido), distinta da unidade energia.

## Delta e comportamento

FloatSafetyInput novo usa filtro de bancada de fechamento 50 ms existente;
abertura retorna Low na próxima chamada, sem aguardar debounce. Boot Unknown,
fechamento em transição Unknown e fechado estável Present. Orientação baseada
no relato levantada=CLOSED/abaixada=OPEN. Curto não detectável pelo contato simples.

firmware/monitor_main.cpp e ambiente explícito water-monitor conectam esse
adaptador físico ao WaterRuntime e ADC G34 ao SoilReading sem calibração.
DisabledPumpDriver permanece sem I/O, nenhum GPIO de bomba escolhido. MANUAL,
sem console de pulso/AUTO/rede. Serial WATER_STATUS não é payload MQTT.
Perfis e lógica energia preservados; padrão água continua water-disabled.

## Verificações reais

- Host completo: exit 0, **152 PASS** (144 anteriores + 8 novos de boia e
  intertravamento). Boot, fechamento estável, bounce, abertura imediata, wrap e
  cancelamento de solicitação local pelo controlador. Solicitação de teste
  existe só em host; o firmware físico não expõe comando de bomba.
- Build water-monitor: exit 0, SUCCESS 16.622 s, mesmas versões fixadas anteriores.
- Upload na COM5: exit 0, SUCCESS 19.179 s, hash verificado, reset RTS. Substituiu
  water-soil34-test, sem erase_flash integral. Eventual monitor COM5 foi fechado
  seletivamente para liberar porta; nenhum outro trabalho interrompido.
- Captura 6 s após reset: exit 0, marcador CIDADE_VIVA_WATER v0.2.0-pump-disabled;
  seis mensagens, reservoir=LOW, state=BLOCKED_LOW_WATER, pumpRequested=OFF,
  pumpOutput=DISABLED. ADC bruto entre 446 e 900 nessa janela; meio da sonda
  desconhecido, sem inferir percentual/qualidade/calibração. Porta liberada.
- Capturador serial estendido para validar estado coerente com indicação,
  pedido OFF, saída DISABLED e raw representável em 12 bits.
- Evidências: `015-water-monitor-serial.txt`, `015-water-monitor-host-tests.txt`;
  logs/builds em `.build/water-monitor`, ignorados.

## Entrega parcial explícita

WM-001–004 concluídos no monitor integrado. Isso não completa o pedido amplo
de todos os requisitos WA: não houve GPIO/acionamento de bomba ou corrente/vazão
medidos. Não houve calibração, AUTO físico, contrato/credenciais MQTT ou integração
com backend. Teste físico de transição PRESENT↔LOW ainda depende do movimento da
boia na nova imagem; os dois sentidos foram testados em host e anteriormente
apenas como contato no perfil de bancada.

Faltam para potência: corrente de regime/partida da bomba, corrente da fonte 5 V,
conexões/proteção do motor e validação do módulo IRF520N ao comando de 3,3 V.
Seu anúncio de compatibilidade não foi tratado como prova sob carga; substituição
não foi imposta. Também não se exige 10 V na bomba nem no ESP32. GPIO deve ser
definido após fechar circuito e manter estado físico OFF em reset/desconexão.

Faltam para AUTO: método/valores de calibração no substrato e limiar/pulso aprovado.
Faltam para rede: contratos normativos completos e configurações locais/segredos.
Sem contratos compartilhados alterados, outros repositórios ou pacote original;
sem compra, commit/push ou publicação.
