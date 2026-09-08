[Início](../README.pt-BR.md) · [English](FAQ.md)

# Perguntas frequentes

### Como instalar ou atualizar?

Baixe VPK e checksum da [pré-release 1.5.0](https://github.com/polarco/VitaTester/releases/tag/v1.5.0).
Verifique o download, transfira e instale com VitaShell em um Vita que já execute
homebrew. O Title ID `VITATESTR` substitui a bolha existente. Faça backup dos
relatórios e logs antes. O projeto não instala desbloqueio nem plugin kernel.

### Por que o gesto de retorno não funciona?

Use exatamente três contatos no painel frontal, todos dentro das coordenadas
x=240–719 e y=110–419 da tela, continuamente por dois segundos. Solte todos os
dedos antes de tentar novamente. Perda de foco ou lacuna de captura cancela o
tempo. Restauração dos clocks do stress ou erro de liberação de dispositivo do
Scanner pode reter a transição até a limpeza terminar. Start/Select não navegam;
PS abre a LiveArea normalmente.

### A saúde da bateria comprova desgaste?

Não. O Scanner separa SOH do sistema de `100 × capacidade cheia / capacidade nominal`.
Informe em **Ajustes** os mAh nominais da bateria instalada para habilitar o
cálculo; zero significa não informado. Carga restante não é desgaste. Leituras
inválidas/inconsistentes tornam o cálculo indisponível. O alerta padrão ocorre
estritamente abaixo de 80%, limite de triagem ajustável, sem diagnóstico ou reparo.

### Por que um teste está indisponível? Devo marcar falha?

Hardware opcional, APIs negadas e ausência de operações userland documentadas
são limites de cobertura. Busca de APs e Bluetooth estão indisponíveis neste
build; Wi-Fi informa apenas estado/sinal/canal. Modelo PCH exato, região e
inventário kernel completo também estão indisponíveis. Use **PULADO** quando
apropriado; erro técnico e veredito humano sobre hardware são evidências separadas.

### Onde ficam relatórios e ajustes?

- Log de stress: `ux0:data/VitaTester/stresslog.txt` (JSON Lines, acrescentado entre sessões).
- Relatórios do Scanner: `ux0:data/VitaTester/scan_YYYYMMDD_HHMMSS.txt` (UTC; sufixos em colisões).
- Ajustes do Scanner: `ux0:data/VitaTester/scanner.cfg`.

**Relatorio** mostra o modelo salvo. Sair preserva cobertura parcial; **Concluir**
exige veredito explícito em cada etapa. Use **Salvar / Repetir gravacao** para
repetir falhas. Sessões anteriores são preservadas. Relatórios excluem áudio bruto,
imagens, conteúdo de dumps/arquivos e identificadores de rede. Revise qualquer
material antes de publicar; não anexe dumps nem informações pessoais.

### CI verde significa validação física?

Não. Simulações host e verificações do pacote validam lógica e estrutura do build.
Qualidade das entradas, tempos, som, câmeras, durabilidade da gravação e comportamento
térmico ainda exigem o [procedimento físico (inglês)](VALIDATION.md). Sessões longas
de stress vêm depois dos portões de parada, restauração de clocks e ciclo de vida.
