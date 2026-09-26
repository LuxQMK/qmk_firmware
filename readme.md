# LuxQMK Firmware Engine

[![LuxQMK Version](https://img.shields.io/badge/Firmware-LuxQMK%20v0.3.1-8a2be2.svg?style=flat)](https://github.com/LuxQMK/qmk_firmware/releases)
[![LuxQMK Studio](https://img.shields.io/badge/Companion%20App-LuxQMK%20Studio%20v1.4.0-00b4d8.svg?style=flat)](https://github.com/LuxQMK/luxqmk_studio)
[![Web App](https://img.shields.io/badge/Web%20App-studio.luxqmk.click-00f0ff.svg?style=flat)](https://studio.luxqmk.click)
[![Website](https://img.shields.io/badge/Website-luxqmk.click-3b82f6.svg?style=flat)](https://luxqmk.click)
[![QMK Base](https://img.shields.io/badge/QMK%20Base-v0.34.4-blue.svg?style=flat)](https://github.com/qmk/qmk_firmware)
[![License: GPL](https://img.shields.io/badge/License-GPLv2%20%2F%20GPLv3-green.svg)](LICENSE)

**LuxQMK** is an advanced, heavily extended fork of [QMK Firmware](https://github.com/qmk/qmk_firmware) (upstream base `v0.34.4`).

It features an extensible userspace runtime engine ([`users/luxqmk/`](users/luxqmk/README.md)), custom Hardware Abstraction Layer (HAL) drivers, real-time dual-layer reactive RGB matrix lighting, CIE1931 perceptual color curves, multi-stop gradient shaders, atomic WebHID direct LED streaming double-buffering, and 1:1 synchronization with **[LuxQMK Studio](https://github.com/LuxQMK/luxqmk_studio)**.

---

## 🌟 Key Features & Extensions

- **Modular Hardware Abstraction Layer (HAL)**:
  - First-class support for **Glorious GMMK 3** (100%, 75%, 65% in ANSI & ISO layouts), logo badge LED, Win Lock status LED, side light diffusers, and rotary encoders.
  - Full support for **Glorious GMMK 2** (96% and 65% in ANSI & ISO layouts) with independent side lighting strips.
  - Universal fallback driver for standard QMK / VIA keyboards.
- **Dual-Layer Reactive RGB Matrix Engine**:
  - Hardware-level real-time blending of ambient background animations with reactive keystroke overlays (Fade, Heatmap, Ripple, Cross, Nexus Star, Wide Wave).
  - Multi-Stop Color Stop Gradient Engine with CIE1931 lightness mapping and spatial density tuning.
  - High-speed direct LED streaming via WebHID for software visualizers (60 FPS).
- **Pro Performance & Gaming Optimizations**:
  - Enforced Full NKRO (N-Key Rollover) on boot.
  - Low-latency debounce algorithms (balanced 5ms default, real-time configurable).
  - 1000 Hz USB polling rate.
- **Extended VIA / WebHID Memory Subsystem**:
  - Dedicated custom EEPROM blocks (up to 1408 bytes) for custom Per-Key RGB profiles, gradient definitions, and hardware lighting states.
  - Full compatibility with VIA v12 protocol and LuxQMK Studio real-time diagnostics.

---

## 📁 Architecture Overview

All custom firmware logic is cleanly isolated in the dedicated userspace directory:

```text
qmk_firmware/
├── users/luxqmk/               # Central LuxQMK Userspace Engine
│   ├── luxqmk.h                # Protocol definitions, custom keycodes, and memory structures
│   ├── luxqmk.c                # EEPROM persistence, WebHID dispatch, NKRO boot, reactive overlay
│   ├── config.h                # System configuration (NKRO, Debounce, EEPROM buffer size)
│   ├── rules.mk                # Build pipeline and automatic hardware driver selection
│   ├── rgb_matrix_user.inc     # Custom animation & gradient shader registrations
│   ├── rgb/
│   │   └── custom_effects.h    # Perceptual CIE1931 color math, gradient & reactive blend algorithms
│   └── boards/                 # Hardware Abstraction Layer (HAL)
│       ├── gmmk3.c / .h        # GMMK 3 (100/75/65 ANSI/ISO), Logo & Lock LEDs, Encoders
│       ├── gmmk2.c / .h        # GMMK 2 (96/65 ANSI/ISO), Sidelights
│       └── generic.c / .h      # Universal VIA / QMK board fallback driver
└── keyboards/gmmk/             # Keyboard targets and matrix layout definitions
```

---

## ⌨️ Hardware Support

LuxQMK includes dedicated hardware drivers for **Glorious GMMK 3** (100%, 75%, 65%), **Glorious GMMK 2** (96%, 65%), and a universal fallback for standard QMK / VIA keyboards.

👉 For the full breakdown of supported models, hardware verification status, and compilation targets, see **[Supported Keyboards & Hardware Status](docs/supported_keyboards.md)**.

---

## 🛠️ Compilation & Building

Ensure you have the [QMK CLI](https://docs.qmk.fm/#/cli) or [QMK MSYS](https://msys.qmk.fm/) installed:

```bash
# General command syntax:
qmk compile -kb <keyboard_path> -km <keymap>

# Example: Glorious GMMK 3 100% ANSI (via keymap)
qmk compile -kb gmmk/gmmk3/p100/ansi -km via
```

*(See [Supported Keyboards Matrix](docs/supported_keyboards.md) for all board paths and keymap names).*

Compiled `.bin` or `.hex` firmware binaries can be flashed directly using **[LuxQMK Studio](https://github.com/LuxQMK/luxqmk_studio)** (integrated Smart Flasher with automatic backup & restore) or via `wb32-dfu-updater_cli` / `dfu-util`.

---

## 🎛️ Companion Software

Configure your keyboard in real time using **[LuxQMK Studio](https://github.com/LuxQMK/luxqmk_studio)**:
- Available as a standalone WebHID web application at **[studio.luxqmk.click](https://studio.luxqmk.click)**
- Official Project Website & Documentation: **[luxqmk.click](https://luxqmk.click)**
- Desktop Windows Installer available in [LuxQMK Studio Releases](https://github.com/LuxQMK/luxqmk_studio/releases)

---

## 📜 License & Acknowledgments

- **LuxQMK Firmware Extension**: Licensed under the GNU General Public License v2 / v3 ([GPLv2+](LICENSE)).
- **QMK Firmware Engine**: LuxQMK is built upon the open-source [QMK Firmware](https://github.com/qmk/qmk_firmware) ecosystem.
- **Hardware Credits**: Thanks to [ell1010](https://github.com/ell1010) ([QMK-OpenRGB](https://github.com/ell1010/QMK-OpenRGB)) — GMMK hardware support in LuxQMK is based on their original implementation.
- **Trademarks**: All product names, logos, and brands (including Glorious, GMMK, WB32) are property of their respective owners and are used solely for compatibility and identification purposes.
