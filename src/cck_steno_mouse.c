/* SPDX-License-Identifier: MIT
 * Mouse gestures use real ZMK mouse behaviors, NOT keyboard HID usages.
 * Every release uses the exact binding and event captured on press.
 */
#include <errno.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/events/position_state_changed.h>
#include <dt-bindings/zmk/pointing.h>
#include <cornix_steno/pointer.h>

struct cck_mouse_latch {
    bool active;
    struct zmk_behavior_binding binding;
    struct zmk_behavior_binding_event event;
};
static struct cck_mouse_latch held[6];
K_MUTEX_DEFINE(cck_mouse_mutex);

int cck_steno_mouse_emit(uint32_t action, bool pressed, int64_t timestamp) {
    if (action <= CCK_MOUSE_NONE || action > CCK_MOUSE_DOWN) return -EINVAL;
    struct cck_mouse_latch *s = &held[action];
    k_mutex_lock(&cck_mouse_mutex, K_FOREVER);
    int err = 0;
    if (pressed && !s->active) {
        const bool scroll = action == CCK_MOUSE_UP || action == CCK_MOUSE_DOWN;
        const uint32_t physical[] = {0, 31, 33, 44, 20, 32};
        s->binding = (struct zmk_behavior_binding){
            .behavior_dev = scroll ? DEVICE_DT_NAME(DT_NODELABEL(msc)) :
                                     DEVICE_DT_NAME(DT_NODELABEL(mkp)),
            .param1 = action == CCK_MOUSE_LEFT ? MB1 :
                      action == CCK_MOUSE_RIGHT ? MB2 :
                      action == CCK_MOUSE_MIDDLE ? MB3 :
                      action == CCK_MOUSE_UP ? MOVE_Y(12) : MOVE_Y(-12),
            .param2 = 0,
        };
        s->event = (struct zmk_behavior_binding_event){
            .position = physical[action], .layer = DT_PROP(DT_NODELABEL(st_mode), layer),
            .timestamp = timestamp,
#if IS_ENABLED(CONFIG_ZMK_SPLIT)
            .source = ZMK_POSITION_STATE_CHANGE_SOURCE_LOCAL,
#endif
        };
        err = zmk_behavior_invoke_binding(&s->binding, s->event, true);
        if (err < 0) {
            /* Compensating key-up for any partially accepted action. */
            (void)zmk_behavior_invoke_binding(&s->binding, s->event, false);
        } else {
            s->active = true;
        }
    } else if (!pressed && s->active) {
        s->event.timestamp = timestamp;
        err = zmk_behavior_invoke_binding(&s->binding, s->event, false);
        s->active = false;
    }
    k_mutex_unlock(&cck_mouse_mutex);
    return err;
}
