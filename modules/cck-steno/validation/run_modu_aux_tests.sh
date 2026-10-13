#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BIN="$(mktemp /tmp/modu-aux-test.XXXXXX)"
trap 'rm -f "$BIN"' EXIT
"${CC:-cc}" -std=c11 -Wall -Wextra -Werror ${SANITIZER_FLAGS:-} \
 -include "$ROOT/validation/modu_runtime_config.h" \
 -I"$ROOT/validation/runtime_stubs" -I"$ROOT/validation/stubs" -I"$ROOT/validation/api_stubs" -I"$ROOT/include" \
 "$ROOT/validation/test_modu_aux.c" "$ROOT/validation/runtime_stub.c" \
 "$ROOT/validation/key_event_capture.c" "$ROOT/validation/quick_invoke_stub.c" \
 "$ROOT/src/steno/modu_aux.c" "$ROOT/src/steno/steno_output.c" \
 "$ROOT/src/steno/steno_dual.c" "$ROOT/src/steno/steno_tab.c" \
 "$ROOT/src/steno/steno_engine.c" "$ROOT/src/steno/steno_decoder.c" \
 "$ROOT/src/steno/steno_dictionary_generated.c" "$ROOT/src/steno/steno_quick_generated.c" \
 -o "$BIN"
"$BIN"
