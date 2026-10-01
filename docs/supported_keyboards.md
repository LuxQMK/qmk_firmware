# Supported Keyboards & Hardware Verification Status

While **LuxQMK Firmware** provides broad, universal compatibility with standard QMK / VIA keyboards, several keyboard families are equipped with dedicated **Hardware Abstraction Layer (HAL)** drivers, custom indicator routines, optical diffuser calibrations, and physical DIP switch handlers.

> [!IMPORTANT]
> **Physical Hardware Testing Notice**:  
> - **Glorious GMMK 3 100% ANSI** serves as the primary physically verified hardware testbed maintained by the core author.  
> - All other **GMMK 3**, **GMMK 2**, and **Keychron Wired (Q / V / C Pro / K Pro)** models have their HAL drivers, layout matrices, DIP switch handlers, and build pipelines fully implemented and compiling without errors in the CI/CD matrix.  
> - If you flash or test one of these models on real hardware, feedback, confirmation, and pull requests are warmly welcome!

---

## 🌟 Universal LuxQMK Core Capabilities (All Keyboards)

Every keyboard compiled with `USER_NAME = luxqmk` (including **Glorious GMMK 3**, **GMMK 2**, **Keychron Wired**, and **Generic QMK / VIA**) inherits the complete suite of LuxQMK userspace features:

1. **Dual-Layer Reactive RGB Compositing Engine**:
   - Hardware blending of **Layer 0 Base Ambient Matrix** (41+ custom & QMK animation modes) with **Layer 1 Keystroke Reactive Overlay** (Fade, Splash, Splash Rainbow, Cross, Nexus Star, Wide Wave, Keystroke Heatmap).
2. **Decoupled Layer Background Dimming & macOS Transparency**:
   - Dedicated brightness dimming controls (`g_layer_dim_enable`, `g_layer_dim_levels[4]`) independent of layer key highlight colors.
   - Built-in macOS Base transparency (Layer 2) allowing seamless ambient lighting during Mac mode switching.
3. **Multi-Stop Color Stop Gradient Engine**:
   - Custom gradient sampling with CIE1931 perceptual lightness curve correction (`luxqmk_gradients.c`).
   - Real-time spatial density tuning (0.5x – 2.0x) and bidirectional angle controls.
4. **Per-Key RGB Custom Palettes & Profiles**:
   - Up to 3 distinct hardware EEPROM profile blocks (FPS, MOBA, MMO/RPG) with custom key color mappings.
5. **Custom Lock LED Indicator Customizer**:
   - Configurable Caps Lock, Num Lock, Scroll Lock, and Windows Lock LED modes (Off, Custom Color with HSV quantization, Pure White).
6. **Physical Hardware DIP Switch Integration**:
   - Native EEPROM and WebHID protocol support for physical hardware Mac/Win and custom DIP switch toggles (`luxqmk_protocol.c`).
7. **Direct 60 FPS WebHID RGB Streaming**:
   - High-performance double-buffered software lighting engine for zero-latency desktop canvas streaming and WASAPI audio visualizers.
8. **Ultra-Low Latency Gaming Performance**:
   - Enforced **Full NKRO** bootup, 1000 Hz (1ms) USB polling, and dynamic debounce matrix configuration (1–16ms, 5ms default).
9. **Universal 4-Layer VIA & WebHID Protocol**:
   - Microsecond EEPROM writes for real-time key remapping, dynamic macros, custom keycodes (`USER00` – `USER15`), and profile backup/restore.

---

## 📋 Hardware Compatibility & Status Matrix

| Keyboard Model / Family | Form Factor | Matrix Layout | MCU / Architecture | Hardware Status |
| :--- | :--- | :--- | :--- | :--- |
| **Glorious GMMK 3 100% ANSI** | Full-Size (100%) | ANSI (112 Keys) | WB32FQ95 (ARM Cortex-M4) | **✅ Tested & Verified (Main Testbed)** |
| **Glorious GMMK 3 100% ISO** | Full-Size (100%) | ISO (113 Keys) | WB32FQ95 (ARM Cortex-M4) | ⚠️ Implemented (Untested on Hardware) |
| **Glorious GMMK 3 75%** | Compact (75%) | ANSI (88) / ISO (89) | WB32FQ95 (ARM Cortex-M4) | ⚠️ Implemented (Untested on Hardware) |
| **Glorious GMMK 3 65%** | Compact (65%) | ANSI (71) / ISO (72) | WB32FQ95 (ARM Cortex-M4) | ⚠️ Implemented (Untested on Hardware) |
| **Glorious GMMK 2 96%** | Compact Full (96%) | ANSI (103) / ISO (104) | WB32FQ95 (ARM Cortex-M4) | ⚠️ Implemented (Untested on Hardware) |
| **Glorious GMMK 2 65%** | Compact (65%) | ANSI (71) / ISO (72) | WB32FQ95 (ARM Cortex-M4) | ⚠️ Implemented (Untested on Hardware) |
| **Keychron Q Series (Wired)** | 60% – 100% / Alice | ANSI / ISO | STM32L432 / STM32F402 (ARM) | ⚠️ Implemented (Untested on Hardware) |
| **Keychron V Series (Wired)** | 60% – 100% / Alice | ANSI / ISO | STM32L432 (ARM Cortex-M4) | ⚠️ Implemented (Untested on Hardware) |
| **Keychron C / K Pro (Wired)** | 60% – 100% | ANSI / ISO | STM32L432 / STM32F4 (ARM) | ⚠️ Implemented (Untested on Hardware) |
| **Generic QMK / VIA** | Universal | Universal | AVR / STM32 / RP2040 / WB32 | ℹ️ Universal Userspace Support |

---

## 🛠️ Board-Specific HAL Specializations

The Hardware Abstraction Layer (`users/luxqmk/boards/`) handles board-specific peripherals, custom indicator LEDs, optical diffuser geometry, and DIP switches:

### 1. Glorious GMMK 3 (100%, 75%, 65% ANSI & ISO)
- **HAL Module**: `users/luxqmk/boards/gmmk3.c` & `gmmk3.h`
- **Hardware Specializations**:
  - Dedicated **Logo Badge LED** indicator (index 124 on 100%, index 98 on 75%, index 81 on 65%) with reactive layer/lock color overrides.
  - Dedicated **Windows Lock LED** indicator (index 92 on 100%, index 68 on 75%).
  - Optical diffuser sidelight strip calibration with center-out wave propagation.
  - Rotary encoder volume, media scrubbing, and press handling.
- **Compilation Commands**:
  ```bash
  # GMMK 3 100% ANSI / ISO
  qmk compile -kb gmmk/gmmk3/p100/ansi -km via
  qmk compile -kb gmmk/gmmk3/p100/iso -km via

  # GMMK 3 75% ANSI / ISO
  qmk compile -kb gmmk/gmmk3/p75/ansi -km via
  qmk compile -kb gmmk/gmmk3/p75/iso -km via

  # GMMK 3 65% ANSI / ISO
  qmk compile -kb gmmk/gmmk3/p65/ansi -km via
  qmk compile -kb gmmk/gmmk3/p65/iso -km via
  ```

---

### 2. Glorious GMMK 2 (96%, 65% ANSI & ISO)
- **HAL Module**: `users/luxqmk/boards/gmmk2.c` & `gmmk2.h`
- **Hardware Specializations**:
  - Dual independent sidelight diffusion strips with spatial wave mapping and optical correction.
  - Rotary encoder integration (where hardware-supported).
- **Compilation Commands**:
  ```bash
  # GMMK 2 96% ANSI / ISO
  qmk compile -kb gmmk/gmmk2/p96/ansi -km via
  qmk compile -kb gmmk/gmmk2/p96/iso -km via

  # GMMK 2 65% ANSI / ISO
  qmk compile -kb gmmk/gmmk2/p65/ansi -km via
  qmk compile -kb gmmk/gmmk2/p65/iso -km via
  ```

---

### 3. Keychron Wired Keyboards (Q Series, V Series, C Pro, K Pro Wired)
- **HAL Module**: `users/luxqmk/boards/keychron.c` & `keychron.h`
- **Hardware Specializations**:
  - Physical **Hardware DIP Switch** decoding (Mac OS / Windows toggle) with EEPROM synchronization and live layer state reporting.
  - Custom **Lock LED Indicators** (Caps Lock, Num Lock, Scroll Lock) rendered directly on dedicated switch LEDs or RGB matrix positions.
  - Native rotary encoder decoding across all knob-equipped models (Q1, Q2, Q3, Q5, Q6, Q10, V1, V2, V3, V5, V6, etc.).
  - High-performance STM32 ARM Cortex-M4 architecture integration.
- **Compilation Commands (Examples)**:
  ```bash
  # Keychron Q1 ANSI / ISO (Knob & Non-Knob)
  qmk compile -kb keychron/q1/rev_0100 -km via
  qmk compile -kb keychron/q1/iso_encoder -km via

  # Keychron Q3 / Q5 / Q6 ANSI
  qmk compile -kb keychron/q3/ansi_encoder -km via
  qmk compile -kb keychron/q5/ansi_encoder -km via
  qmk compile -kb keychron/q6/ansi_encoder -km via

  # Keychron V1 / V3 / V6 ANSI
  qmk compile -kb keychron/v1/ansi_encoder -km via
  qmk compile -kb keychron/v3/ansi_encoder -km via
  qmk compile -kb keychron/v6/ansi_encoder -km via

  # Keychron C1 Pro / C2 Pro
  qmk compile -kb keychron/c1_pro/ansi_white -km via
  qmk compile -kb keychron/c2_pro/ansi_rgb -km via
  ```

---

### 4. Generic VIA / QMK Keyboards (Universal Fallback)
- **HAL Module**: `users/luxqmk/boards/generic.c` & `generic.h`
- **Hardware Specializations**:
  - Universal fallback driver for any QMK/VIA-compatible keyboard equipped with RGB Matrix or backlighting.
  - Automatically activates the complete LuxQMK engine (Dual-Layer Reactive, CIE1931 Gradients, Per-Key RGB, Direct WebHID streaming, custom EEPROM blocks, Debounce tuning, Full NKRO).
- **Compilation Command**:
  ```bash
  qmk compile -kb <vendor>/<keyboard_name> -km via
  ```
