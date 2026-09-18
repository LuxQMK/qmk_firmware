#pragma once

#include "luxqmk.h"

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

/**
 * Custom LuxQMK Wave Effect: Smooth directional wave with real-time reversal support
 */
static hsv_t CUSTOM_LUXQMK_WAVE_math(hsv_t hsv, uint8_t i, uint8_t time) {
    hsv.h += (g_led_config.point[i].x / 2) + (g_custom_rgb_reverse ? -time : time);
    hsv.s = 255;
    hsv.v = rgb_matrix_config.hsv.v;
    return hsv;
}

static bool CUSTOM_LUXQMK_WAVE(effect_params_t* params) {
    return effect_runner_i(params, &CUSTOM_LUXQMK_WAVE_math);
}

/**
 * Custom Dynamic Cycle Effect: Center-out gradient cycle with directional control
 */
static hsv_t CUSTOM_CYCLE_DYNAMIC_math(hsv_t hsv, int16_t dx, int16_t dy, uint8_t dist, uint8_t time) {
    hsv.h = g_custom_rgb_reverse ? (3 * dist / 2 - time) : (3 * dist / 2 + time);
    return hsv;
}

static bool CUSTOM_CYCLE_DYNAMIC(effect_params_t* params) {
    return effect_runner_dx_dy_dist(params, &CUSTOM_CYCLE_DYNAMIC_math);
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
