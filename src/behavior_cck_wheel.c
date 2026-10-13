/* SPDX-License-Identifier: MIT
 * CCK-BALL rotary pulses -> Studio-editable keymap cells 60..63.
 * Called by standard ZMK sensor-rotate (balanced 10 ms press/release pairs).
 * A release always uses the binding captured on press, even if the active
 * layer or a Studio binding changes in between. No physical key/steno event
 * is synthesized, and no encoder click switch is invented.
 */
#define DT_DRV_COMPAT zmk_behavior_cck_wheel
#include <errno.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/sys/util.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/keymap.h>
#include <zmk/virtual_key_position.h>
#include <zmk/events/position_state_changed.h>

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)
#define FIRST_SLOT 60u
#define SLOT_COUNT 4u
struct wheel_latch {
    bool active;
    struct zmk_behavior_binding binding;
    struct zmk_behavior_binding_event event;
};
static struct wheel_latch latches[SLOT_COUNT];

static int slot_index(const struct zmk_behavior_binding *b,
                      struct zmk_behavior_binding_event e) {
    if (b == NULL || b->param2 != 0 || b->param1 < FIRST_SLOT ||
        b->param1 >= FIRST_SLOT + SLOT_COUNT) return -EINVAL;
    unsigned i = b->param1 - FIRST_SLOT;
    /* Internal binding is not a general-purpose key or combo. */
    if (e.position != ZMK_VIRTUAL_KEY_POSITION_SENSOR(i / 2u)) return -EINVAL;
    return (int)i;
}
static int on_pressed(struct zmk_behavior_binding *router,
                      struct zmk_behavior_binding_event e) {
    int i = slot_index(router, e);
    if (i < 0) return i;
    struct wheel_latch *s = &latches[i];
    if (s->active) return -EBUSY; /* Do not replace an unreleased key. */
    const uint8_t fallback = zmk_keymap_layer_default();
    for (int index = (int)zmk_keymap_highest_layer_active(); index >= 0; --index) {
        const uint8_t id = zmk_keymap_layer_index_to_id((uint8_t)index);
        if (id == ZMK_KEYMAP_LAYER_ID_INVAL ||
            (!zmk_keymap_layer_active(id) && id != fallback)) continue;
        const struct zmk_behavior_binding *b =
            zmk_keymap_get_layer_binding_at_idx(id, (uint8_t)router->param1);
        if (!b || !b->behavior_dev) continue;
        if (strcmp(b->behavior_dev, DEVICE_DT_NAME(DT_NODELABEL(trans))) == 0) continue;
        if (strcmp(b->behavior_dev, router->behavior_dev) == 0) return -ELOOP;
        s->binding = *b;
        s->event = e;
        s->event.position = router->param1;
        s->event.layer = id;
#if IS_ENABLED(CONFIG_ZMK_SPLIT)
        s->event.source = ZMK_POSITION_STATE_CHANGE_SOURCE_LOCAL;
#endif
        int err = zmk_behavior_invoke_binding(&s->binding, s->event, true);
        if (err == ZMK_BEHAVIOR_TRANSPARENT) continue;
        if (err < 0) {
            /* Some composite behaviors can partially press before failing. */
            (void)zmk_behavior_invoke_binding(&s->binding, s->event, false);
            return err;
        }
        s->active = true;
        return ZMK_BEHAVIOR_OPAQUE;
    }
    return ZMK_BEHAVIOR_OPAQUE;
}
static int on_released(struct zmk_behavior_binding *router,
                       struct zmk_behavior_binding_event e) {
    int i = slot_index(router, e);
    if (i < 0) return i;
    struct wheel_latch *s = &latches[i];
    if (!s->active) return ZMK_BEHAVIOR_OPAQUE;
    s->event.timestamp = e.timestamp;
    int err = zmk_behavior_invoke_binding(&s->binding, s->event, false);
    s->active = false;
    return err;
}
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
static int metadata(const struct device *dev, struct behavior_parameter_metadata *out) {
    (void)dev;
    if (!out) return -EINVAL;
    static const struct behavior_parameter_value_metadata v[] = {
        {.display_name = "Internal wheel slot (do not assign to physical keys)",
         .type = BEHAVIOR_PARAMETER_VALUE_TYPE_RANGE, .range = {.min=60, .max=63}},
    };
    static const struct behavior_parameter_metadata_set sets[] = {
        {.param1_values=v, .param1_values_len=ARRAY_SIZE(v)},
    };
    *out=(struct behavior_parameter_metadata){.sets=sets,.sets_len=ARRAY_SIZE(sets)};
    return 0;
}
#endif
static const struct behavior_driver_api api = {
    .binding_pressed=on_pressed, .binding_released=on_released,
    .locality=BEHAVIOR_LOCALITY_CENTRAL,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .get_parameter_metadata=metadata,
#endif
};
#define CCK_WHEEL_INST(n) BEHAVIOR_DT_INST_DEFINE(n,NULL,NULL,NULL,NULL,POST_KERNEL, \
                                        CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,&api);
DT_INST_FOREACH_STATUS_OKAY(CCK_WHEEL_INST)
#endif
