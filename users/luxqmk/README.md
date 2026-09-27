# LuxQMK Userspace Engine (v0.3.1)

Dedicated QMK Userspace architecture for **LuxQMK**, providing native out-of-the-box VIA support (`VIA_ENABLE = yes`), modular hardware abstraction, dual-layer reactive RGB matrix lighting, custom VIA/WebHID channels, multi-stop gradient sampling, real-time configurable debouncing, forced boot NKRO, and atomic direct lighting double-buffering.

---

## 🌟 Architecture & Capabilities

```text
users/luxqmk/
├── luxqmk.h               # Central protocol definitions, custom keycodes, structs & EEPROM layout
├── luxqmk.c               # Core runtime: EEPROM, WebHID dispatch, NKRO, reactive overlay, debounce
├── config.h               # Performance tuning (FORCE_NKRO, DEBOUNCE, VIA_EEPROM_CUSTOM_CONFIG_SIZE 1408)
├── rules.mk               # Build rules and automated hardware module selection
├── rgb_matrix_user.inc    # Custom RGB matrix effect registrations
├── rgb/
│   └── custom_effects.h   # Multi-stop gradient shaders & reactive blend math
└── boards/                # Hardware Abstraction Layer (HAL)
    ├── gmmk3.c / .h       # GMMK 3 (100%, 75%, 65% ANSI/ISO), Logo badge, Win Lock LED (Index 92)
    ├── gmmk2.c / .h       # GMMK 2 (96%, 65% ANSI/ISO), side lighting strips
    └── generic.c / .h     # Standard fallback driver for universal QMK / VIA keyboards
```

---

## 🎨 Dual-Layer Reactive Lighting & Hardware Architecture

LuxQMK introduces a hardware-accelerated **Dual-Layer Compositing Engine**:
1. **Layer 0 (Base Ambient Matrix)**: Renders 41+ hardware animations (*LuxQMK Wave*, *Dynamic Cycle*, *Rainbow Chevron*, *Breathing*, *Starlight*) with 10 multi-stop gradient presets and spatial density scaling (32..255).
2. **Layer 1 (Keystroke Reactive Overlay)**: Evaluates switch actuation timestamps in real-time across 7 reactive algorithms:
   - `REACTIVE_MODE_FADE` (1): Fading trail per actuated key (200–2000ms decay).
   - `REACTIVE_MODE_SPLASH` (2): Expanding circular single-color ripple.
   - `REACTIVE_MODE_SPLASH_RAINBOW` (3): Expanding radial rainbow shockwave.
   - `REACTIVE_MODE_CROSS` (4): Orthogonal (+) laser beam expansion.
   - `REACTIVE_MODE_NEXUS` (5): Geometric diagonal (X) star shockwave.
   - `REACTIVE_MODE_WIDE` (6): Thick high-intensity radial energy wave.
   - `REACTIVE_MODE_HEATMAP` (7): Thermal typing cadence color mapping.
3. **Blending Pipeline**: Supports **Additive Glow** (`qadd8(bg, react)`) and **Alpha Blend Override** (`out = (react*α + bg*(255-α))/255`).
4. **Hardware Sidelight Isolation (13 Modes)**: Dedicated side diffuser strips (`g_sidelight_mode`) operate with independent Hue, Saturation, Brightness, Speed, Gradient preset, and Direction Reverse.
5. **Dedicated Logo / Badge LED**: Supports 3 modes and **7 distinct lock state combinations** (Caps Lock, Num Lock, Caps+Num, Scroll Lock, Caps+Scroll, Num+Scroll, and Caps+Num+Scroll) for keyboards equipped with a dedicated logo or status LED (such as GMMK 3 magnetic badge or other hardware logo diffusers via `LUXQMK_CAP_LOGO_LED`).
6. **Win Lock Indicator**: Special override for Windows Key lock indicator (LED index 92 on GMMK 3).
7. **Active Layer Dimming (`g_layer_dim_level`)**: Highlights active key bindings on layers 1–3 while dimming transparent keys (`KC_TRNS`/`KC_NO`) from 0% (black) to 100% (ambient).
8. **3 Per-Key Gaming Profiles**: Pre-programmed FPS, MOBA, and MMO/RPG presets stored in hardware EEPROM (1296 bytes).

---

## ⚡ Performance, Debounce & NKRO

- **Customizable Debounce Engine**: 3 algorithms configurable via WebHID and stored in EEPROM (`USER_VAL_DEBOUNCE_TIME`):
  - **Asymmetric Eager (Eager PR)**: **0ms press latency** (instantaneous interrupt report on switch contact close, debounce filtering on release).
  - **Symmetric Defer (QMK Default / Safe)**: Official standard upstream QMK algorithm (`sym_defer_pk`). Full debounce window filtering before actuation/release for maximum mechanical switch chatter immunity.
  - **Symmetric Eager**: Eager actuation on both press and release.
  - **Latency Windows**: 0ms (Hall-Effect/Optical), 2ms, 5ms, 8ms, 16ms.
- **Permanent Boot NKRO**: `FORCE_NKRO` and `keymap_config.nkro = 1` enforced upon MCU boot with 1000Hz (1ms) USB polling rate.
- **Dedicated EEPROM Partition (1408 Bytes)**:
  - `0x00 - 0x6F` (112 Bytes): System configuration header (Reverse, Layer lighting, Dim level, Layer 1-3 colors, Logo modes/colors, Win lock, Reactive mode/blend, Debounce time, Gradient presets, Sidelights).
  - `0x70 - 0x57F` (1296 Bytes): 3 Per-Key Gaming Profiles &times; 144 LEDs &times; 3 Bytes (RGB).

---

## 🔑 Custom Keycodes (Contiguous Range: 0x7E00 - 0x7E09 / QK_KB_0 - QK_KB_9)

| Keycode | Constant | Hex Code | Dec Code | Operational Behavior |
| :--- | :--- | :--- | :--- | :--- |
| `RGB_REV` | `QK_KB_0` (`QK_USER_0`) | `0x7E00` | `32256` | Toggle Reverse RGB Animation Direction |
| `RGB_DEN_INC` | `QK_KB_1` (`QK_USER_1`) | `0x7E01` | `32257` | Increase spatial effect density (+16, max 255) |
| `RGB_DEN_DEC` | `QK_KB_2` (`QK_USER_2`) | `0x7E02` | `32258` | Decrease spatial effect density (-16, min 32) |
| `RGB_DEN_STEP`| `QK_KB_3` (`QK_USER_3`) | `0x7E03` | `32259` | Cycle Density Steps (64 -> 96 -> 128 -> ... -> 255 -> 64) |
| `RGB_DEN_RST` | `QK_KB_4` (`QK_USER_4`) | `0x7E04` | `32260` | Reset Density to baseline (1.0x / 128) |
| `RGB_GRAD_STEP`| `QK_KB_5` (`QK_USER_5`) | `0x7E05` | `32261` | Cycle Active Multi-Stop Gradient Preset (0..9) |
| `RGB_REACT_STEP`| `QK_KB_6` (`QK_USER_6`) | `0x7E06` | `32262` | Cycle Reactive Overlay Mode (Fade -> Splash -> Cross -> Nexus -> Wide -> Heatmap -> Off) |
| `RGB_RSPD_INC` | `QK_KB_7` (`QK_USER_7`) | `0x7E07` | `32263` | Reactive Speed + (shorter fade trail / faster response, +16) |
| `RGB_RSPD_DEC` | `QK_KB_8` (`QK_USER_8`) | `0x7E08` | `32264` | Reactive Speed - (longer fade trail / slower response, -16) |
| `RGB_RSPD_STEP`| `QK_KB_9` (`QK_USER_9`) | `0x7E09` | `32265` | Cycle Reactive Speed Step (32 -> 64 -> 96 -> 127 -> 32) |
| `M0` – `M15` | `QK_MACRO_0..15` | `0x7700..0x770F` | `30464..30479` | Autonomous hardware macro execution from EEPROM |

---

## 🛠️ Compilation Examples

```bash
# GMMK 3 100% ANSI via
qmk compile -kb gmmk/gmmk3/p100/ansi -km via

# GMMK 3 75% ANSI via
qmk compile -kb gmmk/gmmk3/p75/ansi -km via

# GMMK 3 65% ANSI via
qmk compile -kb gmmk/gmmk3/p65/ansi -km via

# GMMK 2 96% ANSI via
qmk compile -kb gmmk/gmmk2/p96/ansi -km via

# GMMK 2 65% ANSI via
qmk compile -kb gmmk/gmmk2/p65/ansi -km via
```
