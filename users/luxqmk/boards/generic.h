#pragma once

#include "luxqmk.h"

/**
 * Generic hardware board module for standard QMK / VIA keyboards
 */
void generic_board_init(void);
void generic_board_indicators_render(void);
uint8_t generic_board_get_logo_led_index(void);
uint8_t generic_board_get_win_led_index(void);
void generic_board_calc_sidelight_coords(uint8_t led_idx, uint8_t ug_first, uint8_t ug_count, uint8_t density,
                                         uint8_t *y_scaled, uint8_t *dist_scaled, uint8_t *opt_step, bool *is_hidden);

