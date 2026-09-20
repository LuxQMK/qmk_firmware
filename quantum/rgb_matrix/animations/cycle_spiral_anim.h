#ifdef ENABLE_RGB_MATRIX_CYCLE_SPIRAL
RGB_MATRIX_EFFECT(CYCLE_SPIRAL)
#    ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

static hsv_t CYCLE_SPIRAL_math(hsv_t hsv, int16_t dx, int16_t dy, uint8_t dist, uint8_t time) {
    extern bool g_custom_rgb_reverse;
    extern uint8_t g_effect_density;
    uint8_t dist_scaled = (g_effect_density == 128) ? dist : (uint8_t)(((uint16_t)dist * g_effect_density) / 128);
    hsv.h = dist_scaled + (g_custom_rgb_reverse ? time : -time) - atan2_8(dy, dx);
    return hsv;
}

bool CYCLE_SPIRAL(effect_params_t* params) {
    return effect_runner_dx_dy_dist(params, &CYCLE_SPIRAL_math);
}

#    endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif     // ENABLE_RGB_MATRIX_CYCLE_SPIRAL
