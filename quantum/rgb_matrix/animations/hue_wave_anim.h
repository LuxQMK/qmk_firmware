#ifdef ENABLE_RGB_MATRIX_HUE_WAVE
RGB_MATRIX_EFFECT(HUE_WAVE)
#    ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

// Change huedelta to adjust range of hue change. 0-255.
// Looks better with a low value and slow speed for subtle change.
// Hue Wave - color changes in a wave to the right
static hsv_t HUE_WAVE_math(hsv_t hsv, uint8_t i, uint8_t time) {
    extern bool g_custom_rgb_reverse;
    uint8_t huedelta = 24;
    uint8_t t        = g_custom_rgb_reverse ? (255 - time) : time;
    hsv.h            = hsv.h + scale8(abs8(g_led_config.point[i].x - t), huedelta);
    return hsv;
}

bool HUE_WAVE(effect_params_t* params) {
    return effect_runner_i(params, &HUE_WAVE_math);
}

#    endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif     // DISABLE_RGB_HUE_WAVE
