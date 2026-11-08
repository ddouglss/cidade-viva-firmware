# Tarefas — ESP32 Água

- [ ] W001 Fotografar e identificar ESP32, bomba, boia, sensor capacitivo e drivers; registrar datasheets.
- [ ] W002 Definir PINOUT aprovado, alimentação/bitola/fusíveis após medição e revisão elétrica.
- [ ] W003 Criar projeto PlatformIO, configurar serial e testes de pino seguro sem bomba.
- [ ] W004 Implementar lógica de boia com falha segura e watchdog de bomba.
- [ ] W005 Implementar `IrrigationStateMachine` com AUTO/MANUAL, limite de pulso e cooldown.
- [ ] W006 Implementar calibração e origem `PHYSICAL_SENSOR` vs `SIMULATED_INPUT`.
- [ ] W007 Integrar MQTT LWT/telemetria, allowlist, TTL, idempotência e ACK.
- [ ] W008 Montar prova de água real com tanque, bomba, mangueira, bandeja e retorno estanque.
- [ ] W009 Testar água baixa, desconexão da boia, reinício com bomba ligada e comando repetido.
- [ ] W010 Integrar backend e dashboard; gravar logs/medições/curto vídeo de cada cenário.

**DoD:** bomba não fica ligada sem água, não excede tempo máximo, medição e simulação claramente diferenciadas, água não atinge eletrônica.
