/* SPDX-License-Identifier: MIT */
#define DT_DRV_COMPAT zmk_behavior_modu_steno_aux
#include <errno.h>
#include <zephyr/device.h>
#include <zephyr/sys/util.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/keymap.h>
#include <cornix_steno/modu_aux.h>
#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)
struct aux_config { struct modu_steno_aux_config keys; uint8_t fn_layer, symbol_layer; };
static int pressed(struct zmk_behavior_binding *binding, struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    if (!dev) return -ENODEV;
    const struct aux_config *cfg = dev->config;
    return modu_steno_aux_pressed(&cfg->keys, event.position,
        zmk_keymap_layer_active(cfg->fn_layer), zmk_keymap_layer_active(cfg->symbol_layer), event.timestamp);
}
static int released(struct zmk_behavior_binding *binding, struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    return modu_steno_aux_released(event.position);
}
static const struct behavior_driver_api api = {
    .binding_pressed = pressed, .binding_released = released,
    .locality = BEHAVIOR_LOCALITY_CENTRAL,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .get_parameter_metadata = zmk_behavior_get_empty_param_metadata,
#endif
};
#define AUX_INST(n) \
    static const struct aux_config config_##n = { \
        .keys = { .normal_key = DT_INST_PROP(n, normal_key), \
                  .symbol_key = DT_INST_PROP(n, symbol_key), \
                  .function_key = DT_INST_PROP(n, function_key), \
                  .one_shot_tab = DT_INST_PROP(n, one_shot_tab) }, \
        .fn_layer = DT_INST_PROP(n, function_layer), \
        .symbol_layer = DT_INST_PROP(n, symbol_layer), \
    }; \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, &config_##n, POST_KERNEL, \
                           CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &api);
DT_INST_FOREACH_STATUS_OKAY(AUX_INST)
#endif
