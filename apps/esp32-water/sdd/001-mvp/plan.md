# Plano — ESP32 Água e Irrigação

## Firmware
Interfaces `IFloatSwitch`, `ISoilSensor`, `IPumpDriver`, `IClock`, `IMqttTransport`. Componentes: `IrrigationStateMachine`, `SafetySupervisor`, `SensorQuality`, `TelemetryPublisher`, `CommandDispatcher` com cache de `commandId`. Priorizar transições locais antes de rotina de rede e leitura não bloqueante.

## Integração física
1. Identificar bomba AM174; medir corrente de regime/partida, vazão e diâmetro de bocal em bancada.
2. Definir fonte 5 V aprovada e driver MOSFET com bornes 3,3 V e margem adequada; diodo/filtro quando especificado.
3. Testar boia por digital com estado seguro em desconexão, antes de ligar motor.
4. Instalar reservatório, bandeja impermeável e retorno evitando MDF úmido.
5. Calibrar sensor em cartucho de solo real removível, ou habilitar simulação rotulada apenas para demonstração.
6. Implementar estado local e limites de pulso/cooldown; integrar MQTT v1 e logs.

## Validação
Teste com boia aberta, fechada e cabo desconectado; reset durante pulso; 20 ciclos rápidos; 5 ciclos reais com retorno de água; se possível osciloscópio/multímetro para ruído de alimentação. Não apresentar vazão comprovada se não houver medição por volume/tempo registrada.
