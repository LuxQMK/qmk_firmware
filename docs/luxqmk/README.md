# LuxQMK Firmware Documentation Hub

Welcome to the dedicated documentation directory for **LuxQMK Firmware**.

---

## 📚 Documentation Index

1. **[Supported Keyboards & Hardware Verification Status](supported_keyboards.md)**
   - Matrix compatibility table for Glorious GMMK 3, GMMK 2, Keychron Wired (Q/V/C/K Pro), and Generic QMK/VIA.
   - Board-specific Hardware Abstraction Layer (HAL) architecture.
   - Verified compilation commands per layout and hardware variant.

2. **[LuxQMK Userspace Engine Specification](../../users/luxqmk/README.md)**
   - Architecture breakdown of modular submodules (`luxqmk_reactive.c`, `luxqmk_gradients.c`, `luxqmk_eeprom.c`, `luxqmk_protocol.c`).
   - Dual-layer reactive RGB matrix rendering pipeline.
   - Decoupled per-layer background dimming and macOS transparency rules.
   - EEPROM memory block layout (1408 bytes) and custom keycode reference (`0x7E00` – `0x7E09`).

3. **[Firmware Release Notes & Changelog](../../users/luxqmk/CHANGELOG.md)**
   - Version history across all stable releases (v0.3.5 baseline down to v0.3.0).

---

## 🌐 External Resources & Companion Suite

- **Official Web Portal**: [luxqmk.click](https://luxqmk.click)
- **WebHID Companion Studio**: [studio.luxqmk.click](https://studio.luxqmk.click)
- **LuxQMK Studio Repository**: [github.com/LuxQMK/luxqmk_studio](https://github.com/LuxQMK/luxqmk_studio)
- **Upstream QMK Documentation**: [docs.qmk.fm](https://docs.qmk.fm/)
