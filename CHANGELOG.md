# Changelog

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
