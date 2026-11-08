# Especificação elétrica e solar — referência não executiva

## Fonte principal
Adaptador AC/DC **externo, fechado e certificado**, nominal 5 V / ~5 A como ponto de partida sujeito à medição. Saída DC → entrada polarizada → chave e proteção geral revisadas por engenheiro → bornes +5 V e 0 V → ramais protegidos para ESP32-01, ESP32-02 e atuadores. Escolher proteção a partir de corrente real de partida, conectores, fios e queda de tensão. Não utilizar módulos de fonte AC com bornes expostos sob a maquete.

## ESP32 e motor
- 3,3 V em entradas GPIO; o motor não passa por pino ESP32.
- Alimentar DevKit pela interface aprovada do fabricante (por exemplo USB do modelo confirmado), sem aplicar 5 V no pino `3V3`.
- Módulo MOSFET/driver com entrada efetivamente reconhecida por 3,3 V e capacidade dimensionada para a bomba AM174; diodo de flyback conforme natureza do motor/driver e datasheet.
- Verificar polaridade, corrente de partida, temperatura do MOSFET, queda de tensão e capacitor somente se necessário. Protoboard para bancada, nunca distribuição final de potência.
- Servo da cancela requer alimentação/controle apropriados; não tratá-lo como motor DC simples.

## Solar autônomo
Painel → controlador de carga compatível → bateria Li-ion com química/proteção adequada → regulador de saída 5 V → quatro LEDs solares com resistores/driver. Referência: Waveshare Solar Power Manager **convencional**; conferir modelo, faixa de entrada, limite de saída 5 V/1 A e dimensões da bateria 14500 antes da compra.

**Proibido** unir diretamente a saída 5 V solar à saída 5 V do adaptador. Postes solares permanecem autônomos. A bateria guarda energia; o painel não armazena. A luz interna da sala pode ser insuficiente para carga — pré-carregar por USB do próprio módulo conforme manual.

## Hidráulica
Bomba e reservatório em compartimento impermeável; retorno estanque; sensor de boia falho implica bomba OFF. Limitar duração e cooldown no ESP32. Mangueira conforme medida real do bocal. Prever alívio de tração e acesso para limpeza.

## Pendências impeditivas
Correntes, fusíveis, bitolas, pinout e conectores só são **APROVADOS** após datasheets e teste de bancada. Os PDFs/desenhos ilustrados antigos **não** substituem esquema final de eletricidade.
