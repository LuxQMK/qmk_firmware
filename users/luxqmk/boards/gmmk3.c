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
#if defined(DRIVER_LED_TOTAL)
    #if DRIVER_LED_TOTAL >= 115
        return 92; // GMMK 3 100% ANSI / ISO
    #elif DRIVER_LED_TOTAL >= 95
        return 72; // GMMK 3 75% ANSI / ISO
    #elif DRIVER_LED_TOTAL >= 80
        return 57; // GMMK 3 65% ANSI / ISO
    #else
        return 92;
    #endif
#else
    return 92;
#endif
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

    // 3. Render Status Lock Indicators (Caps Lock, Num Lock, Scroll Lock) if defined
    led_t host_leds_status = host_keyboard_led_state();
    uint8_t caps_idx = board_get_caps_led_index();
    if (caps_idx != NO_LED && caps_idx < DRIVER_LED_TOTAL && host_leds_status.caps_lock) {
        if (g_caps_lock_mode == LOCK_INDICATOR_MODE_OFF) {
            rgb_matrix_set_color(caps_idx, 0, 0, 0);
        } else if (g_caps_lock_mode == LOCK_INDICATOR_MODE_COLOR) {
            uint8_t val = rgb_matrix_get_val();
            if (val == 0) val = 255;
            HSV hsv = { g_caps_lock_color.h, g_caps_lock_color.s, val };
            RGB rgb = hsv_to_rgb(hsv);
            rgb_matrix_set_color(caps_idx, rgb.r, rgb.g, rgb.b);
        }
    }

    uint8_t num_idx = board_get_num_led_index();
    if (num_idx != NO_LED && num_idx < DRIVER_LED_TOTAL && host_leds_status.num_lock) {
        if (g_num_lock_mode == LOCK_INDICATOR_MODE_OFF) {
            rgb_matrix_set_color(num_idx, 0, 0, 0);
        } else if (g_num_lock_mode == LOCK_INDICATOR_MODE_COLOR) {
            uint8_t val = rgb_matrix_get_val();
            if (val == 0) val = 255;
            HSV hsv = { g_num_lock_color.h, g_num_lock_color.s, val };
            RGB rgb = hsv_to_rgb(hsv);
            rgb_matrix_set_color(num_idx, rgb.r, rgb.g, rgb.b);
        }
    }

    uint8_t scroll_idx = board_get_scroll_led_index();
    if (scroll_idx != NO_LED && scroll_idx < DRIVER_LED_TOTAL && host_leds_status.scroll_lock) {
        if (g_scroll_lock_mode == LOCK_INDICATOR_MODE_OFF) {
            rgb_matrix_set_color(scroll_idx, 0, 0, 0);
        } else if (g_scroll_lock_mode == LOCK_INDICATOR_MODE_COLOR) {
            uint8_t val = rgb_matrix_get_val();
            if (val == 0) val = 255;
            HSV hsv = { g_scroll_lock_color.h, g_scroll_lock_color.s, val };
            RGB rgb = hsv_to_rgb(hsv);
            rgb_matrix_set_color(scroll_idx, rgb.r, rgb.g, rgb.b);
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

uint8_t board_get_caps_led_index(void) {
#if defined(CAPS_LOCK_LED_INDEX)
    return CAPS_LOCK_LED_INDEX;
#elif defined(CAPS_LED_INDEX)
    return CAPS_LED_INDEX;
#elif defined(DRIVER_LED_TOTAL)
    #if DRIVER_LED_TOTAL >= 115
        return 58; // GMMK 3 100% ANSI / ISO
    #elif DRIVER_LED_TOTAL >= 95
        return 43; // GMMK 3 75% ANSI / ISO
    #elif DRIVER_LED_TOTAL >= 80
        return 28; // GMMK 3 65% ANSI / ISO
    #else
        return NO_LED;
    #endif
#else
    return NO_LED;
#endif
}

uint8_t board_get_num_led_index(void) {
#if defined(NUM_LOCK_LED_INDEX)
    return NUM_LOCK_LED_INDEX;
#elif defined(NUM_LED_INDEX)
    return NUM_LED_INDEX;
#elif defined(DRIVER_LED_TOTAL) && (DRIVER_LED_TOTAL >= 115)
    return 33; // GMMK 3 100% ANSI / ISO
#else
    return NO_LED;
#endif
}

uint8_t board_get_scroll_led_index(void) {
#if defined(SCROLL_LOCK_LED_INDEX)
    return SCROLL_LOCK_LED_INDEX;
#elif defined(SCROLL_LED_INDEX)
    return SCROLL_LED_INDEX;
#elif defined(DRIVER_LED_TOTAL) && (DRIVER_LED_TOTAL >= 115)
    return 14; // GMMK 3 100% ANSI / ISO
#else
    return NO_LED;
#endif
}

void board_calc_sidelight_coords(uint8_t led_idx, uint8_t ug_first, uint8_t ug_count, uint8_t density,
                                 uint8_t *y_scaled, uint8_t *dist_scaled, uint8_t *opt_step, bool *is_hidden) {
    gmmk3_board_calc_sidelight_coords(led_idx, ug_first, ug_count, density, y_scaled, dist_scaled, opt_step, is_hidden);
}

