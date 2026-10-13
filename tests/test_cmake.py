"""Run real CMake configuration with an explicitly synthetic ZMK header/API.
Not an ARM/Zephyr build. Verify both known API signatures and failure guards.
"""
from pathlib import Path
import tempfile,subprocess,json
R=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix='cck-cmake-') as td:
 t=Path(td);case=0
 for central,arity,endpoint,bad in [(True,1,'zmk_endpoint_set_preferred_transport',False),(True,2,'zmk_endpoints_select_transport',False),(False,1,'zmk_endpoint_set_preferred_transport',False),(True,0,'bogus',True),(True,1,'bogus',True)]:
  case+=1;d=t/str(case);d.mkdir();a=d/'fake-zmk';(a/'include/zmk').mkdir(parents=True)
  sig='uint8_t layer'+(', bool locking' if arity==2 else '')
  h=(f'int zmk_keymap_layer_activate({sig});\nint zmk_keymap_layer_deactivate({sig});\n' if arity else '')
  h+='void zmk_keymap_get_layer_binding_at_idx(void);\nvoid zmk_keymap_layer_index_to_id(void);\nvoid zmk_keymap_layer_default(void);\n'
  (a/'include/zmk/keymap.h').write_text(h)
  (a/'include/zmk/endpoints.h').write_text(f'int {endpoint}(int transport);\n')
  (d/'empty.c').write_text('int main(void){return 0;}\n')
  code='cmake_minimum_required(VERSION 3.20)\nproject(cck_shape C)\nadd_executable(app empty.c)\n'
  code+=f'set(APPLICATION_SOURCE_DIR "{a}")\nset(CONFIG_ZMK_SPLIT ON)\nset(CONFIG_ZMK_SPLIT_ROLE_CENTRAL '+('ON' if central else 'OFF')+')\n'
  for k in ['CORNIX_STENO','CCK_WHEEL_REMAP','CCK_USB_AUTO_SELECT']:
   code+=f'set(CONFIG_{k} '+('ON' if central else 'OFF')+')\n'
  code+='function(target_sources_ifdef feature target)\n if(${feature})\n  target_sources(${target} ${ARGN})\n endif()\nendfunction()\n'
  code+=f'add_subdirectory("{R}" cck)\nget_target_property(d app COMPILE_DEFINITIONS)\nget_target_property(s app SOURCES)\nfile(WRITE "${{CMAKE_BINARY_DIR}}/result.txt" "${{d}}\\n${{s}}")\n'
  (d/'CMakeLists.txt').write_text(code)
  result=subprocess.run(['cmake','-S',str(d),'-B',str(d/'b')],capture_output=True,text=True)
  assert (result.returncode!=0)==bad,(case,result.stdout,result.stderr)
  if not bad:
   s=(d/'b/result.txt').read_text()
   if central:
    assert f'CCK_LAYER_API_ARITY={arity}' in s and endpoint in s and 'behavior_cck_wheel.c' in s
   else:assert 'behavior_cck_wheel.c' not in s and 'steno_engine.c' not in s and 'usb_auto_select.c' not in s
  print('CMake actual configure / mock ZMK headers:',case,'PASS')

with tempfile.TemporaryDirectory(prefix='cck-v111-cmake-') as td:
 for ip_ok in [0,1,2]:
  d=Path(td)/str(ip_ok);d.mkdir();a=d/'fake-zmk';(a/'include/zmk').mkdir(parents=True);(a/'include/drivers').mkdir()
  (a/'include/zmk/keymap.h').write_text('int zmk_keymap_layer_activate(int l); int zmk_keymap_layer_deactivate(int l); void zmk_keymap_get_layer_binding_at_idx(void); void zmk_keymap_layer_index_to_id(void); void zmk_keymap_layer_default(void);')
  if ip_ok: (a/'include/drivers/input_processor.h').write_text('struct zmk_input_processor_state {}; struct api {void *handle_event;};' if ip_ok==2 else 'wrong header')
  (d/'empty.c').write_text('int main(void){return 0;}')
  code='cmake_minimum_required(VERSION 3.20)\nproject(assist C)\nadd_executable(app empty.c)\n'
  code+=f'set(APPLICATION_SOURCE_DIR "{a}")\nset(CONFIG_ZMK_SPLIT ON)\nset(CONFIG_ZMK_SPLIT_ROLE_CENTRAL ON)\nset(CONFIG_CORNIX_STENO ON)\nset(CONFIG_ZMK_POINTING ON)\nset(CONFIG_CCK_STENO_POINTER_ASSIST ON)\n'
  code+='function(target_sources_ifdef feature target)\n if(${feature})\n target_sources(${target} ${ARGN})\n endif()\nendfunction()\n'
  code+=f'add_subdirectory("{R}" cck)\nget_target_property(s app SOURCES)\nfile(WRITE "${{CMAKE_BINARY_DIR}}/result.txt" "${{s}}")\n'
  (d/'CMakeLists.txt').write_text(code)
  p=subprocess.run(['cmake','-S',str(d),'-B',str(d/'b')],capture_output=True,text=True)
  assert (p.returncode==0)==(ip_ok==2),(ip_ok,p.stdout,p.stderr)
  if ip_ok==2:
   txt=(d/'b/result.txt').read_text();assert 'cck_steno_mouse.c' in txt and 'input_processor_cck_scroll.c' in txt
  print('v1.1.1 actual CMake/source selection, mock input processor header',ip_ok,': PASS')
