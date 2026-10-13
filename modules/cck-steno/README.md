# Cornix v2.2.6 STENO adapter for MODU-C

Active hardware bindings: ../../config/modu.keymap and modu-steno*.dtsi.
The config/cornix.keymap in this module is an immutable canonical-coordinate regression fixture, not the MODU firmware keymap. Legacy tools under tools/ target this fixture by default. Do not edit it to change the active MODU layout.

Run validation/run_host_tests.sh for original semantic regressions and validation/run_modu_aux_tests.sh for new real-output-FIFO integration. These are native-host tests, not a Zephyr compile or hardware test. See ../../VALIDATION.md.

Original MIT SPDX headers are retained; surrounding MODU firmware keeps its manufacturer non-commercial license. The Cornix LED hardware is disabled on MODU. The unchanged decoder/dictionary hashes are in ../../docs/cornix-core-sha256.json.
