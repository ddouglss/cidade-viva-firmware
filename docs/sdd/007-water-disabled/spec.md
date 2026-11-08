# Água: primeira imagem física com bomba desabilitada

2026-11-07. Usuário retirou unidade energia e conectou segunda unidade para água.
Consulta confirmou identidade diferente (não publicar MAC), D0WD-V3 rev. 3.1,
cristal 40 MHz, flash 4 MB, CP210x COM5. Perfil genérico esp32dev apenas para
boot/USB, não aprova GPIO ou revisão da PCB. Sem novos fios de água autorizados.

WA-001/002/008/010, W003/W004: integrar WaterRuntime real à entrada Arduino;
adaptadores separados: boia ausente UNKNOWN, solo ausente INVALID sem percentual,
driver da bomba desabilitado e sem acesso a GPIO. Nenhuma entrada/saída de
aplicação configurada. Runtime MANUAL, sem parâmetros AUTO, pumpRequested OFF,
FAULT_SENSOR por boia desconhecida. Nenhuma simulação de água presente.

Serial 115200 identifica esp32-water e informa DISABLED, UNKNOWN, INVALID/NONE;
heartbeat de 1 s apenas diagnóstico. Sem console, rede, MQTT, calibração física,
driver, alimentação externa ou acionamento de bomba. GPIO e componentes pendentes.

Testes host dos adaptadores/runtime, build completo independente do app água,
upload autorizado na segunda unidade e captura real de boot/heartbeat. Gravação
substitui programa anterior nessa unidade; não executar erase_flash integral.
Não alterar firmware da primeira unidade nem contratos. Não inferir nível de
reservatório, estado elétrico real da bomba ou vazão a partir de intenção.
