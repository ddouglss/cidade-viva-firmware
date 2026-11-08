# Plano

1. Especificar antes de implementar; manter SDD 001 e compartilhados intactos.
2. Criar include/src/tests e host.json próprios em cada app.
3. Runner PowerShell compila cada app isoladamente, C++17, warnings como erros,
   sem bibliotecas externas ou instalação global. Compilador portátil fixado.
4. Executar testes com relógio informado, sem sleeps ou hardware.
5. Registrar versões, resultados, hashes e lacunas; manter E/W compostas abertas.

PlatformIO físico fica pendente: não criar environment com board especulativa.
host.json configura apenas a compilação nativa; migração para PlatformIO após
validação da placa não deve misturar os dois projetos.
