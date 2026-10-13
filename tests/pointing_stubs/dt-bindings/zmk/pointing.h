/* v0.3 pointing.h subset: exact MB bits and low16 vertical encoding. */
#pragma once
#define MB1 1
#define MB2 2
#define MB3 4
#define MOVE_Y(v) ((v) & 0xffff)
