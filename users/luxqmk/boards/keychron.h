#pragma once

#include "luxqmk.h"

/**
 * Hardware board module for Keychron Family (Q, V, C Pro, S series)
 */
void keychron_board_init(void);
void keychron_board_indicators_render(void);
uint8_t keychron_board_get_logo_led_index(void);
uint8_t keychron_board_get_win_led_index(void);
uint8_t keychron_board_get_caps_led_index(void);
uint8_t keychron_board_get_num_led_index(void);
uint8_t keychron_board_get_scroll_led_index(void);
void keychron_board_calc_sidelight_coords(uint8_t led_idx, uint8_t ug_first, uint8_t ug_count, uint8_t density,
                                          uint8_t *y_scaled, uint8_t *dist_scaled, uint8_t *opt_step, bool *is_hidden);
