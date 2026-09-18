#ifdef ENABLE_RGB_MATRIX_CYCLE_PINWHEEL
RGB_MATRIX_EFFECT(CYCLE_PINWHEEL)
#    ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

static hsv_t CYCLE_PINWHEEL_math(hsv_t hsv, int16_t dx, int16_t dy, uint8_t time) {
    extern bool g_custom_rgb_reverse;
    hsv.h = atan2_8(dy, dx) + (g_custom_rgb_reverse ? -time : time);
    return hsv;
}

bool CYCLE_PINWHEEL(effect_params_t* params) {
    return effect_runner_dx_dy(params, &CYCLE_PINWHEEL_math);
}

#    endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif     // ENABLE_RGB_MATRIX_CYCLE_PINWHEEL
