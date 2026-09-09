# Changelog

## 1.5.1 — 2026-09-08

Startup correction; SFO `01.51`. Candidate awaiting a new physical launch test.

- Fix capture/logger/worker CPU affinity arguments to use VitaSDK user masks
  `0x10000/0x20000/0x40000` and their union; validate worker readback with the same ABI.
- Preserve initialization error codes and stage names instead of returning -1.
- Record startup stages in append-only `ux0:data/VitaTester/startup.txt`; show
  runtime initialization errors on screen with native PS exit behavior and no load.
- Reject the old invalid affinity values in host mocks; cover priority, mutex,
  capture-create and capture-start error propagation.
- Device readback confirmed the failing installation matched 1.5.0 exactly,
  with no stress log directory. The affinity mismatch is confirmed against the
  pinned SDK; whether it fully explains the reported immediate exit awaits hardware.
- Preserve the published 1.4.0/1.5.0 VPKs, artwork and MIT attribution.

## Presentation update — 2026-09-08

Documentation/presentation correction; archive 3.4.1. Application stays 1.5.0; no new binary.

- Add original vector banner and 1200×630 social artwork, labeled as illustrations.
- Publish equivalent English/Portuguese READMEs, usage guides and FAQs with direct
  VPK/checksum links and explicit pre-release/physical-validation status.
- Add bilingual contribution guidance, issue forms and a pull request template.
- Refresh repository metadata and 1.5.0 release notes; preserve tags, release
  assets, the 1.4.0 release, runtime sources, original artwork and MIT attribution.

## 1.5.0 — 2026-09-08

Feature; SFO `01.50`. Physical validation of all three modules pending.

- Add independent Input Test, Stress Test and Scanner modules with CMake-generated
  registration. Preserve original diagram/assets, MIT license and upstream history.
- Return through a three-finger central hold; isolate stress commands and block
  transitions until clock restoration and device cleanup complete.
- Add passive battery/system/storage/configuration/dump-metadata inventory, separate
  SOH/calculated health, optional nominal capacity and configurable screening limit.
- Add explicit sensor, audio, microphone, camera, Wi-Fi and read-only file stages;
  unsupported AP scanning/Bluetooth remain unavailable with human skip verdicts.
- Add paginated UTF-8 reports, partial/final state, collision-safe session names,
  complete synchronized temporary replacement and retry after persistence errors.
- Extend host tests and sanitizers to Scanner, gesture, module isolation, failed
  cleanup, disk faults, canceled reads and preserved drawing commands. Keep SDK digest.

## 1.4.0 — 2026-09-08

Feature; SFO `01.40`. Host/build validated; physical validation pending.

- Preserve the original button, analog and multi-touch visualization. All app
  commands now use the front panel; Start/Select are diagnostic inputs and PS
  keeps its native LiveArea action.
- Separate 8 ms capture/control, rendering, three CPU workers, and append-only
  synchronized JSON Lines logging. Verify priorities and CPU affinities before
  stress; request/read back 444/222/222 MHz with transactional rollback.
- Free/guided diagnostics, configurable time limits, debounced suspicion and
  recovery, and explicit inconclusive periods for system interception, capture
  errors and stale samples. No inference of an exact physical failure time.
- Stop and restore baseline on touch, lifecycle events, polling gaps or log
  failure; never automatically restart after resume. Show battery temperature,
  effective clocks, capture timing and worker progress.
- Add host tests with simulated Vita APIs, ASan/UBSan, pinned VitaSDK container,
  package verification and CI. Preserve SMOKE's MIT license and source history.

## 1.3 — upstream (SFO 01.30)

Inherited from `a2f0c9f4dfa0970aadbf0463fcae733943aa6de8` in
[NamelessGhoul0/VitaTester](https://github.com/NamelessGhoul0/VitaTester).
Original multi-touch visualization and rear-panel coordinate correction.
