# CCK Ball 4x6 — MODU STENO + Dual Studio v1.1.1

**Source candidate, not a prebuilt UF2 or hardware-tested release.**

RIGHT = central / trackball / USB Studio. LEFT = peripheral.
Original CCK scan and sensor pins, MODU Korean STENO engine, no-click dual rotary wheels.
Physical position 23 (directly below Backspace 11) toggles STENO.
Four Studio-editable virtual positions 60..63 route encoder CW/CCW pulses.
USB and BLE Studio enabled; Studio locking disabled.

New STENO changes: position 00 is !, 11 is ?, position 12 is ~ tap / Tab hold (200ms).
I and A vowels anchor JLKI arrows. EO anchors left/right/middle mouse buttons,
I/K wheel scrolling and physical trackball scroll, with no extra Space on release.
BASE Esc/Tab/Backspace, mode23, Select58 and all wheel defaults are unchanged.

Read [the Korean guide](docs/GUIDE_KO.md) before building and [source keymaps](docs/KEYMAPS.html).

Standard ZMK/DYA **key assignment** only. No DYA runtime macro/combo extension or
NTK dynamic steno dictionary writes are claimed. No automatic reset/erase/flash.

Run `bash tests/run.sh`. GitHub Actions builds left/right/reset after source tests.
Donor algorithm tests use their original test fixture; CCK coordinates and wheel
routing have additional integration tests. Host tests are not real Zephyr builds.
