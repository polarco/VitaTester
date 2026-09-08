# VitaTester 1.5.0 — Input Test, Stress Test & Scanner

**Pre-release · Physical Vita validation pending for all three modules.**
Automated tests pass; this build does not certify hardware condition.

**[Download VPK](https://github.com/polarco/VitaTester/releases/download/v1.5.0/VitaTester-1.5.0.vpk)** ·
[SHA-256 file](https://github.com/polarco/VitaTester/releases/download/v1.5.0/VitaTester-1.5.0.vpk.sha256) ·
[English guide](https://github.com/polarco/VitaTester/blob/master/README.md) ·
[Guia em português](https://github.com/polarco/VitaTester/blob/master/README.pt-BR.md)

## What’s included

- **Input Test:** original button, analog and multi-touch diagram, without stress HUD.
- **Stress Test:** optional checked CPU load, free/guided diagnostics, capture timing
  and persistent logging; saved clocks restored on stop and system interruption.
- **Scanner:** passive system/storage/battery inventory; separate system SOH and
  calculated battery health; cancellable sensor, speaker, microphone, camera,
  Wi-Fi and read-only file stages; explicit pass/fail/skip verdicts and UTF-8 reports.

AP scanning and Bluetooth remain unavailable. Denied or unsupported queries do
not prove hardware failure. Partial reports preserve coverage gaps and errors;
reports exclude raw audio, images, dump/file contents and network identifiers.

## Install & navigate

Install with VitaShell on an already homebrew-capable console. Title ID
`VITATESTR` replaces an existing VitaTester bubble; back up reports/logs first.
SFO: `01.50`. No kernel plugin is required. Touch a module card to begin.

Hold exactly three fingers centrally on the front screen for two seconds to
return to the menu; release before repeating. Start/Select remain diagnostic
inputs. PS retains native LiveArea behavior. Module transitions wait for stress
workers, clock restoration and device cleanup. Returning from system interruption
never automatically restarts load or active tests.

## Validation

Production-code host simulations, concurrent worker tests, ASan/UBSan,
original-drawing comparison, pinned VitaSDK build and VPK verification passed in
[release CI](https://github.com/polarco/VitaTester/actions/runs/34237819400).
All unpacked files in the local and CI VPKs match. Archive timestamps can differ;
the checksum below identifies the release asset exactly.

Follow the [physical validation procedure](https://github.com/polarco/VitaTester/blob/master/docs/VALIDATION.md)
before prolonged stress use. Forced close, kernel failure or power loss can
prevent cleanup or lose pending records.

## Integrity

`VitaTester-1.5.0.vpk` · **897,663 bytes** · SHA-256:

```text
c9e813447211bd9e52353376419d280ddf8bc973cb977ad42f5f9c90e7a51da0
```

Download both files into one folder and run:

```sh
sha256sum -c VitaTester-1.5.0.vpk.sha256
```

The presentation update changes documentation only: application **1.5.0**, tag,
VPK and checksum are unchanged. The **1.4.0 pre-release remains preserved**.
MIT copyright of SMOKE and upstream history/artwork remain intact; credits to
SMOKE, Carlanga, NamelessGhoul0 and the upstream contributors. Fork: Filipe Fabiani.

## Português brasileiro

A versão 1.5.0 reúne **Input Test**, **Stress Test** e **Scanner** independentes.
O desenho original permanece no Input Test; carga e diagnóstico guiado ficam no
Stress Test. O Scanner coleta inventário passivo, separa SOH da saúde calculada
e oferece etapas ativas canceláveis com vereditos humanos e relatórios parciais/finais.
Busca de APs e Bluetooth estão indisponíveis; consulta negada não comprova defeito.

Instale o VPK acima pelo VitaShell em console que já execute homebrew. Faça backup
dos relatórios/logs antes de substituir a bolha `VITATESTR`. Toque no módulo;
para voltar, mantenha três dedos no centro frontal por dois segundos e solte-os
antes de repetir. Start/Select são diagnósticos; PS mantém a LiveArea nativa.

**A validação física dos três módulos está pendente.** Testes host, sanitizers,
build e verificação VPK passaram, sem comprovar comportamento real do hardware.
Siga o procedimento físico antes de stress prolongado. A revisão de apresentação
não altera aplicativo, tag, VPK ou checksum; a pré-release 1.4.0 foi preservada.
O [README em português](https://github.com/polarco/VitaTester/blob/master/README.pt-BR.md)
reúne instalação, controles, perguntas frequentes e guia de uso completo.
