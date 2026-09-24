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
            RGB col = hsv_to_rgb(hsv);
            rgb_matrix_set_color(win_idx, col.r, col.g, col.b);
        }
    }
#endif
}

void gmmk2_board_calc_sidelight_coords(uint8_t led_idx, uint8_t ug_first, uint8_t ug_count, uint8_t density,
                                       uint8_t *y_scaled, uint8_t *dist_scaled, uint8_t *opt_step, bool *is_hidden) {
    uint8_t half_count = (ug_count > 0) ? (ug_count / 2) : 1;
    uint8_t span = (half_count > 1) ? (half_count - 1) : 1;
    *is_hidden = false;

    uint8_t k = (led_idx < ug_first + half_count)
                    ? (led_idx - ug_first)
                    : ((ug_first + ug_count - 1) - led_idx);
    if (k > span) k = span;
    *opt_step = k;
    *y_scaled = (uint8_t)(((uint32_t)k * 255 * density) / (span * 128));

    uint8_t dist_sym = (uint8_t)abs((int16_t)(2 * k) - (int16_t)span);
    if (dist_sym > span) dist_sym = span;
    *dist_scaled = (uint8_t)(((uint32_t)dist_sym * 255 * density) / (span * 128));
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

void board_calc_sidelight_coords(uint8_t led_idx, uint8_t ug_first, uint8_t ug_count, uint8_t density,
                                 uint8_t *y_scaled, uint8_t *dist_scaled, uint8_t *opt_step, bool *is_hidden) {
    gmmk2_board_calc_sidelight_coords(led_idx, ug_first, ug_count, density, y_scaled, dist_scaled, opt_step, is_hidden);
}

