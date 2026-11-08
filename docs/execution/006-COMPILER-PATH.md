# Caminho compartilhado do compilador host

Data: 2026-11-07. O usuário executou a tarefa correta no VS Code, mas recebeu
Compiler missing. A instalação original foi feita sob dados do aplicativo Codex;
o caminho LOCALAPPDATA não ficou acessível da mesma forma no editor externo.
Verificações feitas dentro do Codex não bastavam para garantir esse acesso.

Correção: copiar a distribuição portátil existente, preservando a origem, para
`%USERPROFILE%/.cidade-viva/toolchains/llvm-mingw-20260602-ucrt-x86_64`.
Sem novo download, PATH global, alteração da lógica ou seleção de board.

Arquivos atualizados: `tools/test-host.ps1`, configurações C/C++ da raiz firmware,
de ambos os apps e da pasta agregadora local `cidade-viva`; `tools/README.md`.
Runner e editor agora derivam o mesmo caminho de USERPROFILE. Erro de compilador
ausente também informa o caminho completo procurado e opção de override.

Os registros 002/005 preservam o caminho histórico; a orientação atual está no
README de tools. O compilador segue LLVM-MinGW 20260602 / Clang 22.1.7.

Validação: distribuição copiada com 9441 arquivos em origem e destino; compilador
novo executou `--version`, exit 0. A tarefa da pasta agregadora foi resolvida e
executada com Windows PowerShell e LOCALAPPDATA temporariamente apontando para
um caminho inexistente apenas no processo de teste; a variável foi restaurada.
Resultado: exit 0, **124 PASS**, InstalledDir no novo USERPROFILE. Evidência em
`006-compiler-test-output.txt`. Isso verifica que o runner não depende mais do
LOCALAPPDATA do Codex. A janela externa do VS Code não foi observada nesta sessão.
