#include "keychron.h"
#include "color.h"

void keychron_board_init(void) {
    // Board-specific hardware initialization for Keychron keyboards
}

uint8_t keychron_board_get_logo_led_index(void) {
    return NO_LED;
}

uint8_t keychron_board_get_win_led_index(void) {
    return NO_LED;
}

uint8_t keychron_board_get_caps_led_index(void) {
#if defined(CAPS_LOCK_LED_INDEX)
    return CAPS_LOCK_LED_INDEX;
#elif defined(CAPS_LED_INDEX)
    return CAPS_LED_INDEX;
#else
    return NO_LED;
#endif
}

uint8_t keychron_board_get_num_led_index(void) {
#if defined(NUM_LOCK_LED_INDEX)
    return NUM_LOCK_LED_INDEX;
#elif defined(NUM_LED_INDEX)
    return NUM_LED_INDEX;
#else
    return NO_LED;
#endif
}

uint8_t keychron_board_get_scroll_led_index(void) {
#if defined(SCROLL_LOCK_LED_INDEX)
    return SCROLL_LOCK_LED_INDEX;
#elif defined(SCROLL_LED_INDEX)
    return SCROLL_LED_INDEX;
#else
    return NO_LED;
#endif
}

static inline void keychron_render_single_indicator(uint8_t led_idx, uint8_t mode, layer_color_t color, bool is_active) {
#ifdef RGB_MATRIX_ENABLE
    if (led_idx == NO_LED || led_idx >= DRIVER_LED_TOTAL) return;
    if (is_active) {
        if (mode == LOCK_INDICATOR_MODE_OFF) {
            rgb_matrix_set_color(led_idx, 0, 0, 0);
        } else if (mode == LOCK_INDICATOR_MODE_COLOR) {
            uint8_t val = rgb_matrix_get_val();
            if (val == 0) val = 255;
            HSV hsv = { color.h, color.s, val };
            RGB rgb = hsv_to_rgb(hsv);
            rgb_matrix_set_color(led_idx, rgb.r, rgb.g, rgb.b);
        } else if (mode == LOCK_INDICATOR_MODE_WHITE) {
            rgb_matrix_set_color(led_idx, 255, 255, 255);
        }
    }
#endif
}

void keychron_board_indicators_render(void) {
#ifdef RGB_MATRIX_ENABLE
    led_t host_leds = host_keyboard_led_state();

    uint8_t caps_idx = keychron_board_get_caps_led_index();
    if (caps_idx != NO_LED) {
        keychron_render_single_indicator(caps_idx, g_caps_lock_mode, g_caps_lock_color, host_leds.caps_lock);
    }

    uint8_t num_idx = keychron_board_get_num_led_index();
    if (num_idx != NO_LED) {
        keychron_render_single_indicator(num_idx, g_num_lock_mode, g_num_lock_color, host_leds.num_lock);
    }

    uint8_t scroll_idx = keychron_board_get_scroll_led_index();
    if (scroll_idx != NO_LED) {
        keychron_render_single_indicator(scroll_idx, g_scroll_lock_mode, g_scroll_lock_color, host_leds.scroll_lock);
    }

#    if defined(MAC_LED_INDEX)
    if (default_layer_state == (1 << 0)) {
        rgb_matrix_set_color(MAC_LED_INDEX, 255, 255, 255);
    } else {
        rgb_matrix_set_color(MAC_LED_INDEX, 0, 0, 0);
    }
#    endif

#    if defined(WIN_LED_INDEX)
    if (default_layer_state == (1 << 2)) {
        rgb_matrix_set_color(WIN_LED_INDEX, 255, 255, 255);
    } else {
        rgb_matrix_set_color(WIN_LED_INDEX, 0, 0, 0);
    }
#    endif
#endif
}

void keychron_board_calc_sidelight_coords(uint8_t led_idx, uint8_t ug_first, uint8_t ug_count, uint8_t density,
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

// Map board interface to Keychron implementation
void board_init(void) {
    keychron_board_init();
}

void board_indicators_render(void) {
    keychron_board_indicators_render();
}

uint8_t board_get_logo_led_index(void) {
    return keychron_board_get_logo_led_index();
}

uint8_t board_get_win_led_index(void) {
    return keychron_board_get_win_led_index();
}

uint8_t board_get_caps_led_index(void) {
    return keychron_board_get_caps_led_index();
}

uint8_t board_get_num_led_index(void) {
    return keychron_board_get_num_led_index();
}

uint8_t board_get_scroll_led_index(void) {
    return keychron_board_get_scroll_led_index();
}

void board_calc_sidelight_coords(uint8_t led_idx, uint8_t ug_first, uint8_t ug_count, uint8_t density,
                                 uint8_t *y_scaled, uint8_t *dist_scaled, uint8_t *opt_step, bool *is_hidden) {
    keychron_board_calc_sidelight_coords(led_idx, ug_first, ug_count, density, y_scaled, dist_scaled, opt_step, is_hidden);
}
