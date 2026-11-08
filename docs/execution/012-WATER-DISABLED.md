# Segunda unidade: firmware água gravado e testado

2026-11-07. Usuário desconectou energia e conectou água. Consulta esptool antes
de programar confirmou MAC diferente da unidade energia (omitidos deste registro),
D0WD-V3 rev. 3.1, 40 MHz, 4 MB, CP210x COM5. A COM foi reutilizada pelo Windows:
isso não equivale a mesma unidade. Usuário autorizou iniciar implementação/teste.

SDD 007 antes do código. Criados `DisabledWaterHardware.hpp` (boia UNKNOWN,
solo INVALID, bomba sem I/O), `firmware/main.cpp` e `platformio.ini` independentes.
Integra WaterRuntime/PumpSafety existentes, sem modificar regras de domínio.
App inicia MANUAL, solicitação OFF e FAULT_SENSOR por boia ausente; não simula
reservatório com água. Nenhum GPIO de aplicação, rede ou sensor físico habilitado.

## Verificações reais

- Host: exit 0, **138 PASS**, incluindo 133 anteriores e cinco novos de água.
  Verificam entrada ausente, boot seguro, recusa manual com boia desconhecida,
  AUTO sem configuração recusado e driver que não habilita saída física.
- `./tools/pio.ps1 run --project-dir apps/esp32-water -e water-disabled`: exit 0,
  SUCCESS em 13.78 s. RAM 21576/327680; flash reportada 270789/1310720 bytes;
  firmware.bin 271152 bytes. Mesmas versões fixadas nas imagens anteriores.
- Upload explícito water-disabled na COM5: exit 0, SUCCESS em 18.75 s, hash
  verificado, reset RTS. Substituiu programa prévio da segunda unidade; não foi
  realizado backup integral novo dessa unidade nesta entrega nem erase_flash
  integral. Backup da unidade energia anterior é distinto e não deve ser tratado
  como backup da água.
- Captura com helper estendido, perfil water-disabled, 6 s e reset: exit 0,
  boot marcador presente e seis heartbeats, todos pumpRequested=OFF,
  pumpOutput=DISABLED, reservoir=UNKNOWN, state=FAULT_SENSOR, measurements=NONE.
  Captura em `012-water-serial-output.txt`; testes em `012-water-host-tests.txt`.
- Banco de compilação água gerado, sem fontes tests no build físico; configs
  IntelliSense raiz/app água/pasta agregadora apontam para ele. Energia mantém
  seu banco independente. Artefatos/bancos ignorados pelo Git.

Nenhuma evidência de bomba elétrica desligada foi medida: driver físico sequer
configurado. Não há vazão, boia ou leitura de solo comprovados. FAULT_SENSOR
nesse perfil é comportamento esperado, não diagnóstico de defeito da placa.
WD-001–004 concluídos no escopo desabilitado; W001–W010 compostas permanecem
abertas para hardware/integracao. Pinouts, contratos, outros repositórios e pacote
original não alterados. Sem commit/push. Porta fechada ao concluir captura.

Contexto energia: usuário já confirmou que o LED vermelho acendeu no teste
anterior; aceite visual de acendimento registrado na tarefa LED-004. Corrente
não medida, sem homologação de postes/semáforos ou pinout geral.

Próximo passo água: identificar boia, sensor, bomba, fonte e driver. Validar boia
com bomba ausente antes de conectar motor. Não alimentar bomba pelo GPIO.
