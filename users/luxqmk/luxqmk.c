#include "luxqmk.h"
#include "via.h"
#include "dynamic_keymap.h"
#include "drivers/led/aw20216s.h"
#include "color.h"
#include "eeprom.h"
#include <lib/lib8tion/lib8tion.h>
#include <stdlib.h>

/**
 * Global configuration variables for lighting, layer colors, and logo mode
 */
bool g_custom_rgb_reverse    = false;
bool g_layer_lighting_enable = true;
uint8_t g_layer_dim_level    = 128; // 0..255 (128 = 50% background brightness)
layer_color_t g_layer_colors[4] = {
    { 0, 0 },       // Layer 0 (Base - default RGB effects)
    { 28, 255 },    // Layer 1 (Fn / Media) -> Amber / Gold
    { 128, 255 },   // Layer 2 (Custom 2) -> Turquoise / Cyan
    { 200, 255 }    // Layer 3 (Custom 3) -> Purple / Magenta
};

uint8_t g_logo_mode = LOGO_MODE_RGB;
layer_color_t g_logo_lock_colors[8] = {
    { 0, 0 },       // 0: No locks active
    { 0, 255 },     // 1: Caps Lock (#FF0000)
    { 165, 255 },   // 2: Num Lock (#001EFF)
    { 8, 255 },     // 3: Caps + Num (#FF3200)
    { 77, 255 },    // 4: Scroll Lock (#32FF00)
    { 43, 255 },    // 5: Caps + Scroll (#FFFF00)
    { 137, 255 },   // 6: Num + Scroll (#00C8FF)
    { 0, 0 }        // 7: Caps + Num + Scroll (#FFFFFF)
};

// Windows Key Lock configuration
uint8_t g_win_lock_mode = WIN_LOCK_MODE_ANIMATION; // 0 = Standard Animation, 1 = Off, 2 = Custom Color
layer_color_t g_win_lock_color = { 0, 255 };       // Default red

// Dual-Layer Reactive Lighting configuration
bool g_reactive_enable         = false;
uint8_t g_reactive_mode        = REACTIVE_MODE_OFF;
layer_color_t g_reactive_color = { 0, 0 };         // Default white
uint8_t g_reactive_speed       = 128;
uint8_t g_reactive_blend       = REACTIVE_BLEND_ADDITIVE; // 0 = Additive Glow, 1 = Override

/**
 * Save user custom configuration to persistent EEPROM storage
 */
void luxqmk_eeprom_save(void) {
    uint8_t buf[VIA_EEPROM_CUSTOM_CONFIG_SIZE];
    memset(buf, 0, sizeof(buf));

    buf[0] = g_custom_rgb_reverse ? 1 : 0;
    buf[1] = g_layer_lighting_enable ? 1 : 0;
    buf[2] = g_layer_dim_level;
    buf[3] = g_layer_colors[1].h;
    buf[4] = g_layer_colors[1].s;
    buf[5] = g_layer_colors[2].h;
    buf[6] = g_layer_colors[2].s;
    buf[7] = g_layer_colors[3].h;
    buf[8] = g_layer_colors[3].s;

    buf[9] = g_logo_mode;
    for (uint8_t i = 1; i < 8; i++) {
        buf[10 + ((i - 1) * 2)] = g_logo_lock_colors[i].h;
        buf[11 + ((i - 1) * 2)] = g_logo_lock_colors[i].s;
    }

    buf[24] = g_win_lock_mode;
    buf[25] = g_win_lock_color.h;
    buf[26] = g_win_lock_color.s;

    buf[27] = g_reactive_enable ? 1 : 0;
    buf[28] = g_reactive_mode;
    buf[29] = g_reactive_color.h;
    buf[30] = g_reactive_color.s;
    buf[31] = (g_reactive_speed & 0x7F) | ((g_reactive_blend & 0x01) << 7);

    via_update_custom_config(buf, 0, sizeof(buf));
}

/**
 * Load user custom configuration from EEPROM upon startup
 */
void luxqmk_eeprom_load(void) {
    uint8_t buf[VIA_EEPROM_CUSTOM_CONFIG_SIZE];
    via_read_custom_config(buf, 0, sizeof(buf));

    uint8_t rev    = buf[0];
    uint8_t enable = buf[1];
    uint8_t dim    = buf[2];
    uint8_t l1_h   = buf[3];
    uint8_t l1_s   = buf[4];
    uint8_t l2_h   = buf[5];
    uint8_t l2_s   = buf[6];
    uint8_t l3_h   = buf[7];
    uint8_t l3_s   = buf[8];

    uint8_t logo_mode = buf[9];

    uint8_t win_lock_mode = buf[24];
    uint8_t win_lock_h    = buf[25];
    uint8_t win_lock_s    = buf[26];

    uint8_t r_enable = buf[27];
    uint8_t r_mode   = buf[28];
    uint8_t r_h      = buf[29];
    uint8_t r_s      = buf[30];
    uint8_t r_spd    = buf[31];

    if (enable == 0xFF) {
        // Uninitialized EEPROM defaults
        g_custom_rgb_reverse    = false;
        g_layer_lighting_enable = true;
        g_layer_dim_level       = 128;
        g_layer_colors[1]       = (layer_color_t){ 28, 255 };
        g_layer_colors[2]       = (layer_color_t){ 128, 255 };
        g_layer_colors[3]       = (layer_color_t){ 200, 255 };

        g_logo_mode             = LOGO_MODE_RGB;
        g_logo_lock_colors[0]   = (layer_color_t){ 0, 0 };
        g_logo_lock_colors[1]   = (layer_color_t){ 0, 255 };    // Caps (#FF0000)
        g_logo_lock_colors[2]   = (layer_color_t){ 165, 255 };  // Num (#001EFF)
        g_logo_lock_colors[3]   = (layer_color_t){ 8, 255 };    // Caps + Num (#FF3200)
        g_logo_lock_colors[4]   = (layer_color_t){ 77, 255 };   // Scroll (#32FF00)
        g_logo_lock_colors[5]   = (layer_color_t){ 43, 255 };   // Caps + Scroll (#FFFF00)
        g_logo_lock_colors[6]   = (layer_color_t){ 137, 255 };  // Num + Scroll (#00C8FF)
        g_logo_lock_colors[7]   = (layer_color_t){ 0, 0 };      // All (#FFFFFF)

        g_win_lock_mode         = WIN_LOCK_MODE_ANIMATION;
        g_win_lock_color        = (layer_color_t){ 0, 255 };

        g_reactive_enable       = false;
        g_reactive_mode         = REACTIVE_MODE_OFF;
        g_reactive_color        = (layer_color_t){ 0, 0 };
        g_reactive_speed        = 128;
        g_reactive_blend        = REACTIVE_BLEND_ADDITIVE;
        luxqmk_eeprom_save();
    } else {
        g_custom_rgb_reverse    = (rev != 0);
        g_layer_lighting_enable = (enable != 0);
        g_layer_dim_level       = dim;
        g_layer_colors[1]       = (layer_color_t){ l1_h, l1_s };
        g_layer_colors[2]       = (layer_color_t){ l2_h, l2_s };
        g_layer_colors[3]       = (layer_color_t){ l3_h, l3_s };

        if (logo_mode == 0xFF) {
            g_logo_mode           = LOGO_MODE_RGB;
            g_logo_lock_colors[0] = (layer_color_t){ 0, 0 };
            g_logo_lock_colors[1] = (layer_color_t){ 0, 255 };
            g_logo_lock_colors[2] = (layer_color_t){ 165, 255 };
            g_logo_lock_colors[3] = (layer_color_t){ 8, 255 };
            g_logo_lock_colors[4] = (layer_color_t){ 77, 255 };
            g_logo_lock_colors[5] = (layer_color_t){ 43, 255 };
            g_logo_lock_colors[6] = (layer_color_t){ 137, 255 };
            g_logo_lock_colors[7] = (layer_color_t){ 0, 0 };
            luxqmk_eeprom_save();
        } else {
            g_logo_mode = logo_mode;
            for (uint8_t i = 1; i < 8; i++) {
                g_logo_lock_colors[i].h = buf[10 + ((i - 1) * 2)];
                g_logo_lock_colors[i].s = buf[11 + ((i - 1) * 2)];
            }
        }

        if (win_lock_mode == 0xFF) {
            g_win_lock_mode  = WIN_LOCK_MODE_ANIMATION;
            g_win_lock_color = (layer_color_t){ 0, 255 };
            luxqmk_eeprom_save();
        } else {
            g_win_lock_mode  = win_lock_mode;
            g_win_lock_color = (layer_color_t){ win_lock_h, win_lock_s };
        }

        if (r_enable == 0xFF) {
            g_reactive_enable = false;
            g_reactive_mode   = REACTIVE_MODE_OFF;
            g_reactive_color  = (layer_color_t){ 0, 0 };
            g_reactive_speed  = 128;
            g_reactive_blend  = REACTIVE_BLEND_ADDITIVE;
            luxqmk_eeprom_save();
        } else {
            g_reactive_enable = (r_enable != 0);
            g_reactive_mode   = r_mode;
            g_reactive_color  = (layer_color_t){ r_h, r_s };
            g_reactive_speed  = (r_spd & 0x7F) < 10 ? 128 : (r_spd & 0x7F);
            g_reactive_blend  = (r_spd >> 7) & 0x01;
        }
    }
}

/**
 * QMK keyboard post-initialization hook
 */
void keyboard_post_init_user(void) {
    board_init();
    luxqmk_eeprom_load();
#if defined(NKRO_ENABLE)
    if (!keymap_config.nkro) {
        keymap_config.nkro = 1;
        eeconfig_update_keymap(&keymap_config);
    }
#endif
}

/**
 * Handle custom VIA / LuxQMK Studio WebHID protocol commands
 */
void via_custom_value_command_kb(uint8_t *data, uint8_t length) {
    uint8_t *command_id = &(data[0]);
    uint8_t channel_id  = data[1];
    uint8_t value_id    = data[2];

    if (channel_id == USER_CUSTOM_CHANNEL) {
        if (*command_id == id_custom_save) {
            luxqmk_eeprom_save();
            return;
        }

        switch (value_id) {
            case USER_VAL_RGB_REVERSE:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_custom_rgb_reverse ? 1 : 0;
                } else if (*command_id == id_custom_set_value) {
                    g_custom_rgb_reverse = (data[3] != 0);
                    luxqmk_eeprom_save();
                }
                return;

            case USER_VAL_LAYER_LIGHTING_ENABLE:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_layer_lighting_enable ? 1 : 0;
                } else if (*command_id == id_custom_set_value) {
                    g_layer_lighting_enable = (data[3] != 0);
                    luxqmk_eeprom_save();
                }
                return;

            case USER_VAL_LAYER_DIM_LEVEL:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_layer_dim_level;
                } else if (*command_id == id_custom_set_value) {
                    g_layer_dim_level = data[3];
                    luxqmk_eeprom_save();
                }
                return;

            case USER_VAL_LAYER_1_COLOR:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_layer_colors[1].h;
                    data[4] = g_layer_colors[1].s;
                } else if (*command_id == id_custom_set_value) {
                    g_layer_colors[1].h = data[3];
                    g_layer_colors[1].s = data[4];
                    luxqmk_eeprom_save();
                }
                return;

            case USER_VAL_LAYER_2_COLOR:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_layer_colors[2].h;
                    data[4] = g_layer_colors[2].s;
                } else if (*command_id == id_custom_set_value) {
                    g_layer_colors[2].h = data[3];
                    g_layer_colors[2].s = data[4];
                    luxqmk_eeprom_save();
                }
                return;

            case USER_VAL_LAYER_3_COLOR:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_layer_colors[3].h;
                    data[4] = g_layer_colors[3].s;
                } else if (*command_id == id_custom_set_value) {
                    g_layer_colors[3].h = data[3];
                    g_layer_colors[3].s = data[4];
                    luxqmk_eeprom_save();
                }
                return;

            case USER_VAL_LOGO_MODE:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_logo_mode;
                } else if (*command_id == id_custom_set_value) {
                    g_logo_mode = data[3];
                    luxqmk_eeprom_save();
                }
                return;

            case USER_VAL_LOGO_COLOR_CAPS:
            case USER_VAL_LOGO_COLOR_NUM:
            case USER_VAL_LOGO_COLOR_SCROLL:
            case USER_VAL_LOGO_COLOR_CAPS_NUM:
            case USER_VAL_LOGO_COLOR_CAPS_SCROLL:
            case USER_VAL_LOGO_COLOR_NUM_SCROLL:
            case USER_VAL_LOGO_COLOR_ALL: {
                uint8_t lock_idx = 0;
                switch (value_id) {
                    case USER_VAL_LOGO_COLOR_CAPS:        lock_idx = 1; break;
                    case USER_VAL_LOGO_COLOR_NUM:         lock_idx = 2; break;
                    case USER_VAL_LOGO_COLOR_CAPS_NUM:    lock_idx = 3; break;
                    case USER_VAL_LOGO_COLOR_SCROLL:      lock_idx = 4; break;
                    case USER_VAL_LOGO_COLOR_CAPS_SCROLL: lock_idx = 5; break;
                    case USER_VAL_LOGO_COLOR_NUM_SCROLL:  lock_idx = 6; break;
                    case USER_VAL_LOGO_COLOR_ALL:         lock_idx = 7; break;
                }
                if (*command_id == id_custom_get_value) {
                    data[3] = g_logo_lock_colors[lock_idx].h;
                    data[4] = g_logo_lock_colors[lock_idx].s;
                } else if (*command_id == id_custom_set_value) {
                    g_logo_lock_colors[lock_idx].h = data[3];
                    g_logo_lock_colors[lock_idx].s = data[4];
                    luxqmk_eeprom_save();
                }
                return;
            }

            case USER_VAL_ACTIVE_LAYER:
                if (*command_id == id_custom_get_value) {
                    data[3] = get_highest_layer(layer_state);
                }
                return;

            case USER_VAL_HOST_LEDS:
                if (*command_id == id_custom_get_value) {
                    led_t leds = host_keyboard_led_state();
                    data[3] = leds.caps_lock ? 1 : 0;
                    data[4] = leds.num_lock ? 1 : 0;
                    data[5] = leds.scroll_lock ? 1 : 0;
                }
                return;

            case USER_VAL_WIN_LOCK_MODE:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_win_lock_mode;
                } else if (*command_id == id_custom_set_value) {
                    g_win_lock_mode = data[3];
                    luxqmk_eeprom_save();
                }
                return;

            case USER_VAL_WIN_LOCK_COLOR:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_win_lock_color.h;
                    data[4] = g_win_lock_color.s;
                } else if (*command_id == id_custom_set_value) {
                    g_win_lock_color.h = data[3];
                    g_win_lock_color.s = data[4];
                    luxqmk_eeprom_save();
                }
                return;

            case USER_VAL_WIN_LOCK_STATE:
                if (*command_id == id_custom_get_value) {
                    data[3] = keymap_config.no_gui ? 1 : 0;
                } else if (*command_id == id_custom_set_value) {
                    keymap_config.no_gui = (data[3] != 0);
                    eeconfig_update_keymap(&keymap_config);
                }
                return;

            case USER_VAL_REACTIVE_ENABLE:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_reactive_enable ? 1 : 0;
                } else if (*command_id == id_custom_set_value) {
                    g_reactive_enable = (data[3] != 0);
                    luxqmk_eeprom_save();
                }
                return;

            case USER_VAL_REACTIVE_MODE:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_reactive_mode;
                } else if (*command_id == id_custom_set_value) {
                    g_reactive_mode = data[3];
                    luxqmk_eeprom_save();
                }
                return;

            case USER_VAL_REACTIVE_COLOR:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_reactive_color.h;
                    data[4] = g_reactive_color.s;
                } else if (*command_id == id_custom_set_value) {
                    g_reactive_color.h = data[3];
                    g_reactive_color.s = data[4];
                    luxqmk_eeprom_save();
                }
                return;

            case USER_VAL_REACTIVE_SPEED:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_reactive_speed;
                } else if (*command_id == id_custom_set_value) {
                    g_reactive_speed = data[3];
                    luxqmk_eeprom_save();
                }
                return;

            case USER_VAL_REACTIVE_BLEND:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_reactive_blend;
                } else if (*command_id == id_custom_set_value) {
                    g_reactive_blend = data[3];
                    luxqmk_eeprom_save();
                }
                return;

            default:
                break;
        }
    }

    *command_id = id_unhandled;
}

/**
 * Optional key record processing hook for keymaps
 */
__attribute__((weak))
bool process_record_user_custom(uint16_t keycode, keyrecord_t *record) {
    return true;
}

/**
 * Central QMK key event processor
 */
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case ORGB:
        case QK_USER_0:
            return false;

        case RGB_REV:
        case QK_USER_1:
            if (record->event.pressed) {
                g_custom_rgb_reverse = !g_custom_rgb_reverse;
                luxqmk_eeprom_save();
            }
            return false;

        default:
            return process_record_user_custom(keycode, record);
    }
}

/**
 * RGB Matrix indicator rendering pipeline (Dual-Layer Reactive -> Layer Lighting -> Board Hardware Modules)
 */
#ifdef RGB_MATRIX_ENABLE
bool rgb_matrix_indicators_user(void) {
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
                RGB r_rgb = hsv_to_rgb_nocie(r_hsv);

                uint8_t bg_r, bg_g, bg_b;
                aw20216s_get_color(i, &bg_r, &bg_g, &bg_b);

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

    // 2. Active Layer Key Lighting Overlay
    if (g_layer_lighting_enable) {
        uint8_t current_layer = get_highest_layer(layer_state);
        if (current_layer > 0 && current_layer < 4) {
            uint8_t hue = g_layer_colors[current_layer].h;
            uint8_t sat = g_layer_colors[current_layer].s;
            uint8_t val = rgb_matrix_get_val();
            if (val == 0) {
                val = 255;
            }
            HSV hsv = { hue, sat, val };
            RGB active_rgb = hsv_to_rgb_nocie(hsv);

            for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
                for (uint8_t c = 0; c < MATRIX_COLS; c++) {
                    uint8_t led = g_led_config.matrix_co[r][c];
                    if (led == NO_LED) {
                        continue;
                    }

                    uint16_t keycode = dynamic_keymap_get_keycode(current_layer, r, c);
                    if (keycode == KC_TRNS || keycode == KC_NO) {
                        if (g_layer_dim_level == 0) {
                            rgb_matrix_set_color(led, 0, 0, 0);
                        } else if (g_layer_dim_level < 255) {
                            uint8_t red, green, blue;
                            aw20216s_get_color(led, &red, &green, &blue);
                            uint8_t r_dim = ((uint16_t)red * g_layer_dim_level) / 255;
                            uint8_t g_dim = ((uint16_t)green * g_layer_dim_level) / 255;
                            uint8_t b_dim = ((uint16_t)blue * g_layer_dim_level) / 255;
                            rgb_matrix_set_color(led, r_dim, g_dim, b_dim);
                        }
                    } else {
                        rgb_matrix_set_color(led, active_rgb.r, active_rgb.g, active_rgb.b);
                    }
                }
            }
        }
    }

    // 3. Render hardware board-specific indicators
    board_indicators_render();

    return true;
}
#endif
