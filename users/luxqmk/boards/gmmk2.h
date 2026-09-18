#pragma once

#include "luxqmk.h"

/**
 * Hardware board module for Glorious GMMK 2 Family (96%, 65% - ANSI / ISO)
 */
void gmmk2_board_init(void);
void gmmk2_board_indicators_render(void);
uint8_t gmmk2_board_get_logo_led_index(void);
uint8_t gmmk2_board_get_win_led_index(void);
