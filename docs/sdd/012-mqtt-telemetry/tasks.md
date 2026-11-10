# Tarefas — SDD 012

- [x] T012-01 Implementar perfil de telemetria independente para cada placa, sem saídas físicas. Evidência: builds PlatformIO `energy-telemetry` e `water-telemetry`.
- [x] T012-02 Compilar ambos os perfis e validar payloads representativos contra o parser do backend. Evidência: 46 testes .NET passaram; execução MQTT real ainda depende de T012-04/05.
- [ ] T012-03 Configurar contas MQTT por dispositivo e ACL no broker de produção.
- [ ] T012-04 Estabelecer ponte privada entre Wi-Fi das placas e broker no servidor.
- [ ] T012-05 Gravar as duas placas e confirmar status/telemetria no banco e no dashboard.
