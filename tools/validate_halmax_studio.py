#!/usr/bin/env python3
"""Validate source only. This is not an ARM firmware build or hardware test."""
from pathlib import Path
import ast, hashlib, json, re, shlex, shutil, subprocess, sys, tempfile
import yaml
ROOT=Path(sys.argv[1] if len(sys.argv)>1 else '.').resolve()
def check(ok,msg):
    if not ok: raise RuntimeError(msg)
    print('PASS:',msg,flush=True)
def run(args,**kwargs):
    subprocess.run(args,check=True,**kwargs)
for p in [ROOT/'tools/validate_halmax_studio.py',ROOT/'tools/check_built_studio.py']:
    ast.parse(p.read_text(encoding='utf-8'))
check(True,'Python syntax')
run([sys.executable,str(ROOT/'tools/validate_basic_260817.py'),str(ROOT)])
refs=json.loads((ROOT/'HALMAX_PRESERVED_SOURCE_SHA256.json').read_text())
for rel,h in refs.items():
    check(hashlib.sha256((ROOT/rel).read_bytes()).hexdigest()==h,'preserved '+rel)
rows=yaml.safe_load((ROOT/'build.yaml').read_text())['include']
check(len(rows)==3,'exactly three build targets')
left,right,reset=rows
check(left['board']=='cornix_left//zmk' and left['snippet']=='studio-rpc-usb-uart','left USB snippet')
flags=dict(v.removeprefix('-D').split('=',1) for v in shlex.split(left['cmake-args']))
check(flags=={'CONFIG_ZMK_STUDIO':'y','CONFIG_ZMK_STUDIO_LOCKING':'n',
              'CONFIG_ZMK_STUDIO_TRANSPORT_BLE':'n','CONFIG_HALMAX_STUDIO_USB_AUTO_SELECT':'y'},'central unlocked USB-only Studio flags')
check(right['board']=='cornix_right//zmk' and 'snippet' not in right and
      right['cmake-args']=='-DCONFIG_ZMK_STUDIO=n','right Studio server disabled')
check(reset['board']=='cornix_right//zmk' and reset['shield']=='settings_reset' and
      reset['snippet']=='studio-rpc-usb-uart nrf52840-nosd' and
      reset['cmake-args']=='-DCONFIG_ZMK_STUDIO=n','reset isolated from normal Studio server')
manifest=yaml.safe_load((ROOT/'config/west.yml').read_text())['manifest']
pins={x['name']:x['revision'] for x in manifest['projects']}
check(pins['zmk']=='6e2ef41e022d555b10f116e395832913f71717b3' and
      pins['zmk-keyboard-cornix']=='ee80f2475b4310e761f5f271371e41172db8b5ca','ZMK and hardware commit pins unchanged')
cm=(ROOT/'CMakeLists.txt').read_text()
check('HALMAX_USB_STUDIO_V1_3_1' in cm and 'target_sources(app PRIVATE src/halmax_studio_usb_auto.c)' in cm,'real application source connected')
with tempfile.TemporaryDirectory(prefix='halmax-studio-test-') as temp:
    tmp=Path(temp)
    for cc in ['gcc','clang']:
        if not shutil.which(cc):
            print('SKIP: compiler unavailable:',cc);continue
        for old in [False,True]:
            api='zmk_endpoints_select_transport' if old else 'zmk_endpoint_set_preferred_transport'
            binary=tmp/(cc+('-old' if old else '-new'))
            cmd=[cc,'-std=c11','-Wall','-Wextra','-Werror','-fno-strict-aliasing',
                 '-I'+str(ROOT/'validation/studio_stubs'),'-DHALMAX_SELECT_USB_TRANSPORT='+api]
            if old: cmd+=['-DHALMAX_TEST_OLD_API=1']
            if cc=='clang': cmd+=['-fsanitize=address,undefined','-fno-omit-frame-pointer']
            run(cmd+[str(ROOT/'validation/test_studio_usb.c'),'-o',str(binary)])
            run([str(binary)])
            check(True,cc+' real USB module host execution: '+api)
    if shutil.which('cmake'):
        for case in ['new','old','right','reset','locking','ble','no_uart','bad_api']:
            d=tmp/case;d.mkdir()
            include=d/'zmkapp/include/zmk';include.mkdir(parents=True)
            api='zmk_endpoints_select_transport' if case=='old' else 'zmk_endpoint_set_preferred_transport'
            (include/'endpoints.h').write_text('' if case=='bad_api' else 'int '+api+'(enum zmk_transport transport);\n')
            body='cmake_minimum_required(VERSION 3.20)\nproject(halmax_config_fixture C)\n'
            body+='add_library(app STATIC dummy.c)\n'
            body+='function(target_sources_ifdef flag)\n if(${flag})\n target_sources(${ARGN})\n endif()\nendfunction()\n'
            body+='set(APPLICATION_SOURCE_DIR "'+str(d/'zmkapp')+'")\n'
            enabled=case not in ['right','reset']
            for flag in ['HALMAX_STUDIO_USB_AUTO_SELECT','ZMK_STUDIO','ZMK_STUDIO_RPC','ZMK_USB','ZMK_SPLIT_ROLE_CENTRAL','ZMK_STUDIO_TRANSPORT_UART']:
                body+='set(CONFIG_'+flag+' '+('TRUE' if enabled else 'FALSE')+')\n'
            if case=='locking': body+='set(CONFIG_ZMK_STUDIO_LOCKING TRUE)\n'
            if case=='ble': body+='set(CONFIG_ZMK_STUDIO_TRANSPORT_BLE TRUE)\n'
            if case=='no_uart': body+='set(CONFIG_ZMK_STUDIO_TRANSPORT_UART FALSE)\n'
            body+='add_subdirectory("'+str(ROOT)+'" firmware)\n'
            body+='get_target_property(s app SOURCES)\nfile(WRITE "${CMAKE_BINARY_DIR}/sources.txt" "${s}")\n'
            body+='get_target_property(d app COMPILE_DEFINITIONS)\nfile(WRITE "${CMAKE_BINARY_DIR}/definitions.txt" "${d}")\n'
            (d/'CMakeLists.txt').write_text(body);(d/'dummy.c').write_text('int dummy;\n')
            proc=subprocess.run(['cmake','-S',str(d),'-B',str(d/'build')],capture_output=True,text=True)
            expected_success=case in ['new','old','right','reset']
            check((proc.returncode==0)==expected_success,'CMake guard '+case)
            if expected_success:
                sources=(d/'build/sources.txt').read_text()
                check(('halmax_studio_usb_auto.c' in sources)==enabled,'source is central-only: '+case)
                if enabled: check(api in (d/'build/definitions.txt').read_text(),'declared API detected: '+case)
            elif 'Halmax:' not in proc.stdout+proc.stderr:
                raise RuntimeError('Unexpected CMake error: '+proc.stderr)
    else: print('SKIP: CMake fixture (cmake unavailable)')
print('SOURCE/HOST VALIDATION PASSED — ARM compilation and real USB Studio remain untested',flush=True)
