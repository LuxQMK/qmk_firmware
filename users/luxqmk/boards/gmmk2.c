#include "gmmk2.h"
#include "color.h"

#ifndef GMMK2_WIN_LED_INDEX
#    define GMMK2_WIN_LED_INDEX 84
#endif

void gmmk2_board_init(void) {
    // Board-specific hardware initialization for GMMK 2
}

uint8_t gmmk2_board_get_logo_led_index(void) {
    // GMMK 2 does not have a dedicated logo badge LED
    return NO_LED;
}

uint8_t gmmk2_board_get_win_led_index(void) {
    return GMMK2_WIN_LED_INDEX;
}

void gmmk2_board_indicators_render(void) {
#ifdef RGB_MATRIX_ENABLE
    // Render Windows Key Lock (Win Lock) indicator on Win key LED
    uint8_t win_idx = gmmk2_board_get_win_led_index();
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
            RGB col = hsv_to_rgb_nocie(hsv);
            rgb_matrix_set_color(win_idx, col.r, col.g, col.b);
        }
    }
#endif
}

// Map board interface to GMMK 2 implementation
void board_init(void) {
    gmmk2_board_init();
}

void board_indicators_render(void) {
    gmmk2_board_indicators_render();
}

uint8_t board_get_logo_led_index(void) {
    return gmmk2_board_get_logo_led_index();
}

uint8_t board_get_win_led_index(void) {
    return gmmk2_board_get_win_led_index();
}
