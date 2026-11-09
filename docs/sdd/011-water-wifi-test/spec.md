# Teste de Wi-Fi do ESP32 da água

Objetivo de bancada em 2026-11-09: conectar a unidade de água como estação a
uma rede Wi-Fi de 2,4 GHz e comprovar a obtenção de IPv4. Escopo associado a
WA-010 e à etapa P0 de conectividade do plano de integração.

O perfil explícito water-wifi-test reutiliza o monitor de água com bomba
desabilitada. O loop continua avaliando a boia e imprimindo o ADC bruto enquanto
o Wi-Fi conecta. A tentativa encerra após 30 s. A próxima tentativa inicia após
reset. O sucesso exige associação e endereço diferente de 0.0.0.0.

As credenciais de uma rede pessoal entram pelo USB, em duas linhas locais
(SSID e senha), após o marcador WIFI_TEST_WAITING. O script usa entrada de senha
oculta. Limites de 32 e 63 bytes, respectivamente, e senha de 8 a 63 bytes.
Credenciais ficam em RAM, com persistência Wi-Fi desativada e sem eco no serial.
Não há parâmetros de credencial no build ou em arquivos do projeto.

As mensagens são diagnóstico serial de bancada. A integração MQTT e o painel
são etapas posteriores. Aceite físico depende de mensagem WIFI_TEST_CONNECTED
com IPv4 obtido em rede do usuário. Build sozinho valida somente compilação.

## Evidência em 2026-11-09

- [x] Build de water-wifi-test com PlatformIO 6.2.0 e espressif32 6.13.0.
- [x] Build de water-monitor.
- [x] CLI do script inicia com pyserial no ambiente PlatformIO.
- [ ] Gravação na unidade água. Windows apresentou somente COM3/COM4 Bluetooth.
- [ ] Captura de associação, IPv4 e alcance a partir do notebook.

Impacto em contrato, GPIO, alimentação e compra: nenhum. O diagnóstico usa a
serial e a rede local para a associação da placa. RFs WA permanecem abertas
até evidência física e integração correspondentes.

## Primeira unidade conectada: energia

O teste começa pela unidade de energia. O perfil explícito energy-wifi-test
deriva de energy-disabled e usa a mesma rotina de bancada e o mesmo script USB.
A entrada de energia informa deviceId=esp32-energy e externalOutputs=DISABLED.
O aceite de rede continua sendo a obtenção de IPv4. A associação não valida MQTT.

- [x] Build de energy-wifi-test e de water-wifi-test com a rotina comum.
- [x] Gravação de energy-wifi-test na COM5, unidade de energia confirmada por
  identificação serial. Verificação do hash pelo esptool passou.
- [x] Após reset, serial mostrou deviceId=esp32-energy, externalOutputs=DISABLED,
  WIFI_TEST_WAITING e ENERGY_ALIVE.
- [ ] Associação e IPv4 na rede do usuário, aguardando credenciais locais.
