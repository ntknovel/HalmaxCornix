# Cornix ZMK BASIC 260817 FIXED v1.3 STUDIO

이 버전은 v1.2의 모든 키맵·콤보·인코더·마우스 ACL0/ACL2·BT 복구·RF 0/+/-/MAX 기능을 유지하면서, 왼쪽 central에 ZMK Studio를 추가한 버전입니다.

## Studio에서 수정 가능한 범위

- 50개 물리 키의 런타임 바인딩
- 12개 기존 레이어의 키
- 내장 ZMK behavior와 이미 정의된 사용자 behavior 배치
- 레이어 이름
- 4개의 추가 예약 레이어 활성화 및 편집

`studio-rpc-usb-uart`가 모든 기본 behavior를 포함하도록 하므로, Keymap Editor에서 자주 사용하는 `&kp`, `&mt`, `&lt`, `&mo`, `&to`, `&tog`, `&sk`, `&bt`, `&out`, 마우스 behavior와 사용자 정의 hold-tap/RF behavior를 키에 배치할 수 있습니다.

## 현재 ZMK Studio 자체 한계

다음은 펌웨어로 강제로 개방할 수 없는 Studio의 현재 한계입니다.

- 콤보 생성·수정
- 인코더 `sensor-bindings` 수정
- tapping-term, flavor, hold-trigger 같은 behavior 속성 수정
- RF/BLE/Kconfig 같은 컴파일 설정 수정
- 새 behavior 정의
- 4개 예약분을 넘는 새 레이어 생성

이 항목들은 계속 Keymap Editor/소스 수정 후 재빌드가 필요합니다.

## 연결 방식

1. 왼쪽에 `cornix_left_basic_260817_fixed_v1_3_studio.uf2`를 플래시합니다.
2. 왼쪽을 USB로 PC에 연결합니다.
3. Media-BT 레이어의 `OUT_USB`를 눌러 USB 출력을 선택합니다.
4. Chrome/Edge의 ZMK Studio 웹 또는 네이티브 앱에서 연결합니다.

Studio locking은 의도적으로 꺼 두었습니다. 별도 unlock 키 없이 연결 즉시 편집할 수 있고, 모든 물리 키를 자유롭게 사용할 수 있습니다. 개인 장치가 아닌 환경에서는 USB 연결 또는 페어링된 호스트의 접근을 주의하세요.

## 매우 중요한 지속 설정 규칙

Studio에서 저장한 키맵은 플래시에 지속됩니다. 이후 `.keymap` 파일을 바꾸고 새 UF2를 플래시해도 Studio 저장값이 우선하여 새 기본 키맵이 바로 보이지 않을 수 있습니다.

새로 빌드한 기본 키맵을 적용하려면 ZMK Studio에서 **Restore Stock Settings**를 실행하세요. 완전 초기화용 `cornix_settings_reset.uf2`도 가능하지만, 이 방식은 Bluetooth 페어링까지 지울 수 있습니다.

## 전력

Studio는 왼쪽 central의 RAM과 활성 기능을 늘립니다. 깊은 절전은 Zephyr 4.1의 split central + Studio 깨우기 문제를 피하기 위해 켜지 않았습니다. 기존의 +8 dBm, BLE 1M, 5분 배터리 보고, RGB/디스플레이 끔 설정은 유지됩니다.
