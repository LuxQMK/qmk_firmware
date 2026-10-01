#include "luxqmk.h"
#include "dynamic_keymap.h"
#include "drivers/led/aw20216s.h"
#include "color.h"
#include <lib/lib8tion/lib8tion.h>
#include <stdlib.h>

#ifdef RGB_MATRIX_ENABLE
/**
 * RGB Matrix indicator rendering pipeline (Dual-Layer Reactive -> Layer Lighting -> Board Hardware Modules)
 */
bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    // 0. Direct Software Live Lighting Stream Override (prevents QMK base animations from bleeding through)
    if (g_direct_lighting_enable) {
        if (timer_elapsed32(g_direct_lighting_timer) > 5000) {
            // Watchdog timeout: automatically restore hardware animations if studio stops
            g_direct_lighting_enable = false;
        } else {
            uint8_t logo_idx = board_get_logo_led_index();
            for (uint8_t i = led_min; i < led_max; i++) {
                if (i == logo_idx && g_logo_mode != LOGO_MODE_RGB) {
                    continue;
                }
                if (i < DRIVER_LED_TOTAL && i < 144) {
                    rgb_matrix_set_color(i, g_direct_leds[i].r, g_direct_leds[i].g, g_direct_leds[i].b);
                }
            }
            board_indicators_render();
            return false; // Suppress QMK base effects during software direct lighting
        }
    }

#if defined(AW20216S_LED_COUNT)
    // 1. Global Multi-Stop Gradient Remapping for all standard QMK rainbow/cycle animations
    if (g_active_gradient > 0 && luxqmk_is_rainbow_effect(rgb_matrix_config.mode)) {
        uint8_t logo_idx = board_get_logo_led_index();
        uint8_t win_idx  = board_get_win_led_index();
        for (uint8_t i = led_min; i < led_max; i++) {
            if (i == logo_idx && g_logo_mode != LOGO_MODE_RGB) {
                continue;
            }
            if (i == win_idx && keymap_config.no_gui && g_win_lock_mode != WIN_LOCK_MODE_ANIMATION) {
                continue;
            }
            uint8_t r = 0, g = 0, b = 0;
            aw20216s_get_color(i, &r, &g, &b);
            uint8_t max_v = r > g ? (r > b ? r : b) : (g > b ? g : b);
            uint8_t min_v = r < g ? (r < b ? r : b) : (g < b ? g : b);
            uint8_t delta = max_v - min_v;
            if (delta >= 6 && max_v > 0) {
                uint8_t hue = luxqmk_fast_rgb_to_hue(r, g, b);
                RGB grad_rgb = luxqmk_sample_gradient(g_active_gradient, hue);
                if (max_v < rgb_matrix_config.hsv.v && rgb_matrix_config.hsv.v > 0) {
                    grad_rgb.r = (uint8_t)(((uint16_t)grad_rgb.r * max_v) / rgb_matrix_config.hsv.v);
                    grad_rgb.g = (uint8_t)(((uint16_t)grad_rgb.g * max_v) / rgb_matrix_config.hsv.v);
                    grad_rgb.b = (uint8_t)(((uint16_t)grad_rgb.b * max_v) / rgb_matrix_config.hsv.v);
                }
                rgb_matrix_set_color(i, grad_rgb.r, grad_rgb.g, grad_rgb.b);
            }
        }
    }
#endif

    // 2. Dedicated Independent Sidelights Rendering (for boards with underglow strips)
    if (g_sidelight_custom_enable && g_sidelight_mode != SIDELIGHT_MODE_FOLLOW_MAIN) {
        uint8_t logo_idx = board_get_logo_led_index();
        uint8_t speed_scaled = qadd8(g_sidelight_speed / 4, 1);
        uint8_t time = scale16by8(g_rgb_timer, speed_scaled);
        uint8_t val = rgb_matrix_config.hsv.v;
        uint8_t density = g_sidelight_density ? g_sidelight_density : 128;

        // Determine underglow range and strip boundaries
        uint8_t ug_first = 255;
        uint8_t ug_count = 0;
        for (uint8_t k = 0; k < DRIVER_LED_TOTAL; k++) {
            if (k != logo_idx && HAS_FLAGS(g_led_config.flags[k], LED_FLAG_UNDERGLOW)) {
                if (ug_first == 255) ug_first = k;
                ug_count++;
            }
        }

        for (uint8_t i = led_min; i < led_max; i++) {
            if (i == logo_idx) {
                continue;
            }
            if (i < DRIVER_LED_TOTAL && HAS_FLAGS(g_led_config.flags[i], LED_FLAG_UNDERGLOW)) {
                uint8_t y_scaled = 0;
                uint8_t dist_scaled = 0;
                uint8_t opt_step = 0;
                bool is_hidden = false;

                board_calc_sidelight_coords(i, ug_first, ug_count, density,
                                            &y_scaled, &dist_scaled, &opt_step, &is_hidden);

                switch (g_sidelight_mode) {
                    case SIDELIGHT_MODE_SOLID_COLOR: {
                        HSV hsv = { g_sidelight_color.h, g_sidelight_color.s, val };
                        RGB rgb = hsv_to_rgb(hsv);
                        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
                        break;
                    }

                    case SIDELIGHT_MODE_BREATHING: {
                        uint8_t breath_val = scale8(abs8(sin8(time)), val);
                        HSV hsv = { g_sidelight_color.h, g_sidelight_color.s, breath_val };
                        RGB rgb = hsv_to_rgb(hsv);
                        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
                        break;
                    }

                    case SIDELIGHT_MODE_CYCLE_RAINBOW: {
                        uint8_t phase = g_sidelight_reverse ? (255 - time) : time;
                        HSV hsv = { phase, 255, val };
                        RGB rgb = hsv_to_rgb(hsv);
                        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
                        break;
                    }

                    case SIDELIGHT_MODE_RAINBOW_WAVE: {
                        // Normal: Top to Bottom, Reverse: Bottom to Top
                        uint8_t phase = time + (g_sidelight_reverse ? y_scaled : (uint8_t)(256 - y_scaled)) + g_sidelight_color.h;
                        HSV hsv = { phase, 255, val };
                        RGB rgb = hsv_to_rgb(hsv);
                        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
                        break;
                    }

                    case SIDELIGHT_MODE_RAINBOW_CENTER_WAVE: {
                        // Normal: Center Outward, Reverse: Outward into Center
                        uint8_t phase = time + (g_sidelight_reverse ? dist_scaled : (uint8_t)(256 - dist_scaled)) + g_sidelight_color.h;
                        HSV hsv = { phase, 255, val };
                        RGB rgb = hsv_to_rgb(hsv);
                        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
                        break;
                    }

                    case SIDELIGHT_MODE_GRADIENT_WAVE: {
                        // Normal: Top to Bottom, Reverse: Bottom to Top
                        uint8_t phase = time + (g_sidelight_reverse ? y_scaled : (uint8_t)(256 - y_scaled));
                        RGB rgb = luxqmk_sample_gradient(g_sidelight_gradient, phase);
                        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
                        break;
                    }

                    case SIDELIGHT_MODE_GRADIENT_CENTER_WAVE: {
                        // Normal: Center Outward, Reverse: Outward into Center
                        uint8_t phase = time + (g_sidelight_reverse ? dist_scaled : (uint8_t)(256 - dist_scaled));
                        RGB rgb = luxqmk_sample_gradient(g_sidelight_gradient, phase);
                        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
                        break;
                    }

                    case SIDELIGHT_MODE_GRADIENT_CYCLE: {
                        uint8_t phase = g_sidelight_reverse ? (255 - time) : time;
                        RGB rgb = luxqmk_sample_gradient(g_sidelight_gradient, phase);
                        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
                        break;
                    }

                    case SIDELIGHT_MODE_GRADIENT_BREATHE: {
                        uint8_t breath_pulse = scale8(abs8(sin8(time)), val);
                        uint8_t t_grad = scale16by8(g_rgb_timer, qadd8(g_sidelight_speed / 16, 1));
                        RGB rgb = luxqmk_sample_gradient(g_sidelight_gradient, t_grad);
                        rgb_matrix_set_color(i, scale8(rgb.r, breath_pulse), scale8(rgb.g, breath_pulse), scale8(rgb.b, breath_pulse));
                        break;
                    }

                    case SIDELIGHT_MODE_SINGLE_WAVE: {
                        uint8_t wave = sin8(time + (g_sidelight_reverse ? y_scaled : (uint8_t)(256 - y_scaled)));
                        uint8_t wave_val = scale8(wave, val);
                        HSV hsv = { g_sidelight_color.h, g_sidelight_color.s, wave_val };
                        RGB rgb = hsv_to_rgb(hsv);
                        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
                        break;
                    }

                    case SIDELIGHT_MODE_DIAGNOSTIC: {
                        // Calibrated optical window diagnostic pattern:
                        // Step 0: Pure Red (Visible TOP on both Left & Right)
                        // Step 1: Orange
                        // Step 2: Yellow
                        // Step 3 & 4: Pure Green (Visible CENTER on both Left & Right)
                        // Step 5: Cyan
                        // Step 6: Magenta
                        // Step 7: Pure Blue (Visible BOTTOM on both Left & Right)
                        // Any LEDs hidden outside the diffuser window are kept dark.
                        if (is_hidden) {
                            rgb_matrix_set_color(i, 0, 0, 0);
                            break;
                        }
                        static const RGB PROGMEM diag_colors[8] = {
                            { 255, 0,   0   }, // 0: Red (Visible Top on BOTH strips)
                            { 255, 128, 0   }, // 1: Orange
                            { 255, 255, 0   }, // 2: Yellow
                            { 0,   255, 0   }, // 3: Pure Green (Visible Center 1)
                            { 0,   255, 0   }, // 4: Pure Green (Visible Center 2)
                            { 0,   255, 255 }, // 5: Cyan
                            { 255, 0,   255 }, // 6: Magenta
                            { 0,   0,   255 }  // 7: Pure Blue (Visible Bottom on BOTH strips)
                        };
                        uint8_t c_idx = (opt_step < 8) ? opt_step : 7;
                        RGB col;
                        memcpy_P(&col, &diag_colors[c_idx], sizeof(RGB));
                        if (val < 255) {
                            col.r = scale8(col.r, val);
                            col.g = scale8(col.g, val);
                            col.b = scale8(col.b, val);
                        }
                        rgb_matrix_set_color(i, col.r, col.g, col.b);
                        break;
                    }

                    case SIDELIGHT_MODE_OFF: {
                        rgb_matrix_set_color(i, 0, 0, 0);
                        break;
                    }

                    default:
                        break;
                }
            }
        }
    }

    // 3. Always ensure board-specific hardware indicators (Logo badge, Win Lock, Caps/Num/Scroll) are rendered
    board_indicators_render();

    return false;
}

bool rgb_matrix_indicators_user(void) {
    // 0. Direct Software Live Lighting Stream (LuxQMK Studio Audio Visualizer / PC FX)
    if (g_direct_lighting_enable) {
        if (timer_elapsed32(g_direct_lighting_timer) > 5000) {
            // Watchdog timeout: automatically restore hardware animations if studio stops
            g_direct_lighting_enable = false;
        } else {
            uint8_t logo_idx = board_get_logo_led_index();
            for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {
                if (i == logo_idx && g_logo_mode != LOGO_MODE_RGB) {
                    // Let hardware logo lock indicator handle logo badge if configured
                    continue;
                }
                if (i < 144) {
                    rgb_matrix_set_color(i, g_direct_leds[i].r, g_direct_leds[i].g, g_direct_leds[i].b);
                }
            }
            // Render hardware board-specific indicators (Caps/Num/Win Lock) on top
            board_indicators_render();
            return false;
        }
    }

    // 1. Dual-Layer Reactive Lighting Overlay
    if (g_reactive_enable && g_reactive_mode != REACTIVE_MODE_OFF && g_last_hit_tracker.count > 0) {
        uint8_t val = rgb_matrix_get_val();
        if (val == 0) {
            val = 255;
        }
        uint8_t hit_count = g_last_hit_tracker.count;
        bool is_bg_none = (rgb_matrix_config.mode == RGB_MATRIX_NONE);
        bool any_active = false;
        uint8_t logo_idx = board_get_logo_led_index();

        for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {
            if (i == logo_idx) continue;

            uint16_t reactive_intensity = 0;
            uint8_t  reactive_hue       = g_reactive_color.h;
            uint8_t  reactive_sat       = g_reactive_color.s;

            switch (g_reactive_mode) {
                case REACTIVE_MODE_FADE: {
                    for (int8_t j = hit_count - 1; j >= 0; j--) {
                        if (g_last_hit_tracker.index[j] == i) {
                            uint16_t tick = scale16by8(g_last_hit_tracker.tick[j], qadd8(g_reactive_speed, 1));
                            if (tick < 255) {
                                reactive_intensity = 255 - tick;
                                any_active = true;
                            }
                            break;
                        }
                    }
                    break;
                }

                case REACTIVE_MODE_SPLASH:
                case REACTIVE_MODE_SPLASH_RAINBOW: {
                    uint16_t sum_int = 0;
                    for (uint8_t j = 0; j < hit_count; j++) {
                        int16_t  dx   = g_led_config.point[i].x - g_last_hit_tracker.x[j];
                        int16_t  dy   = g_led_config.point[i].y - g_last_hit_tracker.y[j];
                        uint8_t  dist = sqrt16(dx * dx + dy * dy);
                        uint16_t tick = scale16by8(g_last_hit_tracker.tick[j], qadd8(g_reactive_speed, 1));

                        int16_t effect = (int16_t)tick - (int16_t)dist;
                        if (effect >= 0 && effect < 28 && tick < 255) {
                            uint16_t wave_int = (28 - effect) * (255 - tick) / 28;
                            sum_int = qadd8(sum_int, wave_int);
                            any_active = true;
                            if (g_reactive_mode == REACTIVE_MODE_SPLASH_RAINBOW) {
                                reactive_hue = (dist + tick) & 0xFF;
                                reactive_sat = 255;
                            }
                        }
                    }
                    reactive_intensity = sum_int;
                    break;
                }

                case REACTIVE_MODE_CROSS: {
                    uint16_t sum_int = 0;
                    for (uint8_t j = 0; j < hit_count; j++) {
                        int16_t  dx   = abs(g_led_config.point[i].x - g_last_hit_tracker.x[j]);
                        int16_t  dy   = abs(g_led_config.point[i].y - g_last_hit_tracker.y[j]);
                        uint16_t tick = scale16by8(g_last_hit_tracker.tick[j], qadd8(g_reactive_speed, 1));

                        if (tick < 255) {
                            if (dx < 10) {
                                int16_t eff = (int16_t)tick - dy;
                                if (eff >= 0 && eff < 24) {
                                    sum_int = qadd8(sum_int, (24 - eff) * (255 - tick) / 24);
                                    any_active = true;
                                }
                            }
                            if (dy < 10) {
                                int16_t eff = (int16_t)tick - dx;
                                if (eff >= 0 && eff < 24) {
                                    sum_int = qadd8(sum_int, (24 - eff) * (255 - tick) / 24);
                                    any_active = true;
                                }
                            }
                        }
                    }
                    reactive_intensity = sum_int;
                    break;
                }

                case REACTIVE_MODE_NEXUS: {
                    uint16_t sum_int = 0;
                    for (uint8_t j = 0; j < hit_count; j++) {
                        int16_t  dx   = abs(g_led_config.point[i].x - g_last_hit_tracker.x[j]);
                        int16_t  dy   = abs(g_led_config.point[i].y - g_last_hit_tracker.y[j]);
                        int16_t  dist = abs(dx - dy);
                        uint16_t tick = scale16by8(g_last_hit_tracker.tick[j], qadd8(g_reactive_speed, 1));

                        if (dist < 12 && tick < 255) {
                            int16_t eff = (int16_t)tick - (dx + dy) / 2;
                            if (eff >= 0 && eff < 24) {
                                sum_int = qadd8(sum_int, (24 - eff) * (255 - tick) / 24);
                                any_active = true;
                            }
                        }
                    }
                    reactive_intensity = sum_int;
                    break;
                }

                case REACTIVE_MODE_WIDE: {
                    uint16_t sum_int = 0;
                    for (uint8_t j = 0; j < hit_count; j++) {
                        int16_t  dx   = g_led_config.point[i].x - g_last_hit_tracker.x[j];
                        int16_t  dy   = g_led_config.point[i].y - g_last_hit_tracker.y[j];
                        uint8_t  dist = sqrt16(dx * dx + dy * dy);
                        uint16_t tick = scale16by8(g_last_hit_tracker.tick[j], qadd8(g_reactive_speed, 1));

                        int16_t effect = (int16_t)tick - (int16_t)dist;
                        if (effect >= 0 && effect < 48 && tick < 255) {
                            uint16_t wave_int = (48 - effect) * (255 - tick) / 48;
                            sum_int = qadd8(sum_int, wave_int);
                            any_active = true;
                        }
                    }
                    reactive_intensity = sum_int;
                    break;
                }

                case REACTIVE_MODE_HEATMAP: {
                    for (int8_t j = hit_count - 1; j >= 0; j--) {
                        if (g_last_hit_tracker.index[j] == i) {
                            uint16_t tick = scale16by8(g_last_hit_tracker.tick[j], qadd8(g_reactive_speed, 1));
                            if (tick < 255) {
                                reactive_intensity = 255 - tick;
                                reactive_hue = scale8(255 - tick, 42);
                                reactive_sat = 255;
                                any_active = true;
                            }
                            break;
                        }
                    }
                    break;
                }

                default:
                    break;
            }

            if (reactive_intensity > 0) {
                uint8_t scaled_val = scale8((uint8_t)reactive_intensity, val);
                HSV r_hsv = { reactive_hue, reactive_sat, scaled_val };
                RGB r_rgb = hsv_to_rgb(r_hsv);

                uint8_t bg_r, bg_g, bg_b;
#if defined(AW20216S_LED_COUNT)
                aw20216s_get_color(i, &bg_r, &bg_g, &bg_b);
#else
                bg_r = 0; bg_g = 0; bg_b = 0;
#endif

                if (g_reactive_blend == REACTIVE_BLEND_ADDITIVE) {
                    rgb_matrix_set_color(i, qadd8(bg_r, r_rgb.r), qadd8(bg_g, r_rgb.g), qadd8(bg_b, r_rgb.b));
                } else {
                    uint8_t alpha = scaled_val;
                    uint8_t out_r = ((uint16_t)r_rgb.r * alpha + (uint16_t)bg_r * (255 - alpha)) / 255;
                    uint8_t out_g = ((uint16_t)r_rgb.g * alpha + (uint16_t)bg_g * (255 - alpha)) / 255;
                    uint8_t out_b = ((uint16_t)r_rgb.b * alpha + (uint16_t)bg_b * (255 - alpha)) / 255;
                    rgb_matrix_set_color(i, out_r, out_g, out_b);
                }
            } else if (is_bg_none) {
                rgb_matrix_set_color(i, 0, 0, 0);
            }
        }

        if (!any_active) {
            g_last_hit_tracker.count = 0;
            if (is_bg_none) {
                rgb_matrix_set_color_all(0, 0, 0);
            }
        }
    }

    // 2. Active Layer Key Lighting & Dimming Overlay (Completely decoupled)
    uint8_t current_layer = get_highest_layer(layer_state | default_layer_state);
    if (current_layer > 0 && current_layer < 4) {
        bool dim_master   = (g_layer_dim_enable & 0x01) != 0;
        bool dim_layer    = (g_layer_dim_enable & (1 << current_layer)) != 0;
        bool do_dim       = dim_master && dim_layer;

        bool color_master = (g_layer_lighting_enable & 0x01) != 0;
        bool color_layer  = (g_layer_lighting_enable & (1 << current_layer)) != 0;
        bool do_color     = color_master && color_layer;

        if (do_dim || do_color) {
            uint8_t hue = g_layer_colors[current_layer].h;
            uint8_t sat = g_layer_colors[current_layer].s;
            uint8_t dim = g_layer_dim_levels[current_layer];
            uint8_t val = rgb_matrix_get_val();
            if (val == 0) {
                val = 255;
            }
            HSV hsv = { hue, sat, val };
            RGB active_rgb = hsv_to_rgb(hsv);

            for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
                for (uint8_t c = 0; c < MATRIX_COLS; c++) {
                    uint8_t led = g_led_config.matrix_co[r][c];
                    if (led == NO_LED) {
                        continue;
                    }

#if defined(DYNAMIC_KEYMAP_ENABLE)
                    uint16_t keycode = dynamic_keymap_get_keycode(current_layer, r, c);
#else
                    uint16_t keycode = keymap_key_to_keycode(current_layer, (keypos_t){ .row = r, .col = c });
#endif
                    if (keycode == KC_TRNS || keycode == KC_NO) {
                        if (do_dim) {
                            if (dim == 0) {
                                rgb_matrix_set_color(led, 0, 0, 0);
                            } else if (dim < 255) {
                                uint8_t red = 0, green = 0, blue = 0;
#if defined(AW20216S_LED_COUNT)
                                aw20216s_get_color(led, &red, &green, &blue);
#endif
                                uint8_t r_dim = ((uint16_t)red * dim) / 255;
                                uint8_t g_dim = ((uint16_t)green * dim) / 255;
                                uint8_t b_dim = ((uint16_t)blue * dim) / 255;
                                rgb_matrix_set_color(led, r_dim, g_dim, b_dim);
                            }
                        }
                    } else {
                        if (do_color) {
                            rgb_matrix_set_color(led, active_rgb.r, active_rgb.g, active_rgb.b);
                        }
                    }
                }
            }
        }
    }

    // 3. Render hardware board-specific indicators
    board_indicators_render();

    return false;
}

bool board_has_sidelights(void) {
    for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {
        if (HAS_FLAGS(g_led_config.flags[i], LED_FLAG_UNDERGLOW)) {
            return true;
        }
    }
    return false;
}
#endif
