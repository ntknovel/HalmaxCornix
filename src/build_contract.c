/* SPDX-License-Identifier: MIT */
#include <zephyr/devicetree.h>
#include <zephyr/sys/util.h>
#if !IS_ENABLED(CONFIG_ZMK_STUDIO) || IS_ENABLED(CONFIG_ZMK_STUDIO_LOCKING)
#error "CCK right must provide unlocked Studio"
#endif
#if !IS_ENABLED(CONFIG_ZMK_STUDIO_TRANSPORT_BLE) || !IS_ENABLED(CONFIG_ZMK_STUDIO_TRANSPORT_UART)
#error "CCK right must provide BLE and USB/UART Studio"
#endif
BUILD_ASSERT(DT_PROP_LEN(DT_NODELABEL(cck_ball_layouts), keys) == 64,
             "Expected 60 matrix positions plus four virtual wheel slots");
BUILD_ASSERT(DT_PROP_LEN(DT_NODELABEL(default_transform), map) == 64, "Transform mismatch");
BUILD_ASSERT(DT_PROP_LEN(DT_NODELABEL(kscan0), row_gpios) == 5, "No new GPIO row is permitted");
BUILD_ASSERT(DT_PROP_LEN(DT_NODELABEL(kscan0), col_gpios) == 6, "Original scanner required");
BUILD_ASSERT(CONFIG_CORNIX_STENO_SOLO_CORRECTION_MS == 500, "Review select tap timing too");
