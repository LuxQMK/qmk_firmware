#include "gmmk3.h"
#include "drivers/led/aw20216s.h"
#include "color.h"

#ifndef GMMK3_WIN_LED_INDEX
#    define GMMK3_WIN_LED_INDEX 92
#endif

void gmmk3_board_init(void) {
    // Board-specific hardware initialization for GMMK 3
}

uint8_t gmmk3_board_get_logo_led_index(void) {
#if defined(DRIVER_LED_TOTAL) && (DRIVER_LED_TOTAL > 0)
    // In GMMK 3 architecture (100%, 75%, 65%), the Logo Badge LED is always the last LED index
    return DRIVER_LED_TOTAL - 1;
#else
    return NO_LED;
#endif
}

uint8_t gmmk3_board_get_win_led_index(void) {
    return GMMK3_WIN_LED_INDEX;
}

void gmmk3_board_indicators_render(void) {
#ifdef RGB_MATRIX_ENABLE
    // 1. Render Glorious Logo Badge LED indicator (beside rotary encoder)
    uint8_t logo_idx = gmmk3_board_get_logo_led_index();
    if (g_logo_mode != LOGO_MODE_RGB && logo_idx < DRIVER_LED_TOTAL) {
        led_t host_leds = host_keyboard_led_state();
        uint8_t lock_state = (host_leds.caps_lock ? 1 : 0) |
                             (host_leds.num_lock ? 2 : 0) |
                             (host_leds.scroll_lock ? 4 : 0);

        if (lock_state > 0) {
            uint8_t val = rgb_matrix_get_val();
            if (val == 0) {
                val = 255;
            }
            HSV hsv = {
                g_logo_lock_colors[lock_state].h,
                g_logo_lock_colors[lock_state].s,
                val
            };
            RGB col = hsv_to_rgb(hsv);
            rgb_matrix_set_color(logo_idx, col.r, col.g, col.b);
        } else if (g_logo_mode == LOGO_MODE_INDICATOR_OFF_IDLE) {
            rgb_matrix_set_color(logo_idx, 0, 0, 0);
        }
    }

    // 2. Render Windows Key Lock (Win Lock) indicator on Win key LED
    uint8_t win_idx = gmmk3_board_get_win_led_index();
    if (keymap_config.no_gui && g_win_lock_mode != WIN_LOCK_MODE_ANIMATION && win_idx < DRIVER_LED_TOTAL) {
        if (g_win_lock_mode == WIN_LOCK_MODE_OFF) {
            rgb_matrix_set_color(win_idx, 0, 0, 0);
        } else if (g_win_lock_mode == WIN_LOCK_MODE_COLOR) {
            uint8_t val = rgb_matrix_get_val();
            if (val == 0) {
                val = 255;
            }
            HSV hsv = {
                g_win_lock_color.h,
                g_win_lock_color.s,
                val
            };
            RGB col = hsv_to_rgb(hsv);
            rgb_matrix_set_color(win_idx, col.r, col.g, col.b);
        }
    }
#endif
}

// Map board interface to GMMK 3 implementation
void board_init(void) {
    gmmk3_board_init();
}

void board_indicators_render(void) {
    gmmk3_board_indicators_render();
}

uint8_t board_get_logo_led_index(void) {
    return gmmk3_board_get_logo_led_index();
}

uint8_t board_get_win_led_index(void) {
    return gmmk3_board_get_win_led_index();
}
