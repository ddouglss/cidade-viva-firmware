# Entrada da boia G27: primeiro contato observado

2026-11-07. Usuário confirmou boia conectada conforme G27/GND e pediu teste.
SDD 008 anterior ao código. Segunda unidade identidade novamente confirmada
(MAC omitido) na COM5. Bomba permanece fora do escopo de acionamento.

## Implementação e testes

BenchFloatContact (header puro): estabilidade 50 ms, SETTLING após boot/mudança,
OPEN/CLOSED sem conversão para nível. Entrada firmware/float27_main.cpp usa somente
GPIO27 INPUT_PULLUP. WaterRuntime permanece com UnconnectedFloatSwitch UNKNOWN,
sensor inválido e DisabledPumpDriver sem I/O. Perfil water-float27-test separado;
water-disabled permanece padrão e não configura GPIO. Capturador serial aceita
perfil float27 e verifica bomba desabilitada/nível desconhecido em cada linha.

- Host: exit 0, **144 PASS**, incluindo 138 anteriores e seis de debounce,
  fronteiras, bounce e wrap. Evidência em `013-float27-host-tests.txt`.
- Build ambos: water-disabled SUCCESS 12.859 s; water-float27-test SUCCESS
  12.535 s; versões e perfil esp32dev/DIO/4 MB anteriores mantidos.
- Somente eventuais processos de monitor PlatformIO COM5/115200 encerrados
  para liberar porta. Identificação esptool confirmou mesma unidade água.
- Upload explícito water-float27-test: exit 0, SUCCESS 18.488 s, hash verificado,
  reset RTS. Substituiu water-disabled nesta unidade. Não alterou unidade energia.
- Captura 6 s após reset: exit 0, boot presente, sete mensagens, raw LOW,
  SETTLING inicial e CLOSED estável após 50 ms; reservoir=UNKNOWN e
  pumpOutput=DISABLED em todas. Captura em `013-float27-serial-output.txt`.
  Porta fechada ao terminar. Não foi observado OPEN nessa janela.

## Aceite parcial e pendências

Entrada e boot do perfil testados; CLOSED pode decorrer de contato da boia ou
ligação fechada, portanto a leitura sozinha não comprova funcionamento mecânico.
FB-004 aguarda usuário mover a peça e informar OPEN/CLOSED nas duas posições.
Não mapear estado para RESERVOIR_PRESENT sem validar orientação/falha segura.
Circuito simples não distingue abertura da boia de desconexão/cabo rompido.

Nenhuma bomba, medição de nível, vazão ou segurança elétrica sob carga validada.
Pinout geral e W004/W009 compostas permanecem pendentes. Sem contratos, outras
camadas, compras, commit/push. Pinos energizados não devem ser reconectados;
desligar USB antes de modificar fiação.
