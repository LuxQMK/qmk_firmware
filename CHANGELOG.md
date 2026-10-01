# LuxQMK Firmware Changelog



## [0.3.6] - Unreleased / In Development

### Added
- **Unified Changelog Pipeline**: Automated markdown parsing in `generate_catalog.py` to inject structured release notes directly into `catalog.json` and CDN distribution manifests.

### Changed
- **Multi-Cloud Compilation Optimization**: Set default parallel compilation pool to 100% free `hybrid` mode (1x 4-thread OCI ARM + 20x 2-thread GitHub Actions runners across 21 shards).
- **CI/CD Resilience**: Enhanced matrix error handling so partial compilation failures no longer cancel catalog generation or CDN deployments.

### Fixed
- **Cloud-Init Ubuntu 24.04 Compatibility**: Resolved missing `liblttng-ust1` dependencies on modern Linux distributions by delegating runner bootstrap to official scripts.
- **CDN Catalog Deployment Safety**: Added multiple strict fail-safe validation barriers preventing empty `catalog.json` distributions from overwriting production endpoints.

---

## [0.3.5] - 2026-10-01

### Added
- **Decoupled Layer Lighting & Dimming**: Introduced independent background dimming controls separate from active key highlight colors.
- **Custom Per-Layer Dimming Levels**: Configurable dimming intensity per layer stored in persistent EEPROM.
- **Mac Layout Support for GMMK 3**: Pre-populated Mac Base (Layer 2) and Mac Fn (Layer 3) across all GMMK 3 VIA keymaps.
- **Layer Bitmask Support**: Ability to ignore designated base layers (such as Mac mode) during layer highlight overlays.

### Changed
- **Modular Userspace Architecture**: Refactored `users/luxqmk/luxqmk.c` into modular submodules (`luxqmk_eeprom.c`, `luxqmk_protocol.c`, `luxqmk_reactive.c`, `luxqmk_gradients.c`).
- **Enhanced Keychron Catalog Tier**: Elevated Keychron V & Q series keyboards to `luxqmk_enhanced` tier with optimized matrix descriptors.

### Fixed
- **Combined Active Layer Reporting**: Fixed `USER_VAL_ACTIVE_LAYER` raw HID query to report combined `layer_state` and `default_layer_state`.
- **Keychron Target Definitions**: Corrected Keychron build targets to use `default` keymap with userspace hooks.

---

## [0.3.4] - 2026-09-29

### Added
- **Dynamic Speed & Density Keycodes**: Added `RGB_DEN_INC`, `RGB_DEN_DEC`, `RGB_DEN_STEP`, `RGB_RSPD_INC`, `RGB_RSPD_DEC`, and `RGB_RSPD_STEP` for direct on-the-fly animation tuning.
- **Reactive Keystroke Heatmap Engine**: Hardware-computed heatmap reactive effect with decay timers.
- **Dynamic Matrix Catalog Generator**: Python-based matrix sharder supporting up to 22 concurrent runners with hardware thread weighting.

### Changed
- **CIE1931 Perceptual Color Math**: Enhanced RGB matrix rendering pipeline with non-linear luminance correction.
- **Debounce Optimizations**: Fine-tuned debounce algorithms for 1000 Hz gaming response while preventing chatter.

### Fixed
- **GMMK 3 Logo LED Synchronization**: Fixed state desynchronization between main matrix animations and RGB badge indicators.

---

## [0.3.3] - 2026-09-25

### Added
- **Dual-Layer Reactive RGB Engine**: Hardware blending of ambient matrix animations with reactive keystroke overlays (Fade, Heatmap, Wide Wave, Ripple).
- **Custom VIA EEPROM Blocks**: Expanded custom memory space up to 1408 bytes for RGB profiles and multi-stop gradient tables.
- **Boot-up NKRO Enforcement**: Guaranteed N-Key Rollover enforcement on USB initialization.

### Changed
- **WB32F3G71xx Driver Optimization**: Tuned DMA PWM transfers for Glorious GMMK 3 and GMMK 2 series.

---

## [0.3.0] - 2026-09-15

### Added
- **Initial LuxQMK Firmware Architecture**: Dedicated `users/luxqmk/` modular ecosystem.
- **Hardware Abstraction Layer (HAL)**: Initial drivers for GMMK 3 (100%, 75%, 65%), GMMK 2 (96%, 65%), and generic VIA boards.
- **WebHID Raw Protocol**: Custom 32-byte packet dispatch for real-time bidirectional communication with LuxQMK Studio.
