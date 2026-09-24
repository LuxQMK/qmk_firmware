# GMMK V2 96% (ISO)

A keyboard made and sold by Glorious LLC. Equipped with the WestBerry G7 ARM Cortex-M4 microcontroller

* Keyboard Maintainer: [GloriousThrall](https://github.com/GloriousThrall)
* Hardware Supported: GMMK V2
* Hardware Availability: [GloriousPCGaming.com](https://www.pcgamingrace.com)

Compile example for this keyboard (using QMK CLI):

    qmk compile -kb gmmk/gmmk2/p96/iso -km via

Flashing example for this keyboard:

    qmk flash -kb gmmk/gmmk2/p96/iso -km via

To reset the board into bootloader mode, do one of the following:

* Hold the Reset switch mounted on the surface of the PCB while connecting the USB cable (remove the spacebar key and press and hold the pin on the right side)
* Hold the Escape key while connecting the USB cable (also erases persistent settings)
* Fn+Backslash will reset the board to bootloader mode if you have flashed the default QMK keymap

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).
