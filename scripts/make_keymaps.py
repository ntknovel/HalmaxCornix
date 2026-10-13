from pathlib import Path
import json,re,html
R=Path(__file__).resolve().parents[1]
data=json.loads((R/'config/cck_layout_reference.json').read_text())
layout=json.loads((R/'config/cck_ball.json').read_text())['layouts']['default_transform']['layout']
E=html.escape
role={'b':'ㅂ','j':'ㅈ','d':'ㄷ','g':'ㄱ','s':'ㅅ','m':'ㅁ','n':'ㄴ','ng':'ㅇ','r':'ㄹ','h':'ㅎ','kh':'ㅋ','t':'ㅌ','ch':'ㅊ','p':'ㅍ','double':'쌍'}
basic={'ESC':'Esc','TAB':'Tab','BACKSPACE':'Backspace','BSPC':'Backspace','BSLH':'\\','FSLH':'/','CAPSLOCK':'Caps','SEMI':';','SQT':"'",'COMMA':',','DOT':'.','LEFT_SHIFT':'LShift','LEFT_CONTROL':'LCtrl','LEFT_WIN':'LWin','LEFT_ALT':'LAlt','DELETE':'Delete','SPACE':'Space','ENTER':'Enter','LEFT_ARROW':'←','RIGHT_ARROW':'→','UP_ARROW':'↑','DOWN_ARROW':'↓','LEFT':'←','RIGHT':'→','UP':'↑','DOWN':'↓','LCTRL':'LCtrl','LALT':'LAlt','RALT':'RAlt','GRAVE':'`','EXCL':'!','AT':'@','HASH':'#','DLLR':'$','PRCNT':'%','CARET':'^','AMPS':'&','ASTERISK':'*','LPAR':'(','RPAR':')','TILDE':'~'}
special={'&cck_tab':('~ / Tab','짧게 ~ · 200ms Tab'), '&st_mode':('속기 전환','BASE ↔ STENO'), '&mxesc':('!','기호: ` / 기능: Esc'), '&mxbsp':('?','기호: BS / 기능: F11'), '&mxtab':('Tab','기능: F12'), '&st_abbr_l':('약어 L','단독 !'), '&st_abbr_r':('약어 R','단독 "'), '&st_vext_l':('겹모 L','단독 ['), '&st_vext_r':('겹모 R','단독 ?'), '&st_symbol_l':('기호 L','단독 ]'), '&st_symbol_r':('기호 R','단독 Backspace'), '&st_v_o':('ㅗ','모음'), '&st_v_eu':('ㅡ','모음'), '&st_v_i':('ㅣ','+ JLKI 방향키'), '&st_v_a':('ㅏ','+ JLKI 방향키'), '&st_vu_enter':('ㅜ','단독 Enter / 500ms'), '&st_veo_space':('ㅓ','Space / +마우스·스크롤'), '&st_sel 0 RALT':('RAlt / 선택','탭 / SELECT SHIFT'), '&bootloader':('Bootloader','오른쪽 central'), '&sys_reset':('Restart','오른쪽 central')}
def label(b):
 if b in special:return special[b]
 if b=='&trans':return ('▽','아래 레이어')
 if b=='&none':return ('×','동작 없음')
 m=re.fullmatch(r'&st_([if])_(\w+)',b)
 if m:return ('초'+role[m[2]] if m[1]=='i' else '종'+role[m[2]],'초성' if m[1]=='i' else '종성')
 if b.startswith('&mx'):
  v=b[3:];return (v,'숫자 / 기호 / F키')
 if b.startswith('&kp ') or b.startswith('&st_arrow '):
  k=b.split(' ',1)[1];return (basic.get(k,k[1:] if re.fullmatch('N[0-9]',k) else k),'')
 if b.startswith('&mo '):return ('Hold '+{'1':'Fn','2':'BT','SNIPE':'Snipe','SYMBOL':'기호'}.get(b[4:],b[4:]),'누르는 동안')
 if b=='&lt 3 F':return ('F / Scroll','탭 F · 홀드 스크롤')
 if b.startswith('&mkp '):return ({'LCLK':'좌클릭','RCLK':'우클릭'}.get(b[5:],b[5:]),'마우스')
 if b.startswith('&out '):return ('USB 출력' if 'USB' in b else 'BLE 출력','')
 if b.startswith('&bt '):
  x=b[4:];return (x.replace('BT_SEL ','BT 프로필 ').replace('BT_CLR_ALL','BT 전체 삭제').replace('BT_CLR','BT 삭제').replace('BT_NXT','BT 다음'),'')
 return (b,'')
svg=[]
for li,layer in enumerate(data['layers']):
 ss=f'<section class="layer" id="L{li}"'+(' hidden' if li else '')+'>'
 ss+=f'<h2>{li} · {E(data["layer_names"][li])}</h2>'
 ss+='<svg viewBox="-10 -48 1820 818" role="img" aria-label="CCK 4x6 keymap"><text class="side" x="10" y="-16">LEFT · peripheral</text><text class="side" x="1200" y="-16">RIGHT · central / USB / trackball</text>'
 for p,(key,b) in enumerate(zip(layout,layer)):
  x,y=key['x']*100,key['y']*100;w=key.get('w',1)*100;h=key.get('h',1)*100
  a,c=label(b)
  if li==5 and p in {20,31,32,33,44}: c={20:'ㅣ·ㅏ ↑\nㅓ 휠↑',31:'ㅣ·ㅏ ←\nㅓ 좌클릭',32:'ㅣ·ㅏ ↓\nㅓ 휠↓',33:'ㅣ·ㅏ →\nㅓ 우클릭',44:'ㅓ 중앙클릭'}[p]
  if p>=60:c=['왼쪽 시계','왼쪽 반시계','오른쪽 시계','오른쪽 반시계'][p-60]
  cl=('wheel' if p>=60 else 'mode' if p==23 else 'role' if b.startswith('&st_') else 'normal')
  attrs=f'data-layer="{li}" data-pos="{p}" data-binding="{E(b,quote=True)}" tabindex="0"'
  ss+=f'<g class="key {cl}" transform="translate({x:g} {y:g})" {attrs}><title>{p:02d} · {E(b)}</title><rect x="3" y="3" width="{w-8:g}" height="{h-8:g}" rx="9"/><text class="number" x="11" y="20">{p:02d}'+(' · 휠' if p>=60 else '')+f'</text><text class="main" x="{w/2-1:g}" y="{h*.51:g}" text-anchor="middle"'+(' style="font-size:13px"' if len(a)>11 else ' style="font-size:16px"' if len(a)>7 else '')+f'>{E(a)}</text>'
  for ci,part in enumerate(c.split('\n')):
   cy=h*(.70+.15*ci) if '\n' in c else h*.78
   ss+=f'<text class="sub" x="{w/2-1:g}" y="{cy:g}" text-anchor="middle">{E(part)}</text>'
  ss+='</g>'
 ss+='<text class="legend" x="5" y="754">60~63: 회전 출력 배정칸 · 물리 클릭 스위치 없음 / 같은 칸을 레이어별로 편집</text></svg></section>'
 if li==5:
  ss=ss.replace('</section>', '<p class="small">12: 단독 200ms 미만 ~ / 홀드 Tab · 00: ! · 11: ? · ㅣ(55) 또는 ㅏ(56)를 먼저 누른 뒤 J/L/K/I는 ←/→/↓/↑ · ㅓ(54)를 먼저 누른 뒤 J/L은 좌/우클릭, I/K는 휠↑/↓, 쉼표(44)는 중앙클릭 · ㅓ+트랙볼은 스크롤. 조작에 사용한 모음은 해제 시 문자를 추가 출력하지 않습니다.</p></section>')
 svg.append(ss)
page='''<!doctype html><html lang="ko"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>CCK BALL 4×6 · STENO v1.1.1 키맵</title><style>
*{box-sizing:border-box}body{margin:0;background:#f3f5f8;color:#142434;font:15px/1.5 system-ui,"Noto Sans CJK KR","Malgun Gothic",sans-serif}header,main{max-width:1500px;margin:auto;padding:18px 24px}header{padding-bottom:0}h1{font-size:25px;margin:3px 0}header p{margin:4px 0;color:#526171}.badge{font-size:12px;letter-spacing:.08em;color:#32637c}nav{display:flex;gap:7px;margin:15px 0;flex-wrap:wrap}button{border:1px solid #c7d2dd;padding:9px 17px;border-radius:8px;background:#fff;color:#16304b;cursor:pointer;font:inherit}button.active{background:#163d56;color:white}main{padding-top:0}.layer{background:white;border:1px solid #d8e0e7;border-radius:14px;padding:14px 18px}h2{font-size:21px;margin:0 0 8px}svg{width:100%;height:auto;display:block}.key{cursor:pointer}.key rect{fill:#fff;stroke:#b6c7d4;stroke-width:1.6}.key:focus rect,.key:hover rect{stroke:#126a97;stroke-width:3}.role rect{fill:#f0f8fd}.mode rect{fill:#e3f3ff;stroke:#267cb0;stroke-width:2.5}.wheel rect{fill:#f0f1ff;stroke:#6f6ab0;stroke-dasharray:5 3}.number{font-size:11px;fill:#526b7b}.main{font-size:21px;fill:#142f42;font-weight:650}.sub{font-size:10px;fill:#536879}.side{font-size:21px;fill:#184868;font-weight:600}.legend{font-size:16px;fill:#526171}footer{padding:12px 3px;color:#526171;font-size:13px}dialog{max-width:650px;border:1px solid #a5b7c6;border-radius:12px;padding:22px}dialog::backdrop{background:#17344a66}pre{white-space:pre-wrap;overflow-wrap:anywhere;background:#f1f4f8;padding:14px}table{border-collapse:collapse;width:100%}td,th{padding:8px;border-bottom:1px solid #e2e7ec;text-align:left}.small{font-size:13px} @media(max-width:800px){header,main{padding:12px}h1{font-size:20px}.layer{min-width:900px}main{overflow-x:auto}}@media print{.layer[hidden]{display:block}section{break-before:page}nav{display:none}}
</style></head><body><header><div class="badge">SOURCE PREVIEW · CCK BALL 4×6 · v1.1.1</div><h1>양쪽 휠 · 속기 모음 보조키 · USB/BLE Studio</h1><p>오른쪽 USB가 Studio 접속부입니다. 23번: Backspace 바로 아래 속기 전환. 실제 펌웨어 소스 기반 보기이며 실물 연결 화면이 아닙니다.</p><nav>'''
page+=''.join(f'<button data-layer="{i}" class="'+('active' if not i else '')+f'">{i} · {E(n)}</button>' for i,n in enumerate(data['layer_names']))
page+='</nav></header><main>'+''.join(svg)+'''<footer>키를 누르면 위치 번호와 실제 바인딩을 확인합니다. 이 HTML은 보기 전용이며 키보드에 입력·저장을 하지 않습니다.<br>STENO 5 / SELECT 6 및 센서 Scroll 3 / Snipe 4는 임의로 이동·삭제하지 마세요. 속기 역할·사전의 동적 앱 저장은 이 펌웨어의 기능이 아닙니다.</footer></main><dialog id="details"><h2 id="dk"></h2><pre id="binding"></pre><p class="small">현재 소스 기본값입니다. Studio에 별도로 저장된 값은 다를 수 있습니다.</p><button id="close">닫기</button></dialog><script>
const q=s=>document.querySelector(s);document.querySelectorAll('nav button').forEach(b=>b.onclick=()=>{document.querySelectorAll('.layer').forEach(l=>l.hidden=l.id!=='L'+b.dataset.layer);document.querySelectorAll('nav button').forEach(x=>x.classList.toggle('active',x===b));});
function show(g){q('#dk').textContent='레이어 '+g.dataset.layer+' · 위치 '+g.dataset.pos;q('#binding').textContent=g.dataset.binding;q('#details').showModal();}
document.querySelectorAll('g.key').forEach(g=>{g.onclick=()=>show(g);g.onkeydown=e=>{if(e.key==='Enter')show(g);};});q('#close').onclick=()=>q('#details').close();
</script></body></html>'''
(R/'docs/KEYMAPS.html').write_text(page)
# Text table gives searchable exact source, not screenshots only.
out=['# CCK BALL 4x6 v1.1.1 — 모든 배정','', '60~63은 인코더 회전의 가상 배정칸입니다.']
for i,layer in enumerate(data['layers']):
 out.extend(['',f'## {i} · {data["layer_names"][i]}','', '| 위치 | 표시 | 실제 바인딩 |','|---:|---|---|'])
 for p,b in enumerate(layer):out.append(f'| {p:02d} | '+ ' / '.join(x for x in label(b) if x).replace('|','/')+f' | `{b}` |')
(R/'docs/KEYMAPS_KO.md').write_text('\n'.join(out)+'\n')
print(R/'docs/KEYMAPS.html')
