# Primeiro teste USB sem atuadores

Autorização do usuário: seguir com flash e diagnóstico serial na placa COM5.
Chip consultado: ESP32-D0WD-V3 revisão 3.1, cristal 40 MHz, flash detectada 4 MB,
ponte CP2102. Foto: placa genérica de 38 pinos, não D1 R32; fabricante exato não
confirmado. Perfil `esp32dev` apenas para diagnóstico USB da unidade observada,
sem homologar headers/GPIO ou escolher definitivamente boards dos dois apps.

EN-001/E002 e WA-001/W003: cobertura parcial de ferramenta/build/boot serial;
nenhum controle de carga nem proteção física de bomba demonstrados.

Criar ferramenta isolada `tools/esp32-smoke` com PlatformIO fixado, Arduino,
C++17, serial UART0 a 115200 baud. Não configurar GPIO externo, sensor, Wi-Fi,
Bluetooth, MQTT ou qualquer atuador. Boot imprime marcador de diagnóstico,
modelo/revisão, flash, CPU e declaração de atuadores não configurados. Heartbeat
a cada 1000 ms é somente frequência de log de diagnóstico, não regra de produto.

Antes de upload: guardar backup de 4 MB da flash em `.build` ignorado, sem
publicar conteúdo (pode conter dados anteriores). Compilar antes de gravar.
Upload substitui programa anterior; não executar erase_flash. Capturar serial
por janela limitada e fechar porta ao terminar. Aceite: build, upload e marcador
mais heartbeats reais, com resultados registrados. Se falhar, registrar falha.

Sem mudança de contratos compartilhados/pacote original. Apps energy/water e
pinouts mantêm PENDING_HARDWARE_VALIDATION para atuadores e periféricos.
