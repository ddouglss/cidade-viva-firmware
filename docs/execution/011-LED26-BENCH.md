# Teste isolado GPIO26: gravado, observação física pendente

2026-11-07. Usuário confirmou montagem igual à instrução e autorizou habilitar.
SDD 006 anterior ao código. Não houve validação visual conclusiva da protoboard.

Implementado BenchLedBlink puro, entrada firmware/led26_main.cpp e ambiente
energy-led26-test explícito. Somente GPIO26 inicia LOW e alterna a cada 500 ms;
sem conexão ao controlador de iluminação/semáforo. Perfil energy-disabled segue
padrão, sem GPIO, com filtro de fonte separado. Pinout pendente recebeu apenas
nota delimitada de bancada, não aprovação geral.

Testes host: exit 0, 133 PASS (128 anteriores + 5 temporais de LED, inclusive
fronteiras, atraso e wrap). Build de ambos ambientes: exit 0, energy-disabled
14.668 s, energy-led26-test 14.415 s. Versões e perfil de boot mantidos.

COM5 inicialmente ocupada pelo monitor PlatformIO do usuário. Encerrados apenas
os processos Python identificados com `-m platformio device monitor --port COM5
--baud 115200`, para liberar porta para a gravação autorizada; sem encerrar outros
terminais ou aplicativos. Consulta chip confirmou mesma identidade anterior.

Upload: `./tools/pio.ps1 run --project-dir apps/esp32-energy -e energy-led26-test
-t upload --upload-port COM5`, exit 0, SUCCESS 19.182 s. Hash verificado no log
local `.build/led26/upload.log`. Substituiu energy-disabled nesta unidade.

Captura por 6 s após reset, perfil led26 adicionado ao helper: exit 0;
marcador CIDADE_VIVA_ENERGY_LED26_TEST v1, 13 mensagens de pedidos, HIGH e LOW
presentes, gpio=26, physicalFeedback=NONE. Captura completa em
`011-led26-serial-output.txt`; testes sanitizados em `011-led26-host-tests.txt`.
Porta liberada ao terminar. Não se afirma medição elétrica ou luz observada.

Sem rede/contrato, novas compras, mudanças água/backend/frontend, commit/push.
Só resistor e LED já relatados. Tarefas compostas E/W e validação do pinout geral
continuam pendentes. LED-004 aguarda usuário observar LED piscando; serial não
conclui esse aceite. Desconectar USB para modificar a ligação.
