[Home](../README.md) · [Português](FAQ.pt-BR.md)

# Frequently asked questions

### How do I install or update?

Download the VPK and checksum from the [1.5.0 pre-release](https://github.com/polarco/VitaTester/releases/tag/v1.5.0).
Verify the download, transfer it and install with VitaShell on an already
homebrew-capable Vita. Title ID `VITATESTR` replaces the existing bubble.
Back up reports and logs first. This project does not install a jailbreak or kernel plugin.

### Why does the return gesture not work?

Use exactly three contacts on the front panel, all inside screen coordinates
x=240–719 and y=110–419, continuously for two seconds. Lift every finger before
trying again. A focus loss or capture gap cancels the timer. Stress clock
restoration or a Scanner device-release error can hold the transition until
cleanup succeeds. Start/Select do not navigate; PS opens LiveArea normally.

### Does battery health prove my battery is worn out?

No. Scanner separates system SOH from `100 × full capacity / nominal capacity`.
Enter the installed battery’s nominal mAh in **Ajustes** to enable the latter;
zero means unset. Remaining charge is not wear. Invalid/inconsistent readings
make the calculation unavailable. The default alert is strictly below 80%,
a configurable screening threshold, not a diagnosis or repair function.

### Why is a test unavailable? Should I mark it failed?

Optional hardware, denied APIs and missing documented userland operations are
coverage limits. AP scanning and Bluetooth are unavailable in this build;
Wi-Fi only reports connection state/signal/channel. Exact PCH model, region and
full kernel inventory are also unavailable. Use **PULADO** when appropriate;
a technical error and a human hardware verdict are separate evidence.

### Where are reports and settings saved?

- Stress log: `ux0:data/VitaTester/stresslog.txt` (append-only JSON Lines).
- Scanner reports: `ux0:data/VitaTester/scan_YYYYMMDD_HHMMSS.txt` (UTC; collision suffixes).
- Scanner settings: `ux0:data/VitaTester/scanner.cfg`.

**Relatorio** shows the saved model. Leaving saves partial coverage; **Concluir**
requires an explicit verdict for every stage. Use **Salvar / Repetir gravacao**
to retry failures. Previous sessions are preserved. Reports exclude raw audio,
images, dump/file contents and network identifiers. Review anything before sharing
publicly; do not attach dumps or personal information.

### Is a green CI badge physical validation?

No. Host simulations and package checks verify software logic and build structure.
Input quality, timing, sound, camera behavior, storage durability and thermal
behavior still require the [physical procedure](VALIDATION.md). Long stress
sessions come after the stop, clock restoration and lifecycle gates.
