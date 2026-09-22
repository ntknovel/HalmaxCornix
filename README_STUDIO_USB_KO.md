# HalmaxCornix USB Studio v1.3.1

대상: ntknovel/HalmaxCornix / cornix-basic-260817-fixed-v1-2.
이것은 **비속기 CORNIX용 소스 패키지**다. KLOR용 또는 속기 CORNIX용이 아니다.
작성일: 2026-09-22. 이 ZIP에는 컴파일된 UF2가 없다.

## 동작

왼쪽 USB 데이터 연결 → USB 출력 자동 선택 → ZMK Studio에서 장치 연결 → 바로 키맵 편집.
별도 unlock 키, 잠금 해제 콤보, 주기적인 unlock 매크로는 없다.
펌웨어 잠금은 처음부터 비활성화한다. 재부팅/재연결 후에도 unlock 키는 필요하지 않다.
브라우저/앱에서 장치 선택과 USB 연결 권한 승인은 여전히 필요하다. USB를 꽂는 것만으로
웹페이지가 자동 실행되는 기능은 아니다.

왼쪽 빌드 옵션:
```
CONFIG_ZMK_STUDIO=y
CONFIG_ZMK_STUDIO_LOCKING=n
CONFIG_ZMK_STUDIO_TRANSPORT_BLE=n
CONFIG_HALMAX_STUDIO_USB_AUTO_SELECT=y
```
왼쪽 build.yaml snippet: `studio-rpc-usb-uart`.
상시 잠금 해제된 설정 편집 통로는 USB로 한정했다. Bluetooth 일반 타이핑과 좌우 BLE
연결, RF 설정은 유지한다. 충전만 되는 연결에는 USB 출력을 강제로 선택하지 않는다.

기존에 BLE 출력을 선택해 저장했더라도 USB 데이터 연결이 준비되면 USB를 자동 선택한다.
따라서 일반 연결 시 OUT_USB 키도 따로 누를 필요가 없다. 연결 중 사용자가 OUT_BLE을
명시적으로 누르는 기능은 막지 않는다. 이 경우 같은 연결에서 USB Studio를 쓰려면
OUT_USB를 선택하거나 USB를 다시 연결한다. USB를 빼면 ZMK 기존 BLE fallback이 적용된다.

## 변경하지 않은 부분

기본 파일의 바이트를 보존했다: `config/cornix.keymap`, `config/cornix.json`,
`config/cornix.conf`, `config/west.yml`, `source/cornix 260817.vil`, RF behavior,
하드웨어 compatibility shim, settings-reset.conf.

10개 사용자 레이어 + 2개 마우스 속도 보조 레이어, 50위치, 31개 콤보,
탭/홀드와 홈로우 모드의 시간값, 양쪽 인코더, ACL0/ACL2, BT 프로필 및 RF 출력은 그대로다.
이 요청에서는 추가 예약 레이어를 새로 넣거나 키를 unlock 용도로 바꾸지 않았다.
기존 v1.3 Studio에 이미 있던 예약 레이어를 패치로 갱신하는 경우에는 그 키맵도 보존한다.

## 기존 저장소에 적용 (권장)

`HALMAX_CORNIX_STUDIO_V1_3_1_PATCH.zip`을 해당 Codespace 저장소 최상단에 업로드한다.
그 저장소의 터미널에서 실행한다:
```bash
cd "$(git rev-parse --show-toplevel)"
git switch -c "halmax-usb-studio-$(date +%Y%m%d-%H%M%S)"
unzip -o HALMAX_CORNIX_STUDIO_V1_3_1_PATCH.zip
python3 halmax_studio_patch/apply.py . --stage
git commit -m "Enable unlocked USB Studio on HalmaxCornix"
git push -u origin "$(git branch --show-current)"
```

적용기는 실제 대상을 확인하고, 기존 수정 파일을 /tmp 아래에 백업하며, 키맵을 건드리지
않는다. 적용 후 소스 검증에 실패하면 이번에 바꾼 파일을 원래대로 되돌린다.
`--stage`는 이번 변경 파일만 git add한다. ZIP과 임시 패치 폴더는 커밋하지 않는다.
커밋되지 않은 기존 수정이나 이해하지 못하는 빌드 구성이 있으면 먼저 중단한다.

PyYAML이 없다는 오류가 나면 현재 Python 환경에 `python3 -m pip install PyYAML`로
설치한 뒤 apply 명령을 다시 실행한다. C 컴파일러/CMake가 없는 경우 해당 호스트 시험은
SKIP으로 표시하며, 실제 ARM 빌드 성공으로 간주하지 않는다.

전체 소스 ZIP은 신규 작업 폴더용이다. 이미 수정해 사용하는 저장소에는 위 패치가 우선이다.

## 빌드와 기록

워크플로: `Build HalmaxCornix USB Studio v1.3.1`.
기존의 pinned ZMK reusable workflow를 그대로 사용한다.
빌드 대상은 left/right/settings_reset 세 개이며 전부 성공한 실행의 산출물을 사용한다.

정상 사용:
- `halmax_cornix_left_studio_v1_3_1.uf2` → 왼쪽
- `halmax_cornix_right_v1_3_1.uf2` → 오른쪽

`halmax_cornix_settings_reset.uf2`는 복구/초기화 전용이며 평상시에 쓰는 펌웨어가 아니다.
reset 타깃의 USB snippets는 Cornix 모듈의 reset 구성에 맞춰 포함했고,
`CONFIG_ZMK_STUDIO=n`으로 설정 편집 서버 자체는 끈다.
Studio 추가만을 이유로 settings_reset을 먼저 기록하지 않는다. 기존 저장된 키맵이나
페어링을 유지하려면 우선 정상 left/right 이미지로 갱신한다.

## Studio 사용

1. 정상 left/right UF2를 각각 기록한다.
2. 데이터 지원 USB 케이블을 왼쪽에 연결한다.
3. Chrome/Edge에서 https://zmk.studio/ 를 열거나 ZMK Studio 앱을 실행한다.
4. USB 장치를 선택하고 연결한다. 키보드의 별도 unlock 키는 누르지 않는다.
5. 키 배정을 변경하고 Studio의 저장 동작으로 확정한다.

잠금이 없으므로 연결한 PC는 키보드의 물리 승인 없이 설정을 바꿀 수 있다.
신뢰하는 PC에 연결해 사용한다.

Studio는 키 배정·레이어 이름 등을 편집할 수 있다. 기존 콤보, 인코더 sensor-bindings,
탭/홀드 시간값, RF 코드는 이번에 그대로 보존되며 Studio GUI만으로 모두 편집할 수
있다는 의미는 아니다. Studio 저장값은 이후 펌웨어의 기본 .keymap보다 우선할 수 있다.
기본값 복원이 필요할 때만 Restore Stock Settings를 사용한다.

## 검증 범위

이 환경에서 실행: 기존 키맵 회귀 검사, 보존 파일 SHA-256 대조, USB 자동 선택 C 소스를
GCC/Clang으로 컴파일한 호스트 시험, Clang ASan/UBSan, 실제 CMake 가드 실행,
업데이트 적용/재적용, 잘못된 대상 거부, ZIP CRC 및 해시 검사.

**이 환경에서 실행하지 못함: ARM/ZMK 전체 빌드, 실제 nRF52840 기록,
Windows USB 열거, 실제 ZMK Studio 저장/재연결 시험.** 외부 Git 다운로드는 DNS 오류로
실패했고 ARM/Zephyr 툴체인은 설치되어 있지 않다. 호스트 시험 결과를 완성 UF2 성공으로
대신하지 않는다. 새 UF2는 위 GitHub Actions가 실제로 성공한 뒤 생성된다.

실제 build/zephyr/.config를 받은 뒤에는 아래 검사도 사용할 수 있다:
```
python3 tools/check_built_studio.py <빌드폴더>/zephyr/.config left
```
이 검사는 설정 파일이 없으면 실패한다. CMake에도 상시 잠금 해제/USB 전용/central 설정
검사가 있어 필수 옵션이 잘못된 구성은 통과시키지 않는다.

## 기준 공개 소스

- https://github.com/ntknovel/HalmaxCornix/tree/cornix-basic-260817-fixed-v1-2
- https://zmk.dev/docs/features/studio
- https://zmk.dev/docs/config/studio
- https://raw.githubusercontent.com/zmkfirmware/zmk/6e2ef41e022d555b10f116e395832913f71717b3/app/src/studio/Kconfig
- https://raw.githubusercontent.com/zmkfirmware/zmk/6e2ef41e022d555b10f116e395832913f71717b3/app/src/studio/rpc.c
- https://raw.githubusercontent.com/hitsmaxft/zmk-keyboard-cornix/ee80f2475b4310e761f5f271371e41172db8b5ca/boards/jzf/cornix/cornix-layouts.dtsi
- https://raw.githubusercontent.com/hitsmaxft/zmk-keyboard-cornix/main/build.yaml

기준 전체 소스 ZIP SHA-256:
`b463cfbc8c8b973f59e98f0eb19d12e1505d26ad48529dbdf001e11a9d745799`
기존 키맵 SHA-256:
`8a4957b311b07c330abbc9bab4fc03bbea8c7cdbe2fe7c1ab6f92721ed8d9f12`
