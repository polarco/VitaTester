![VitaTester — PS Vita illustration with Input Test, Stress Test and Scanner symbols](docs/assets/banner.svg)

# VitaTester

See your inputs. Observe behavior under load. Explore your Vita’s hardware.
Three independent diagnostic modules for PS Vita, built on SMOKE’s VitaTester.

**English** · [Português brasileiro](README.pt-BR.md)

[![Version 1.5.0 pre-release](https://img.shields.io/badge/1.5.0-pre--release-0891b2)](https://github.com/polarco/VitaTester/releases/tag/v1.5.0)
[![Validate VitaTester](https://github.com/polarco/VitaTester/actions/workflows/build.yml/badge.svg?branch=master)](https://github.com/polarco/VitaTester/actions/workflows/build.yml)
[![MIT license](https://img.shields.io/badge/license-MIT-64748b)](LICENSE)

## Download

> **Startup fix candidate: 1.5.1.** The 1.5.0 package was reported to return to
> LiveArea immediately on a physical Vita. This source tree corrects thread CPU
> affinity masks and records startup stages; the new physical launch test is pending.
> Published 1.5.0 downloads below remain unchanged. See [startup troubleshooting](docs/STARTUP.md).

**[Download VitaTester 1.5.0 (.vpk)](https://github.com/polarco/VitaTester/releases/download/v1.5.0/VitaTester-1.5.0.vpk)** ·
[SHA-256 file](https://github.com/polarco/VitaTester/releases/download/v1.5.0/VitaTester-1.5.0.vpk.sha256) ·
[Release notes](https://github.com/polarco/VitaTester/releases/tag/v1.5.0)

> **Pre-release — physical Vita validation pending.** Automated tests and the
> VitaSDK build pass. Results are diagnostic evidence, not certification of hardware condition.

<details>
<summary>Verify the download (SHA-256)</summary>

Download the VPK and checksum file into the same folder, then run:

```sh
sha256sum -c VitaTester-1.5.0.vpk.sha256
```

Expected SHA-256 for the 897,663-byte VPK:

```text
c9e813447211bd9e52353376419d280ddf8bc973cb977ad42f5f9c90e7a51da0
```

</details>

## Three modules, one toolkit

| Module | What you can do |
|---|---|
| **Input Test** | Observe buttons, both analog sticks and front/rear multi-touch on the original diagram. No stress HUD. |
| **Stress Test** | Start optional CPU load, compare capture timing and worker progress, use free/guided input diagnostics and save a persistent log. |
| **Scanner** | Inspect passive system/battery/storage evidence; run sensor, speaker, microphone, camera, Wi-Fi and bounded file-read stages; save partial or completed reports. |

The app opens with stress **off**. Scanner collects passive inventory on entry;
active stages require a touch command. Scanner and stress load never run together.
The banner is an original illustration, not an application screenshot.

## Quick installation

1. Use a Vita that can already run homebrew and download the VPK above.
2. Transfer it to the console and install it with VitaShell.
3. Open **VitaTester**, then touch a module card.

Title ID: `VITATESTR` · SFO: `01.50`. Installation replaces an existing VitaTester
bubble; back up any existing reports/logs you want to preserve. No kernel plugin is required.
The application’s command labels are in Portuguese; documentation is available in both languages.

## Controls

| Action | Control |
|---|---|
| Choose a module | Touch its menu card. |
| Return to the menu | Hold exactly **three fingers in the center of the front screen for 2 seconds**; lift all fingers before repeating. |
| Start/stop stress | Touch **Iniciar / Parar stress**. |
| Run a Scanner stage | In **Testes**, choose a stage and touch **Iniciar**, **Parar** or **Repetir**. Confirm **PASSOU / FALHOU / PULADO** (pass/fail/skip). |
| Return to LiveArea / close | Press **PS** / close the bubble in LiveArea. |

Start and Select remain diagnostic inputs. The return gesture waits for stress
cleanup and clock restoration. PS/system events cancel load and active Scanner
stages; returning never starts them automatically. See the [user guide](docs/USAGE.md)
for the gesture area, timers, reporting and error recovery.

## Validation status

| Evidence | Status |
|---|---|
| Production-code host simulations, ASan/UBSan | Passed for 1.5.0 |
| Pinned VitaSDK build, VPK identity/assets | Passed — [release CI](https://github.com/polarco/VitaTester/actions/runs/34237819400) |
| Physical input, timing, device behavior, storage and thermal tests | **Pending for all three modules** |

Follow the [physical validation procedure](docs/VALIDATION.md) before prolonged
stress use. Unsupported or denied queries are inconclusive; they do not prove
hardware failure. Battery readings describe the battery, not processor temperature.

## Documentation & community

- [User guide and build instructions](docs/USAGE.md) · [FAQ](docs/FAQ.md)
- [Scanner design and limits](docs/SCANNER.md) · [Log format](docs/LOG.md)
- [Physical validation](docs/VALIDATION.md) · [Changelog](CHANGELOG.md)
- [Contributing](CONTRIBUTING.md) · [Report a bug, suggest an improvement or submit physical results](https://github.com/polarco/VitaTester/issues/new/choose)

## Credits & license

[MIT](LICENSE) · **Copyright (c) 2015 SMOKE**, preserved verbatim.
Based on [NamelessGhoul0/VitaTester](https://github.com/NamelessGhoul0/VitaTester),
with the history of **SMOKE, Carlanga and NamelessGhoul0** preserved.
Thanks to d3m3vilurr and coderobe (multi-touch), xerpi (vita2dlib),
UrielTapia97 (original images/icons) and Ruben_Wolfe451 (math help).
Fork work for 1.4.0–1.5.0 and presentation: **Filipe Fabiani**.
