#!/usr/bin/env python3
import json
from pathlib import Path
records = [json.loads(line) for line in Path('build-host/runtime.jsonl').read_text().splitlines()]
assert records
for r in records:
    assert r['schema'] == 1 and r['app'] == '1.5.2'
    assert r['last_good_us'] <= r['mono_us']
    assert r['last_confirmed_seq'] < r['seq']
    assert len(r['clocks_mhz']) == len(r['device_us']) == len(r['capture_rc']) == 3
    assert all(0 <= t <= 2**63 - 1 for t in r['device_us'])
    assert r['utc'].endswith('Z')
assert any(r['event'] == 'stress' and r['state'] == 'started' for r in records)
assert any(r['state'] == 'system_intercepted' for r in records)
assert any(r['state'] == 'power_suspend_or_resume' for r in records)
assert any(r['state'] == 'delayed_inconclusive' for r in records)
print(f'JSONL: {len(records)} production-formatted records parsed and checked')
