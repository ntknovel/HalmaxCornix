/* SPDX-License-Identifier: MIT */
#pragma once
#include <stdbool.h>
#include <stdint.h>
/* Internal mouse action IDs; never sent as keyboard HID codes. */
enum cck_steno_mouse_action {
    CCK_MOUSE_NONE = 0, CCK_MOUSE_LEFT, CCK_MOUSE_RIGHT,
    CCK_MOUSE_MIDDLE, CCK_MOUSE_UP, CCK_MOUSE_DOWN,
};
int cck_steno_mouse_emit(uint32_t action, bool pressed, int64_t timestamp);
/* Claims a sole EO gesture; never cancels an ordinary multi-role stroke. */
bool cornix_steno_engine_claim_pointer(bool allow_empty, uint32_t *epoch);
bool cornix_steno_dual_claim_pointer(uint32_t *epoch);
