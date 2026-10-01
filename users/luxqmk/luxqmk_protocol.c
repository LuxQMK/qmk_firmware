#include "luxqmk.h"
#include "via.h"
#include "bootloader.h"
#include "version.h"
#include <string.h>

#if defined(VIA_ENABLE)
/**
 * Standard VIA command hook
 */
bool via_command_kb(uint8_t *data, uint8_t length) {
    uint8_t command_id = data[0];
    if (command_id == 0x0B) { // id_bootloader_jump
        bootloader_jump();
        return true;
    }
    return false;
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
        if (*command_id == 0x0A) { // id_custom_load / discard RAM changes
            luxqmk_eeprom_reload();
            return;
        }

        switch (value_id) {
            case USER_VAL_BOOTLOADER_JUMP:
                if (*command_id == id_custom_set_value) {
                    bootloader_jump();
                }
                return;

            case USER_VAL_RGB_REVERSE:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_custom_rgb_reverse ? 1 : 0;
                } else if (*command_id == id_custom_set_value) {
                    g_custom_rgb_reverse = (data[3] != 0);
                }
                return;

            case USER_VAL_LAYER_LIGHTING_ENABLE:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_layer_lighting_enable;
                } else if (*command_id == id_custom_set_value) {
                    g_layer_lighting_enable = (data[3] == 1) ? 0x0B : data[3];
                }
                return;

            case USER_VAL_LAYER_DIM_ENABLE:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_layer_dim_enable;
                } else if (*command_id == id_custom_set_value) {
                    g_layer_dim_enable = (data[3] == 1) ? 0x0B : data[3];
                }
                return;

            case USER_VAL_LAYER_DIM_LEVEL:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_layer_dim_levels[1];
                    data[4] = g_layer_dim_levels[2];
                    data[5] = g_layer_dim_levels[3];
                } else if (*command_id == id_custom_set_value) {
                    if (data[5] == 0xAA && data[3] >= 1 && data[3] <= 3) {
                        g_layer_dim_levels[data[3]] = data[4];
                        if (data[3] == 1) {
                            g_layer_dim_level = data[4];
                        }
                    } else {
                        g_layer_dim_levels[1] = data[3];
                        g_layer_dim_level     = data[3];
                        g_layer_dim_levels[2] = data[4];
                        g_layer_dim_levels[3] = data[5];
                    }
                }
                return;

            case USER_VAL_LAYER_1_COLOR:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_layer_colors[1].h;
                    data[4] = g_layer_colors[1].s;
                } else if (*command_id == id_custom_set_value) {
                    g_layer_colors[1].h = data[3];
                    g_layer_colors[1].s = data[4];
                }
                return;

            case USER_VAL_LAYER_2_COLOR:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_layer_colors[2].h;
                    data[4] = g_layer_colors[2].s;
                } else if (*command_id == id_custom_set_value) {
                    g_layer_colors[2].h = data[3];
                    g_layer_colors[2].s = data[4];
                }
                return;

            case USER_VAL_LAYER_3_COLOR:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_layer_colors[3].h;
                    data[4] = g_layer_colors[3].s;
                } else if (*command_id == id_custom_set_value) {
                    g_layer_colors[3].h = data[3];
                    g_layer_colors[3].s = data[4];
                }
                return;

            case USER_VAL_LOGO_MODE:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_logo_mode;
                } else if (*command_id == id_custom_set_value) {
                    g_logo_mode = data[3];
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
                }
                return;
            }

            case USER_VAL_ACTIVE_LAYER:
                if (*command_id == id_custom_get_value) {
                    data[3] = get_highest_layer(layer_state | default_layer_state);
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
                }
                return;

            case USER_VAL_WIN_LOCK_COLOR:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_win_lock_color.h;
                    data[4] = g_win_lock_color.s;
                } else if (*command_id == id_custom_set_value) {
                    g_win_lock_color.h = data[3];
                    g_win_lock_color.s = data[4];
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

            case USER_VAL_CAPS_LOCK_MODE:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_caps_lock_mode;
                } else if (*command_id == id_custom_set_value) {
                    g_caps_lock_mode = data[3];
                }
                return;

            case USER_VAL_CAPS_LOCK_COLOR:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_caps_lock_color.h;
                    data[4] = g_caps_lock_color.s;
                } else if (*command_id == id_custom_set_value) {
                    g_caps_lock_color.h = data[3];
                    g_caps_lock_color.s = data[4];
                }
                return;

            case USER_VAL_NUM_LOCK_MODE:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_num_lock_mode;
                } else if (*command_id == id_custom_set_value) {
                    g_num_lock_mode = data[3];
                }
                return;

            case USER_VAL_NUM_LOCK_COLOR:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_num_lock_color.h;
                    data[4] = g_num_lock_color.s;
                } else if (*command_id == id_custom_set_value) {
                    g_num_lock_color.h = data[3];
                    g_num_lock_color.s = data[4];
                }
                return;

            case USER_VAL_SCROLL_LOCK_MODE:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_scroll_lock_mode;
                } else if (*command_id == id_custom_set_value) {
                    g_scroll_lock_mode = data[3];
                }
                return;

            case USER_VAL_SCROLL_LOCK_COLOR:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_scroll_lock_color.h;
                    data[4] = g_scroll_lock_color.s;
                } else if (*command_id == id_custom_set_value) {
                    g_scroll_lock_color.h = data[3];
                    g_scroll_lock_color.s = data[4];
                }
                return;

            case USER_VAL_REACTIVE_ENABLE:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_reactive_enable ? 1 : 0;
                } else if (*command_id == id_custom_set_value) {
                    g_reactive_enable = (data[3] != 0);
                }
                return;

            case USER_VAL_REACTIVE_MODE:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_reactive_mode;
                } else if (*command_id == id_custom_set_value) {
                    g_reactive_mode = data[3];
                }
                return;

            case USER_VAL_REACTIVE_COLOR:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_reactive_color.h;
                    data[4] = g_reactive_color.s;
                } else if (*command_id == id_custom_set_value) {
                    g_reactive_color.h = data[3];
                    g_reactive_color.s = data[4];
                }
                return;

            case USER_VAL_REACTIVE_SPEED:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_reactive_speed;
                } else if (*command_id == id_custom_set_value) {
                    g_reactive_speed = data[3];
                }
                return;

            case USER_VAL_REACTIVE_BLEND:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_reactive_blend;
                } else if (*command_id == id_custom_set_value) {
                    g_reactive_blend = data[3];
                }
                return;

            case USER_VAL_LUXQMK_VERSION:
                if (*command_id == id_custom_get_value) {
                    data[3] = LUXQMK_VERSION_MAJOR;
                    data[4] = LUXQMK_VERSION_MINOR;
                    data[5] = LUXQMK_VERSION_PATCH;
                    uint16_t caps = 0;
#ifdef RGB_MATRIX_ENABLE
                    caps |= (LUXQMK_CAP_REACTIVE_OVERLAY | LUXQMK_CAP_DIRECTION_REVERSE | LUXQMK_CAP_LAYER_LIGHTING | LUXQMK_CAP_HEATMAP | LUXQMK_CAP_DIRECT_LIGHTING | LUXQMK_CAP_MULTI_GRADIENTS | LUXQMK_CAP_PERKEY_PROFILES | LUXQMK_CAP_NKRO);
                    if (board_get_logo_led_index() != 255) {
                        caps |= LUXQMK_CAP_LOGO_LED;
                    }
                    if (board_get_win_led_index() != 255) {
                        caps |= LUXQMK_CAP_WIN_LOCK;
                    }
                    if (board_has_sidelights()) {
                        caps |= LUXQMK_CAP_SIDELIGHTS;
                    }
#endif
#if defined(DIP_SWITCH_ENABLE)
                    caps |= LUXQMK_CAP_DIP_SWITCHES;
#endif
                    data[6] = (uint8_t)(caps & 0xFF);
                    data[7] = (uint8_t)((caps >> 8) & 0xFF);
                }
                return;

            case USER_VAL_QMK_VERSION:
                if (*command_id == id_custom_get_value) {
                    const char *ver = QMK_VERSION;
                    uint8_t i = 0;
                    while (ver[i] != '\0' && (3 + i) < length) {
                        data[3 + i] = (uint8_t)ver[i];
                        i++;
                    }
                    if ((3 + i) < length) {
                        data[3 + i] = '\0';
                    }
                }
                return;

            case USER_VAL_DEBOUNCE_TIME:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_debounce_time;
                } else if (*command_id == id_custom_set_value) {
                    g_debounce_time = (data[3] > 30) ? 5 : data[3];
                }
                return;

            case USER_VAL_GRADIENT_PRESET:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_active_gradient;
                } else if (*command_id == id_custom_set_value) {
                    g_active_gradient = (data[3] < GRADIENT_PRESETS_TOTAL) ? data[3] : GRADIENT_PRESET_RAINBOW;
                }
                return;

            case USER_VAL_GRADIENT_CUSTOM_COUNT: {
                uint8_t prof = (data[3] >= LUXQMK_USER_GRADIENTS_COUNT) ? 0 : data[3];
                if (*command_id == id_custom_get_value) {
                    data[4] = g_user_gradients[prof].count;
                } else if (*command_id == id_custom_set_value) {
                    uint8_t cnt = data[4];
                    if (cnt < 2) cnt = 2;
                    if (cnt > LUXQMK_MAX_GRADIENT_STOPS) cnt = LUXQMK_MAX_GRADIENT_STOPS;
                    g_user_gradients[prof].count = cnt;
                }
                return;
            }

            case USER_VAL_GRADIENT_CUSTOM_STOP: {
                uint8_t prof = (data[3] >= LUXQMK_USER_GRADIENTS_COUNT) ? 0 : data[3];
                uint8_t stop = (data[4] >= LUXQMK_MAX_GRADIENT_STOPS) ? 0 : data[4];
                if (*command_id == id_custom_get_value) {
                    data[5] = g_user_gradients[prof].stops[stop].pos;
                    data[6] = g_user_gradients[prof].stops[stop].r;
                    data[7] = g_user_gradients[prof].stops[stop].g;
                    data[8] = g_user_gradients[prof].stops[stop].b;
                } else if (*command_id == id_custom_set_value) {
                    g_user_gradients[prof].stops[stop].pos = data[5];
                    g_user_gradients[prof].stops[stop].r   = data[6];
                    g_user_gradients[prof].stops[stop].g   = data[7];
                    g_user_gradients[prof].stops[stop].b   = data[8];
                }
                return;
            }

            case USER_VAL_GRADIENT_SAVE_EEPROM: {
                uint8_t prof = (data[3] >= LUXQMK_USER_GRADIENTS_COUNT) ? 0 : data[3];
                if (*command_id == id_custom_set_value) {
                    g_eeprom_user_gradients[prof] = g_user_gradients[prof];
                    luxqmk_eeprom_save();
                }
                return;
            }

            case USER_VAL_EFFECT_DENSITY:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_effect_density;
                } else if (*command_id == id_custom_set_value) {
                    g_effect_density = (data[3] == 0) ? 128 : data[3];
                }
                return;

#if defined(RGB_MATRIX_ENABLE)
            case USER_VAL_DIRECT_LIGHTING_ENABLE:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_direct_lighting_enable ? 1 : 0;
                } else if (*command_id == id_custom_set_value) {
                    g_direct_lighting_enable = (data[3] != 0);
                    g_direct_lighting_timer  = timer_read32();
                    if (g_direct_lighting_enable) {
                        memset(g_direct_staging, 0, sizeof(g_direct_staging));
                        memset(g_direct_leds, 0, sizeof(g_direct_leds));
                        rgb_matrix_set_color_all(0, 0, 0);
                    }
                }
                return;

            case USER_VAL_DIRECT_LIGHTING_BLOCK:
                if (*command_id == id_custom_set_value) {
                    uint8_t raw_idx   = data[3];
                    uint8_t start_idx = raw_idx & 0x7F;
                    bool is_flush     = (raw_idx & 0x80) != 0;
                    uint8_t count     = data[4];
                    g_direct_lighting_enable = true;
                    g_direct_lighting_timer  = timer_read32();
                    for (uint8_t i = 0; i < count; i++) {
                        uint8_t led_idx = start_idx + i;
                        if (led_idx < 144) {
                            g_direct_staging[led_idx].r = data[5 + (i * 3) + 0];
                            g_direct_staging[led_idx].g = data[5 + (i * 3) + 1];
                            g_direct_staging[led_idx].b = data[5 + (i * 3) + 2];
                        }
                    }
                    if (is_flush || (start_idx + count >= DRIVER_LED_TOTAL)) {
                        memcpy(g_direct_leds, g_direct_staging, sizeof(g_direct_leds));
                    }
                }
                return;

            case USER_VAL_PERKEY_PROFILE_GET_BLOCK: {
                uint8_t prof = (data[3] >= LUXQMK_PERKEY_PROFILES_COUNT) ? 0 : data[3];
                uint8_t start_idx = data[4];
                uint8_t count = data[5];
                if (count > 8) count = 8;
                for (uint8_t i = 0; i < count; i++) {
                    uint8_t led_idx = start_idx + i;
                    if (led_idx < LUXQMK_PERKEY_MAX_LEDS) {
                        data[6 + (i * 3) + 0] = g_per_key_profiles[prof][led_idx].r;
                        data[6 + (i * 3) + 1] = g_per_key_profiles[prof][led_idx].g;
                        data[6 + (i * 3) + 2] = g_per_key_profiles[prof][led_idx].b;
                    } else {
                        data[6 + (i * 3) + 0] = 0;
                        data[6 + (i * 3) + 1] = 0;
                        data[6 + (i * 3) + 2] = 0;
                    }
                }
                return;
            }

            case USER_VAL_PERKEY_PROFILE_SET_BLOCK: {
                if (*command_id == id_custom_set_value) {
                    uint8_t prof = (data[3] >= LUXQMK_PERKEY_PROFILES_COUNT) ? 0 : data[3];
                    uint8_t start_idx = data[4];
                    uint8_t count = data[5];
                    if (count > 8) count = 8;
                    for (uint8_t i = 0; i < count; i++) {
                        uint8_t led_idx = start_idx + i;
                        if (led_idx < LUXQMK_PERKEY_MAX_LEDS) {
                            g_per_key_profiles[prof][led_idx].r = data[6 + (i * 3) + 0];
                            g_per_key_profiles[prof][led_idx].g = data[6 + (i * 3) + 1];
                            g_per_key_profiles[prof][led_idx].b = data[6 + (i * 3) + 2];
                        }
                    }
                }
                return;
            }

            case USER_VAL_PERKEY_PROFILE_SAVE_EEPROM: {
                if (*command_id == id_custom_set_value) {
                    uint8_t prof = data[3];
                    if (prof < LUXQMK_PERKEY_PROFILES_COUNT) {
                        memcpy(g_eeprom_per_key_profiles[prof], g_per_key_profiles[prof], sizeof(g_per_key_profiles[prof]));
                    } else {
                        memcpy(g_eeprom_per_key_profiles, g_per_key_profiles, sizeof(g_per_key_profiles));
                    }
                    luxqmk_eeprom_save();
                }
                return;
            }

            case USER_VAL_PERKEY_PROFILE_ACTIVE: {
                if (*command_id == id_custom_get_value) {
                    data[3] = g_active_perkey_profile;
                } else if (*command_id == id_custom_set_value) {
                    g_active_perkey_profile = (data[3] < LUXQMK_PERKEY_PROFILES_COUNT) ? data[3] : 0;
                }
                return;
            }
#endif

            case USER_VAL_SIDELIGHT_ENABLE:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_sidelight_custom_enable ? 1 : 0;
                } else if (*command_id == id_custom_set_value) {
                    g_sidelight_custom_enable = (data[3] != 0);
                }
                return;

            case USER_VAL_SIDELIGHT_MODE:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_sidelight_mode;
                } else if (*command_id == id_custom_set_value) {
                    g_sidelight_mode = (data[3] < SIDELIGHT_MODES_TOTAL) ? data[3] : SIDELIGHT_MODE_FOLLOW_MAIN;
                }
                return;

            case USER_VAL_SIDELIGHT_COLOR:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_sidelight_color.h;
                    data[4] = g_sidelight_color.s;
                } else if (*command_id == id_custom_set_value) {
                    g_sidelight_color.h = data[3];
                    g_sidelight_color.s = data[4];
                }
                return;

            case USER_VAL_SIDELIGHT_SPEED:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_sidelight_speed;
                } else if (*command_id == id_custom_set_value) {
                    g_sidelight_speed = data[3];
                }
                return;

            case USER_VAL_SIDELIGHT_GRADIENT:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_sidelight_gradient;
                } else if (*command_id == id_custom_set_value) {
                    g_sidelight_gradient = (data[3] < GRADIENT_PRESETS_TOTAL) ? data[3] : GRADIENT_PRESET_RAINBOW;
                }
                return;

            case USER_VAL_SIDELIGHT_REVERSE:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_sidelight_reverse ? 1 : 0;
                } else if (*command_id == id_custom_set_value) {
                    g_sidelight_reverse = (data[3] != 0);
                }
                return;

            case USER_VAL_SIDELIGHT_DENSITY:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_sidelight_density;
                } else if (*command_id == id_custom_set_value) {
                    g_sidelight_density = (data[3] == 0) ? 128 : data[3];
                }
                return;

            case USER_VAL_NKRO_STATE:
                if (*command_id == id_custom_get_value) {
#if defined(NKRO_ENABLE)
                    data[3] = keymap_config.nkro ? 1 : 0;
#else
                    data[3] = 0;
#endif
                } else if (*command_id == id_custom_set_value) {
#if defined(NKRO_ENABLE)
                    keymap_config.nkro = (data[3] != 0);
                    eeconfig_update_keymap(&keymap_config);
                    clear_keyboard();
#endif
                }
                return;

            case USER_VAL_DIP_SWITCH_COUNT:
                if (*command_id == id_custom_get_value) {
#if defined(DIP_SWITCH_ENABLE)
                    data[3] = LUXQMK_MAX_DIP_SWITCHES;
#else
                    data[3] = 0;
#endif
                }
                return;

            case USER_VAL_DIP_SWITCH_STATE:
                if (*command_id == id_custom_get_value) {
#if defined(DIP_SWITCH_ENABLE) && defined(DIP_SWITCH_PINS)
                    uint8_t sw_idx = data[3];
                    static const pin_t dip_pins[] = DIP_SWITCH_PINS;
                    uint8_t num_switches = sizeof(dip_pins) / sizeof(pin_t);
                    if (sw_idx < num_switches) {
                        data[4] = (gpio_read_pin(dip_pins[sw_idx]) == 0) ? 1 : 0;
                    } else {
                        data[4] = 0;
                    }
#else
                    data[4] = 0;
#endif
                }
                return;

            case USER_VAL_DIP_SWITCH_GET_POS: {
                uint8_t sw_idx  = (data[3] >= LUXQMK_MAX_DIP_SWITCHES) ? 0 : data[3];
                uint8_t pos_idx = (data[4] >= LUXQMK_MAX_DIP_POSITIONS) ? 0 : data[4];
                if (*command_id == id_custom_get_value) {
                    data[5] = g_dip_switch_configs[sw_idx].pos[pos_idx].target_layer;
                    data[6] = g_dip_switch_configs[sw_idx].pos[pos_idx].swap_gui_alt;
                    data[7] = g_dip_switch_configs[sw_idx].pos[pos_idx].perkey_profile;
                    data[8] = g_dip_switch_configs[sw_idx].pos[pos_idx].win_lock_state;
                }
                return;
            }

            case USER_VAL_DIP_SWITCH_SET_POS: {
                uint8_t sw_idx  = (data[3] >= LUXQMK_MAX_DIP_SWITCHES) ? 0 : data[3];
                uint8_t pos_idx = (data[4] >= LUXQMK_MAX_DIP_POSITIONS) ? 0 : data[4];
                if (*command_id == id_custom_set_value) {
                    g_dip_switch_configs[sw_idx].pos[pos_idx].target_layer   = data[5];
                    g_dip_switch_configs[sw_idx].pos[pos_idx].swap_gui_alt   = data[6];
                    g_dip_switch_configs[sw_idx].pos[pos_idx].perkey_profile = data[7];
                    g_dip_switch_configs[sw_idx].pos[pos_idx].win_lock_state = data[8];
                }
                return;
            }

            case USER_VAL_DIP_SWITCH_SAVE_EEPROM: {
                if (*command_id == id_custom_set_value) {
                    luxqmk_eeprom_save();
                }
                return;
            }

            case USER_VAL_RELOAD_EEPROM: {
                if (*command_id == id_custom_set_value) {
                    luxqmk_eeprom_reload();
                }
                return;
            }

            default:
                break;
        }
    }

    *command_id = id_unhandled;
}
#endif
