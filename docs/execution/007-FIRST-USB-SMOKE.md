# Primeiro ensaio USB — build pronto, upload pendente

Data: 2026-11-07. Usuário autorizou seguir com diagnóstico mínimo na COM5.
SDD escrita antes do código: `docs/sdd/004-usb-smoke/`.

## Identificação realmente consultada

ESP32-D0WD-V3 rev. 3.1, CPU dual-core 240 MHz, cristal 40 MHz, flash detectada
4 MB (fabricante c4, dispositivo 6016). Ponte CP2102/CP210x reconhecida pelo
Windows na COM5 após instalação de driver. MAC omitido deste relatório.
Fotos não confirmam fabricante/revisão exatos da PCB de 38 pinos. Não é D1 R32.

## Implementação e build

`tools/esp32-smoke/{platformio.ini,src/main.cpp,README.md,capture_serial.py}`:
projeto isolado de diagnóstico; perfil genérico esp32dev apenas para USB nesta
unidade, sem alterar boards dos apps energy/water ou homologar GPIO.
Sem sensores/cargas/rede; serial 115200 com marcador de boot e heartbeat de 1 s.
Script de captura tem janela limitada e fecha a COM ao terminar. Não foi executado
contra o diagnóstico, pois o upload não aconteceu.

`./tools/pio.ps1 run --project-dir tools/esp32-smoke -e usb-smoke`: exit 0,
PlatformIO 6.2.0, espressif32 6.13.0, Arduino package 3.20017.241212+sha.dcc1105b,
GCC Xtensa 8.4.0+2021r2-patch5, C++17. Build físico compilado/linkado com sucesso:
RAM 21472/327680 bytes, flash reportada 268921/1310720 bytes. firmware.bin:
269280 bytes. Isso comprova build, não execução física do diagnóstico.

## Leituras de preservação e falhas observadas

Antes de qualquer upload, tentou-se backup completo de 4 MB em `.build/usb-smoke`:

1. read_flash via stub a 115200: exit 1, `Corrupt data, expected 0x1000 bytes but
   received 0x10 bytes`, após cerca de 74 KB lidos.
2. Via stub a 460800: ficou sem progresso; encerrados somente os processos
   esptool correspondentes a essa leitura, para liberar a COM. Sem flash escrita.
3. Via ROM (`--no-stub`) a 115200: leitura de 4096 bytes passou, exit 0.
4. Via ROM a 460800: leitura de 65536 bytes passou em 3.9 s, exit 0.
5. Via ROM a 460800, 4 MB: exit 1 após cerca de 820224 bytes, erro
   `Invalid head of packet (0xFF): Possible serial noise or corruption`.

Não foi obtido backup integral válido. Logs e probes ficam locais em `.build`,
ignorados, sem publicar conteúdo potencialmente sensível da flash anterior.
Build `.pio` também foi confirmado ignorado pelo Git.

**Não houve upload/write_flash/erase_flash.** Houve leitura, stub temporário em
RAM e resets RTS usados pelo esptool. Não se afirma falha do chip ou causa elétrica:
erros só demonstram que a transferência longa não se mostrou confiável.

## Próximo passo e pendências

Testar cabo USB de dados diferente e conexão direta a outra porta do computador,
sem hub e sem componentes externos; reconfirmar COM e repetir leitura antes de
upload. Se persistir, investigar transporte/driver/placa com evidências.
Depois de comunicação estável e backup: upload autorizado do diagnóstico,
captura real de boot/heartbeat e fechamento da COM.

USB-001 completo. USB-002 parcial (build passou, backup não); USB-003 pendente;
USB-004 concluído como registro fiel de resultado parcial. Nenhum requisito E/W
composto ou pinout foi marcado completo. Sem mudanças nos contratos, alimentação,
compras, outros repositórios ou pacote original. Sem commit/push.
