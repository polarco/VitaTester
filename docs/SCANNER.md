# Scanner 1.5.0: implementation and verification limits

`main.c` owns graphics and module navigation. A `src/module_NAME.c` file exports
`const VtModule vt_module_NAME`; CMake discovers files with CONFIGURE_DEPENDS and
writes declarations/registry into the build directory. Hooks cover entry, touch
commands, snapshot update, drawing, system interruption and cooperative exit.
Optional hooks can be NULL. Adding a module does not require editing a registry.

`original.c` retains the upstream texture drawing sequence and coordinates.
`tests/original-draw.json` records commands from upstream commit
`a2f0c9f4dfa0970aadbf0463fcae733943aa6de8`. The top exit-combination instruction
is replaced by the approved native PS/LiveArea notice. `navigation.c` owns the
2-second three-contact gesture with release latch and 50 ms continuity limit.

The capture thread alone owns stress clocks/commands. The UI requests disabling
stress and waits for `stress_idle` in a subsequently published snapshot. A saved
clock baseline or active load prevents that acknowledgement. Existing stress
worker threads remain parked; they do not execute stress load in other modules.

`scanner.c` owns one worker, serial jobs and all Scanner file/device work.
Requests publish arguments with atomics; snapshots copy under a mutex. The
cancellation epoch is captured at submission, so a system event before a worker
starts also invalidates that request. Active loops check cancellation and focus.
No worker calls vita2d. Camera acquisition uses a 256 KiB CDRAM allocation and
160×120 ABGR frames; the UI copies completed snapshots into a texture only after
previous rendering finishes. Neither buffers nor textures enter reports.
Ownership is retained and transitions blocked if device release fails.

`scan_model.c` is shared by screen and file. Each inventory row records value,
unit, source, UTC epoch and availability. Error does not mean absence. The model
keeps 220 rows; saturation adds an explicit coverage warning. Dump metadata is
limited to 32 entries per directory, with a visible incompleteness marker. Usual
locations queried are `ux0:data`, `ux0:` and `ud0:PSP2CORE`; inaccessible locations
are recorded. Config reads are bounded at 8192 bytes, explicitly reported when
capped. File selection is paginated; reads are bounded at 1 MiB.

Battery nominal values accept 0 (unset) or 100–10000 mAh, and thresholds 1–100%.
Full capacity must be positive and at most 10000 mAh, remaining charge must be
consistent with full capacity, and the resulting ratio must not exceed 120%.
System SOH is valid at 0–100% when the device has a battery. Remaining charge is
used only to reject inconsistent readings, never as the health numerator.
Screening applies independently to valid SOH and calculated health, strictly
below the threshold. A reading exactly at 80% does not trigger the default alert.

AP scanning and Bluetooth have no documented operation in the pinned interface
used here. Exact device PCH/region and a full kernel module inventory are unavailable.
The SDK-declared VshBridge read queries for real firmware, taiHEN name and insertion
may be denied on a particular firmware; return values are retained, without private
declarations, code injection or a kernel helper. Configured plugin evidence and
visible process-module evidence are separate. Boot-file presence is inconclusive.

Reports reserve a new filename exclusively, write a temporary sibling completely,
sync the mount, rename and sync again. A failed stage keeps the in-memory report
and is retriable. A failure after rename can leave the updated file visible while
still reporting uncertain durability. A crash between reservation and replacement
may leave an empty reserved filename or temporary file; the next session skips it.
No old session is reused. A filesystem/kernel call that stalls cannot be forcibly
interrupted safely; the graphics thread remains responsive and transition waits.
These filesystem guarantees still need confirmation on actual Vita storage.

Host tests compile production capture and Scanner code with explicit fake SDK
entry points. They validate logic and error paths, not ABI behavior on hardware,
acoustic output, camera image correctness, sensor calibration or flash durability.
The VPK is a pre-release pending the physical procedure in [VALIDATION.md](VALIDATION.md).

Primary references used alongside the headers inside the pinned Docker image:

- [scePower capacity, SOH and units](https://docs.vitasdk.org/group__ScePowerUser.html).
- [VitaSDK headers](https://github.com/vitasdk/vita-headers): motion, audioin/out,
  camera, netctl, sysmem, modulemgr, vshbridge and filesystem declarations.
- [VitaSDK camera sample](https://github.com/vitasdk/samples/blob/master/camera/src/main.c):
  CDRAM-backed acquisition and pixel format.
