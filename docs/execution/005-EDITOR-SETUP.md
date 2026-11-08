# Correção do ambiente C++ no VS Code

Data: 2026-11-07. Erro reportado: IntelliSense 1696 em TrafficController.cpp,
sem conseguir localizar cstdint e initializer_list.

## Diagnóstico e alterações

Compilador Clang local encontrado no caminho esperado pelo runner, mas não havia
configuração de IntelliSense no repositório. Microsoft C/C++ 1.35.6 já instalada.
Nenhuma configuração de compilador/include do usuário foi encontrada no settings
global consultado; ele não foi modificado.

- `.vscode/c_cpp_properties.json`: Clang x64, C++17 e includes dos dois apps.
- `apps/esp32-energy/.vscode/c_cpp_properties.json` e equivalente water: suportam
  abertura independente de cada pasta, mantendo includes próprios.
- `.vscode/tasks.json`: teste conjunto como build padrão e duas tarefas separadas.
- `tools/README.md`: abertura correta do workspace, tarefas e limpeza de cache.
- `005-editor-test-output.txt`: saída final sanitizada da compilação/testes.

O compilador é identificado por `${env:LOCALAPPDATA}/CidadeVivaToolchains/` com
a versão já fixada, sem paths de usuário hardcoded nem lista manual de headers
internos. Nenhum diagnóstico foi desabilitado ou suprimido.

## Verificação executada

- Probe C++17 incluindo cstdint, initializer_list e optional com `-fsyntax-only`:
  exit 0, headers padrão disponíveis.
- TrafficController.cpp com include local, C++17 e warnings como erros:
  `-fsyntax-only`, exit 0.
- A primeira execução por Windows PowerShell sem política de processo encontrou
  scripts desabilitados. Tarefas corrigidas para `-ExecutionPolicy RemoteSigned`
  restrito à invocação, sem alterar política global/CurrentUser.
- `powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File ./tools/test-host.ps1`:
  exit 0, **124 PASS** (53 energia, 71 água), mesmo comando da tarefa conjunta.
- Arquivos JSON e caminhos de compilador/include verificados após gravação.

Não foi observado o painel Problems do VS Code em execução; portanto, não se
afirma que o cache visual do editor já foi atualizado. A configuração é carregada
na raiz firmware ou quando cada app é aberto como workspace; abrir apenas um pai
diferente exige escolher a pasta correta. Recarregar janela/resetar banco de
IntelliSense pode ser necessário para diagnósticos antigos.

Sem alteração de lógica, contratos, hardware, dependências ou instalação global.
Sem commit/push. Nenhum build/upload ESP32 realizado. Documentos compartilhados,
SDDs de negócio e pinouts permanecem intactos.
