#ifdef ENABLE_RGB_MATRIX_RAINBOW_BEACON
RGB_MATRIX_EFFECT(RAINBOW_BEACON)
#    ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

static hsv_t RAINBOW_BEACON_math(hsv_t hsv, int8_t sin, int8_t cos, uint8_t i, uint8_t time) {
    extern bool g_custom_rgb_reverse;
    extern uint8_t g_effect_density;
    int8_t s = g_custom_rgb_reverse ? -sin : sin;
    int16_t term = ((g_led_config.point[i].y - k_rgb_matrix_center.y) * 2 * cos + (g_led_config.point[i].x - k_rgb_matrix_center.x) * 2 * s) / 128;
    hsv.h += (g_effect_density == 128) ? term : (int16_t)(((int32_t)term * g_effect_density) / 128);
    return hsv;
}

bool RAINBOW_BEACON(effect_params_t* params) {
    return effect_runner_sin_cos_i(params, &RAINBOW_BEACON_math);
}

#    endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif     // ENABLE_RGB_MATRIX_RAINBOW_BEACON
