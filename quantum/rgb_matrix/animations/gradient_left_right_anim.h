#ifdef ENABLE_RGB_MATRIX_GRADIENT_LEFT_RIGHT
RGB_MATRIX_EFFECT(GRADIENT_LEFT_RIGHT)
#    ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

bool GRADIENT_LEFT_RIGHT(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    extern bool g_custom_rgb_reverse;
    extern uint8_t g_effect_density;

    hsv_t   hsv   = rgb_matrix_config.hsv;
    uint8_t scale = scale8(64, rgb_matrix_config.speed);
    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        // The x range will be 0..224, map this to 0..7
        // Relies on hue being 8-bit and wrapping
        uint8_t x = g_custom_rgb_reverse ? (224 - g_led_config.point[i].x) : g_led_config.point[i].x;
        uint8_t x_scaled = (g_effect_density == 128) ? x : (uint8_t)(((uint16_t)x * g_effect_density) / 128);
        hsv.h     = rgb_matrix_config.hsv.h + (scale * x_scaled >> 5);
        rgb_t rgb = rgb_matrix_hsv_to_rgb(hsv);
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
    return rgb_matrix_check_finished_leds(led_max);
}

#    endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif     // ENABLE_RGB_MATRIX_GRADIENT_LEFT_RIGHT
