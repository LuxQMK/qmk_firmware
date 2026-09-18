#ifdef ENABLE_RGB_MATRIX_CYCLE_UP_DOWN
RGB_MATRIX_EFFECT(CYCLE_UP_DOWN)
#    ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

static hsv_t CYCLE_UP_DOWN_math(hsv_t hsv, uint8_t i, uint8_t time) {
    extern bool g_custom_rgb_reverse;
    hsv.h = g_led_config.point[i].y + (g_custom_rgb_reverse ? time : -time);
    return hsv;
}

bool CYCLE_UP_DOWN(effect_params_t* params) {
    return effect_runner_i(params, &CYCLE_UP_DOWN_math);
}

#    endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif     // ENABLE_RGB_MATRIX_CYCLE_UP_DOWN

