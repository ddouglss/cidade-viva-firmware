# Plano

1. Identificar chip/flash sem alterar flash; conferir perfil genérico instalado.
2. Implementar somente diagnóstico serial, separado dos dois firmwares de produto.
3. Backup local ignorado, build reprodutível, upload autorizado na COM5.
4. Capturar boot/heartbeat serial; registrar evidências reais e pendências.

Continuação após reconexão autorizada: se leitura monolítica não progredir,
preservar por leitura ROM de blocos de 64 KiB, com número limitado de tentativas.
Só montar backup completo após ler todos os 4 MiB; fragmentos/temporários ficam
ignorados. Isso é estratégia de transporte para leitura, não regra de produto.
