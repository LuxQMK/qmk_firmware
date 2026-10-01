#include "luxqmk.h"
#include "via.h"
#include "dynamic_keymap.h"
#include "eeprom.h"
#include <string.h>

#if defined(RGB_MATRIX_ENABLE)
/**
 * Initialize default gaming profiles for Profile 1 (FPS), Profile 2 (MOBA), Profile 3 (MMO/RPG)
 * Resolves physical key assignments from matrix and active base keymap.
 */
void luxqmk_init_default_perkey_profiles(void) {
    memset(g_per_key_profiles, 0, sizeof(g_per_key_profiles));
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        for (uint8_t c = 0; c < MATRIX_COLS; c++) {
            uint8_t led = g_led_config.matrix_co[r][c];
            if (led == NO_LED || led >= LUXQMK_PERKEY_MAX_LEDS) continue;

#if defined(DYNAMIC_KEYMAP_ENABLE)
            uint16_t kc = dynamic_keymap_get_keycode(0, r, c);
#else
            uint16_t kc = keymap_key_to_keycode(0, (keypos_t){ .row = r, .col = c });
#endif
            // Profile 0: FPS / Shooter Game Mode (CS2, Valorant, CoD, Apex)
            if (kc == KC_W || kc == KC_A || kc == KC_S || kc == KC_D) {
                g_per_key_profiles[0][led] = (RGB){ 0, 255, 255 }; // Neon Cyan (WASD)
            } else if (kc == KC_SPC || kc == KC_LSFT || kc == KC_LCTL) {
                g_per_key_profiles[0][led] = (RGB){ 255, 102, 0 }; // Flame Orange (Jump/Sprint/Crouch)
            } else if (kc == KC_R || kc == KC_G || kc == KC_Q || kc == KC_E || kc == KC_F || kc == KC_TAB) {
                g_per_key_profiles[0][led] = (RGB){ 255, 255, 0 }; // Electric Yellow (Interact/Abilities)
            } else if (kc >= KC_1 && kc <= KC_5) {
                g_per_key_profiles[0][led] = (RGB){ 255, 0, 50 };  // Hot Crimson (Weapon select)
            } else if (kc == KC_ESC) {
                g_per_key_profiles[0][led] = (RGB){ 255, 0, 0 };   // Pure Red
            }

            // Profile 1: MOBA / Battle Arena Mode (LoL, Dota 2)
            if (kc == KC_Q || kc == KC_W || kc == KC_E || kc == KC_R) {
                g_per_key_profiles[1][led] = (RGB){ 0, 229, 255 }; // Bright Cyan (Abilities / Ultimate)
            } else if (kc == KC_D || kc == KC_F) {
                g_per_key_profiles[1][led] = (RGB){ 255, 215, 0 }; // Radiant Gold (Summoner Spells)
            } else if (kc >= KC_1 && kc <= KC_7) {
                g_per_key_profiles[1][led] = (RGB){ 255, 0, 128 }; // Hot Pink (Active items / Wards)
            } else if (kc == KC_B || kc == KC_P) {
                g_per_key_profiles[1][led] = (RGB){ 155, 81, 224 }; // Purple (Recall / Shop)
            } else if (kc == KC_TAB || kc == KC_SPC || kc == KC_ESC) {
                g_per_key_profiles[1][led] = (RGB){ 255, 255, 255 }; // White
            }

            // Profile 2: MMO / RPG / Strategy Mode (WoW, Diablo, PoE)
            if ((kc >= KC_1 && kc <= KC_0) || kc == KC_MINS || kc == KC_EQL) {
                g_per_key_profiles[2][led] = (RGB){ 255, 50, 0 };  // Flame Red (Action bar)
            } else if (kc == KC_Q || kc == KC_W || kc == KC_E || kc == KC_R || kc == KC_T || kc == KC_Y ||
                       kc == KC_F || kc == KC_G || kc == KC_Z || kc == KC_X || kc == KC_C || kc == KC_V) {
                g_per_key_profiles[2][led] = (RGB){ 0, 255, 136 }; // Emerald/Lime (Expanded hotkeys)
            } else if (kc == KC_LSFT || kc == KC_LCTL || kc == KC_LALT) {
                g_per_key_profiles[2][led] = (RGB){ 180, 0, 255 }; // Deep Violet (Combo modifiers)
            } else if (kc == KC_M || kc == KC_I || kc == KC_C || kc == KC_P || kc == KC_ESC || kc == KC_SPC) {
                g_per_key_profiles[2][led] = (RGB){ 255, 204, 0 }; // Gold (Map / Inventory / Stats)
            } else if (kc == KC_UP || kc == KC_DOWN || kc == KC_LEFT || kc == KC_RGHT) {
                g_per_key_profiles[2][led] = (RGB){ 0, 191, 255 }; // Azure Blue (Navigation)
            }
        }
    }

    // Default Sidelights & Logo LED values for all 3 profiles
    uint8_t logo_idx = board_get_logo_led_index();
    for (uint8_t i = 0; i < DRIVER_LED_TOTAL && i < LUXQMK_PERKEY_MAX_LEDS; i++) {
        bool is_matrix_key = false;
        for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
            for (uint8_t c = 0; c < MATRIX_COLS; c++) {
                if (g_led_config.matrix_co[r][c] == i) {
                    is_matrix_key = true;
                    break;
                }
            }
            if (is_matrix_key) break;
        }
        if (!is_matrix_key || i == logo_idx) {
            g_per_key_profiles[0][i] = (RGB){ 0, 255, 255 }; // Cyan
            g_per_key_profiles[1][i] = (RGB){ 255, 215, 0 }; // Gold
            g_per_key_profiles[2][i] = (RGB){ 0, 255, 136 }; // Emerald
        }
    }

    memcpy(g_eeprom_per_key_profiles, g_per_key_profiles, sizeof(g_per_key_profiles));
}
#endif

/**
 * Save user custom configuration to persistent EEPROM storage
 */
void luxqmk_eeprom_save(void) {
#if defined(VIA_ENABLE) && defined(VIA_EEPROM_CUSTOM_CONFIG_SIZE)
    uint8_t header[160];
    memset(header, 0, sizeof(header));

    header[0] = g_custom_rgb_reverse ? 1 : 0;
    header[1] = g_layer_lighting_enable;
    header[2] = g_layer_dim_level;
    header[3] = g_layer_colors[1].h;
    header[4] = g_layer_colors[1].s;
    header[5] = g_layer_colors[2].h;
    header[6] = g_layer_colors[2].s;
    header[7] = g_layer_colors[3].h;
    header[8] = g_layer_colors[3].s;

    header[9] = g_logo_mode;
    for (uint8_t i = 1; i < 8; i++) {
        header[10 + ((i - 1) * 2)] = g_logo_lock_colors[i].h;
        header[11 + ((i - 1) * 2)] = g_logo_lock_colors[i].s;
    }

    header[24] = g_win_lock_mode;
    header[25] = g_win_lock_color.h;
    header[26] = g_win_lock_color.s;

    header[27] = g_reactive_enable ? 1 : 0;
    header[28] = g_reactive_mode;
    header[29] = g_reactive_color.h;
    header[30] = g_reactive_color.s;
    header[31] = (g_reactive_speed & 0x7F) | ((g_reactive_blend & 0x01) << 7);
    header[32] = g_debounce_time;

    // Multi-Stop Gradient Persistence (Committed EEPROM Profiles)
    header[33] = g_active_gradient;
    // Profile 0
    header[34] = g_eeprom_user_gradients[0].count;
    for (uint8_t s = 0; s < LUXQMK_MAX_GRADIENT_STOPS; s++) {
        uint8_t base = 35 + (s * 4);
        header[base + 0] = g_eeprom_user_gradients[0].stops[s].pos;
        header[base + 1] = g_eeprom_user_gradients[0].stops[s].r;
        header[base + 2] = g_eeprom_user_gradients[0].stops[s].g;
        header[base + 3] = g_eeprom_user_gradients[0].stops[s].b;
    }
    // Profile 1
    header[67] = g_eeprom_user_gradients[1].count;
    for (uint8_t s = 0; s < LUXQMK_MAX_GRADIENT_STOPS; s++) {
        uint8_t base = 68 + (s * 4);
        header[base + 0] = g_eeprom_user_gradients[1].stops[s].pos;
        header[base + 1] = g_eeprom_user_gradients[1].stops[s].r;
        header[base + 2] = g_eeprom_user_gradients[1].stops[s].g;
        header[base + 3] = g_eeprom_user_gradients[1].stops[s].b;
    }

    // Effect Spatial Density (0..255, 128 = 1.0x)
    header[100] = g_effect_density;

    // Active Per-Key Profile ID (0..2)
    header[101] = g_active_perkey_profile;

    // Dedicated Independent Sidelights Persistence
    header[102] = g_sidelight_custom_enable ? 1 : 0;
    header[103] = g_sidelight_mode;
    header[104] = g_sidelight_color.h;
    header[105] = g_sidelight_color.s;
    header[106] = g_sidelight_speed;
    header[107] = g_sidelight_gradient;
    header[108] = g_sidelight_reverse ? 1 : 0;
    header[109] = g_sidelight_density;

    // Hardware DIP / Physical Slider Switches Configuration (bytes 110..133)
    for (uint8_t s = 0; s < LUXQMK_MAX_DIP_SWITCHES; s++) {
        for (uint8_t p = 0; p < LUXQMK_MAX_DIP_POSITIONS; p++) {
            uint8_t base = 110 + (s * LUXQMK_MAX_DIP_POSITIONS * 4) + (p * 4);
            header[base + 0] = g_dip_switch_configs[s].pos[p].target_layer;
            header[base + 1] = g_dip_switch_configs[s].pos[p].swap_gui_alt;
            header[base + 2] = g_dip_switch_configs[s].pos[p].perkey_profile;
            header[base + 3] = g_dip_switch_configs[s].pos[p].win_lock_state;
        }
    }

    // Lock Indicators Configuration (bytes 134..142)
    header[134] = g_caps_lock_mode;
    header[135] = g_caps_lock_color.h;
    header[136] = g_caps_lock_color.s;
    header[137] = g_num_lock_mode;
    header[138] = g_num_lock_color.h;
    header[139] = g_num_lock_color.s;
    header[140] = g_scroll_lock_mode;
    header[141] = g_scroll_lock_color.h;
    header[142] = g_scroll_lock_color.s;

    // Per-Layer Dimming Levels (bytes 143..144) & Dimming Enable Bitmask (byte 145)
    header[143] = g_layer_dim_levels[2];
    header[144] = g_layer_dim_levels[3];
    header[145] = g_layer_dim_enable;

    via_update_custom_config(header, 0, sizeof(header));
#if defined(RGB_MATRIX_ENABLE)
    via_update_custom_config(g_eeprom_per_key_profiles, 160, sizeof(g_eeprom_per_key_profiles));
#endif
#endif
}

/**
 * Load user custom configuration from EEPROM upon startup
 */
void luxqmk_eeprom_load(void) {
#if defined(VIA_ENABLE) && defined(VIA_EEPROM_CUSTOM_CONFIG_SIZE)
    uint8_t header[160];
    via_read_custom_config(header, 0, sizeof(header));

    uint8_t rev    = header[0];
    uint8_t enable = header[1];
    uint8_t dim    = header[2];
    uint8_t l1_h   = header[3];
    uint8_t l1_s   = header[4];
    uint8_t l2_h   = header[5];
    uint8_t l2_s   = header[6];
    uint8_t l3_h   = header[7];
    uint8_t l3_s   = header[8];

    uint8_t logo_mode = header[9];

    uint8_t win_lock_mode = header[24];
    uint8_t win_lock_h    = header[25];
    uint8_t win_lock_s    = header[26];

    uint8_t r_enable = header[27];
    uint8_t r_mode   = header[28];
    uint8_t r_h      = header[29];
    uint8_t r_s      = header[30];
    uint8_t r_spd    = header[31];
    uint8_t db       = header[32];

    uint8_t grad_preset = header[33];

    uint8_t side_enable = header[102];
    uint8_t side_mode   = header[103];
    uint8_t side_h      = header[104];
    uint8_t side_s      = header[105];
    uint8_t side_spd    = header[106];
    uint8_t side_grad   = header[107];
    uint8_t side_rev    = header[108];
    uint8_t side_dens   = header[109];

    if (enable == 0xFF) {
        // Uninitialized EEPROM defaults
        g_custom_rgb_reverse    = false;
        g_layer_lighting_enable = 0x0B;
        g_layer_dim_enable      = 0x0B;
        g_layer_dim_level       = 128;
        g_layer_dim_levels[1]   = 128;
        g_layer_dim_levels[2]   = 255;
        g_layer_dim_levels[3]   = 128;
        g_layer_colors[1]       = (layer_color_t){ 28, 255 };
        g_layer_colors[2]       = (layer_color_t){ 128, 255 };
        g_layer_colors[3]       = (layer_color_t){ 200, 255 };

        g_logo_mode             = LOGO_MODE_RGB;
        g_logo_lock_colors[0]   = (layer_color_t){ 0, 0 };
        g_logo_lock_colors[1]   = (layer_color_t){ 0, 255 };    // Caps (#FF0000)
        g_logo_lock_colors[2]   = (layer_color_t){ 165, 255 };  // Num (#001EFF)
        g_logo_lock_colors[3]   = (layer_color_t){ 8, 255 };    // Caps + Num (#FF3000)
        g_logo_lock_colors[4]   = (layer_color_t){ 77, 255 };   // Scroll (#30FF00)
        g_logo_lock_colors[5]   = (layer_color_t){ 43, 255 };   // Caps + Scroll (#FCFF00)
        g_logo_lock_colors[6]   = (layer_color_t){ 137, 255 };  // Num + Scroll (#00C6FF)
        g_logo_lock_colors[7]   = (layer_color_t){ 0, 0 };      // All (#FFFFFF)

        g_win_lock_mode         = WIN_LOCK_MODE_ANIMATION;
        g_win_lock_color        = (layer_color_t){ 0, 0 };

        g_caps_lock_mode        = LUXQMK_DEFAULT_CAPS_LOCK_MODE;
        g_caps_lock_color       = (layer_color_t){ 0, 0 };
        g_num_lock_mode         = LUXQMK_DEFAULT_NUM_LOCK_MODE;
        g_num_lock_color        = (layer_color_t){ 0, 0 };
        g_scroll_lock_mode      = LUXQMK_DEFAULT_SCROLL_LOCK_MODE;
        g_scroll_lock_color     = (layer_color_t){ 0, 0 };

        g_reactive_enable       = false;
        g_reactive_mode         = REACTIVE_MODE_OFF;
        g_reactive_color        = (layer_color_t){ 0, 0 };
        g_reactive_speed        = 128;
        g_reactive_blend        = REACTIVE_BLEND_ADDITIVE;
        g_debounce_time         = 5;

        g_active_gradient       = GRADIENT_PRESET_RAINBOW;
        g_effect_density        = 128;
        g_active_perkey_profile = 0;

        g_sidelight_custom_enable = false;
        g_sidelight_mode          = SIDELIGHT_MODE_FOLLOW_MAIN;
        g_sidelight_color         = (layer_color_t){ 0, 255 };
        g_sidelight_speed         = 128;
        g_sidelight_gradient      = GRADIENT_PRESET_RAINBOW;
        g_sidelight_reverse       = false;
        g_sidelight_density       = 128;

        g_dip_switch_configs[0].pos[0] = (dip_switch_pos_config_t){ .target_layer = 2,    .swap_gui_alt = 1,    .perkey_profile = 0xFF, .win_lock_state = 0xFF }; // Pos 0 (Mac)
        g_dip_switch_configs[0].pos[1] = (dip_switch_pos_config_t){ .target_layer = 0,    .swap_gui_alt = 0,    .perkey_profile = 0xFF, .win_lock_state = 0xFF }; // Pos 1 (Win)

#if defined(RGB_MATRIX_ENABLE)
        luxqmk_init_default_perkey_profiles();
#endif
        luxqmk_eeprom_save();
    } else {
        g_custom_rgb_reverse    = (rev != 0);
        g_layer_lighting_enable = (enable == 1) ? 0x0B : enable;
        uint8_t dim_en          = header[145];
        g_layer_dim_enable      = (dim_en == 0xFF || dim_en == 1) ? 0x0B : dim_en;
        g_layer_dim_level       = dim;
        g_layer_dim_levels[1]   = dim;
        uint8_t dim2            = header[143];
        uint8_t dim3            = header[144];
        g_layer_dim_levels[2]   = (dim2 == 0xFF) ? 255 : dim2;
        g_layer_dim_levels[3]   = (dim3 == 0xFF) ? dim : dim3;
        g_layer_colors[1]       = (layer_color_t){ l1_h, l1_s };
        g_layer_colors[2]       = (layer_color_t){ l2_h, l2_s };
        g_layer_colors[3]       = (layer_color_t){ l3_h, l3_s };
        g_debounce_time         = (db == 0xFF || db > 30) ? 5 : db;

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
                g_logo_lock_colors[i].h = header[10 + ((i - 1) * 2)];
                g_logo_lock_colors[i].s = header[11 + ((i - 1) * 2)];
            }
        }

        if (win_lock_mode == 0xFF) {
            g_win_lock_mode  = WIN_LOCK_MODE_ANIMATION;
            g_win_lock_color = (layer_color_t){ 0, 0 };
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
        } else {
            g_reactive_enable = (r_enable != 0);
            g_reactive_mode   = r_mode;
            g_reactive_color  = (layer_color_t){ r_h, r_s };
            g_reactive_speed  = (r_spd & 0x7F) < 10 ? 128 : (r_spd & 0x7F);
            g_reactive_blend  = (r_spd >> 7) & 0x01;
        }

        // Multi-Stop Gradient Deserialization
        if (grad_preset != 0xFF && grad_preset < GRADIENT_PRESETS_TOTAL) {
            g_active_gradient = grad_preset;
        } else {
            g_active_gradient = GRADIENT_PRESET_RAINBOW;
        }

        // Profile 0
        uint8_t p0_cnt = header[34];
        if (p0_cnt >= 2 && p0_cnt <= LUXQMK_MAX_GRADIENT_STOPS) {
            g_user_gradients[0].count = p0_cnt;
            for (uint8_t s = 0; s < LUXQMK_MAX_GRADIENT_STOPS; s++) {
                uint8_t base = 35 + (s * 4);
                g_user_gradients[0].stops[s].pos = header[base + 0];
                g_user_gradients[0].stops[s].r   = header[base + 1];
                g_user_gradients[0].stops[s].g   = header[base + 2];
                g_user_gradients[0].stops[s].b   = header[base + 3];
            }
        }
        // Profile 1
        uint8_t p1_cnt = header[67];
        if (p1_cnt >= 2 && p1_cnt <= LUXQMK_MAX_GRADIENT_STOPS) {
            g_user_gradients[1].count = p1_cnt;
            for (uint8_t s = 0; s < LUXQMK_MAX_GRADIENT_STOPS; s++) {
                uint8_t base = 68 + (s * 4);
                g_user_gradients[1].stops[s].pos = header[base + 0];
                g_user_gradients[1].stops[s].r   = header[base + 1];
                g_user_gradients[1].stops[s].g   = header[base + 2];
                g_user_gradients[1].stops[s].b   = header[base + 3];
            }
        }

        g_eeprom_user_gradients[0] = g_user_gradients[0];
        g_eeprom_user_gradients[1] = g_user_gradients[1];

        uint8_t density = header[100];
        g_effect_density = (density == 0xFF || density == 0) ? 128 : density;

        // Dedicated Independent Sidelights Deserialization
        if (side_enable == 0xFF) {
            g_sidelight_custom_enable = false;
            g_sidelight_mode          = SIDELIGHT_MODE_FOLLOW_MAIN;
            g_sidelight_color         = (layer_color_t){ 0, 255 };
            g_sidelight_speed         = 128;
            g_sidelight_gradient      = GRADIENT_PRESET_RAINBOW;
            g_sidelight_reverse       = false;
            g_sidelight_density       = 128;
        } else {
            g_sidelight_custom_enable = (side_enable != 0);
            g_sidelight_mode          = (side_mode < SIDELIGHT_MODES_TOTAL) ? side_mode : SIDELIGHT_MODE_FOLLOW_MAIN;
            g_sidelight_color         = (layer_color_t){ side_h, side_s };
            g_sidelight_speed         = (side_spd == 0xFF) ? 128 : side_spd;
            g_sidelight_gradient      = (side_grad < GRADIENT_PRESETS_TOTAL) ? side_grad : GRADIENT_PRESET_RAINBOW;
            g_sidelight_reverse       = (side_rev != 0);
            g_sidelight_density       = (side_dens == 0xFF || side_dens == 0) ? 128 : side_dens;
        }

        // Hardware DIP / Physical Slider Switches Deserialization (bytes 110..133)
        for (uint8_t s = 0; s < LUXQMK_MAX_DIP_SWITCHES; s++) {
            for (uint8_t p = 0; p < LUXQMK_MAX_DIP_POSITIONS; p++) {
                uint8_t base = 110 + (s * LUXQMK_MAX_DIP_POSITIONS * 4) + (p * 4);
                uint8_t t  = header[base + 0];
                uint8_t m  = header[base + 1];
                uint8_t pk = header[base + 2];
                uint8_t w  = header[base + 3];

                if (t <= 3 || t == 0xFF) g_dip_switch_configs[s].pos[p].target_layer = t;
                if (m <= 1 || m == 0xFF) g_dip_switch_configs[s].pos[p].swap_gui_alt = m;
                if (pk <= 2 || pk == 0xFF) g_dip_switch_configs[s].pos[p].perkey_profile = pk;
                if (w <= 1 || w == 0xFF) g_dip_switch_configs[s].pos[p].win_lock_state = w;
            }
        }

        // Lock Indicators Deserialization (bytes 134..142)
        uint8_t caps_mode = header[134];
        if (caps_mode == 0xFF) {
            g_caps_lock_mode   = LUXQMK_DEFAULT_CAPS_LOCK_MODE;
            g_caps_lock_color  = (layer_color_t){ 0, 0 };
        } else {
            g_caps_lock_mode   = caps_mode;
            g_caps_lock_color  = (layer_color_t){ header[135], header[136] };
        }

        uint8_t num_mode = header[137];
        if (num_mode == 0xFF) {
            g_num_lock_mode    = LUXQMK_DEFAULT_NUM_LOCK_MODE;
            g_num_lock_color   = (layer_color_t){ 0, 0 };
        } else {
            g_num_lock_mode    = num_mode;
            g_num_lock_color   = (layer_color_t){ header[138], header[139] };
        }

        uint8_t scroll_mode = header[140];
        if (scroll_mode == 0xFF) {
            g_scroll_lock_mode = LUXQMK_DEFAULT_SCROLL_LOCK_MODE;
            g_scroll_lock_color= (layer_color_t){ 0, 0 };
        } else {
            g_scroll_lock_mode = scroll_mode;
            g_scroll_lock_color= (layer_color_t){ header[141], header[142] };
        }

#if defined(RGB_MATRIX_ENABLE)
        // Per-Key Custom RGB Lighting Profiles Deserialization
        uint8_t active_prof = header[101];
        g_active_perkey_profile = (active_prof < LUXQMK_PERKEY_PROFILES_COUNT) ? active_prof : 0;

        via_read_custom_config(g_per_key_profiles, 160, sizeof(g_per_key_profiles));
        memcpy(g_eeprom_per_key_profiles, g_per_key_profiles, sizeof(g_per_key_profiles));
#endif
    }
#endif
}

/**
 * Hardware DIP / Physical Slider Switch state apply and init
 */
void luxqmk_dip_switch_apply(uint8_t index, bool active) {
    if (index >= LUXQMK_MAX_DIP_SWITCHES) return;
    uint8_t pos_idx = active ? 1 : 0;
    dip_switch_pos_config_t *cfg = &g_dip_switch_configs[index].pos[pos_idx];

    // 1. Layer switching
    if (cfg->target_layer <= 3) {
        default_layer_set(1UL << cfg->target_layer);
    }

    // 2. GUI/Alt Swap (Mac layout modifier swap)
    if (cfg->swap_gui_alt == 1) {
        keymap_config.swap_lalt_lgui = 1;
        eeconfig_update_keymap(&keymap_config);
    } else if (cfg->swap_gui_alt == 0) {
        keymap_config.swap_lalt_lgui = 0;
        eeconfig_update_keymap(&keymap_config);
    }

    // 3. Per-Key Profile switching
#if defined(RGB_MATRIX_ENABLE)
    if (cfg->perkey_profile <= 2) {
        g_active_perkey_profile = cfg->perkey_profile;
    }
#endif

    // 4. Win Lock switching
    if (cfg->win_lock_state == 1) {
        keymap_config.no_gui = 1;
        eeconfig_update_keymap(&keymap_config);
    } else if (cfg->win_lock_state == 0) {
        keymap_config.no_gui = 0;
        eeconfig_update_keymap(&keymap_config);
    }
}

void luxqmk_dip_switch_init(void) {
#if defined(DIP_SWITCH_ENABLE)
    dip_switch_read(true);
    #if defined(DIP_SWITCH_PINS)
    static const pin_t dip_pins[] = DIP_SWITCH_PINS;
    uint8_t num_switches = sizeof(dip_pins) / sizeof(pin_t);
    for (uint8_t i = 0; i < num_switches && i < LUXQMK_MAX_DIP_SWITCHES; i++) {
        bool active = (gpio_read_pin(dip_pins[i]) == 0);
        luxqmk_dip_switch_apply(i, active);
    }
    #endif
#endif
}

#if defined(DIP_SWITCH_ENABLE)
bool dip_switch_update_user(uint8_t index, bool active) {
    luxqmk_dip_switch_apply(index, active);
    // Return false to prevent board-level default handler (e.g. Keychron dip_switch_update_kb) from overriding LuxQMK EEPROM config
    return false;
}
#endif

/**
 * QMK EEPROM initialization hook (called on initial EEPROM clear / factory reset)
 */
void eeconfig_init_user(void) {
#if defined(NKRO_ENABLE)
    keymap_config.nkro = 1;
    eeconfig_update_keymap(&keymap_config);
#endif
    luxqmk_eeprom_save();
}

/**
 * Reload EEPROM configuration into RAM and re-apply RGB matrix settings live
 */
void luxqmk_eeprom_reload(void) {
    luxqmk_eeprom_load();
#ifdef RGB_MATRIX_ENABLE
    eeconfig_read_rgb_matrix(&rgb_matrix_config);
    if (rgb_matrix_config.enable) {
        rgb_matrix_enable_noeeprom();
        rgb_matrix_mode_noeeprom(rgb_matrix_config.mode);
    } else {
        rgb_matrix_disable_noeeprom();
    }
    rgb_matrix_sethsv_noeeprom(rgb_matrix_config.hsv.h, rgb_matrix_config.hsv.s, rgb_matrix_config.hsv.v);
    rgb_matrix_set_speed_noeeprom(rgb_matrix_config.speed);
#endif
}
