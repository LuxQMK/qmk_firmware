#include "generic.h"
#include "color.h"

void generic_board_init(void) {
    // Standard initialization for generic boards
}

uint8_t generic_board_get_logo_led_index(void) {
    return NO_LED;
}

uint8_t generic_board_get_win_led_index(void) {
    return NO_LED;
}

void generic_board_indicators_render(void) {
    // Default indicator handling for generic QMK keyboards
}

void generic_board_calc_sidelight_coords(uint8_t led_idx, uint8_t ug_first, uint8_t ug_count, uint8_t density,
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

// Map board interface to generic implementation
void board_init(void) {
    generic_board_init();
}

void board_indicators_render(void) {
    generic_board_indicators_render();
}

uint8_t board_get_logo_led_index(void) {
    return generic_board_get_logo_led_index();
}

uint8_t board_get_win_led_index(void) {
    return generic_board_get_win_led_index();
}

void board_calc_sidelight_coords(uint8_t led_idx, uint8_t ug_first, uint8_t ug_count, uint8_t density,
                                 uint8_t *y_scaled, uint8_t *dist_scaled, uint8_t *opt_step, bool *is_hidden) {
    generic_board_calc_sidelight_coords(led_idx, ug_first, ug_count, density, y_scaled, dist_scaled, opt_step, is_hidden);
}

