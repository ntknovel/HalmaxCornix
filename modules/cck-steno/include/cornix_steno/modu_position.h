/* SPDX-License-Identifier: MIT
 * CCK-BALL 4x6 (60 scan positions + 4 virtual wheels) -> immutable engine
 * coordinates. Retained function name is an internal MODU adapter ABI only.
 * No GPIO or MODU scanner data is used here.
 */
#pragma once
#include <stdint.h>
static inline uint32_t modu_steno_engine_position(uint32_t physical) {
    static const uint32_t positions[64] = {
        50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61,
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 30,
        12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
        24, 25, 26, 27, 28, 29, 32, 33, 34, 35, 36, 37,
        38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
        UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX,
    };
    return physical < 64 ? positions[physical] : UINT32_MAX;
}
