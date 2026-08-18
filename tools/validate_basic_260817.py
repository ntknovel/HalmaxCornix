#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path
import hashlib
import json
import re
import shutil
import subprocess
import sys
import yaml

ROOT = Path(sys.argv[1] if len(sys.argv) > 1 else '.').resolve()
KEYMAP = (ROOT / 'config/cornix.keymap').read_text(encoding='utf-8')
CONF = (ROOT / 'config/cornix.conf').read_text(encoding='utf-8')
RESET_CONF = (ROOT / 'config/settings_reset.conf').read_text(encoding='utf-8')
META = json.loads((ROOT / 'config/cornix.json').read_text(encoding='utf-8'))
SOURCE = json.loads((ROOT / 'source/cornix 260817.vil').read_text(encoding='utf-8'))


def fail(msg: str) -> None:
    raise RuntimeError(msg)


def layer_block(node: str) -> str:
    m = re.search(r'\b' + re.escape(node) + r'\s*\{.*?bindings\s*=\s*<(.*?)>;', KEYMAP, re.S)
    if not m:
        fail(f'missing layer: {node}')
    return m.group(1)


def combo_block(cid: int) -> str:
    m = re.search(rf'\bcombo_{cid:02d}\s*\{{(.*?)\n\s*\}};', KEYMAP, re.S)
    if not m:
        fail(f'missing combo_{cid:02d}')
    return m.group(1)


# YAML / JSON / project structure.
for path in [ROOT/'build.yaml', ROOT/'config/west.yml', ROOT/'.github/workflows/build.yml',
             ROOT/'zephyr/module.yml', ROOT/'dts/bindings/behaviors/zmk,behavior-rf-power.yaml']:
    yaml.safe_load(path.read_text(encoding='utf-8'))

layers = ['base_layer','nav_layer','mouse_layer','edit_layer','media_bt_layer',
          'numpad_layer','symbol_layer','fn_layer','punct_layer','plain_layer',
          'mouse_acl2_layer','mouse_acl0_layer']
for node in layers:
    refs = re.findall(r'&[A-Za-z_][A-Za-z0-9_]*', layer_block(node))
    if len(refs) != 50:
        fail(f'{node}: expected 50 bindings, got {len(refs)}')
if KEYMAP.count('sensor-bindings = <') != 12:
    fail('expected sensor bindings on all 12 layers')
if len(META['layouts']['LAYOUT_50']['layout']) != 50 or len(META['sensors']) != 2:
    fail('Keymap Editor metadata mismatch')
if len(SOURCE['layout']) != 10:
    fail('source VIL user-layer count mismatch')

# Exact combo masks reconstructed from the Vial keycodes at each physical position.
base_plain = {4,7,8,11,12,13,14,16,17,19,20}
base_numpad = {6,15}
base_only = {0,1,2,3,5,9,10,21,22,23,24,25,26,27,28,29,31}
for cid in base_plain:
    if not re.search(r'layers\s*=\s*<BASE PLAIN>\s*;', combo_block(cid)):
        fail(f'combo_{cid:02d}: expected BASE PLAIN')
for cid in base_numpad:
    if not re.search(r'layers\s*=\s*<BASE NUMPAD>\s*;', combo_block(cid)):
        fail(f'combo_{cid:02d}: expected BASE NUMPAD')
for cid in base_only:
    if not re.search(r'layers\s*=\s*<BASE>\s*;', combo_block(cid)):
        fail(f'combo_{cid:02d}: expected BASE only')
if not re.search(r'layers\s*=\s*<PLAIN>\s*;', combo_block(30)):
    fail('combo_30: expected PLAIN only')
if KEYMAP.count('timeout-ms = <40>;') != 31:
    fail('expected 31 active combos with 40ms timeout')

# Correct positional layer-tap conversion for Vial LT4(Esc) / LT7(Delete).
for node, trigger in [
    ('ltl200', '<6 7 8 9 10 11 18 19 20 21 22 23 31 32 33 34 35 36 37 44 45 46 47 48 49>'),
    ('ltr200', '<0 1 2 3 4 5 12 13 14 15 16 17 24 25 26 27 28 29 30 38 39 40 41 42 43>'),
]:
    m = re.search(rf'\b{node}:.*?\{{(.*?)\n\s*\}};', KEYMAP, re.S)
    if not m:
        fail(f'missing {node}')
    block = m.group(1)
    for required in ['flavor = "balanced";', 'tapping-term-ms = <200>;',
                     'require-prior-idle-ms = <150>;', 'hold-trigger-on-release;',
                     f'hold-trigger-key-positions = {trigger};']:
        if required not in block:
            fail(f'{node}: missing {required}')
base = layer_block('base_layer')
if '&ltl200 MEDIA_BT ESC' not in base or '&ltr200 FN DEL' not in base:
    fail('correct LT4/LT7 bindings are missing')
if '&lt200 ' in KEYMAP:
    fail('stale generic lt200 remains')

# KC_ACL0 / KC_ACL2 momentary speed-mode conversion.
for token in ['#define MOUSE_ACL2 10', '#define MOUSE_ACL0 11',
              '&mo MOUSE_ACL0', '&mo MOUSE_ACL2',
              'm0mv: mouse_move_acl0', 'm2mv: mouse_move_acl2',
              'm0sc: mouse_scroll_acl0', 'm2sc: mouse_scroll_acl2',
              'm0mv_input_listener:', 'm2mv_input_listener:',
              'm0sc_input_listener:', 'm2sc_input_listener:']:
    if token not in KEYMAP:
        fail(f'missing mouse acceleration token: {token}')

for node, expected in [
    ('m0mv', ['time-to-max-speed-ms = <0>;', 'acceleration-exponent = <0>;']),
    ('m2mv', ['time-to-max-speed-ms = <0>;', 'acceleration-exponent = <0>;']),
    ('m0sc', ['time-to-max-speed-ms = <0>;', 'acceleration-exponent = <0>;']),
    ('m2sc', ['time-to-max-speed-ms = <0>;', 'acceleration-exponent = <0>;']),
]:
    m = re.search(rf'\b{node}:.*?\{{(.*?)\n\s*\}};', KEYMAP, re.S)
    if not m:
        fail(f'missing acceleration behavior: {node}')
    for item in expected:
        if item not in m.group(1):
            fail(f'{node}: missing {item}')

acl0 = layer_block('mouse_acl0_layer')
acl2 = layer_block('mouse_acl2_layer')
for token in ['&m0mv MOVE_Y(-150)', '&m0mv MOVE_Y(150)',
              '&m0mv MOVE_X(-150)', '&m0mv MOVE_X(150)',
              '&m0sc MOVE_Y(3)', '&m0sc MOVE_Y(-3)']:
    if token not in acl0:
        fail(f'ACL0 layer missing {token}')
for token in ['&m2mv MOVE_Y(-600)', '&m2mv MOVE_Y(600)',
              '&m2mv MOVE_X(-600)', '&m2mv MOVE_X(600)',
              '&m2sc MOVE_Y(10)', '&m2sc MOVE_Y(-10)']:
    if token not in acl2:
        fail(f'ACL2 layer missing {token}')

# Management, dead-layer recovery, and runtime TX controls.
for token in ['#include <dt-bindings/zmk/outputs.h>', '&bt BT_CLR', '&bt BT_CLR_ALL',
              '&out OUT_BLE', '&out OUT_USB', '&mo EDIT', '&mo SYMBOL',
              '&rf0', '&rfu', '&rfd', '&rfm']:
    if token not in KEYMAP:
        fail(f'missing management token: {token}')
for node, command in [('rf0',1),('rfu',2),('rfd',0),('rfm',3)]:
    m = re.search(rf'\b{node}:\s*{node}\s*\{{(.*?)\n\s*\}};', KEYMAP, re.S)
    if not m or f'command = <{command}>;' not in m.group(1):
        fail(f'RF behavior {node} has wrong command')

for symbol in ['CONFIG_BT_CTLR_TX_PWR_PLUS_8=y', 'CONFIG_BT_CTLR_PHY_2M=n',
               'CONFIG_ZMK_BATTERY_REPORT_INTERVAL=300', 'CONFIG_ZMK_POINTING=y',
               'CONFIG_BT_HCI_VS=y', 'CONFIG_BT_CTLR_TX_PWR_DYNAMIC_CONTROL=y']:
    if symbol not in CONF:
        fail(f'missing config: {symbol}')
if 'CONFIG_ZMK_POINTING=n' not in RESET_CONF or 'CONFIG_BT_CTLR_TX_PWR_DYNAMIC_CONTROL=n' not in RESET_CONF:
    fail('settings_reset.conf is not minimal')

# RF module integration.
for rel in ['CMakeLists.txt','Kconfig','zephyr/module.yml',
            'dts/bindings/behaviors/zmk,behavior-rf-power.yaml',
            'src/behaviors/behavior_rf_power.c']:
    if not (ROOT/rel).is_file():
        fail(f'missing RF module file: {rel}')
if 'src/behaviors/behavior_rf_power.c' not in (ROOT/'CMakeLists.txt').read_text(encoding='utf-8'):
    fail('RF source is not connected to CMake')

# No STENO engine or dictionary is present in the build tree.
for rel in ['config/cornix_steno.dtsi','include/cornix_steno','src/steno']:
    if (ROOT/rel).exists():
        fail(f'unexpected STENO path: {rel}')
scan = KEYMAP + CONF + (ROOT/'CMakeLists.txt').read_text(encoding='utf-8') + (ROOT/'Kconfig').read_text(encoding='utf-8')
for forbidden in ['CORNIX_STENO', 'steno_dictionary', 'behavior_steno']:
    if forbidden.lower() in scan.lower():
        fail(f'unexpected STENO symbol: {forbidden}')

# Compile the custom RF source against pinned-API-shaped stubs.
cc = shutil.which('cc') or shutil.which('gcc') or shutil.which('clang')
api_result = 'SKIP (no C compiler)'
if cc:
    stubs = ROOT/'validation/api_stubs'
    cmd = [cc, '-std=c11', '-Wall', '-Wextra', '-Werror',
           '-Wno-unused-function', '-Wno-unused-variable',
           '-include', str(stubs/'config.h'), '-I'+str(stubs),
           '-fsyntax-only', str(ROOT/'src/behaviors/behavior_rf_power.c')]
    subprocess.run(cmd, check=True, stdout=subprocess.DEVNULL)
    api_result = 'PASS'

# Build matrix and workflow hardening.
build = yaml.safe_load((ROOT/'build.yaml').read_text(encoding='utf-8'))
rows = build.get('include', [])
if len(rows) != 3:
    fail('build.yaml must contain exactly left/right/reset rows')
if rows[2].get('shield') != 'settings_reset' or 'snippet' in rows[2]:
    fail('settings reset row must be shield-only without Studio/USB snippet')
workflow = (ROOT/'.github/workflows/build.yml').read_text(encoding='utf-8')
for watched in ['src/**','dts/**','zephyr/**','CMakeLists.txt','Kconfig']:
    if watched not in workflow:
        fail(f'workflow does not watch {watched}')
if '6e2ef41e022d555b10f116e395832913f71717b3' not in workflow:
    fail('workflow is not pinned')

print('Cornix ZMK BASIC 260817 FIXED v1.2 validation: PASS')
print('- layers: 10 user + 2 internal mouse-speed layers, all 50 positions')
print('- encoders: 2 x 12 layers')
print('- combos: 31 with reconstructed BASE/PLAIN/NUMPAD masks')
print('- LT4(Esc) / LT7(Delete): positional balanced conversion PASS')
print('- KC_ACL0 precision / KC_ACL2 maximum momentary modes: PASS')
print('- Editing/Symbols entry + BT recovery/output keys: PASS')
print('- runtime RF 0/+/-/MAX module: PASS')
print(f'- RF C API-shaped syntax: {api_result}')
print('- settings_reset isolation: PASS')
print('- STENO engine/dictionary: absent')
print('- keymap SHA-256:', hashlib.sha256((ROOT/'config/cornix.keymap').read_bytes()).hexdigest())
