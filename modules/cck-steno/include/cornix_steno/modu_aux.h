/* SPDX-License-Identifier: MIT */
#pragma once
#include <stdbool.h>
#include <stdint.h>
struct modu_steno_aux_config {
    uint32_t normal_key, symbol_key, function_key;
    bool one_shot_tab;
};
uint32_t modu_steno_aux_choose(const struct modu_steno_aux_config *cfg,
                               uint64_t selectors, bool function_layer, bool symbol_layer);
int modu_steno_aux_pressed(const struct modu_steno_aux_config *cfg, uint32_t position,
                           bool function_layer, bool symbol_layer, int64_t timestamp);
int modu_steno_aux_released(uint32_t position);
void modu_steno_aux_reset(void);
