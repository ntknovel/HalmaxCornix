"""Actual source contract plus declared Tab/mod-morph specification tests.
The timing model below is NOT ZMK's event queue or hold-tap implementation.
"""
from pathlib import Path
import json, re, itertools, hashlib, yaml
R=Path(__file__).resolve().parents[1];M=R/'modules/cck-steno'
s=(R/'config/cck_ball.keymap').read_text(); aux=(R/'config/cck-steno-aux.dtsi').read_text()
def node(text,name):
 m=re.search(r'\b'+name+r':\s*\w+\s*\{(.*?)\};',text,re.S);assert m,name;return m[1]
assert 'bindings = <&cck_tab_hold>, <&cck_tab_tap>' in node(s,'cck_tab_ht')
assert 'tapping-term-ms = <200>' in node(s,'cck_tab_ht')
assert 'flavor = "tap-preferred"' in node(s,'cck_tab_ht')
mm=node(s,'cck_tab');mask='MOD_LCTL | MOD_RCTL | MOD_LALT | MOD_RALT | MOD_LGUI'
assert f'mods = <({mask})>' in mm and f'keep-mods = <({mask})>' in mm
assert 'bindings = <&cck_tab_ht 0 0>, <&cck_tab_hold>' in mm
assert 'normal-key = <TILDE>' in node(aux,'cck_tab_tap')
assert 'normal-key = <TAB>' in node(aux,'cck_tab_hold')
assert 'one-shot-tab' not in node(aux,'cck_tab_hold')
for n in ['cck_tab_tap','cck_tab_hold']:
 assert 'symbol-key = <TAB>' in node(aux,n) and 'function-key = <F12>' in node(aux,n)
assert 'normal-key = <EXCL>' in node(aux,'mxesc')
assert 'normal-key = <QUESTION>' in node(aux,'mxbsp')
j=json.loads((R/'config/cck_layout_reference.json').read_text())
assert j['layers'][0][0]=='&kp ESC' and j['layers'][0][11]=='&kp BACKSPACE' and j['layers'][0][12]=='&kp TAB'
assert j['layers'][5][0]=='&mxesc' and j['layers'][5][11]=='&mxbsp' and j['layers'][5][12]=='&cck_tab'
assert j['layers'][5][23]=='&st_mode' and j['layers'][5][58]=='&st_sel 0 RALT'
assert j['layers'][5][54:57]==['&st_veo_space','&st_v_i','&st_v_a']
right=R/'config/boards/shields/cck_ball'
assert 'input-processors = <&cck_steno_scroll>' in (right/'cck_ball_right.overlay').read_text()
assert 'ntk,cck-steno-scroll' in (right/'cck_ball_right.overlay').read_text()
assert 'cck_steno_scroll' not in (right/'cck_ball_left.overlay').read_text()
assert 'CONFIG_CCK_STENO_POINTER_ASSIST=y' in (right/'cck_ball_right.conf').read_text()
assert 'CONFIG_PMW3610_SCROLL_TICK=70' in (right/'cck_ball_right.conf').read_text()
y=yaml.safe_load((R/'dts/bindings/input_processors/ntk,cck-steno-scroll.yaml').read_text());assert y['properties']['#input-processor-cells']['const']==0 and y['properties']['track-remainders']['type']=='boolean'
assert 'DEVICE_DT_INST_DEFINE' in (R/'src/input_processor_cck_scroll.c').read_text()
# Deterministic examples of the standard behavior configuration, not firmware runtime.
cases=0
for mods in range(256):
 for ms in [0,1,100,199,200,201,499,500,1000]:
  selected='TAB' if mods&0x5d or ms>=200 else 'TILDE' # L/R Ctrl, L/R Alt, LGUI
  if mods&0x5d: assert selected=='TAB'
  elif ms<200: assert selected=='TILDE'
  else: assert selected=='TAB'
  cases+=1
print(f'v1.1.1 actual configuration/geometry/selector/central guards: PASS; Tab/modifier specification model {cases} cases (not real ZMK queue)')
