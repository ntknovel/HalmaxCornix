/* SPDX-License-Identifier: MIT
 * MODU adaptation: one FIFO for generated taps and physical auxiliary events.
 * Every admitted DOWN reserves capacity for its future UP. Queue pressure can
 * reject new input but cannot strand a successfully admitted held key.
 */
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <zmk/events/keycode_state_changed.h>
#include <cornix_steno/output.h>
LOG_MODULE_REGISTER(cornix_steno_output, CONFIG_ZMK_LOG_LEVEL);

/* ZMK reserves bare UP/DOWN as keycode macros. These are private queue
 * actions, not keycodes: keep their identifiers namespaced. Do not #undef
 * ZMK's direction keys, which remain available to the keymap and engine. */
enum cst_output_action {
    CST_OUTPUT_TAP,
    CST_OUTPUT_PRESS,
    CST_OUTPUT_RELEASE
};
struct output_item { uint32_t key; enum cst_output_action action; };
static struct output_item queue[CONFIG_CORNIX_STENO_OUTPUT_QUEUE_SIZE];
static size_t queue_head, queue_tail, queue_count, reserved_releases;
/* There are only 13 auxiliary bindings, but permit all 61 physical switches. */
#define HELD_SLOTS 64
struct key_ref { uint32_t key; unsigned count; };
static struct key_ref requested[HELD_SLOTS], actual[HELD_SLOTS];
static bool active_pressed, scheduled;
static uint32_t active_key;
K_MUTEX_DEFINE(output_mutex);
static struct k_work_delayable output_work;

static struct key_ref *find_ref(struct key_ref *refs, uint32_t key, bool allocate) {
    struct key_ref *empty = NULL;
    for (size_t i = 0; i < HELD_SLOTS; i++) {
        if (refs[i].count && refs[i].key == key) return &refs[i];
        if (!refs[i].count && !empty) empty = &refs[i];
    }
    if (allocate && empty) { empty->key = key; return empty; }
    return NULL;
}
/* Called under output_mutex: serialize HID events with flush and refcounts. */
static void send_ref(uint32_t key, bool pressed) {
    struct key_ref *ref = find_ref(actual, key, pressed);
    if (!ref) { if (pressed) LOG_ERR("STENO held-key capacity exhausted"); return; }
    if (pressed) {
        if (ref->count++ != 0) return;
    } else {
        if (--ref->count != 0) return;
    }
    const int err = raise_zmk_keycode_state_changed_from_encoded(key, pressed, k_uptime_get());
    if (err < 0) LOG_WRN("STENO key event failed: %d", err);
}
static void append(uint32_t key, enum cst_output_action action) {
    queue[queue_tail] = (struct output_item){key, action};
    queue_tail = (queue_tail + 1U) % ARRAY_SIZE(queue);
    queue_count++;
}
static void schedule(k_timeout_t delay) {
    scheduled = true;
    k_work_reschedule(&output_work, delay);
}
static void output_work_handler(struct k_work *work) {
    ARG_UNUSED(work);
    k_mutex_lock(&output_mutex, K_FOREVER);
    scheduled = false;
    if (active_pressed) {
        send_ref(active_key, false);
        active_pressed = false;
        active_key = 0;
        /* Keep the gap even if a new item arrives just after this release. */
        schedule(K_MSEC(CONFIG_CORNIX_STENO_OUTPUT_GAP_MS));
    } else if (queue_count) {
        const struct output_item item = queue[queue_head];
        queue_head = (queue_head + 1U) % ARRAY_SIZE(queue);
        queue_count--;
        send_ref(item.key, item.action != CST_OUTPUT_RELEASE);
        if (item.action == CST_OUTPUT_TAP) {
            active_key = item.key;
            active_pressed = true;
            schedule(K_MSEC(CONFIG_CORNIX_STENO_OUTPUT_HOLD_MS));
        } else {
            /* Preserve distinct reports for a rapid DOWN/UP pair. */
            schedule(K_MSEC(item.action == CST_OUTPUT_PRESS ? CONFIG_CORNIX_STENO_OUTPUT_HOLD_MS
                                                : CONFIG_CORNIX_STENO_OUTPUT_GAP_MS));
        }
    }
    k_mutex_unlock(&output_mutex);
}
static int output_init(void) {
    k_work_init_delayable(&output_work, output_work_handler);
    return 0;
}
SYS_INIT(output_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
int cornix_steno_output_enqueue(uint32_t encoded_key) {
    return cornix_steno_output_enqueue_sequence(&encoded_key, 1);
}
int cornix_steno_output_enqueue_sequence(const uint32_t *keys, size_t count) {
    if (!count) return 0;
    if (!keys) return -EINVAL;
    k_mutex_lock(&output_mutex, K_FOREVER);
    if (count > ARRAY_SIZE(queue) - queue_count - reserved_releases) {
        k_mutex_unlock(&output_mutex);
        LOG_WRN("STENO output queue full; rejected %u keys", (unsigned)count);
        return -ENOSPC;
    }
    for (size_t i = 0; i < count; i++) append(keys[i], CST_OUTPUT_TAP);
    if (!scheduled) schedule(K_NO_WAIT);
    k_mutex_unlock(&output_mutex);
    return 0;
}
int cornix_steno_output_enqueue_event(uint32_t key, bool pressed) {
    k_mutex_lock(&output_mutex, K_FOREVER);
    struct key_ref *ref = find_ref(requested, key, pressed);
    if (!pressed && !ref) { k_mutex_unlock(&output_mutex); return 0; }
    if (pressed) {
        if (!ref || ARRAY_SIZE(queue) - queue_count - reserved_releases < 2) {
            k_mutex_unlock(&output_mutex);
            return -ENOSPC;
        }
        ref->count++;
        reserved_releases++;
        append(key, CST_OUTPUT_PRESS);
    } else {
        ref->count--;
        reserved_releases--;
        append(key, CST_OUTPUT_RELEASE);
    }
    if (!scheduled) schedule(K_NO_WAIT);
    k_mutex_unlock(&output_mutex);
    return 0;
}
void cornix_steno_output_flush(void) {
    k_mutex_lock(&output_mutex, K_FOREVER);
    k_work_cancel_delayable(&output_work);
    queue_head = queue_tail = queue_count = reserved_releases = 0;
    active_pressed = scheduled = false;
    active_key = 0;
    memset(requested, 0, sizeof(requested));
    for (size_t i = 0; i < HELD_SLOTS; i++) {
        if (actual[i].count) {
            raise_zmk_keycode_state_changed_from_encoded(actual[i].key, false, k_uptime_get());
        }
    }
    memset(actual, 0, sizeof(actual));
    k_mutex_unlock(&output_mutex);
}
size_t cornix_steno_output_pending(void) {
    k_mutex_lock(&output_mutex, K_FOREVER);
    const size_t pending = queue_count + (active_pressed ? 1U : 0U);
    k_mutex_unlock(&output_mutex);
    return pending;
}
