#pragma once

/**
 * LuxQMK - Userspace Configuration
 */

// Enable matrix keypress coordinate tracking for advanced reactive lighting effects
#if defined(RGB_MATRIX_ENABLE)
#    define RGB_MATRIX_KEYPRESSES
#    define RGB_MATRIX_CUSTOM_USER
#endif

// Allocate dedicated EEPROM storage space for VIA / LuxQMK Studio custom configuration
#if defined(__AVR__)
#    define VIA_EEPROM_CUSTOM_CONFIG_SIZE 32
#else
#    define VIA_EEPROM_CUSTOM_CONFIG_SIZE 1536
#endif

// Expand wear leveling SPI Flash allocation on ARM to accommodate custom EEPROM + dynamic keymap & macros
#if !defined(__AVR__)
#    if defined(WEAR_LEVELING_BACKING_SIZE)
#        undef WEAR_LEVELING_BACKING_SIZE
#    endif
#    define WEAR_LEVELING_BACKING_SIZE 16384

#    if defined(WEAR_LEVELING_LOGICAL_SIZE)
#        undef WEAR_LEVELING_LOGICAL_SIZE
#    endif
#    define WEAR_LEVELING_LOGICAL_SIZE 8192
#endif

// Default contact debounce latency in milliseconds (filters mechanical switch chatter)
#ifndef DEBOUNCE
#    define DEBOUNCE 5
#endif
