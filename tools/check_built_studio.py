#!/usr/bin/env python3
"""Read actual compiled .config, e.g. python tools/check_built_studio.py build/zephyr/.config left."""
from pathlib import Path
import re,sys
if len(sys.argv)!=3 or sys.argv[2] not in ('left','right','reset'):
    raise SystemExit('Usage: check_built_studio.py <actual .config> <left|right|reset>')
p=Path(sys.argv[1]); mode=sys.argv[2]
if not p.is_file(): raise SystemExit('FAIL: compiled .config is missing; no build is being claimed')
v={}
for line in p.read_text().splitlines():
    if line.startswith('CONFIG_') and '=' in line:
        k,val=line.split('=',1);v[k]=val
    m=re.fullmatch(r'# (CONFIG_\w+) is not set',line)
    if m:v[m[1]]='n'
expected=({'CONFIG_ZMK_STUDIO':'y','CONFIG_ZMK_STUDIO_RPC':'y',
           'CONFIG_ZMK_STUDIO_LOCKING':'n','CONFIG_ZMK_STUDIO_TRANSPORT_BLE':'n',
           'CONFIG_ZMK_STUDIO_TRANSPORT_UART':'y','CONFIG_ZMK_USB':'y',
           'CONFIG_USB_CDC_ACM':'y','CONFIG_ZMK_SPLIT_ROLE_CENTRAL':'y',
           'CONFIG_HALMAX_STUDIO_USB_AUTO_SELECT':'y'}
          if mode=='left' else {'CONFIG_ZMK_STUDIO':'n','CONFIG_HALMAX_STUDIO_USB_AUTO_SELECT':'n'})
for key,want in expected.items():
    actual=v.get(key,'n')
    if actual!=want:raise SystemExit(f'FAIL: {key}={actual}; expected {want}')
    print(f'PASS: {key}={actual}')
if v.get('CONFIG_ZMK_STUDIO_LOCK_ON_DISCONNECT','n')=='y':
    raise SystemExit('FAIL: disconnect locking is active')
print('Compiled Studio configuration checked:',mode)
