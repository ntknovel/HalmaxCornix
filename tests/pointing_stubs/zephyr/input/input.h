#pragma once
#include <stdint.h>
struct input_event { const struct device *dev; uint8_t sync;uint8_t type;uint16_t code;int32_t value;};
