/* SPDX-License-Identifier: MIT */
#pragma once
#include <stdbool.h>
#include <zmk/keymap.h>
#ifndef CCK_LAYER_API_ARITY
#error "CMake must inspect the actual ZMK keymap header"
#endif
static inline int cck_layer_activate(uint8_t layer) {
#if CCK_LAYER_API_ARITY == 1
    return zmk_keymap_layer_activate(layer);
#elif CCK_LAYER_API_ARITY == 2
    return zmk_keymap_layer_activate(layer, false);
#else
#error "Unknown ZMK layer API"
#endif
}
static inline int cck_layer_deactivate(uint8_t layer) {
#if CCK_LAYER_API_ARITY == 1
    return zmk_keymap_layer_deactivate(layer);
#else
    return zmk_keymap_layer_deactivate(layer, false);
#endif
}
