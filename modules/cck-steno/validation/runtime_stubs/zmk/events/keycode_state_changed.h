#pragma once
#include <stdbool.h>
#include <stdint.h>
/* Real ZMK includes keys.h transitively through zmk/keys.h.
 * Keep the keycode macro namespace visible, including UP and DOWN. */
#include <dt-bindings/zmk/keys.h>
int test_raise_keycode(uint32_t key, bool state, int64_t ts);
static inline int raise_zmk_keycode_state_changed_from_encoded(uint32_t key, bool state, int64_t ts) {
    return test_raise_keycode(key, state, ts);
}
