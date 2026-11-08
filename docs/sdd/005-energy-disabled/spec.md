# Firmware energia com saídas externas desabilitadas

Autorização: usuário confirmou nenhuma conexão externa e solicitou implementar
versão energia sem atuadores, para gravar quando USB reconectado.

Escopo EN-001/009 e E002: integrar núcleo existente ao Arduino/PlatformIO no app
esp32-energy, com serial 115200, identificação esp32-energy e heartbeat de 1 s
(somente diagnóstico). Usar perfil genérico esp32dev validado para boot/USB da
unidade COM5 (D0WD-V3, 4 MB); não homologar PCB/header ou GPIO. Nenhum número
de GPIO de aplicação, sensor, driver, Wi-Fi ou MQTT nesta entrega.

Adaptador DisabledEnergyHardware implementa IEnergyHardware: aplicação de saída
é no-op, botão ausente retorna false, lux ausente retorna inválido, nunca medição
inventada. Saídas indisponíveis não podem ser habilitadas por configuração/método
de runtime. Não publicar intenção lógica como atuação física.

Runtime inicia MANUAL/OFF, trânsito ALL_RED lógico por ausência de parâmetros;
nenhum tempo/limiar de produto é criado. Serial informa explicitamente
externalOutputs=DISABLED, hardware=PENDING_HARDWARE_VALIDATION e nenhuma medição.
Sem console de comandos nem auto-start de automação. Loop avalia runtime e cede
ao RTOS, sem espera longa. Não alterar lógica água nem contratos compartilhados.

Aceite: testes host incluindo adaptador desabilitado; build ESP32 completo do app
sem fontes de testes/diagnóstico antigo; verificar artefatos. Upload e execução
dessa nova imagem não são declarados até placa conectada e captura real.
Nenhuma tarefa composta E/W fica concluída só por build.
