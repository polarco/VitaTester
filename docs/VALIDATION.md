# Validation — VitaTester 1.5.0 (including inherited stress gates)

## Automated status

- Host diagnostic, clock rollback, writer and queue tests: PASS.
- Actual capture/control C module with simulated Vita APIs: PASS.
- AddressSanitizer and UndefinedBehaviorSanitizer: PASS.
- Production JSONL records parsed and checked: PASS.
- Pinned VitaSDK build and VPK structure/identity/asset verification: PASS.
- GitHub Actions: check the run for the published commit; local results alone
  are not a remote CI result.
- Physical Vita: **NOT RUN**. No installation, plugin change, jailbreak
  recovery or thermal session was performed during implementation.

## Physical gates, in order

1. Install the candidate only on an already homebrew-capable console. Back up
   an existing VitaTester log if needed. Launch with load off and select Stress Test; confirm original
   button indicators, analog movement and both panels. Press Start and Select
   together: they must remain diagnostic input without exiting.
2. Record the normal baseline clocks and observe capture for 60 seconds. Verify
   that unused buttons stay clear and small analog jitter does not create
   independent activity. Check battery temperature against the battery reading
   from another trusted tool, accounting for sampling time.
3. Start stress by touch. Verify priorities `capture < UI < logger < workers`
   numerically, worker affinities `0x10000/0x20000/0x40000` (SDK user CPU masks), all three progress counters increasing,
   and effective clocks `444/222/222`. Compare normal/stress FPS and polling.
   **Acceptance: p99 polling ≤20 ms** in each mode; every gap >50 ms must be
   marked inconclusive. Log all sample counts and API returns.
4. Stop by touch. Verify the previous ARM/GPU/BUS baseline, counters stopped,
   and synchronized `stopped_touch` entry. Start/stop repeatedly; baseline must
   not drift. Independently inspect clock restoration when possible.
5. Start, press PS, return from LiveArea. Verify restoration and no automatic
   restart. Repeat with the quick menu/overlay, Power suspend/resume, and then
   close the bubble. Confirm resulting clocks independently and inspect the
   final committed sequence. A missing final record on forced close is not
   evidence of successful cleanup.
6. Test guided press/release, each stick, slow rear slide and finger removal.
   Omit rear touch once and retain rear fingers once. Expect one suspicion
   onset and one recovery, without load cancellation from the anomaly. Gaps
   and interception must invalidate/restart the stage, not count as failures.
7. With load off, verify an unwritable log prevents start. Exercise an I/O fault
   only in a controlled test environment; do not remove mounted media during
   writes. Verify load cancellation and baseline restoration on write failure.
8. Only after gates 1–7 pass, compare a longer normal session and stress session,
   recording ambient conditions, battery temperature age, battery %, charging,
   clocks, polling and worker progress. Stop on undesirable device behavior.

Archive physical logs privately. Report console model, system state, candidate
SHA-256 and pass/fail for each gate; do not promote simulated results to physical
validation. No exact time of a physical defect can be inferred from these tests.

## 1.5.0 module/Scanner gates — all physically pending

1. Open each menu card by touch. In Input Test, compare the diagram against the
   original, test all buttons including Start/Select, both analogs and all front/
   rear contacts. Verify no stress HUD/commands and native PS behavior.
2. Hold three fingers centrally for two seconds; release before repeating. Move
   one outside the region and interrupt with LiveArea/suspend: neither case should
   finish a previously started gesture after return.
3. Start stress, return by gesture and confirm the previous ARM/GPU/BUS baseline
   before entering Scanner. Repeat with LiveArea/suspension; no auto-restart.
4. Inspect Scanner passive inventory against known device information. Compare
   full/remaining capacity and SOH; test unset nominal, a valid nominal and 80% limit.
   A denied query is inconclusive. Configured plugins must not be called loaded.
5. Run all sensor stages, left/right tones with external outputs disconnected,
   microphone RMS/peak, both camera previews and Wi-Fi state. Confirm every verdict.
   Use PULADO for optional/unavailable hardware, AP scan and Bluetooth coverage.
6. Select files in each available mount, including directories exceeding 64 entries.
   Check read bytes stop at 1 MiB and files remain unchanged. Insertion and physical
   mapping must remain separate. Never open a dump to perform this check.
7. Interrupt each active test with Parar, PS and suspend; confirm buffers/devices
   release and a new explicit start is needed. Check camera orientation/colors,
   tone duration/channel/ramping, actual cancellation latency and repeated entry/exit.
8. Review partial, completed and skipped reports on screen and filesystem. Start
   sessions in the same second; preserve all files. Exercise unavailable/full storage
   where safe, retry Save, and verify the report remains visible after failures.
   Validate rename/replacement/sync behavior on real Vita filesystems.

The existing stress scheduler, clock and thermal gates remain mandatory. Host
simulation and CI approval alone do not close any physical gate above.
