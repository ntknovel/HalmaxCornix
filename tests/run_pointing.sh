#!/usr/bin/env bash
set -euo pipefail
R="$(cd "$(dirname "$0")/.." && pwd)"; M="$R/modules/cck-steno"
T="$(mktemp -d)"; trap 'rm -rf "$T"' EXIT
for cc in gcc clang; do
 command -v "$cc" >/dev/null || continue
 san=(); [[ "$cc" == clang ]] && san=(-fsanitize=address,undefined -fno-omit-frame-pointer)
 "$cc" -std=c11 -Wall -Wextra -Werror -Wno-misleading-indentation "${san[@]}" \
  -include "$R/tests/pointing_stubs/config.h" \
  -I"$R/tests/pointing_stubs" -I"$M/validation/runtime_stubs" -I"$M/validation/stubs" \
  -I"$M/include" -I"$M/validation" -I"$M/validation/api_stubs" \
  "$R/tests/test_pointing.c" "$R/src/cck_steno_mouse.c" \
  "$M/validation/runtime_stub.c" "$M/validation/output_capture.c" \
  "$M/validation/quick_invoke_stub.c" "$M/validation/key_event_capture.c" \
  "$M/src/steno/steno_engine.c" "$M/src/steno/steno_dual.c" "$M/src/steno/steno_tab.c" \
  "$M/src/steno/steno_decoder.c" "$M/src/steno/steno_dictionary_generated.c" \
  "$M/src/steno/steno_quick_generated.c" -o "$T/pointing-$cc"
 "$T/pointing-$cc"
done
# Original engine regressions with the NEW firmware assist actually enabled.
for name in test_engine test_engine_comprehensive; do
 gcc -std=c11 -Wall -Wextra -Werror -include "$R/tests/pointing_stubs/config.h" \
  -I"$R/tests/pointing_stubs" -I"$M/validation/runtime_stubs" -I"$M/validation/stubs" \
  -I"$M/include" -I"$M/validation" -I"$M/validation/api_stubs" \
  "$M/validation/$name.c" "$R/src/cck_steno_mouse.c" "$R/tests/mouse_native_stub.c" \
  "$M/validation/runtime_stub.c" "$M/validation/output_capture.c" \
  "$M/validation/quick_invoke_stub.c" "$M/validation/key_event_capture.c" \
  "$M/src/steno/steno_engine.c" "$M/src/steno/steno_decoder.c" \
  "$M/src/steno/steno_dictionary_generated.c" "$M/src/steno/steno_quick_generated.c" -o "$T/$name"
 "$T/$name"
done
