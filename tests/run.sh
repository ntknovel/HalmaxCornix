#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
M="$ROOT/modules/cck-steno"
TMP="$(mktemp -d /tmp/cck-host.XXXXXX)"
trap 'rm -rf "$TMP"' EXIT
python3 "$ROOT/tests/test_package.py"
bash "$M/validation/run_host_tests.sh"
bash "$M/validation/run_modu_aux_tests.sh"
cc -std=c11 -Wall -Wextra -Werror -I"$ROOT/tests/wheel_stubs" "$ROOT/tests/test_wheel.c" -o "$TMP/wheel"
"$TMP/wheel"
cc -std=c11 -Wall -Wextra -Werror -include "$M/validation/modu_runtime_config.h" \
 -I"$M/validation/runtime_stubs" -I"$M/validation/stubs" -I"$M/include" -I"$M/validation" \
 "$ROOT/tests/test_cck.c" "$M/validation/runtime_stub.c" "$M/validation/output_capture.c" \
 "$M/validation/quick_invoke_stub.c" "$M/validation/key_event_capture.c" \
 "$M/src/steno/steno_engine.c" "$M/src/steno/steno_decoder.c" \
 "$M/src/steno/steno_dictionary_generated.c" "$M/src/steno/steno_quick_generated.c" -o "$TMP/cck"
"$TMP/cck"
for API in zmk_endpoint_set_preferred_transport zmk_endpoints_select_transport; do
 EXTRA=(); if [[ "$API" == "zmk_endpoints_select_transport" ]]; then EXTRA=(-DCORNIX_TEST_OLD_API=1); fi
 cc -std=c11 -Wall -Wextra -Werror "${EXTRA[@]}" -DCORNIX_SELECT_USB_TRANSPORT="$API" \
 -I"$ROOT/tests/studio_stubs" "$ROOT/tests/test_usb.c" -o "$TMP/usb"
 "$TMP/usb"
done
python3 "$ROOT/tests/test_cmake.py"
python3 "$ROOT/tests/test_api_shapes.py"
if command -v clang >/dev/null 2>&1; then
 clang -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer \
 -I"$ROOT/tests/wheel_stubs" "$ROOT/tests/test_wheel.c" -o "$TMP/wheel-san"
 "$TMP/wheel-san"
 CC=clang SANITIZER_FLAGS='-fsanitize=address,undefined -fno-omit-frame-pointer' bash "$M/validation/run_modu_aux_tests.sh"
fi
bash "$ROOT/tests/run_pointing.sh"
python3 "$ROOT/tests/test_v111.py"
printf '\nCCK source/host checks finished. No ARM compilation or physical Studio test is claimed.\n'
