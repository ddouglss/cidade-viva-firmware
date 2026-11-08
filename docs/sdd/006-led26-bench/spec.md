# Teste isolado de LED vermelho no GPIO26

Usuário autorizou habilitar e confirmou montagem conforme instrução:
G26 → resistor 220 ohms em série → anodo LED vermelho; catodo → GND.
GPIO26 identificado pela serigrafia da placa e referência ESP32, sem inferir Dn.
Não houve inspeção visual conclusiva do circuito montado nem medição elétrica;
confirmação de ligação é relato do usuário. Placa destinada à energia, COM5.

Escopo de bancada, não MAIN_LIGHT definitivo nem semáforo. Novo ambiente explícito
energy-led26-test configura somente GPIO26 como saída, inicialmente LOW, alterna
HIGH/LOW a cada 500 ms sem bloqueio. Tempo apenas de diagnóstico. Demais saídas,
sensores e Wi-Fi/MQTT continuam ausentes/desabilitados. Somente um LED avulso
com resistor em série, sem motor, poste 5 V ou fonte externa.

Manter energy-disabled como padrão e entrada Arduino separada, sem habilitar
GPIO nesse perfil existente. Versões/board de boot validadas mantidas. Serial
declara BENCH_LED26_ONLY e estados solicitados, nunca medição física de luz.

Aceite: testes temporais host, ambos ambientes compilam sem cruzar entradas,
identidade da placa conferida antes de upload autorizado, capturar HIGH e LOW
pela serial. LED acender fisicamente precisa confirmação do usuário; serial não
aprova circuito/GPIO sob carga. Pinout geral permanece pendente.
