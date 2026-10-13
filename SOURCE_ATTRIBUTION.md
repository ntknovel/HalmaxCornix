# Source provenance and licensing

- Hardware / initial five-layer keymap: user's original CCK-BALL-4x6-Keyball-Stable-GitHub-Upload-v1.0.zip. Exact archive retained in reference/ for comparison. Upstream weekinro/CCK-BALL, 4x6 branch. Original source notices retained.
- Korean STENO: MODU-C_STENO_v0.3.1_Compact71, inside the supplied MODU-C_STENO_Auto_Builder_v1.1.1.zip. Its MIT notices, decoder, generated dictionaries, engine and regression fixtures are retained. No MODU hardware controller is used.
- Arrow metadata and USB-selection source: CORNIX_STENO_DUAL_STUDIO_v2.2.10_Source.zip, MIT notices retained. Decoder/keymap convenience changes from that separate product were NOT imported.
- New CCK wheel router, board adapter, source tests, guides and preview generation: MIT.
- ZMK and the PMW3610 driver are fetched by west under their own licenses. They are not repackaged here. Original external branches remain mutable; this is not a fully pinned reproducible toolchain archive.

Tests use explicit API-shaped mocks/stubs, never ship those stubs into the firmware. Embedded input fixtures under modules/cck-steno/config belong to donor regression tests; actual firmware config is the root config/ directory.
