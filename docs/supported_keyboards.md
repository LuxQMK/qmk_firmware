# Supported Keyboards & Hardware Verification Status

While LuxQMK inherits broad compatibility with standard QMK / VIA keyboards, the following hardware models are equipped with dedicated Hardware Abstraction Layer (HAL) drivers and matrix configurations.

> [!IMPORTANT]
> **Physical Hardware Testing Notice**:  
> Currently, **only the Glorious GMMK 3 100% ANSI** is physically verified and tested on actual hardware by the maintainer.  
> All other GMMK 3 / GMMK 2 models and ISO variants have their HAL drivers, layout mappings, and build pipelines fully implemented (ported/adapted from ell1010's open-source work and official matrix definitions) and compile without errors, but have **not yet been physically flashed or tested on real hardware**.  
> If you own one of these keyboards, community testing, bug reports, and confirmation are warmly welcome!

---

## 🌟 Universal LuxQMK Core Capabilities (All Keyboards)

Every keyboard built with `USER_NAME = luxqmk` (including **Generic QMK / VIA**, **GMMK 2**, and **GMMK 3**) inherits the complete suite of LuxQMK userspace engine features:

- **Dual-Layer Reactive RGB Matrix Engine**: Real-time hardware blending of ambient background animations with reactive keystroke overlays (Fade, Heatmap, Ripple, Cross, Nexus Star, Wide Wave).
- **Multi-Stop Color Stop Gradient Engine**: CIE1931 perceptual lightness curve, spatial density tuning (0.5x – 2.0x), and direction controls.
- **Per-Key RGB Custom Palettes & Profiles**: Full color remapping with EEPROM block persistence (up to 1408 bytes).
- **Direct 60 FPS WebHID Streaming**: High-performance software lighting and Web Audio Visualizer support.
- **Gaming Performance Tuning**: Enforced Full NKRO bootup, dynamic low-latency debounce engine (1–16ms, 5ms default), 1000 Hz polling.

---

## 📋 Hardware Compatibility & Status Matrix

| Keyboard Model | Layout Form Factor | Matrix Layout | Hardware Status |
| :--- | :--- | :--- | :--- |
| **Glorious GMMK 3 100% ANSI** | Full-size (100%) | ANSI (112 Keys) | **✅ Tested & Verified (Main Testbed)** |
| **Glorious GMMK 3 100% ISO**  | Full-size (100%) | ISO (113 Keys)  | ⚠️ Implemented (Untested on Hardware) |
| **Glorious GMMK 3 75%**       | Compact (75%)    | ANSI (88) / ISO (89) | ⚠️ Implemented (Untested on Hardware) |
| **Glorious GMMK 3 65%**       | Compact (65%)    | ANSI (71) / ISO (72) | ⚠️ Implemented (Untested on Hardware) |
| **Glorious GMMK 2 96%**       | Compact Full (96%) | ANSI (103) / ISO (104) | ⚠️ Implemented (Untested on Hardware) |
| **Glorious GMMK 2 65%**       | Compact (65%)    | ANSI (71) / ISO (72) | ⚠️ Implemented (Untested on Hardware) |
| **Generic QMK / VIA**         | Universal        | Variable | ℹ️ Universal Userspace (Untested) |

---

## 🛠️ Board-Specific HAL Specializations

The Hardware Abstraction Layer (`users/luxqmk/boards/`) handles board-specific peripheral mapping, custom indicator LEDs, and optical diffuser geometry:

### Glorious GMMK 3 (100%, 75%, 65%)
- **HAL Driver**: `users/luxqmk/boards/gmmk3.c` & `.h`
- **Board Specializations**:
  - Dedicated Logo badge LED indicator mapping (RGB animation vs Lock color modes)
  - Dedicated Windows Key Lock indicator LED (index 92)
  - Sidelight diffuser strip optical window calibration with center-out wave dynamics
  - Rotary encoder volume / media remapping
- **Compilation Commands**:
  ```bash
  # 100% ANSI / ISO
  qmk compile -kb gmmk/gmmk3/p100/ansi -km via
  qmk compile -kb gmmk/gmmk3/p100/iso -km via

  # 75% ANSI / ISO
  qmk compile -kb gmmk/gmmk3/p75/ansi -km via
  qmk compile -kb gmmk/gmmk3/p75/iso -km via

  # 65% ANSI / ISO
  qmk compile -kb gmmk/gmmk3/p65/ansi -km via
  qmk compile -kb gmmk/gmmk3/p65/iso -km via
  ```

### Glorious GMMK 2 (96%, 65%)
- **HAL Driver**: `users/luxqmk/boards/gmmk2.c` & `.h`
- **Board Specializations**:
  - Dual independent side light strips with smooth wave dynamics and GMMK 2 optical diffuser mapping
  - Rotary encoder handling (where hardware-supported)
- **Compilation Commands**:
  ```bash
  # 96% ANSI / ISO
  qmk compile -kb gmmk/gmmk2/p96/ansi -km via
  qmk compile -kb gmmk/gmmk2/p96/iso -km via

  # 65% ANSI / ISO
  qmk compile -kb gmmk/gmmk2/p65/ansi -km via
  qmk compile -kb gmmk/gmmk2/p65/iso -km via
  ```

### Generic VIA / QMK Keyboards
- **HAL Driver**: `users/luxqmk/boards/generic.c` & `.h`
- **Board Specializations**:
  - Standard fallback driver providing universal compatibility for any QMK/VIA keyboard equipped with RGB Matrix or backlighting.
  - Automatically activates the complete LuxQMK engine (Dual-Layer Reactive, CIE1931 Gradients, Per-Key RGB, Direct WebHID streaming, custom EEPROM blocks, Debounce tuning, NKRO).
