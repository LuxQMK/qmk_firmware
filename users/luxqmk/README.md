# LuxQMK Userspace Engine (v0.3.0)

Dedicated QMK Userspace architecture for **LuxQMK**, providing modular hardware abstraction, dual-layer reactive RGB matrix lighting, custom VIA/WebHID channels, CIE1931 perceptual curves, multi-stop gradient sampling, and atomic direct lighting double-buffering.

---

## 🌟 Architecture & Capabilities

```text
users/luxqmk/
├── luxqmk.h               # Central protocol definitions, custom keycodes, and structs
├── luxqmk.c               # Core runtime: EEPROM, WebHID dispatch, NKRO, reactive overlay
├── config.h               # Performance tuning (NKRO, Debounce, Custom EEPROM size)
├── rules.mk               # Build rules and automated hardware module selection
├── rgb_matrix_user.inc    # RGB matrix effect registrations
├── rgb/
│   └── custom_effects.h   # Multi-stop gradient shaders & reactive blend math
└── boards/                # Hardware Abstraction Layer (HAL)
    ├── gmmk3.c / .h       # GMMK 3 (100%, 75%, 65% ANSI/ISO), Logo badge, Win Lock LED
    ├── gmmk2.c / .h       # GMMK 2 (96%, 65% ANSI/ISO), side lighting strips
    └── generic.c / .h     # Standard fallback driver for universal QMK / VIA keyboards
```

---

## 🔑 Custom Keycodes (QK_KB_0 Range: 0x7E00 - 0x7E09)

| Keycode | Value | Description |
| :--- | :--- | :--- |
| `RGB_REV` | `0x7E00` (32256) | Toggle Reverse RGB Animation Direction |
| `RGB_DEN_INC` | `0x7E01` (32257) | Increase spatial effect density (+16) |
| `RGB_DEN_DEC` | `0x7E02` (32258) | Decrease spatial effect density (-16) |
| `RGB_DEN_STEP` | `0x7E03` (32259) | Cycle Density Steps (64 -> 96 -> 128 -> ... -> 255) |
| `RGB_DEN_RST` | `0x7E04` (32260) | Reset Density to baseline (1.0x / 128) |
| `RGB_GRAD_STEP` | `0x7E05` (32261) | Cycle Active Multi-Stop Gradient Preset (0..9) |
| `RGB_REACT_STEP`| `0x7E06` (32262) | Cycle Reactive Overlay Mode (Fade -> Heatmap -> Off) |
| `RGB_RSPD_INC` | `0x7E07` (32263) | Reactive Speed + (shorter fade trail / faster response) |
| `RGB_RSPD_DEC` | `0x7E08` (32264) | Reactive Speed - (longer fade trail / slower response) |
| `RGB_RSPD_STEP`| `0x7E09` (32265) | Cycle Reactive Speed Step (32 -> 64 -> 96 -> 127) |

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
