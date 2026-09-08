# VitaTester 1.4.0

Input visualization and optional CPU stress for PS Vita, based on SMOKE's
VitaTester. **Build and simulated host tests pass; validation on a physical
Vita is still pending.** This is a diagnostic candidate, not proof of a hardware
fault. See the [physical test procedure](docs/VALIDATION.md) before prolonged use.

The application starts with stress **off**. Three finite integer/float workers
use userland cores 0–2 at requested ARM/GPU/BUS clocks of **444/222/222 MHz**.
Only the UI thread calls vita2d. Capture runs approximately every 8 ms, ahead of
UI, logger and workers in scheduler priority. Effective priorities, worker
CPU affinities and clocks must pass checks before stress can start.

[Download candidate and SHA-256](https://github.com/polarco/VitaTester/releases/tag/v1.4.0)

## Installation and controls

Install `VitaTester-1.4.0.vpk` with VitaShell on a console that can already run
homebrew. Title ID remains `VITATESTR`; installation replaces an existing
VitaTester bubble. The program creates `ux0:data/VitaTester/stresslog.txt`.
No kernel plugin or modified clock API is required.

All commands are on the **front touchscreen**, along the bottom:

| Command | Behavior |
|---|---|
| Iniciar / Parar stress | Start checked load, or stop and restore the saved clocks |
| Modo livre / Modo guiado | Toggle free observation and the guided sequence |
| Limit name/value | Select the next configurable timer |
| − / + | Change the selected timer by 1 second, from 1 to 120 seconds |

Lift all front fingers between commands. Command contacts remain visible and
are included as diagnostic activity. Physical buttons, including Start/Select,
never control the application. The screen permanently displays:

**PS: voltar à LiveArea • Para fechar, encerre a bolha**

PS retains its native behavior. There is no custom double click. Entering an
overlay/LiveArea, suspension/resume callbacks, system events, or a polling gap
over 50 ms cancel load. Resume remains stopped; touch Start explicitly again.

## Diagnostics

| Timer | Default | Interpretation |
|---|---:|---|
| Mantido | 10 s | Continuously held digital button: suspicion |
| Inativo | 30 s | No transition after an observed press/release cycle, with independent activity in the last 5 s: suspicion |
| Etapa | 10 s | Valid observation time allotted to each guided stage |
| Fantasma | 2 s | Persistent rear contacts during “remove fingers”: suspicion |
| Persistencia | 10 s | Continuous rear contact in free mode: warning only |

The guided sequence requests press/release for Up, Down, Left, Right, Cross,
Circle, Square, Triangle, L, R, Start and Select, movement of each analog stick,
rear touch/slide, and removal of rear fingers. Each stage advances at its timer,
recording `observed` or `not_observed`; the result measures the requested action,
not a conclusive physical defect. Move a stick well away from center and slide
at least 24 rear device-coordinate units. Toggle free/guided to restart.

Independent analog activity uses a 20-unit dead zone and at least 12 units of
movement from the last activity anchor. Normal small jitter does not implicate
unused buttons. A digital button that has never completed a cycle is never
marked inactive. Unprompted absence of rear touch proves nothing.

Yellow means suspicion/warning; red means unavailable capture or an operational
error. Alerts log their onset and recovery once. Input anomalies alone do not
stop stress. System interception, API errors and stale samples are distinct log
states. Inference pauses for invalid capture and gaps over 50 ms; the current
guided stage restarts its valid-time window afterward. Previous alert evidence
remains visible until a recovery is observed. Last observed functioning and
detection time delimit a possible failure window, not an exact failure instant.

## HUD and recording

The center HUD preserves the original surrounding controls and both touch
panels. It shows stress duration, **battery temperature** in °C (not processor
temperature), battery percentage, external power, effective clocks, FPS, maximum
poll interval, p99 poll interval, worker progress and last synchronized log
sequence. Poll p99 uses 1 ms histogram bins (rounded upward); `>=101 ms` is the
overflow bin. Worker progress counts completed finite calculation blocks.

[Log format and examples](docs/LOG.md) describe schema 1 JSON Lines. The dedicated
logger appends, handles partial writes and calls `sceIoSyncByFd` after each
record. The queue holds 256 records. An open/write/sync failure or full queue
blocks/revokes stress and restores baseline clocks, without waiting for disk.
Restart the app after resolving the recording failure. Failed restoration keeps
the original baseline and retries; a new baseline is not saved over that error.

Forced bubble termination, a kernel hang or power loss can prevent cleanup and
lose records in transit. Synchronization reduces loss but is not an absolute
durability guarantee. On reopening, a newline isolates any torn last record;
older sessions are never truncated. A log consumer must tolerate a malformed
last line from a previous interrupted session.

## Build and tests

On a Linux host with Docker, Python 3 and a C11 compiler:

```sh
./scripts/test-host.sh
./scripts/build.sh
sha256sum build/VitaTester.vpk
```

The build uses `vitasdk/vitasdk:2026.08-20260815`, pinned as
`vitasdk/vitasdk@sha256:7f5eee50ff95b73c8c847dbfef6227aa0035886444b4d0254c21da8369ff1efc`.
CMake builds the original assets and all runtime modules into SFO `01.40`.
The verifier checks ZIP integrity, exact asset inventory, ELF32 ARM, the packaged
SELF against the same build and application identity/version. CI runs the same
host tests and build. CI artifacts are labeled hardware-pending.

Tests exercise the production diagnostic, clock transaction, queue, complete
write routine and capture/control source with simulated APIs and time. They
cover temporal boundaries, recovery, legitimate inactivity, analog noise,
guided touch, overlay and power events, partial clock failure, idempotent
cleanup, write/sync failure and saturation. They run normally and under
AddressSanitizer/UndefinedBehaviorSanitizer. Host tests cannot prove actual Vita
scheduler timing, callback delivery, thermal behavior or physical input quality.

## Credits and license

MIT license; **Copyright (c) 2015 SMOKE** is preserved verbatim in [LICENSE](LICENSE).
The full upstream history through `a2f0c9f4dfa0970aadbf0463fcae733943aa6de8`
is retained. Credits to **SMOKE, Carlanga and NamelessGhoul0**, and to original
contributors d3m3vilurr and coderobe for multi-touch work; xerpi for vita2dlib,
UrielTapia97 for original images/icons, and Ruben_Wolfe451 for math help.

This fork's 1.4.0 work: Filipe Fabiani. The upstream source is
[NamelessGhoul0/VitaTester](https://github.com/NamelessGhoul0/VitaTester).
API references: [VitaSDK power](https://docs.vitasdk.org/group__ScePowerUser.html),
[VitaSDK headers](https://github.com/vitasdk/vita-headers). The pinned build's
headers define battery temperature as degrees Celsius ×100 and lower numeric
thread priorities as higher scheduling priority.
