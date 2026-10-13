from pathlib import Path
import json,re,zipfile,hashlib,yaml,sys
R=Path(__file__).resolve().parents[1];M=R/'modules/cck-steno'
s=(R/'config/cck_ball.keymap').read_text();truth=json.loads((R/'config/cck_layout_reference.json').read_text())
# Split ampersand bindings after removing comments. Assertions use actual source.
def bindings(body):
 body=re.sub(r'/\*.*?\*/|//[^\n]*','',body,flags=re.S)
 return [' '.join(x.split()) for x in re.findall(r'&\w+[^&]*',body)]
layers=[bindings(x) for x in re.findall(r'layer_\d+\s*\{\s*display-name[^;]*;\s*bindings\s*=\s*<(.*?)>;',s,re.S)]
assert len(layers)==8 and all(len(x)==64 for x in layers)
assert layers==truth['layers']
assert layers[0][11]=='&kp BACKSPACE' and layers[0][23]=='&st_mode' and layers[5][23]=='&st_mode'
assert layers[5][58]=='&st_sel 0 RALT'
assert len(re.findall(r'sensor-bindings\s*=\s*<&wheel_left &wheel_right>',s))==8
assert 'bindings = <&cck_wheel 60>, <&cck_wheel 61>' in s
assert 'bindings = <&cck_wheel 62>, <&cck_wheel 63>' in s
assert 'tapping-term-ms = <500>' in s and 'hold-preferred' in s
assert re.search(r'&macro_release &mo SELECT_NAV &kp RSHIFT',s)
for i,l in enumerate(layers):assert l[60:]==[('&st_arrow ' if i==5 else '&kp ')+k for k in ['UP','DOWN','LEFT','RIGHT']]
H=R/'config/boards/shields/cck_ball'
dtsi=(H/'cck_ball.dtsi').read_text();layout=(H/'cck_ball-layouts.dtsi').read_text()
assert 'zmk,physical-layout = &cck_ball_layouts' in dtsi
assert 'zmk,physical-layouts =' not in dtsi
rc=re.findall(r'RC\((\d+),(\d+)\)',dtsi);assert len(rc)==64
assert rc[-4:]==[('5',str(i)) for i in range(4)]
assert rc[11]==('0','11') and rc[23]==('1','11')
assert len(re.findall(r'&key_physical_attrs',layout))==64
assert len(json.loads((R/'config/cck_ball.json').read_text())['layouts']['default_transform']['layout'])==64
assert 'ZMK_SPLIT_BLE_ROLE_CENTRAL' not in (H/'Kconfig.defconfig').read_text()
right=(H/'cck_ball_right.conf').read_text();left=(H/'cck_ball_left.conf').read_text()
for v in ['CONFIG_ZMK_SPLIT_ROLE_CENTRAL=y','CONFIG_ZMK_STUDIO=y','CONFIG_ZMK_STUDIO_LOCKING=n','CONFIG_ZMK_STUDIO_TRANSPORT_BLE=y','CONFIG_CORNIX_STENO=y','CONFIG_CCK_WHEEL_REMAP=y']:assert v in right
for v in ['CONFIG_ZMK_STUDIO=n','CONFIG_CORNIX_STENO=n','CONFIG_CCK_WHEEL_REMAP=n']:assert v in left
build=yaml.safe_load((R/'build.yaml').read_text())['include'];assert len(build)==3
assert build[1]['shield']=='cck_ball_right' and build[1]['snippet']=='studio-rpc-usb-uart'
assert 'snippet' not in build[0]; assert build[2]['shield']=='settings_reset'
assert (R/'zephyr/module.yml').is_file()
assert 'rsource "modules/cck-steno/Kconfig"' in (R/'Kconfig').read_text()
assert 'add_subdirectory(modules/cck-steno)' in (R/'CMakeLists.txt').read_text()
with zipfile.ZipFile(R/'reference/CCK_BALL_4x6_v1.0_ORIGINAL.zip') as z:
 def original(suffix):
  ns=[n for n in z.namelist() if n.endswith('/'+suffix) or n==suffix];assert len(ns)==1;return z.read(ns[0]).decode()
 old=original('config/cck_ball.keymap')
 oldlayers=[bindings(x) for x in re.findall(r'\b[\w-]+\s*\{\s*bindings\s*=\s*<(.*?)>;',old,re.S)]
 assert len(oldlayers)==5
 expected={(0,23),(1,13),(1,14),(1,23),(2,6),(2,7),(2,8),(2,9)}
 actual={(i,j) for i in range(5) for j in range(60) if layers[i][j]!=oldlayers[i][j]};assert actual==expected,(actual,expected)
 # Hardware pin assignments and transform entries for all 60 scan positions stay identical.
 oldd=original('config/boards/shields/cck_ball/cck_ball.dtsi')
 assert re.findall(r'RC\((\d+),(\d+)\)',oldd)==rc[:60]
 for prop in ['row-gpios','a-gpios','b-gpios']:
  pat=prop+r'\s*=(.*?);';assert re.findall(pat,oldd,re.S)==re.findall(pat,dtsi,re.S)
 for side in ['left','right']:
  path=f'config/boards/shields/cck_ball/cck_ball_{side}.overlay'
  oldside=original(path);new=(R/path).read_text()
  assert new.replace("        input-processors = <&cck_steno_scroll>;\n", "").startswith(oldside) # only listener processor and appended nodes differ
  assert '&spi3 { status = "disabled"; };' in new
 oldright=original('config/boards/shields/cck_ball/cck_ball_right.conf')
 assert right.startswith(oldright)
 assert (R/'config/west.yml').read_text()==original('config/west.yml')
 # All original geometry preserved.
 oldlayout=original('config/boards/shields/cck_ball/cck_ball-layouts.dtsi')
 assert re.findall(r'<&key_physical_attrs[^>]+>',oldlayout)==re.findall(r'<&key_physical_attrs[^>]+>',layout)[:60]
assert 'CST_OUTPUT_PRESS' in (M/'src/steno/steno_output.c').read_text()
assert 'enum action { TAP, DOWN, UP }' not in (M/'src/steno/steno_output.c').read_text()
assert 'cornix_steno_studio_hid_metadata' in (M/'src/behaviors/behavior_steno_arrow.c').read_text()
assert len(re.findall(r'quick_\d+:',s))==12
print('Package: PASS — 8x64 bindings; 60 original scan coordinates; 4 remappable wheels; mode23; right Studio; original PMW/GPIO; exactly 8 documented edits to original 300 physical bindings')
