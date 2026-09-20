#pragma once

/**
 * LuxQMK - Userspace Configuration
 */

// Enable matrix keypress coordinate tracking for advanced reactive lighting effects
#define RGB_MATRIX_KEYPRESSES

// Enable custom user-defined RGB Matrix animations
#define RGB_MATRIX_CUSTOM_USER

// Allocate dedicated EEPROM storage space for VIA / LuxQMK Studio custom configuration (104 bytes)
#define VIA_EEPROM_CUSTOM_CONFIG_SIZE 104

// Enforce Full N-Key Rollover (NKRO) by default upon keyboard startup
#define FORCE_NKRO

// Default contact debounce latency in milliseconds (filters mechanical switch chatter)
#ifndef DEBOUNCE
#    define DEBOUNCE 5
#endif
