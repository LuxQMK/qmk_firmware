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

void gmmk3_board_calc_sidelight_coords(uint8_t led_idx, uint8_t ug_first, uint8_t ug_count, uint8_t density,
                                       uint8_t *y_scaled, uint8_t *dist_scaled, uint8_t *opt_step, bool *is_hidden) {
    uint8_t half_count = (ug_count > 0) ? (ug_count / 2) : 1;
    *is_hidden = false;

    if (led_idx < ug_first + half_count) {
        // LEFT STRIP: Visible optical window is segments 1..8 (7 steps, center at 4.5)
        uint8_t k_left = led_idx - ug_first; // 0..9
        if (k_left < 1 || k_left > 8) {
            *is_hidden = true;
        }
        int16_t norm_left = (int16_t)k_left - 1; // 0 at segment 1, 7 at segment 8
        if (norm_left < 0) norm_left = 0;
        if (norm_left > 7) norm_left = 7;
        *opt_step = (uint8_t)norm_left;
        *y_scaled = (uint8_t)(((uint32_t)norm_left * 255 * density) / (7 * 128));

        uint8_t dist_sym = (uint8_t)abs((int16_t)(2 * k_left) - 9); // distance from center (4.5)
        if (dist_sym > 7) dist_sym = 7;
        *dist_scaled = (uint8_t)(((uint32_t)dist_sym * 255 * density) / (7 * 128));
    } else {
        // RIGHT STRIP: Visible optical window is segments 2..9 (7 steps, center at 5.5)
        uint8_t k_right = (ug_first + ug_count - 1) - led_idx; // 0..9
        if (k_right < 2 || k_right > 9) {
            *is_hidden = true;
        }
        int16_t norm_right = (int16_t)k_right - 2; // 0 at segment 2, 7 at segment 9
        if (norm_right < 0) norm_right = 0;
        if (norm_right > 7) norm_right = 7;
        *opt_step = (uint8_t)norm_right;
        *y_scaled = (uint8_t)(((uint32_t)norm_right * 255 * density) / (7 * 128));

        uint8_t dist_sym = (uint8_t)abs((int16_t)(2 * k_right) - 11); // distance from center (5.5)
        if (dist_sym > 7) dist_sym = 7;
        *dist_scaled = (uint8_t)(((uint32_t)dist_sym * 255 * density) / (7 * 128));
    }
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

void board_calc_sidelight_coords(uint8_t led_idx, uint8_t ug_first, uint8_t ug_count, uint8_t density,
                                 uint8_t *y_scaled, uint8_t *dist_scaled, uint8_t *opt_step, bool *is_hidden) {
    gmmk3_board_calc_sidelight_coords(led_idx, ug_first, ug_count, density, y_scaled, dist_scaled, opt_step, is_hidden);
}

