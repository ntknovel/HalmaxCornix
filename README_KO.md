# Cornix ZMK BASIC 260817 FIXED v1.2

`cornix 260817.vil`을 기준으로 만든 **속기 엔진 없는 일반 Cornix ZMK 펌웨어 소스**입니다.

## v1.2 핵심 추가: KC_ACL0 / KC_ACL2

원본 Mouse 레이어의 `KC_ACL0`, `KC_ACL2`를 ZMK에서 다시 사용할 수 있게 복원했습니다.

```text
Mouse 레이어 왼쪽 홈행
A 자리: ACL0 정밀 속도
S 자리: ACL2 최대 속도
```

두 키는 QMK/Vial처럼 **누르고 있는 동안만** 속도를 바꿉니다.

```text
ACL0: 커서 150, 스크롤 3   — ZMK 기본 최대값의 약 1/4
ACL2: 커서 600, 스크롤 10 — 즉시 최대 속도
```

일반 Mouse 레이어의 `&mmv`는 기존 ZMK 300ms 가속 곡선을 그대로 유지합니다. ACL0/ACL2를 누르면 내부 helper layer가 이동·스크롤 위치만 constant-speed behavior로 바꿉니다. 두 키를 동시에 누르면 ACL0 정밀 속도가 우선합니다.

Keymap Editor에는 기존 10개 사용자 레이어 외에 다음 두 내부 레이어가 추가로 표시됩니다.

```text
Mouse ACL2 Fast
Mouse ACL0 Slow
```

이 두 레이어는 직접 편집할 필요가 없습니다.

## v1.1에서 이미 수정된 항목

- `LT4(Esc)`와 `LT7(Delete)` positional balanced layer-tap
- Plain/Numpad 콤보 레이어 마스크 복구
- Editing/Symbols 레이어 진입키
- `BT_CLR`, `BT_CLR_ALL`, `OUT_BLE`, `OUT_USB`
- 런타임 송신출력 `TX 0 / TX + / TX - / TX MAX`
- 안전한 settings-reset 분리

## Codespaces 적용

```bash
cd /workspaces/zmkCorne
git switch fix-zmk-compat
git pull --rebase origin fix-zmk-compat
git switch -c cornix-basic-260817-fixed-v1-2

unzip -o Cornix_ZMK_BASIC_260817_FIXED_v1.2_PATCH_ON_FIX_COMPAT.zip
rm -f Cornix_ZMK_BASIC_260817_FIXED_v1.2_PATCH_ON_FIX_COMPAT.zip

python3 tools/validate_basic_260817.py .
git add -A
git commit -m "Add KC_ACL0 and KC_ACL2 mouse speed modes"
git push -u origin cornix-basic-260817-fixed-v1-2
```

GitHub Actions의 left/right/settings-reset 세 작업이 모두 성공하기 전에는 플래시하지 마십시오.

## 플래시

기존 v1.1 ZMK가 정상이라면 새 left/right UF2만 갱신하면 됩니다. RMK에서 처음 넘어가거나 페어링이 꼬였으면 settings-reset을 양쪽에 먼저 적용하십시오.
