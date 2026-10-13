/* SPDX-License-Identifier: MIT
 * Auxiliary keys latch their encoding on keydown. Releasing Fn/Symbol first
 * never changes which encoded key is released. The original output FIFO keeps
 * a just-completed Korean chord ahead of the following physical number key.
 */
#include <errno.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <cornix_steno/modu_aux.h>
#include <cornix_steno/engine.h>
#include <cornix_steno/dual.h>
#include <cornix_steno/output.h>
#include <cornix_steno/roles.h>

static struct { bool down, deferred_tab; uint32_t key; } held[71];
K_MUTEX_DEFINE(aux_mutex);

uint32_t modu_steno_aux_choose(const struct modu_steno_aux_config *cfg,
                               uint64_t selectors, bool function_layer, bool symbol_layer) {
    if (function_layer || (selectors & CST_VEXT_MASK)) return cfg->function_key;
    if (symbol_layer || (selectors & CST_SYMBOL_MASK)) return cfg->symbol_key;
    return cfg->normal_key;
}
int modu_steno_aux_pressed(const struct modu_steno_aux_config *cfg, uint32_t position,
                           bool function_layer, bool symbol_layer, int64_t timestamp) {
    if (!cfg || position >= 71) return -EINVAL;
    k_mutex_lock(&aux_mutex, K_FOREVER);
    if (held[position].down) { k_mutex_unlock(&aux_mutex); return 0; }
    const bool steno = cornix_steno_engine_is_active();
    if (steno) cornix_steno_dual_prepare_aux(timestamp);
    const uint64_t selectors = steno ? cornix_steno_engine_prepare_aux(timestamp) : 0;
    const uint32_t key = modu_steno_aux_choose(cfg, selectors, function_layer, symbol_layer);
    const bool defer = steno && cfg->one_shot_tab && !function_layer && !symbol_layer && !selectors;
    int err = defer ? 0 : cornix_steno_output_enqueue_event(key, true);
    if (!err) {
        held[position].down = true;
        held[position].key = key;
        held[position].deferred_tab = defer;
    }
    k_mutex_unlock(&aux_mutex);
    return err;
}
int modu_steno_aux_released(uint32_t position) {
    if (position >= 71) return -EINVAL;
    k_mutex_lock(&aux_mutex, K_FOREVER);
    if (!held[position].down) { k_mutex_unlock(&aux_mutex); return 0; }
    const int err = held[position].deferred_tab ? cornix_steno_output_enqueue(held[position].key)
                    : cornix_steno_output_enqueue_event(held[position].key, false);
    memset(&held[position], 0, sizeof(held[position]));
    k_mutex_unlock(&aux_mutex);
    return err;
}
void modu_steno_aux_reset(void) {
    k_mutex_lock(&aux_mutex, K_FOREVER);
    memset(held, 0, sizeof(held));
    k_mutex_unlock(&aux_mutex);
}
