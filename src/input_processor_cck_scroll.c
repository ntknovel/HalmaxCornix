/* SPDX-License-Identifier: MIT
 * Runs only on the physical PMW3610 listener of the RIGHT central.
 * It does not change SPI/GPIO/CPI/driver code or switch keyboard layers.
 * Native driver Scroll/Snipe and normal pointer packets pass through unchanged.
 */
#define DT_DRV_COMPAT ntk_cck_steno_scroll
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <drivers/input_processor.h>
#include <zmk/keymap.h>
#include <cornix_steno/pointer.h>

BUILD_ASSERT(CONFIG_PMW3610_SCROLL_TICK > 0, "Scroll tick must be positive");

struct cck_scroll_data {
    uint32_t epoch;
    int64_t x_remainder, y_remainder;
    bool frame_decided, frame_scroll;
};

static int cck_scroll_handle(const struct device *dev, struct input_event *event,
                             uint32_t param1, uint32_t param2,
                             struct zmk_input_processor_state *state) {
    (void)param1; (void)param2; (void)state;
    if (!dev || !event || !dev->data) return -EINVAL;
    struct cck_scroll_data *d = dev->data;
    const bool xy = event->type == INPUT_EV_REL &&
                    (event->code == INPUT_REL_X || event->code == INPUT_REL_Y);
    if (!xy) {
        if (event->sync) d->frame_decided = d->frame_scroll = false;
        return ZMK_INPUT_PROC_CONTINUE;
    }
    /* Decide once at the first nonzero axis of a synchronized input frame.
     * Do not consume a solo Space/EO for zero-only sensor notifications.
     * A key release between X/Y will not split one frame into scroll+pointer. */
    if (!d->frame_decided && event->value != 0) {
        uint32_t epoch = 0;
        d->frame_decided = true;
        d->frame_scroll =
            zmk_keymap_highest_layer_active() == DT_PROP(DT_NODELABEL(st_mode), layer) &&
            cornix_steno_dual_claim_pointer(&epoch);
        if (!d->frame_scroll || d->epoch != epoch) {
            d->x_remainder = d->y_remainder = 0;
        }
        d->epoch = d->frame_scroll ? epoch : 0;
    }
    if (d->frame_scroll) {
        const bool horizontal = event->code == INPUT_REL_X;
        int64_t *remainder = horizontal ? &d->x_remainder : &d->y_remainder;
        /* A 64-bit accumulator also handles adversarial int32 input safely. */
        int64_t total = (int64_t)event->value + *remainder;
        int64_t ticks = total / CONFIG_PMW3610_SCROLL_TICK;
        *remainder = total % CONFIG_PMW3610_SCROLL_TICK;
        /* Same default sign as the original driver's scroll branch. */
        ticks = -ticks;
        if ((horizontal && IS_ENABLED(CONFIG_PMW3610_INVERT_SCROLL_X)) ||
            (!horizontal && IS_ENABLED(CONFIG_PMW3610_INVERT_SCROLL_Y))) ticks = -ticks;
        if (ticks > 127) ticks = 127;
        if (ticks < -127) ticks = -127;
        event->code = horizontal ? INPUT_REL_HWHEEL : INPUT_REL_WHEEL;
        event->value = (int32_t)ticks;
    }
    if (event->sync) d->frame_decided = d->frame_scroll = false;
    /* Preserve sync even on fractional/zero scroll; input_listener flushes it. */
    return ZMK_INPUT_PROC_CONTINUE;
}

static const struct zmk_input_processor_driver_api api = {.handle_event = cck_scroll_handle};
#define CCK_SCROLL_INST(n) \
    static struct cck_scroll_data data_##n; \
    DEVICE_DT_INST_DEFINE(n, NULL, NULL, &data_##n, NULL, POST_KERNEL, \
                          CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &api);
DT_INST_FOREACH_STATUS_OKAY(CCK_SCROLL_INST)
