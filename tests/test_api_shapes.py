"""Compile actual custom C files with metadata ON and both layer API shapes.
This uses declared host stubs, not Zephyr or an ARM toolchain.
"""
from pathlib import Path
import tempfile,subprocess,shutil
R=Path(__file__).resolve().parents[1];M=R/'modules/cck-steno'
with tempfile.TemporaryDirectory(prefix='cck-api-') as td:
 s=Path(td);shutil.copytree(M/'validation/api_stubs',s,dirs_exist_ok=True)
 p=s/'drivers/behavior.h';t=p.read_text().replace('struct behavior_parameter_metadata { int dummy; };','''
struct behavior_parameter_value_metadata {
 char *display_name;
 union {uint32_t value; struct {int32_t min,max;} range;};
 enum {BEHAVIOR_PARAMETER_VALUE_TYPE_NIL, BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
       BEHAVIOR_PARAMETER_VALUE_TYPE_RANGE, BEHAVIOR_PARAMETER_VALUE_TYPE_HID_USAGE,
       BEHAVIOR_PARAMETER_VALUE_TYPE_LAYER_ID}type;
};
struct behavior_parameter_metadata_set {
 size_t param1_values_len; const struct behavior_parameter_value_metadata *param1_values;
 size_t param2_values_len; const struct behavior_parameter_value_metadata *param2_values;
};
struct behavior_parameter_metadata {size_t sets_len;const struct behavior_parameter_metadata_set *sets;};
''');p.write_text(t)
 c=(s/'config.h').read_text().replace('#define CONFIG_ZMK_BEHAVIOR_METADATA 0','#define CONFIG_ZMK_BEHAVIOR_METADATA 1').replace('#define CONFIG_CORNIX_STENO_LED 1','#define CONFIG_CORNIX_STENO_LED 0').replace('#define CONFIG_CORNIX_STENO_MAX_POSITIONS 50','#define CONFIG_CORNIX_STENO_MAX_POSITIONS 64')
 (s/'config.h').write_text(c)
 sources=[p for p in (M/'src/behaviors').glob('*.c') if 'led' not in p.name and 'rf_power' not in p.name]
 sources+=[p for p in (M/'src/steno').glob('*.c') if p.name!='steno_led.c']
 for arity in [1,2]:
  h='#pragma once\n#include <stdbool.h>\n#include <stdint.h>\nstatic inline bool zmk_keymap_layer_active(uint8_t l){(void)l;return false;}\n'
  for name in ['activate','deactivate']:
   h+=f'static inline int zmk_keymap_layer_{name}(uint8_t l'+(',bool locking' if arity==2 else '')+') {(void)l;'+('(void)locking;' if arity==2 else '')+'return 0;}\n'
  (s/'zmk/keymap.h').write_text(h)
  for cc in [x for x in ['gcc','clang'] if shutil.which(x)]:
   for source in sources:
    args=[cc,'-std=c11','-Wall','-Wextra','-Werror','-Wno-unused-function','-Wno-unused-variable',f'-DCCK_LAYER_API_ARITY={arity}','-include',str(s/'config.h'),'-I'+str(s),'-I'+str(M/'include'),'-fsyntax-only',str(source)]
    p=subprocess.run(args,capture_output=True,text=True)
    assert p.returncode==0,(source,cc,arity,p.stderr)
   print(f'{cc} custom C API-shaped compile: {len(sources)} files, metadata ON, layer arity {arity}: PASS (not ARM)')
