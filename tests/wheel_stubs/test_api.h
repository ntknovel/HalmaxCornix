#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#define CONFIG_ZMK_SPLIT 1
#define CONFIG_ZMK_BEHAVIOR_METADATA 1
#define IS_ENABLED(x) x
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#define DT_HAS_COMPAT_STATUS_OKAY(x) 1
#define DT_NODELABEL(x) x
#define STRINGIFY_(x) #x
#define DEVICE_DT_NAME(x) STRINGIFY_(x)
#define DT_INST_FOREACH_STATUS_OKAY(fn)
#define ZMK_BEHAVIOR_OPAQUE 0
#define ZMK_BEHAVIOR_TRANSPARENT 1
#define ZMK_KEYMAP_LAYER_ID_INVAL UINT8_MAX
#define ZMK_POSITION_STATE_CHANGE_SOURCE_LOCAL UINT8_MAX
#define ZMK_VIRTUAL_KEY_POSITION_SENSOR(x) (64+(x))
struct device { const void *config; };
struct zmk_behavior_binding {uint16_t local_id; const char *behavior_dev; uint32_t param1,param2;};
struct zmk_behavior_binding_event {int layer; uint32_t position; int64_t timestamp; uint8_t source;};
struct behavior_parameter_value_metadata {
 char *display_name;
 union {uint32_t value;struct{int32_t min,max;}range;};
 enum {BEHAVIOR_PARAMETER_VALUE_TYPE_NIL,BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
       BEHAVIOR_PARAMETER_VALUE_TYPE_RANGE,BEHAVIOR_PARAMETER_VALUE_TYPE_HID_USAGE,
       BEHAVIOR_PARAMETER_VALUE_TYPE_LAYER_ID}type;
};
struct behavior_parameter_metadata_set {size_t param1_values_len;const struct behavior_parameter_value_metadata *param1_values;size_t param2_values_len;const struct behavior_parameter_value_metadata *param2_values;};
struct behavior_parameter_metadata {size_t sets_len;const struct behavior_parameter_metadata_set *sets;};
#define BEHAVIOR_LOCALITY_CENTRAL 0
struct behavior_driver_api {
 int locality;
 int (*binding_pressed)(struct zmk_behavior_binding *,struct zmk_behavior_binding_event);
 int (*binding_released)(struct zmk_behavior_binding *,struct zmk_behavior_binding_event);
 int (*get_parameter_metadata)(const struct device*,struct behavior_parameter_metadata*);
};
uint8_t zmk_keymap_highest_layer_active(void);
uint8_t zmk_keymap_layer_default(void);
uint8_t zmk_keymap_layer_index_to_id(uint8_t);
bool zmk_keymap_layer_active(uint8_t);
const struct zmk_behavior_binding *zmk_keymap_get_layer_binding_at_idx(uint8_t,uint8_t);
int zmk_behavior_invoke_binding(const struct zmk_behavior_binding*,struct zmk_behavior_binding_event,bool);
