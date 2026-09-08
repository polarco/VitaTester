#!/usr/bin/env python3
"""Compare draw commands with the pinned original input diagram fixture."""
import json
import re
from pathlib import Path
root = Path(__file__).resolve().parent.parent
source = (root / 'src/original.c').read_text().replace('select_texture', 'select')
commands = re.findall(r'vita2d_draw_texture(?:_rotate)?\([^;]+;', source)
expected = json.loads((root / 'tests/original-draw.json').read_text())
assert commands == expected, 'original texture order/coordinates changed'
for expression in [
    '(85 + lx / 8)', '(802 + rx / 8)',
    '(lerp(touch.report[i].x, 1919, 960) - 50)',
    '(lerp(touch.report[i].y, 1087, 544) - 56.5)',
    '(lerp(touch.report[i].y, 1285, 855) - 113)',
]:
    assert expression in source, expression
assert 'if (stress) vt_draw_hud' in source
assert 'EXIT_COMBO' not in source
print('original diagram: texture commands, order, coordinates, analog and multitouch transforms OK')
