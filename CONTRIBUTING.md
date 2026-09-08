# Contributing / Como contribuir

English and Brazilian Portuguese issues and pull requests are welcome.
Contribuições em inglês e português brasileiro são bem-vindas.

Use the [issue forms](https://github.com/polarco/VitaTester/issues/new/choose)
for bugs, feature requests and physical validation results. Include the app
version, module, known console model (or unknown), and reproducible steps.
Do not upload crash dumps, raw recordings, credentials or personal information.
Review and redact any optional report excerpts before posting.

Use os formulários para bugs, sugestões e resultados físicos. Informe versão,
módulo, modelo conhecido (ou desconhecido) e passos reproduzíveis. Não publique
dumps, gravações brutas, credenciais ou dados pessoais. Revise e remova dados
sensíveis de trechos opcionais de relatórios antes de enviar.

## Working on a change / Preparando uma mudança

Keep changes focused and preserve the original artwork, upstream attribution and
[MIT license](LICENSE). Update documentation and [CHANGELOG](CHANGELOG.md) for
user-visible changes. Keep both READMEs equivalent. Documentation-only edits do
not bump the application version or require a replacement VPK.

Mantenha mudanças focadas e preserve arte original, créditos e licença MIT.
Atualize documentação e CHANGELOG para mudanças visíveis. Mantenha equivalência
entre os READMEs. Edições apenas documentais não alteram a versão do app nem o VPK.

## Proportional validation / Validação proporcional

| Change / Mudança | Evidence / Evidência |
|---|---|
| Docs, translation, presentation / Documentos, tradução, apresentação | Check links, language parity and GitHub rendering / Conferir links, equivalência e renderização no GitHub. |
| Runtime logic / Lógica do app | Run `./scripts/test-host.sh` (normal + ASan/UBSan); add focused coverage when behavior changes / Executar testes e cobrir comportamento alterado. |
| Vita APIs, build or assets / APIs Vita, build ou assets | Also run `./scripts/build.sh`, which verifies the VPK / Também executar build e verificação VPK. |
| Hardware-dependent behavior / Comportamento físico | Record relevant [physical gates](docs/VALIDATION.md), or explicitly mark them pending / Registrar portões relevantes ou indicar pendência. |

Build requirements and the pinned SDK are in the [user guide](docs/USAGE.md).
Host simulations never establish physical success. Do not claim a hardware defect
from an unavailable query. A physical result reports the tested device/session;
it does not automatically validate every model or promote a pre-release.

Requisitos e SDK fixado estão no [guia](docs/USAGE.pt-BR.md). Simulações não comprovam
sucesso físico. Consulta indisponível não comprova defeito. Um resultado físico
descreve o aparelho/sessão testado, sem validar todos os modelos ou promover a pré-release.
