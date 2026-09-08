[Início](../README.pt-BR.md) · [English](USAGE.md)

# Guia de uso — VitaTester 1.5.0

Input Test, Stress Test e Scanner são módulos independentes, baseados no
VitaTester de SMOKE. **Build e simulações host aprovados; validação em Vita
físico pendente.** Este candidato fornece evidências de diagnóstico, sem
comprovar defeito. Siga o [procedimento físico (inglês)](VALIDATION.md) antes
 de uso prolongado.

## Instalação e navegação

Baixe VPK e checksum da [pré-release 1.5.0](https://github.com/polarco/VitaTester/releases/tag/v1.5.0),
verifique com `sha256sum -c VitaTester-1.5.0.vpk.sha256` e instale pelo VitaShell
em um console que já execute homebrew. O Title ID `VITATESTR` substitui uma
bolha existente. Faça backup dos logs e relatórios que desejar preservar.
Nenhum plugin kernel ou API de clocks modificada é necessário.

O aplicativo abre o menu por toque com stress desligado. Input Test usa o
desenho original sem HUD de stress. Stress Test conserva as funções da 1.4.0.
O Scanner inicia somente inventário passivo.

Toque no cartão para escolher o módulo. Para retornar, mantenha exatamente
três dedos no painel frontal, todos no centro (x=240–719, y=110–419 da tela),
por dois segundos. Solte todos antes de repetir. Perda de foco ou lacuna de
captura cancela o tempo. Os contatos continuam visíveis no Input Test e no
Stress Test. Não há botão de menu sobre o desenho original. Falha de
restauração dos clocks impede sair do stress até restaurar o baseline salvo.

| Comando por toque no rodapé do Stress Test | Efeito |
|---|---|
| Iniciar / Parar stress | Inicia carga após verificações ou para e restaura clocks salvos. |
| Modo livre / Modo guiado | Alterna observação livre e sequência guiada. |
| Nome/valor do limite | Seleciona o próximo temporizador configurável. |
| − / + | Ajusta o limite em 1 segundo, entre 1 e 120 segundos. |

Solte os dedos frontais entre comandos. Os toques de comando são registrados
como atividade diagnóstica. Botões físicos, inclusive Start/Select, nunca
controlam o aplicativo. PS mantém o comportamento nativo; não existe duplo
clique personalizado. A tela mostra permanentemente:

**PS: voltar à LiveArea • Para fechar, encerre a bolha**

Overlay/LiveArea, callbacks de suspensão/retomada, eventos do sistema ou lacuna
de polling acima de 50 ms cancelam a carga. Retomar mantém o stress parado;
é necessário tocar em Iniciar novamente.

## Carga e captura

Três workers finitos de cálculo inteiro/flutuante usam os cores userland 0–2
com clocks ARM/GPU/BUS solicitados de **444/222/222 MHz**. Somente a thread da
interface chama vita2d. A captura ocorre aproximadamente a cada 8 ms, com
prioridade acima de UI, logger e workers. Prioridades efetivas, afinidades de
CPU e clocks precisam passar nas verificações antes de iniciar carga.

## Scanner

SOH do sistema e saúde calculada da bateria aparecem separadamente. Em
**Ajustes**, configure capacidade nominal opcional em mAh e limite de triagem
(padrão **80%**). Os valores persistem em `ux0:data/VitaTester/scanner.cfg`.
A saúde calculada é `100 × capacidade cheia / capacidade nominal informada`;
carga restante nunca mede desgaste. Leituras inválidas/inconsistentes desabilitam
o cálculo. Nominal aceita zero (ausente) ou 100–10000 mAh; o limite aceita
1–100%. O alerta ocorre estritamente abaixo do limite: 80% não dispara o padrão.

O inventário passivo inclui bateria, consultas de firmware reportado/real,
Vita/Vita TV, memória livre acessível ao processo, evidências de configuração,
módulos visíveis ao processo, evidência consultável de taiHEN, espaço das
montagens e **somente metadados** de crash dumps. Plugins configurados não são
chamados de carregados. Arquivos de boot não comprovam Ensō persistente ativo.
Modelo PCH exato, região e inventário kernel inacessível permanecem indisponíveis.
Nomes de montagens não identificam o cartão físico que sustenta um caminho.

Em **Testes**, selecione uma etapa e toque em **Iniciar**, **Parar** ou **Repetir**.
Confirme explicitamente **PASSOU / FALHOU / PULADO** após cada etapa:

- Sensores mostram vetores/timestamps; magnetômetro mostra matriz NED e estabilidade.
- Áudio toca 440 Hz durante três segundos por canal, com amplitude moderada e
  rampas. Desconecte fones/saídas externas para avaliar os alto-falantes.
- Microfone mostra RMS/pico em dBFS temporários, sem salvar áudio.
- Câmeras frontal e traseira têm previews temporários separados.
- Wi-Fi mostra conexão/sinal/canal. Busca de APs e Bluetooth estão explicitamente
  indisponíveis na interface userland documentada fixada. Nenhum helper kernel é adicionado.

Para **Slots / leitura**, use **Arquivo**, escolha ux0/uma0/imc0 e um arquivo
regular existente. Diretórios são paginados em lotes de 64. A leitura se limita
a 1 MiB, em blocos de 4 KiB, sem escrever no arquivo selecionado. Consultas de
inserção ficam separadas do resultado de leitura; mapeamento físico pode ser
inconclusivo. Dumps são excluídos do seletor. Ausência opcional ou API
inacessível não reprova hardware automaticamente; confirme **PULADO** quando indisponível.

**Relatorio** mostra o mesmo modelo UTF-8 salvo como
`ux0:data/VitaTester/scan_YYYYMMDD_HHMMSS.txt` (UTC, sufixo numérico em colisões).
Relatórios são salvos após coleta passiva, tentativas, vereditos e conclusão.
**Concluir** exige veredito explícito para todas as etapas; cobertura indisponível
continua visível. Sair preserva relatório parcial. **Salvar / Repetir gravacao**
repete falhas de gravação enquanto os resultados continuam na tela.

As escritas são completas e sincronizadas antes da substituição por arquivo
temporário. Sessões anteriores são preservadas. Áudio, imagens, conteúdo de
dumps/arquivos e identificadores de rede nunca entram nos relatórios do Scanner.
PS/eventos cancelam testes ativos; retornar não reinicia testes. Scanner e
stress não executam carga juntos. Falha de liberação de dispositivo mantém
posse do recurso e bloqueia transição para permitir repetição. Veja
[arquitetura e limites (inglês)](SCANNER.md) para detalhes de disponibilidade e persistência.

## Diagnóstico livre e guiado

| Temporizador | Padrão | Interpretação |
|---|---:|---|
| Mantido | 10 s | Botão digital continuamente pressionado: suspeita. |
| Inativo | 30 s | Sem transição após ciclo observado de pressionar/soltar, com atividade independente nos últimos 5 s: suspeita. |
| Etapa | 10 s | Tempo válido de observação em cada etapa guiada. |
| Fantasma | 2 s | Contatos traseiros persistentes durante “retire os dedos”: suspeita. |
| Persistencia | 10 s | Contato traseiro contínuo no modo livre: somente aviso. |

A sequência guiada solicita pressionar/soltar cima, baixo, esquerda, direita,
cruz, círculo, quadrado, triângulo, L, R, Start e Select, mover cada analógico,
tocar/deslizar atrás e retirar os dedos traseiros. Cada etapa avança pelo
seu temporizador, registrando `observed` ou `not_observed`: mede a ação
solicitada, sem concluir defeito físico. Afaste bem o analógico do centro e
deslize ao menos 24 unidades de coordenadas do dispositivo traseiro.
Alternar livre/guiado reinicia a sequência.

Atividade analógica independente usa zona morta de 20 unidades e deslocamento
mínimo de 12 unidades desde a última âncora de atividade. Ruído pequeno não
implica botões sem uso. Botão sem ciclo completo observado nunca é marcado
inativo. Ausência de toque traseiro sem solicitação não comprova nada.

Amarelo indica suspeita/aviso; vermelho, captura indisponível ou erro operacional.
Alertas registram início e recuperação uma vez. Anomalias de entrada isoladas
não param stress. Interceptação pelo sistema, erro de API e amostra antiga são
estados separados. Inferência pausa com captura inválida ou lacunas acima de
50 ms; a etapa guiada reinicia a janela válida depois. Evidência anterior
continua visível até recuperação. Último funcionamento observado e detecção
delimitam uma possível janela de falha, nunca o instante exato de um defeito.

## HUD e gravação

O HUD central preserva controles ao redor e os dois painéis de toque. Mostra
duração do stress, **temperatura da bateria** em °C (não do processador), carga,
alimentação externa, clocks efetivos, FPS, intervalo máximo e p99 de polling,
progresso dos workers e última sequência sincronizada do log. O p99 usa bins
de 1 ms arredondados para cima; `>=101 ms` é o bin de excesso. Progresso conta
blocos finitos de cálculo concluídos.

O [formato do log (inglês)](LOG.md) descreve JSON Lines schema 1 em
`ux0:data/VitaTester/stresslog.txt`. Logger dedicado acrescenta registros,
trata escritas parciais e chama `sceIoSyncByFd` a cada registro. Fila: 256
registros. Falha de abrir/escrever/sincronizar ou fila cheia bloqueia/cancela
stress e restaura clocks sem aguardar disco. Reinicie o app depois de resolver
a falha de gravação. Restauração malsucedida conserva o baseline original e
repete; não o substitui por um baseline novo.

Fechamento forçado da bolha, travamento kernel ou queda de energia podem impedir
limpeza e perder registros em trânsito. Sincronizar reduz perdas, sem garantia
absoluta de durabilidade. Ao reabrir, uma quebra de linha isola o último registro
incompleto; sessões antigas nunca são truncadas. Leitores devem tolerar e
reportar uma última linha malformada de sessão interrompida.

## Compilação e testes

Em Linux com Docker, Python 3 e compilador C11:

```sh
./scripts/test-host.sh
./scripts/build.sh
sha256sum build/VitaTester.vpk
```

Execute na raiz do repositório. O build usa `vitasdk/vitasdk:2026.08-20260815`,
fixado como `vitasdk/vitasdk@sha256:7f5eee50ff95b73c8c847dbfef6227aa0035886444b4d0254c21da8369ff1efc`.
CMake empacota assets originais e módulos com SFO `01.50`. O verificador confere
integridade ZIP, inventário exato de assets, ELF32 ARM, SELF do mesmo build e
identidade/versão. CI executa os mesmos testes/build e identifica artefatos como
pendentes de hardware. Builds podem diferir nos timestamps ZIP; o checksum da
release identifica exatamente o VPK distribuído.

Os testes exercitam código de produção de diagnóstico, transação de clocks,
fila, escrita completa e captura/controle com APIs e tempo simulados. Cobrem
limites temporais, recuperação, inatividade legítima, ruído analógico, toque
guiado, overlay/eventos de energia, falha parcial de clocks, limpeza idempotente,
falha de escrita/sync e saturação. Scanner cobre limites de saúde, medidas
independentes, vereditos, relatórios parciais/finais, colisões, substituição
malsucedida, repetição de limpeza, cancelamento tardio e leitura limitada.
Comandos de textura são comparados ao desenho upstream fixado. Execuções normais
e com AddressSanitizer/UndefinedBehaviorSanitizer passaram. Testes host não
comprovam escalonamento real, entrega de callbacks, comportamento térmico,
qualidade física de entrada ou durabilidade do armazenamento no Vita.

## Créditos e licença

Licença [MIT](../LICENSE); **Copyright (c) 2015 SMOKE** preservado integralmente.
Histórico upstream até `a2f0c9f4dfa0970aadbf0463fcae733943aa6de8` mantido.
Créditos: SMOKE, Carlanga, NamelessGhoul0, d3m3vilurr e coderobe (multitoque),
xerpi (vita2dlib), UrielTapia97 (imagens/ícones) e Ruben_Wolfe451 (matemática).
Trabalho do fork 1.4.0–1.5.0: Filipe Fabiani. Veja
[NamelessGhoul0/VitaTester](https://github.com/NamelessGhoul0/VitaTester).
As referências de API estão no [guia original](USAGE.md) e no
[documento do Scanner](SCANNER.md). Os headers fixados definem temperatura da
bateria em °C ×100 e prioridades numéricas menores como mais altas.
