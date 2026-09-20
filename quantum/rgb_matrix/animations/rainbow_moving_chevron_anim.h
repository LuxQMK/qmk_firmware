#ifdef ENABLE_RGB_MATRIX_RAINBOW_MOVING_CHEVRON
RGB_MATRIX_EFFECT(RAINBOW_MOVING_CHEVRON)
#    ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

static hsv_t RAINBOW_MOVING_CHEVRON_math(hsv_t hsv, uint8_t i, uint8_t time) {
    extern bool g_custom_rgb_reverse;
    extern uint8_t g_effect_density;
    uint8_t dy = abs8(g_led_config.point[i].y - k_rgb_matrix_center.y);
    uint8_t x  = g_led_config.point[i].x;
    uint8_t spatial = (g_effect_density == 128) ? (dy + x) : (uint8_t)(((uint16_t)(dy + x) * g_effect_density) / 128);
    hsv.h += spatial + (g_custom_rgb_reverse ? time : -time);
    return hsv;
}

bool RAINBOW_MOVING_CHEVRON(effect_params_t* params) {
    return effect_runner_i(params, &RAINBOW_MOVING_CHEVRON_math);
}

#    endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif     // ENABLE_RGB_MATRIX_RAINBOW_MOVING_CHEVRON
