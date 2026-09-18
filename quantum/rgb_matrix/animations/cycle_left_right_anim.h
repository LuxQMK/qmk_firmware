#ifdef ENABLE_RGB_MATRIX_CYCLE_LEFT_RIGHT
RGB_MATRIX_EFFECT(CYCLE_LEFT_RIGHT)
#    ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

static hsv_t CYCLE_LEFT_RIGHT_math(hsv_t hsv, uint8_t i, uint8_t time) {
    extern bool g_custom_rgb_reverse;
    hsv.h = g_led_config.point[i].x + (g_custom_rgb_reverse ? time : -time);
    return hsv;
}

bool CYCLE_LEFT_RIGHT(effect_params_t* params) {
    return effect_runner_i(params, &CYCLE_LEFT_RIGHT_math);
}

#    endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif     // ENABLE_RGB_MATRIX_CYCLE_LEFT_RIGHT

