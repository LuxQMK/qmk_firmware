#ifdef ENABLE_RGB_MATRIX_HUE_PENDULUM
RGB_MATRIX_EFFECT(HUE_PENDULUM)
#    ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

// Change huedelta to adjust range of hue change. 0-255.
// Looks better with a low value and slow speed for subtle change.
// Hue Pendulum - color changes in a wave to the right before reversing direction
static hsv_t HUE_PENDULUM_math(hsv_t hsv, uint8_t i, uint8_t time) {
    extern bool g_custom_rgb_reverse;
    uint8_t huedelta = 12;
    uint8_t x        = g_custom_rgb_reverse ? (224 - g_led_config.point[i].x) : g_led_config.point[i].x;
    hsv.h            = hsv.h + scale8(abs8(sin8(time) + x - 128) * 2, huedelta);
    return hsv;
}

bool HUE_PENDULUM(effect_params_t* params) {
    return effect_runner_i(params, &HUE_PENDULUM_math);
}

#    endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif     // DISABLE_RGB_HUE_PENDULUM
