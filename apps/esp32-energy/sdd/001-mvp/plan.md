# Plano — ESP32 Energia

## Firmware em C++/PlatformIO
Componentes propostos: `BoardConfig`, `WifiManager`, `MqttClient`, `TelemetryPublisher`, `CommandDispatcher`, `LightController`, `TrafficStateMachine`, `CrossingButton`, `SafetySupervisor`, `ClockService`.

Separar lógica pura de GPIO/MQTT com interfaces `IGpio`, `ILightSensor`, `IClock`, `ITransport` e fakes em testes. Use `millis()`/máquinas de estado, sem `delay()` longo. Fixar `board = wemos_d1_uno32` **somente quando a placa real corresponder**; versões de framework e bibliotecas devem ser travadas depois do primeiro build reprodutível.

## Etapas
1. Simulador C++/software sem hardware e contrato v1 validado.
2. Upload via USB de dados, LED individual e logs serial; identificar revisão GPIO.
3. BH1750 via I²C, depois INA219 se comprado; verificar pull-ups e alimentação.
4. Dois postes via driver; automação AUTO/MANUAL com histerese.
5. Semáforos/botão/buzzer conectados após identificação, máquina de estados independente.
6. MQTT e ACK idempotente; casos de perda/reconexão e boot.
7. Opcional cancela após homologação do motor/driver, nunca forçar P0.

## Segurança
Não energizar sinaleiros antes de verificar tensão/resistores; não usar GPIO strapping sem estudo; não unir sistema solar aos 5 V principais.
