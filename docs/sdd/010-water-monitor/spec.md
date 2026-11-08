# Integração local dos sensores água, saída de bomba bloqueada

Usuário autorizou implementar/gravar restante na unidade água. Executar somente
regras disponíveis: integrar boia real ao supervisor e ADC bruto ao monitor.
Não inventar calibração/AUTO, credenciais/contratos MQTT ou circuito de bomba.
Permanecem pendentes corrente/fonte/proteção e acionamento do módulo IRF520N.
Ausência desses dados impede habilitar GPIO de potência, não bloqueia sensores.

Relato já testado: boia levantada=CLOSED, abaixada=OPEN, fios G27/GND. Perfil
water-monitor traduz indicação elétrica CLOSED estável para Present, OPEN para
Low, transição fechando para Unknown. Isso é indicação conforme orientação
relatada, não homologação de montagem do reservatório. Contato abrindo bloqueia
na primeira avaliação; fechamento exige 50 ms de estabilidade (valor de bancada
existente, não homologação de tempo final). Cabo aberto bloqueia; curto não é
distinguível de contato fechado. Boot Unknown até avaliar entrada.

G34 segue ADC físico 12 bits com ADC_11db, sem percentual/calibração. Runtime
MANUAL, sem comandos de pulso/console e sem AUTO habilitado. DisabledPumpDriver
permanece sem I/O; nenhuma definição de GPIO de bomba. Sem dados de corrente,
fonte, proteção e validação do acionamento, este perfil não pode energizar bomba.

Serial de 1 s relata indicação da boia, estado real do controlador, pedido OFF,
saída DISABLED, solo bruto e percentual indisponível. Não reportar vazão ou
atuação física. LED/perfis anteriores e firmware energia preservados.

Aceite: testes de abertura imediata, fechamento estável, bounce/wrap e integração
do bloqueio com controlador; build, identidade, upload/captura do perfil. Fonte
de verdade adicional: SDDs 001–009. Requisitos integrais WA/AUTO/MQTT/ensaio
hidráulico e hardware continuam pendentes.
