#pragma once
#include <stdint.h>
#include <stdbool.h>
struct zmk_behavior_binding {uint16_t local_id;const char *behavior_dev;uint32_t param1,param2;};
struct zmk_behavior_binding_event {int layer;uint32_t position;int64_t timestamp;uint8_t source;};
int zmk_behavior_invoke_binding(const struct zmk_behavior_binding*,struct zmk_behavior_binding_event,bool);
