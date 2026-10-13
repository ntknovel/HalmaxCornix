from pathlib import Path
from playwright.sync_api import sync_playwright
import json
r=Path(__file__).resolve().parents[1]
with sync_playwright() as p:
 b=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox'])
 page=b.new_page(viewport={'width':1600,'height':1100},device_scale_factor=1)
 errors=[];page.on('pageerror',lambda err:errors.append(str(err)))
 # Browser policy blocks file navigation; render the known source text without changing policy.
 page.set_content((r/'docs/KEYMAPS.html').read_text(),wait_until='load')
 assert page.locator('g.key').count()==512
 assert page.locator('nav button').count()==8
 for i in range(8):
  page.locator(f'nav button[data-layer="{i}"]').click()
  assert page.locator(f'#L{i}').is_visible()
  assert page.locator(f'#L{i} g.key').count()==64
  for pos in [0,11,12,23,54,55,56,58,60,63]:
   key=page.locator(f'#L{i} g[data-pos="{pos}"]')
   key.click()
   assert page.locator('#details').is_visible()
   assert page.locator('#binding').inner_text()==key.get_attribute('data-binding')
   page.locator('#close').click()
  if i in [0,5]:
   page.evaluate('document.activeElement && document.activeElement.blur()')
   page.mouse.move(1580,10)
   bounds=page.locator('main').bounding_box();height=int(bounds['y']+bounds['height']+8)
   page.screenshot(path=str(r/'docs'/('BASE_SOURCE_PREVIEW.png' if i==0 else 'STENO_SOURCE_PREVIEW.png')),clip={'x':0,'y':0,'width':1600,'height':height})
 assert not errors,errors
 b.close()
(r/'verification/HTML_TEST.txt').write_text('PASS: actual source HTML rendered in Chromium set_content (file navigation blocked; no policy changes); 8 layers, 512 displayed bindings, 80 key dialogs checked.\nNo device connected. Read-only source diagram, not actual Studio screenshot.\n',encoding='utf-8')
print('HTML all 8 layers + 80 binding popups: PASS')
