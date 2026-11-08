# Sensor de solo: primeira captura ADC bruta no G34

2026-11-07. Usuário confirmou sensor compatível com 3,3 V e ligação correta,
autorizando executar testes. Não substitui datasheet nem inspeção da correção.
Identificação antes do upload: mesma unidade água na COM5 (MAC omitido).

SDD 009 escrita antes do código. Perfil water-soil34-test e entrada
firmware/soil34_main.cpp, sem alterar lógica de domínio: ADC1/G34 INPUT, resolução
12 bits e ADC_11db conforme API da versão Arduino instalada. ISoilSensor retorna
SoilReading::physical(raw, observedAt), sem calibração/percentual. Aquisição e
serial a cada 1 s. Não interpreta counts como volts ou umidade calibrada.

Leitura diagnóstica da boia G27 mantida com BenchFloatContact; WaterRuntime
continua MANUAL/UNKNOWN com DisabledPumpDriver. Não há GPIO de bomba, AUTO,
rede, GPIO de carga ou comandos externos. Relato de boia levantada=CLOSED e
abaixada=OPEN registrado, sem aprovar instalação/fail-safe hidráulico definitivo.

## Executado

- Host: exit 0, **144 PASS**. Sem testes espelho novos: representação raw sem
  percentual e debounce já são cobertos e foram reutilizados. Saída sanitizada
  em `014-soil34-host-tests.txt`.
- Três builds: water-disabled SUCCESS 16.749 s; water-float27-test 16.401 s;
  water-soil34-test 16.070 s. Versões fixadas mantidas, fontes firmware separadas.
- Encerrados somente eventuais monitores PlatformIO COM5/115200 antes da consulta
  e upload, para liberar serial; nenhum outro processo de trabalho interrompido.
- Upload: `./tools/pio.ps1 run --project-dir apps/esp32-water -e water-soil34-test
  -t upload --upload-port COM5`, exit 0, SUCCESS 19.475 s, hash verificado/reset RTS.
- Captura: helper com perfil soil34, reset e 8 s, exit 0. Boot próprio confirmado,
  oito amostras, **mínimo 3409 e máximo 3446**, contagens ADC. Boia OPEN nessa
  janela, reservoir=UNKNOWN e pumpOutput=DISABLED. Serial completa em
  `014-soil34-serial-output.txt`. Porta fechada ao terminar.
- Helper valida faixa de representação de 12 bits e ausência de percentual/
  acionamento nas mensagens; não diagnostica qualidade física da sonda por isso.
- Banco de compilação água atualizado para entradas Arduino dos perfis; ignorado
  pelo Git, como artefatos .pio e logs locais .build.

## O que ainda não foi testado

Não se sabe em que meio estava a sonda durante a captura. Os valores não foram
rotulados como secos/úmidos. SB-004 aguarda resposta com substrato e definição de
referências físicas. Não existe calibração de umidade, percentual confiável,
detecção inequívoca de cabo solto, medição de tensão ou prova de sensor saudável
só por essa série. Nenhuma bomba ou vazão foi testada. Driver IRF520N continua
sem aprovação para acionamento direto pelo ESP32; rede/contratos não alterados.

Próximo ensaio: comparar leituras no substrato seco e úmido com eletrônica seca,
sem mergulhar componentes/conector e sem bomba. Depois especificar método de
calibração e limites com evidências, sem escolher thresholds por estes números.
Sem commit/push, alteração de energia/backend/frontend ou pacote original.
