#pragma once

#include "luxqmk.h"

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

/**
 * Custom LuxQMK Wave Effect: Smooth directional wave sampling active gradient
 */
static bool CUSTOM_LUXQMK_WAVE_run(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    uint8_t time = scale16by8(g_rgb_timer, qadd8(rgb_matrix_config.speed / 4, 1));
    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        uint8_t x_scaled = (g_effect_density == 128) ? g_led_config.point[i].x : (uint8_t)(((uint16_t)g_led_config.point[i].x * g_effect_density) / 128);
        uint8_t phase = x_scaled + (g_custom_rgb_reverse ? -time : time) + rgb_matrix_config.hsv.h;
        RGB rgb = luxqmk_sample_gradient(g_active_gradient, phase);
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
    return rgb_matrix_check_finished_leds(led_max);
}

static bool CUSTOM_LUXQMK_WAVE(effect_params_t* params) {
    return CUSTOM_LUXQMK_WAVE_run(params);
}

/**
 * Custom Dynamic Cycle Effect: Center-out radial gradient cycle with directional control
 */
static bool CUSTOM_CYCLE_DYNAMIC_run(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    uint8_t time = scale16by8(g_rgb_timer, qadd8(rgb_matrix_config.speed / 4, 1));
    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        int16_t dx = g_led_config.point[i].x - 112;
        int16_t dy = g_led_config.point[i].y - 32;
        uint8_t dist = sqrt16(dx * dx + dy * dy);
        uint8_t dist_scaled = (g_effect_density == 128) ? (dist * 3 / 2) : (uint8_t)(((uint16_t)dist * 3 * g_effect_density) / 256);
        uint8_t phase = (g_custom_rgb_reverse ? (dist_scaled - time) : (dist_scaled + time)) + rgb_matrix_config.hsv.h;
        RGB rgb = luxqmk_sample_gradient(g_active_gradient, phase);
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
    return rgb_matrix_check_finished_leds(led_max);
}

static bool CUSTOM_CYCLE_DYNAMIC(effect_params_t* params) {
    return CUSTOM_CYCLE_DYNAMIC_run(params);
}

/**
 * Custom Multi-Stop Gradient Cycle Effect: Dynamic cycle through active multi-stop gradient
 */
static bool CUSTOM_GRADIENT_CYCLE_run(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    uint8_t time = scale16by8(g_rgb_timer, qadd8(rgb_matrix_config.speed / 4, 1));
    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        uint8_t x = g_led_config.point[i].x;
        uint8_t x_scaled = (g_effect_density == 128) ? x : (uint8_t)(((uint16_t)x * g_effect_density) / 128);
        uint8_t phase = (g_custom_rgb_reverse ? (x_scaled - time) : (x_scaled + time)) + rgb_matrix_config.hsv.h;
        RGB rgb = luxqmk_sample_gradient(g_active_gradient, phase);
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
    return rgb_matrix_check_finished_leds(led_max);
}

static bool CUSTOM_GRADIENT_CYCLE(effect_params_t* params) {
    return CUSTOM_GRADIENT_CYCLE_run(params);
}

/**
 * Custom Multi-Stop Gradient Wave Effect: Directional horizontal wave through active gradient
 */
static bool CUSTOM_GRADIENT_WAVE_run(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    uint8_t time = scale16by8(g_rgb_timer, qadd8(rgb_matrix_config.speed / 4, 1));
    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        uint8_t x_scaled = (g_effect_density == 128) ? g_led_config.point[i].x : (uint8_t)(((uint16_t)g_led_config.point[i].x * g_effect_density) / 128);
        uint8_t phase = x_scaled + (g_custom_rgb_reverse ? -time : time) + rgb_matrix_config.hsv.h;
        RGB rgb = luxqmk_sample_gradient(g_active_gradient, phase);
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
    return rgb_matrix_check_finished_leds(led_max);
}

static bool CUSTOM_GRADIENT_WAVE(effect_params_t* params) {
    return CUSTOM_GRADIENT_WAVE_run(params);
}

/**
 * Custom Multi-Stop Gradient Spiral Effect: Center-out radial spiral through active gradient
 */
static bool CUSTOM_GRADIENT_SPIRAL_run(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    uint8_t time = scale16by8(g_rgb_timer, qadd8(rgb_matrix_config.speed / 4, 1));
    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        int16_t dx = g_led_config.point[i].x - 112;
        int16_t dy = g_led_config.point[i].y - 32;
        uint8_t dist = sqrt16(dx * dx + dy * dy);
        uint8_t dist_scaled = (g_effect_density == 128) ? (dist * 3 / 2) : (uint8_t)(((uint16_t)dist * 3 * g_effect_density) / 256);
        uint8_t phase = (g_custom_rgb_reverse ? (dist_scaled - time) : (dist_scaled + time)) + rgb_matrix_config.hsv.h;
        RGB rgb = luxqmk_sample_gradient(g_active_gradient, phase);
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
    return rgb_matrix_check_finished_leds(led_max);
}

static bool CUSTOM_GRADIENT_SPIRAL(effect_params_t* params) {
    return CUSTOM_GRADIENT_SPIRAL_run(params);
}

/**
 * Custom Multi-Stop Gradient Breathing Effect: Smooth pulsation through active gradient
 */
static bool CUSTOM_GRADIENT_BREATHE_run(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    uint8_t time = scale16by8(g_rgb_timer, qadd8(rgb_matrix_config.speed / 4, 1));
    RGB rgb = luxqmk_sample_gradient(g_active_gradient, time);
    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
    return rgb_matrix_check_finished_leds(led_max);
}

static bool CUSTOM_GRADIENT_BREATHE(effect_params_t* params) {
    return CUSTOM_GRADIENT_BREATHE_run(params);
}

/**
 * Custom Hardware Per-Key RGB Profiles (1: FPS, 2: MOBA, 3: MMO/RPG)
 * Renders user-configured per-key color maps from persistent EEPROM/RAM
 * Scaled by master hardware brightness rgb_matrix_config.hsv.v
 */
static inline bool luxqmk_render_per_key_profile(uint8_t prof_idx, effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    uint8_t val = rgb_matrix_config.hsv.v;
    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        if (i < LUXQMK_PERKEY_MAX_LEDS) {
            RGB c = g_per_key_profiles[prof_idx][i];
            if (val < 255) {
                c.r = scale8(c.r, val);
                c.g = scale8(c.g, val);
                c.b = scale8(c.b, val);
            }
            rgb_matrix_set_color(i, c.r, c.g, c.b);
        }
    }
    return rgb_matrix_check_finished_leds(led_max);
}

static bool CUSTOM_PER_KEY_PROFILE_1_run(effect_params_t* params) {
    return luxqmk_render_per_key_profile(0, params);
}

static bool CUSTOM_PER_KEY_PROFILE_1(effect_params_t* params) {
    return CUSTOM_PER_KEY_PROFILE_1_run(params);
}

static bool CUSTOM_PER_KEY_PROFILE_2_run(effect_params_t* params) {
    return luxqmk_render_per_key_profile(1, params);
}

static bool CUSTOM_PER_KEY_PROFILE_2(effect_params_t* params) {
    return CUSTOM_PER_KEY_PROFILE_2_run(params);
}

static bool CUSTOM_PER_KEY_PROFILE_3_run(effect_params_t* params) {
    return luxqmk_render_per_key_profile(2, params);
}

static bool CUSTOM_PER_KEY_PROFILE_3(effect_params_t* params) {
    return CUSTOM_PER_KEY_PROFILE_3_run(params);
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
