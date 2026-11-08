# Boia: teste de contato no G27, bomba sempre desabilitada

2026-11-07. Usuário confirmou boia conectada entre G27 e GND e autorizou testar.
Pinagem baseada na inscrição G27 da foto da unidade de água; sem inferir Dn.
Componente de dois fios relatado, comportamento elétrico/polaridade de nível
ainda não caracterizados. Nenhuma alimentação externa na boia; bomba ausente.

Novo perfil explícito water-float27-test, mantendo water-disabled padrão.
Somente GPIO27 INPUT_PULLUP; não configurar saída de bomba. Contato fechado com
GND lê LOW, aberto lê HIGH. Janela de estabilidade 50 ms apenas de diagnóstico;
reportar SETTLING até estabilizar e após mudanças, depois OPEN/CLOSED. Não usar
esse contato ainda para inferir RESERVOIR_PRESENT/LOW. WaterRuntime permanece
com boia de domínio UNKNOWN e DisabledPumpDriver, mesmo que contato feche.

Serial 115200: marcador próprio e estado de contato em mudanças/heartbeat de 1 s,
sempre reservoir=UNKNOWN e pumpOutput=DISABLED. Não publicar MQTT nem umidade.
Testes host de debounce/wrap; build separado de ambos perfis; confirmar unidade,
gravar e capturar boot/leitura. Somente usuário pode movimentar boia e confirmar
ambos estados/posição. Cabo rompido e boia aberta são indistinguíveis neste
circuito simples; não alegar diagnóstico de falha elétrica nem homologar safety.

Antes de acionamento futuro: definir montagem em que falta de água/desconexão
não autorize bomba, identificar driver/fonte/bomba e ensaiar proteções. Nenhum
comando de bomba integra esta versão. Pinout final continua pendente.
