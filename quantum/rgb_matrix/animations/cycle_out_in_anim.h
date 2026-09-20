#ifdef ENABLE_RGB_MATRIX_CYCLE_OUT_IN
RGB_MATRIX_EFFECT(CYCLE_OUT_IN)
#    ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

static hsv_t CYCLE_OUT_IN_math(hsv_t hsv, int16_t dx, int16_t dy, uint8_t dist, uint8_t time) {
    extern bool g_custom_rgb_reverse;
    extern uint8_t g_effect_density;
    uint8_t d = (g_effect_density == 128) ? (3 * dist / 2) : (uint8_t)(((uint16_t)dist * 3 * g_effect_density) / 256);
    hsv.h = d + (g_custom_rgb_reverse ? -time : time);
    return hsv;
}

bool CYCLE_OUT_IN(effect_params_t* params) {
    return effect_runner_dx_dy_dist(params, &CYCLE_OUT_IN_math);
}

#    endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif     // ENABLE_RGB_MATRIX_CYCLE_OUT_IN

