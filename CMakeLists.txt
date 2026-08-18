# SPDX-License-Identifier: MIT

target_sources_ifdef(CONFIG_ZMK_BEHAVIOR_RF_POWER app PRIVATE
  src/behaviors/behavior_rf_power.c
)
