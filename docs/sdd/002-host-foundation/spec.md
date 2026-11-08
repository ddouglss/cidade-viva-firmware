# Fundação host dos dois firmwares

Data: 2026-11-07. Escopo autorizado neste chat: Codex implementa firmware;
Antigravity mantém a fundação backend. Esta decisão supera apenas a antiga
divisão de execução, sem alterar contratos ou documentos compartilhados.

## Escopo e rastreabilidade

Dois projetos C++17 independentes, sem GPIO, rede, serial ou board definitiva.
Alvos físicos: **PENDING_HARDWARE_VALIDATION**. Host não comprova execução ESP32.

| Requisito | Fatia verificável | Pendente |
|---|---|---|
| WA-001/002/006/008/010 | supervisor puro: boot OFF, boia baixa/indeterminada bloqueia, pulso limitado e cooldown local | GPIO, driver, loop físico e ensaio |
| WA-003/004/005 | nenhuma automação/calibração nesta fatia | origem, sensor, limiares e AUTO |
| WA-007/009 | nenhuma comunicação nesta fatia | TTL, deduplicação persistente, ACK e MQTT |
| EN-001/006/009 | modelo de intenção inicial conservadora e invariante de verdes | polaridade, arranque físico e ciclo |
| EN-003/004/007 | somente fronteira lógica/hardware | limiares, estabilidade, tempos e travessia |

## Comportamento local de água

Parâmetros provisórios existentes em SDD 001: maxPulseMs=3000 e cooldownMs=15000.
Não são homologação hidráulica. API interna não aceita comandos remotos.
Uma solicitação local informa tempo monotônico uint32 e estado atual da boia.
Somente WATER_PRESENT permite iniciar; LOW/UNKNOWN interrompem imediatamente
na próxima avaliação. Solicitação durante pulso não reinicia nem estende o prazo.
Duração zero ou acima do máximo é recusada; duração válida termina em cooldown.
Ao atingir máximo local (inclusive igualdade), timeout tem prioridade sobre término
normal e força FAULT_TIMEOUT. Todo desligamento após pulso inicia cooldown no
instante da observação OFF, conservador se o loop atrasar. Cooldown libera em
elapsed >= 15000, inclusive após wrap uint32. Fonte deve ser monotônica modular,
avaliada em intervalo menor que meia faixa uint32; reset cria nova instância OFF.

Lacuna: SDD 001 não define recuperação de falhas nem ACK_FAULT. Nesta fundação
FAULT_TIMEOUT fica retida até nova instância; retorno da boia permite sair do
bloqueio somente após cooldown, sem reinício automático de pulso. Esta política
conservadora é interna e provisória; recuperação operacional depende de nova SDD.
Configuração inválida bloqueia permanentemente OFF. Não há AUTO nem bypass manual.

## Energia e lacunas

Representar intenção lógica de arranque com ambos sinais vermelhos, iluminação
principal e buzzer OFF. Não selecionar níveis elétricos ou aplicar essa intenção
ao hardware. Apenas verificar invariante de ausência de verdes simultâneos.
Não implementar fases/temporização, histerese, fallback de sensor ou atuação
manual até especificar parâmetros e identificar componentes.

## Aceite e limites

Testar boot/reset, boias, interrupção, limites exatos, cooldown, solicitação ocupada,
timeout atrasado, falha retida e wrap temporal. Testar intenção inicial e matriz
dos verdes em energia. Nenhum teste alega vazão, medição, GPIO ou energia solar.
Sem dependência entre apps/repositórios, MQTT/simulador, commit, push ou hardware.
