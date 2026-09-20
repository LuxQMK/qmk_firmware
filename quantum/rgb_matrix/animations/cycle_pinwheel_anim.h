#ifdef ENABLE_RGB_MATRIX_CYCLE_PINWHEEL
RGB_MATRIX_EFFECT(CYCLE_PINWHEEL)
#    ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

static hsv_t CYCLE_PINWHEEL_math(hsv_t hsv, int16_t dx, int16_t dy, uint8_t time) {
    extern bool g_custom_rgb_reverse;
    extern uint8_t g_effect_density;
    uint8_t angle = atan2_8(dy, dx);
    uint8_t angle_scaled = (g_effect_density == 128) ? angle : (uint8_t)(((uint16_t)angle * g_effect_density) / 128);
    hsv.h = angle_scaled + (g_custom_rgb_reverse ? -time : time);
    return hsv;
}

bool CYCLE_PINWHEEL(effect_params_t* params) {
    return effect_runner_dx_dy(params, &CYCLE_PINWHEEL_math);
}

#    endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif     // ENABLE_RGB_MATRIX_CYCLE_PINWHEEL
