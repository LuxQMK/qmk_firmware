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
