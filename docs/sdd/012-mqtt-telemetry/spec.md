# SDD 012 — Telemetria MQTT segura de bancada

## Objetivo
Publicar status e telemetria dos dois ESP32 no contrato MQTT v1, sem habilitar atuadores ou afirmar medições não instaladas. IDs: EN-002/EN-005 e WA-009/WA-010 (parciais).

## Comportamento
- Perfis PlatformIO separados `energy-telemetry` e `water-telemetry`; os perfis atuais permanecem disponíveis.
- Wi-Fi e credenciais MQTT são configurados via USB serial e persistidos em NVS, nunca no Git. Broker deve ser IPv4 privado alcançável na rede das placas; não publicar o broker na internet.
- Após a instalação física, uma queda persistente do Wi-Fi ativa uma rede WPA2 de configuração exclusiva da placa, com senha aleatória gerada e gravada em NVS. A página local `http://192.168.4.1` permite trocar SSID, senha Wi-Fi e IPv4 privado do gateway sem alterar a credencial MQTT. A senha da rede de configuração deve ser registrada pelo operador antes de instalar a placa; não é publicada pela API nem pelo site de produção.
- Status retained QoS 1, LWT offline retained QoS 1, telemetria QoS 1 não retained, `schemaVersion=1.0` e IDs/tópicos existentes.
- Energia: `measurements={}`, `quality=INVALID`, `mode=MANUAL`, saídas desabilitadas. Não declarar lux, corrente, energia solar ou luz física.
- Água: ADC bruto G34 quando disponível; boia G27 apenas quando nível conhecido; bomba sempre desabilitada. Sem percentual de umidade não calibrado e sem vazão presumida.
- Sem horário NTP válido, omitir `timestamp` e usar `UNSYNCHRONIZED_TIME` apenas para leitura válida; leitura inválida mantém `INVALID`.
- Reconectar após quedas com intervalo limitado. Uma publicação com QoS 1 pode ser duplicada; `messageId` único por boot/dispositivo/seq.

## Limites
Esta entrega não recebe comandos MQTT nem habilita GPIO de atuadores. A conexão privada da rede local ao broker no Coolify exige gateway VPN ou outro desenho aprovado. O portal de configuração é servido pela própria placa e exige proximidade da rede Wi-Fi dela; o servidor em produção não acessa a porta USB/COM do notebook.
