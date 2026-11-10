# Plano — SDD 012

1. Adicionar transporte compartilhado de status/LWT/telemetria com MQTT QoS 1 e configuração local via serial USB/NVS.
2. Criar entradas Arduino separadas para energia e água, reutilizando os controladores locais existentes e os GPIO G27/G34 já testados na água.
3. Reservar credenciais MQTT distintas e ACL por dispositivo no broker. Manter listener apenas na rede privada de containers; o acesso externo depende de ponte VPN privada.
4. Validar build dos dois perfis e contrato dos payloads com parser do backend. Teste ponta a ponta e upload ficam condicionados à disponibilidade das placas e da ponte.
5. Permitir rotação da senha MQTT por USB usando as credenciais Wi-Fi já gravadas em NVS. O comando de rotação altera somente host/usuário/senha MQTT, exige configuração anterior válida e não aciona saídas físicas.
6. Abrir AP WPA2 com senha aleatória persistida no boot e mantê-lo ativo junto à conexão STA, como fazia o perfil `WifiBench`. Servir HTTP local apenas para SSID, senha Wi-Fi e IPv4 privado da ponte. Testar ambos os perfis e registrar a senha de cada AP antes da instalação definitiva.
7. Na primeira gravação do perfil MQTT, se `cv-network` não contiver Wi-Fi e o perfil de bancada tiver salvo SSID/senha em `cv-wifi`, migrar esses dois valores para NVS do perfil novo. Permitir completar host/usuário/senha MQTT via serial sem redigitar o Wi-Fi.

Decisões existentes: tópicos, QoS, `schemaVersion`, origem/qualidade, proteção física local. Propostas desta SDD: provisioning serial e intervalo de telemetria de 15 s/status de 20 s; não constituem tempos de controle físico.
