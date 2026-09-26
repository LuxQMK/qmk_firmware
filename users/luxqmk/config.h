#pragma once

/**
 * LuxQMK - Userspace Configuration
 */

// Enable matrix keypress coordinate tracking for advanced reactive lighting effects
#define RGB_MATRIX_KEYPRESSES

// Enable custom user-defined RGB Matrix animations
#define RGB_MATRIX_CUSTOM_USER

// Allocate dedicated EEPROM storage space for VIA / LuxQMK Studio custom configuration (1408 bytes: 104B general + 3x432B per-key RGB profiles)
#define VIA_EEPROM_CUSTOM_CONFIG_SIZE 1408

// Expand wear leveling SPI Flash allocation to 16KB backing (8KB logical EEPROM) to accommodate 1408B custom EEPROM + dynamic keymap & macros
#if defined(WEAR_LEVELING_BACKING_SIZE)
#    undef WEAR_LEVELING_BACKING_SIZE
#endif
#define WEAR_LEVELING_BACKING_SIZE 16384

#if defined(WEAR_LEVELING_LOGICAL_SIZE)
#    undef WEAR_LEVELING_LOGICAL_SIZE
#endif
#define WEAR_LEVELING_LOGICAL_SIZE 8192

// Full N-Key Rollover (NKRO) supported with persistent runtime toggle via WebHID / Studio

// Default contact debounce latency in milliseconds (filters mechanical switch chatter)
#ifndef DEBOUNCE
#    define DEBOUNCE 5
#endif
