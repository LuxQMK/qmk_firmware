# LuxQMK Firmware Changelog

All notable changes to the **LuxQMK Firmware Userspace Engine** are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [0.3.5] - 2026-10-01

### Added
- **Decoupled Layer Background Dimming**: Dedicated brightness dimming controls (`g_layer_dim_enable`, `g_layer_dim_levels[4]`) independent of layer key highlight colors.
- **macOS Base Transparency**: Layer 2 (macOS Base) has dimming and key highlighting disabled by default (`0x0B` bitmask) for seamless ambient backlighting.
- **Modular Architecture Refactoring**: Refactored core userspace engine into clean, modular C submodules (`luxqmk_reactive.c`, `luxqmk_gradients.c`, `luxqmk_eeprom.c`, `luxqmk_protocol.c`).
- **GMMK 3 & Mac Layouts**: Populated Mac Base (Layer 2) and Mac Fn (Layer 3) in VIA keymaps for GMMK 3 ANSI/ISO.

### Changed
- **Dynamic Layer State Handler**: Return combined `layer_state` and `default_layer_state` in `USER_VAL_ACTIVE_LAYER` over WebHID protocol.
- **4-Layer Authentic Display**: Aligned layer lighting enable bitmasks to support live 4-layer EEPROM highlight and dimming persistence.

---

## [0.3.4] - 2026-09-28

### Added
- **Custom Lock LED Indicators**: Real-time configurable Lock LED indicators (Caps, Num, Scroll, Win Lock) with Keychron and GMMK 3 board support.

### Fixed
- **EEPROM Stack Buffer Guard**: Expanded `luxqmk_eeprom_save` header buffer to 160 bytes to prevent stack overflow during DIP switch persistence.
- **DIP Switch Board Handler**: Prevented board-level handler override on Keychron hardware DIP switches.

---

## [0.3.3] - 2026-09-25

### Added
- **Hardware DIP Switches**: Configurable hardware DIP switches (Mac/Win 2-position & multi-position) in EEPROM & WebHID protocol.

### Fixed
- **Inverted Switch Mapping**: Aligned physical switch position defaults (pos0 = Mac/Left, pos1 = Win/Right).

---

## [0.3.2] - 2026-09-22

### Added
- **Universal VIA Compilation**: Native `VIA_ENABLE = yes` enabled across all keyboards compiled with LuxQMK userspace.
- **Dynamic Hardware Feature Extraction**: Added VIA support tags and dynamic hardware capabilities extraction in catalog generator.

### Fixed
- **ISO Keymap Layout Macros**: Corrected keymap layout macros for GMMK 2 (65% & 96%) and GMMK 3 ISO models.

---

## [0.3.1] - 2026-09-18

### Added
- **Runtime NKRO Toggle**: Added runtime NKRO state toggle over WebHID protocol alongside permanent boot NKRO.

---

## [0.3.0] - 2026-09-15

### Added
- **Dual-Layer Compositing Engine**: Layer 0 Base Ambient Matrix (41+ effects) + Layer 1 Keystroke Reactive Overlay (Fade, Splash, Splash Rainbow, Cross, Nexus, Wide, Heatmap).
- **Multi-Stop Gradient Engine**: Custom gradient sampling with CIE1931 lightness curve correction (`luxqmk_gradients.c`).
- **Modular Hardware Abstraction Layer (HAL)**: Hardware abstraction for GMMK 3, GMMK 2, Keychron, and Generic QMK boards.
- **Direct Software Streaming**: Atomic double-buffering for live 60 FPS studio lighting.
- **Permanent Boot NKRO Enforcement**: High-speed 1000Hz (1ms) USB polling with forced NKRO upon MCU boot.
