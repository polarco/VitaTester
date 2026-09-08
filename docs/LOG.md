# JSON Lines schema 1

Path: `ux0:data/VitaTester/stresslog.txt`. Appended across sessions; blank lines
are permitted. `seq` starts at 1 per process session. `utc` is console wall clock
at enqueue, formatted UTC; it may be wrong if the console clock is wrong.
`mono_us` is the process monotonic observation time. Device timestamps remain
separate in `device_us` in controller/front/rear order; historical samples are
merged chronologically, mapped to the current polling interval, and never
silently treated as fresh after a gap. It is not a raw 125 Hz touch trace.

Every record includes these groups:

| Fields | Meaning |
|---|---|
| `schema`, `app`, `session`, `seq`, `utc`, `mono_us` | Format/app identity and record ordering |
| `event`, `input`, `state`, `last_good_us` | Event and last observed functioning; zero means unknown |
| `battery_temp_c`, `thermal_rc`, `thermal_age_us` | Last valid battery temperature, raw API return, age in microseconds; temperature null until available |
| `battery_pct`, `external_power` | Raw battery/AC API values |
| `clocks_mhz` | Effective ARM/GPU/BUS |
| `clock_set_rc`, `clock_restore_rc` | Individual API returns, same clock order; zero initially also means no call yet |
| `clock_baseline_mhz`, `elapsed_us`, `stress` | Saved restoration target, stress duration and load state at enqueue |
| `capture_rc`, `new_samples`, `device_us` | API counts/errors, new sample counts this poll, last processed timestamps |
| `valid`, `focus_rc`, `intercept_rc`, `intercept`, `overlay`, `callback_rc` | Capture and lifecycle evidence |
| `max_poll_us`, `p99_poll_us`, `fps`, `workers` | Performance and finite block progress |
| `priorities`, `affinity` | Capture/UI/logger/three workers, then worker affinity masks |
| `last_confirmed_seq`, `max_write_ms`, `max_queue_delay_ms` | Last sync-confirmed record and worst disk/queue delays so far |
| `buttons`, `axes`, `contacts`, `limits_s` | Last input state and configured held/idle/stage/ghost/persistence timers |

Events include `session`, `mode`, `limits`, `button`, `guided_result`, `suspect`,
`warning`, `recovery`, `capture_quality`, `capture_state`, `api_error`,
`api_recovery`, `clocks`, `stress`, `lifecycle`, `logger_delay`, and `sample`.
General samples are emitted once a second. New disk/queue latency maxima over
50 ms are also recorded, subject to the same bounded queue. The confirmation
number necessarily refers to an earlier record; synchronization of the current
record cannot be claimed inside that record. Queue saturation or an I/O error
can prevent the failure itself from being written; the red HUD remains the
failure indication and recording stays disabled for that process session.

An abbreviated example (the actual file includes all fields above):

```json
{"schema":1,"session":"example","seq":42,"utc":"2026-09-08T15:00:00Z","mono_us":45000000,"event":"suspect","input":"R","state":"inactive","last_good_us":14000000,"battery_temp_c":32.5,"thermal_age_us":400000,"clocks_mhz":[444,222,222],"valid":true}
```

Here R last completed a press/release cycle at 14 s; at 45 s, inactivity with
independent recent activity triggered suspicion. That interval is evidence for
a possible failure window. It does not prove R failed at 14 s, 45 s, or at all.
A later transition emits `recovery` and updates the evidence.

Never truncate old sessions to repair a torn tail. A new process first appends
and syncs a newline, so a partial last JSON record cannot merge with its session.
A reader should skip blank lines and explicitly report malformed old records.
Neither sync nor process cleanup guarantees survival after kernel failure or
power loss.
