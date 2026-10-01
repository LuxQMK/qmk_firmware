# LuxQMK Firmware Changelog

All notable changes to the **LuxQMK Firmware Userspace Engine** will be documented in this file.

---

## [0.3.6] - 2026-10-01

### Added
- **Decoupled Layer Background Dimming**: Dedicated brightness dimming controls (`g_layer_dim_enable`, `g_layer_dim_levels[4]`) independent of layer key highlight colors.
- **macOS Base Transparency**: Layer 2 (macOS Base) has dimming and key highlighting disabled by default (`0x0B` bitmask) for seamless ambient backlighting.
- **Win Lock Matrix LED Integration**: Added hardware Win Lock LED indicator override for supported keyboards (e.g., GMMK 3 index 92).

### Changed
- **4-Layer EEPROM Architecture**: Expanded custom EEPROM partition (1408 bytes) for authentic 4-layer dynamic highlight and dim level persistence.
- **Dynamic Lock Indicator State Synchronization**: Extended lock indicator combination support for Caps Lock, Num Lock, Scroll Lock, and Win Lock with real-time HSV quantization.

### Fixed
- **RGB Matrix Dual-Layer Alpha Compositing**: Corrected alpha blend arithmetic overflow and improved color fidelity on transparent key matrix regions.

---

## [0.3.5] - 2026-09-28

### Added
- **Multi-Stop Gradient Pipeline**: Integrated 10 multi-stop gradient presets with CIE1931 lightness curve correction (`luxqmk_gradients.c`).
- **Spatial Density & Direction Controls**: New keycodes `RGB_DEN_INC`, `RGB_DEN_DEC`, `RGB_DEN_STEP`, `RGB_DEN_RST`, and `RGB_REV` for real-time spatial gradient wavelength scaling and direction reversal.
- **Reactive Keystroke Heatmap Shader**: Real-time typing cadence heatmap algorithm (`REACTIVE_MODE_HEATMAP`) with customizable decay duration.

### Changed
- **WebHID Packet Dispatcher**: Accelerated custom EEPROM block transfer throughput for live lighting updates.

---

## [0.3.4] - 2026-09-22

### Added
- **3 Dedicated Per-Key Gaming Profiles**: Pre-programmed hardware EEPROM storage for FPS, MOBA, and MMO/RPG lighting maps (`0x70` - `0x57F`, 1296 bytes).
- **Dedicated Sidelight Lighting Driver**: 13 independent sidelight animation modes (`g_sidelight_mode`) with custom hue, saturation, value, speed, and direction reverse.
- **Configurable Debounce Engine**: Runtime selection between Asymmetric Eager (0ms press latency) and Symmetric Defer switch filtering.

### Fixed
- **Bootloader Entry Stability**: Resolved WB32 / STM32 soft reset timing glitches when triggering bootloader jump from WebHID.

---

## [0.3.0] - 2026-09-15

### Added
- **Dual-Layer Compositing Engine**: Layer 0 Base Ambient Matrix (41+ effects) + Layer 1 Keystroke Reactive Overlay (Fade, Splash, Splash Rainbow, Cross, Nexus, Wide).
- **Permanent Boot NKRO Enforcement**: High-speed 1000Hz (1ms) USB polling with forced NKRO upon MCU boot.
- **Modular Hardware Abstraction Layer (HAL)**: Hardware drivers for GMMK 3 (100%, 75%, 65% ANSI/ISO), GMMK 2 (96%, 65% ANSI/ISO), and Keychron / universal QMK boards.
