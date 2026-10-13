#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <zephyr/device.h>
#include <zephyr/input/input.h>
#define ZMK_INPUT_PROC_CONTINUE 0
#define ZMK_INPUT_PROC_STOP 1
struct zmk_input_processor_state { uint8_t input_device_index;int16_t *remainder;};
struct zmk_input_processor_driver_api {int (*handle_event)(const struct device*,struct input_event*,uint32_t,uint32_t,struct zmk_input_processor_state*);};
