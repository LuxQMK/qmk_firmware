#include "luxqmk.h"
#include "via.h"
#include "dynamic_keymap.h"
#include "drivers/led/aw20216s.h"
#include "color.h"
#include "eeprom.h"
#include "version.h"
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

// Performance & Switch Debounce configuration (ms)
uint8_t g_debounce_time        = 5; // Default 5ms

// Direct Software Live Lighting Streaming (LuxQMK Studio Audio Visualizer / PC FX)
bool g_direct_lighting_enable    = false;
uint32_t g_direct_lighting_timer = 0;
RGB g_direct_staging[144]        = {{0, 0, 0}};
RGB g_direct_leds[144]           = {{0, 0, 0}};

// Multi-Stop Gradient Global State & Default User Profiles
uint8_t g_active_gradient = GRADIENT_PRESET_RAINBOW;
uint8_t g_effect_density  = 128; // 128 = 1.0x standard spatial density
user_gradient_t g_user_gradients[LUXQMK_USER_GRADIENTS_COUNT] = {
    {
        .count = 3,
        .stops = {
            { 0,   0,   255, 255 }, // Cyan #00FFFF (0%)
            { 85,  255, 0,   128 }, // Hot Pink / Magenta #FF0080 (33.3%)
            { 170, 255, 255, 0   }  // Electric Yellow #FFFF00 (66.7%)
        }
    },
    {
        .count = 4,
        .stops = {
            { 0,   18,  10,  143 }, // Deep Indigo #120A8F (0%)
            { 75,  255, 0,   128 }, // Hot Pink #FF0080 (29.4%)
            { 155, 255, 110, 0   }, // Radiant Orange #FF6E00 (60.8%)
            { 225, 255, 215, 0   }  // Synth Gold #FFD700 (88.2%)
        }
    }
};
user_gradient_t g_eeprom_user_gradients[LUXQMK_USER_GRADIENTS_COUNT];

// Per-Key Custom RGB Lighting Profiles State
uint8_t g_active_perkey_profile = 0;
RGB g_per_key_profiles[LUXQMK_PERKEY_PROFILES_COUNT][LUXQMK_PERKEY_MAX_LEDS] = {{{0, 0, 0}}};
RGB g_eeprom_per_key_profiles[LUXQMK_PERKEY_PROFILES_COUNT][LUXQMK_PERKEY_MAX_LEDS] = {{{0, 0, 0}}};

// Sidelight / Underglow Custom Separate Effect State
bool g_sidelight_custom_enable  = false;
uint8_t g_sidelight_mode        = SIDELIGHT_MODE_FOLLOW_MAIN;
layer_color_t g_sidelight_color = { 0, 255 }; // Default red/custom
uint8_t g_sidelight_speed       = 128;
uint8_t g_sidelight_gradient    = GRADIENT_PRESET_RAINBOW;
bool g_sidelight_reverse        = false;
uint8_t g_sidelight_density     = 128;

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

/**
 * Built-in Gradient Stop Tables (stored in Flash ROM)
 * Equal interval distribution (0..255) for smooth seamless circular animations
 */
static const gradient_stop_t PROGMEM GRADIENT_CYBERPUNK_STOPS[] = {
    { 0,   0,   255, 255 }, // Cyan #00FFFF (0%)
    { 85,  255, 0,   128 }, // Magenta #FF0080 (33.3%)
    { 170, 255, 255, 0   }  // Electric Yellow #FFFF00 (66.7%)
};

static const gradient_stop_t PROGMEM GRADIENT_SYNTHWAVE_STOPS[] = {
    { 0,   75,  0,   130 }, // Indigo / Deep Purple (0%)
    { 64,  255, 0,   128 }, // Hot Pink (25%)
    { 128, 255, 100, 0   }, // Neon Orange (50%)
    { 192, 255, 215, 0   }  // Golden Yellow (75%)
};

static const gradient_stop_t PROGMEM GRADIENT_SUNSET_STOPS[] = {
    { 0,   45,  10,  85  }, // Twilight Purple (0%)
    { 85,  235, 45,  55  }, // Sunset Crimson (33.3%)
    { 170, 255, 190, 40  }  // Amber Gold (66.7%)
};

static const gradient_stop_t PROGMEM GRADIENT_TOXIC_LIME_STOPS[] = {
    { 0,   166, 255, 0   }, // Acid Lime #A6FF00 (0%)
    { 85,  243, 255, 0   }, // Toxic Yellow #F3FF00 (33.3%)
    { 170, 0,   229, 58  }  // Radioactive Green #00E53A (66.7%)
};

static const gradient_stop_t PROGMEM GRADIENT_OCEAN_STOPS[] = {
    { 0,   0,   20,  80  }, // Deep Navy (0%)
    { 64,  0,   140, 255 }, // Azure Blue (25%)
    { 128, 0,   255, 200 }, // Aqua Mint (50%)
    { 192, 135, 206, 250 }  // Sky Blue (75%)
};

static const gradient_stop_t PROGMEM GRADIENT_FIRE_ICE_STOPS[] = {
    { 0,   0,   200, 255 }, // Ice Blue (0%)
    { 64,  255, 255, 255 }, // Frost White (25%)
    { 128, 255, 80,  0   }, // Blazing Flame (50%)
    { 192, 180, 0,   0   }  // Deep Crimson (75%)
};

static const gradient_stop_t PROGMEM GRADIENT_PASTEL_STOPS[] = {
    { 0,   218, 182, 252 }, // Lavender #DAB6FC (0%)
    { 64,  168, 240, 219 }, // Mint #A8F0DB (25%)
    { 128, 255, 209, 178 }, // Peach #FFD1B2 (50%)
    { 192, 255, 182, 193 }  // Pastel Pink #FFB6C1 (75%)
};

/**
 * Fast Multi-Stop Gradient Sampler (Real-time 60-120 FPS piecewise linear interpolation)
 */
RGB luxqmk_sample_gradient(uint8_t gradient_id, uint8_t phase) {
    if (gradient_id == GRADIENT_PRESET_RAINBOW) {
        hsv_t hsv = { phase, 255, rgb_matrix_config.hsv.v };
        return hsv_to_rgb(hsv);
    }

    const gradient_stop_t *stops_ptr = NULL;
    uint8_t stop_count = 0;
    bool is_progmem = true;
    gradient_stop_t ram_stops[LUXQMK_MAX_GRADIENT_STOPS];

    switch (gradient_id) {
        case GRADIENT_PRESET_CYBERPUNK:
            stops_ptr = GRADIENT_CYBERPUNK_STOPS;
            stop_count = sizeof(GRADIENT_CYBERPUNK_STOPS) / sizeof(gradient_stop_t);
            break;
        case GRADIENT_PRESET_SYNTHWAVE:
            stops_ptr = GRADIENT_SYNTHWAVE_STOPS;
            stop_count = sizeof(GRADIENT_SYNTHWAVE_STOPS) / sizeof(gradient_stop_t);
            break;
        case GRADIENT_PRESET_SUNSET:
            stops_ptr = GRADIENT_SUNSET_STOPS;
            stop_count = sizeof(GRADIENT_SUNSET_STOPS) / sizeof(gradient_stop_t);
            break;
        case GRADIENT_PRESET_TOXIC_LIME:
            stops_ptr = GRADIENT_TOXIC_LIME_STOPS;
            stop_count = sizeof(GRADIENT_TOXIC_LIME_STOPS) / sizeof(gradient_stop_t);
            break;
        case GRADIENT_PRESET_OCEAN:
            stops_ptr = GRADIENT_OCEAN_STOPS;
            stop_count = sizeof(GRADIENT_OCEAN_STOPS) / sizeof(gradient_stop_t);
            break;
        case GRADIENT_PRESET_FIRE_ICE:
            stops_ptr = GRADIENT_FIRE_ICE_STOPS;
            stop_count = sizeof(GRADIENT_FIRE_ICE_STOPS) / sizeof(gradient_stop_t);
            break;
        case GRADIENT_PRESET_PASTEL:
            stops_ptr = GRADIENT_PASTEL_STOPS;
            stop_count = sizeof(GRADIENT_PASTEL_STOPS) / sizeof(gradient_stop_t);
            break;
        case GRADIENT_PRESET_CUSTOM_1:
        case GRADIENT_PRESET_CUSTOM_2: {
            uint8_t prof_idx = (gradient_id == GRADIENT_PRESET_CUSTOM_1) ? 0 : 1;
            stop_count = g_user_gradients[prof_idx].count;
            if (stop_count < 2) stop_count = 2;
            if (stop_count > LUXQMK_MAX_GRADIENT_STOPS) stop_count = LUXQMK_MAX_GRADIENT_STOPS;
            for (uint8_t s = 0; s < stop_count; s++) {
                ram_stops[s] = g_user_gradients[prof_idx].stops[s];
            }
            stops_ptr = ram_stops;
            is_progmem = false;
            break;
        }
        default: {
            hsv_t hsv = { phase, 255, rgb_matrix_config.hsv.v };
            return hsv_to_rgb(hsv);
        }
    }

    if (stop_count == 0 || stops_ptr == NULL) {
        hsv_t hsv = { phase, 255, rgb_matrix_config.hsv.v };
        return hsv_to_rgb(hsv);
    }

    // Helper macro to fetch a stop from either PROGMEM or RAM
    #define GET_STOP(idx, target) do { \
        if (is_progmem) { \
            memcpy_P(&(target), &stops_ptr[idx], sizeof(gradient_stop_t)); \
        } else { \
            target = stops_ptr[idx]; \
        } \
    } while(0)

    gradient_stop_t s0, s1;
    GET_STOP(0, s0);
    GET_STOP(stop_count - 1, s1);

    RGB out_rgb = { s0.r, s0.g, s0.b };

    // Boundary conditions: before first stop or after last stop
    if (phase <= s0.pos) {
        // Seamless wrap: interpolate from last stop to first stop
        uint16_t wrap_span = (255 - s1.pos) + s0.pos;
        if (wrap_span == 0) {
            out_rgb = (RGB){ s0.r, s0.g, s0.b };
        } else {
            uint8_t progress = (uint8_t)(((uint16_t)(phase + (255 - s1.pos)) * 255) / wrap_span);
            out_rgb = (RGB){
                lerp8by8(s1.r, s0.r, progress),
                lerp8by8(s1.g, s0.g, progress),
                lerp8by8(s1.b, s0.b, progress)
            };
        }
    } else if (phase >= s1.pos) {
        // Seamless wrap: interpolate from last stop to first stop
        uint16_t wrap_span = (255 - s1.pos) + s0.pos;
        if (wrap_span == 0) {
            out_rgb = (RGB){ s1.r, s1.g, s1.b };
        } else {
            uint8_t progress = (uint8_t)(((uint16_t)(phase - s1.pos) * 255) / wrap_span);
            out_rgb = (RGB){
                lerp8by8(s1.r, s0.r, progress),
                lerp8by8(s1.g, s0.g, progress),
                lerp8by8(s1.b, s0.b, progress)
            };
        }
    } else {
        // Piecewise linear segment search
        for (uint8_t i = 0; i < stop_count - 1; i++) {
            gradient_stop_t cur, next;
            GET_STOP(i, cur);
            GET_STOP(i + 1, next);

            if (phase >= cur.pos && phase <= next.pos) {
                uint8_t span = next.pos - cur.pos;
                if (span == 0) {
                    out_rgb = (RGB){ cur.r, cur.g, cur.b };
                } else {
                    uint8_t progress = (uint8_t)(((uint16_t)(phase - cur.pos) * 255) / span);
                    out_rgb = (RGB){
                        lerp8by8(cur.r, next.r, progress),
                        lerp8by8(cur.g, next.g, progress),
                        lerp8by8(cur.b, next.b, progress)
                    };
                }
                break;
            }
        }
    }

    #undef GET_STOP

    // Scale by hardware brightness
    if (rgb_matrix_config.hsv.v < 255) {
        out_rgb.r = scale8(out_rgb.r, rgb_matrix_config.hsv.v);
        out_rgb.g = scale8(out_rgb.g, rgb_matrix_config.hsv.v);
        out_rgb.b = scale8(out_rgb.b, rgb_matrix_config.hsv.v);
    }

    return out_rgb;
}

/**
 * Save user custom configuration to persistent EEPROM storage
 */
void luxqmk_eeprom_save(void) {
#if defined(VIA_ENABLE) && defined(VIA_EEPROM_CUSTOM_CONFIG_SIZE)
    uint8_t header[112];
    memset(header, 0, sizeof(header));

    header[0] = g_custom_rgb_reverse ? 1 : 0;
    header[1] = g_layer_lighting_enable ? 1 : 0;
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

    via_update_custom_config(header, 0, sizeof(header));
    via_update_custom_config(g_eeprom_per_key_profiles, 112, sizeof(g_eeprom_per_key_profiles));
#endif
}

/**
 * Load user custom configuration from EEPROM upon startup
 */
void luxqmk_eeprom_load(void) {
#if defined(VIA_ENABLE) && defined(VIA_EEPROM_CUSTOM_CONFIG_SIZE)
    uint8_t header[112];
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

        luxqmk_init_default_perkey_profiles();
        luxqmk_eeprom_save();
    } else {
        g_custom_rgb_reverse    = (rev != 0);
        g_layer_lighting_enable = (enable != 0);
        g_layer_dim_level       = dim;
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
            g_win_lock_color = (layer_color_t){ 0, 255 };
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

        // Per-Key Custom RGB Lighting Profiles Deserialization
        uint8_t active_prof = header[101];
        g_active_perkey_profile = (active_prof < LUXQMK_PERKEY_PROFILES_COUNT) ? active_prof : 0;

        via_read_custom_config(g_per_key_profiles, 112, sizeof(g_per_key_profiles));
        memcpy(g_eeprom_per_key_profiles, g_per_key_profiles, sizeof(g_per_key_profiles));
    }
#endif
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

/**
 * Handle custom VIA / LuxQMK Studio WebHID protocol commands
 */
#if defined(VIA_ENABLE)
bool via_command_kb(uint8_t *data, uint8_t length) {
    uint8_t command_id = data[0];
    if (command_id == 0x0B) { // id_bootloader_jump
        bootloader_jump();
        return true;
    }
    return false;
}

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
                    data[3] = g_layer_lighting_enable ? 1 : 0;
                } else if (*command_id == id_custom_set_value) {
                    g_layer_lighting_enable = (data[3] != 0);
                }
                return;

            case USER_VAL_LAYER_DIM_LEVEL:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_layer_dim_level;
                } else if (*command_id == id_custom_set_value) {
                    g_layer_dim_level = data[3];
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
                    caps |= (LUXQMK_CAP_REACTIVE_OVERLAY | LUXQMK_CAP_DIRECTION_REVERSE | LUXQMK_CAP_LAYER_LIGHTING | LUXQMK_CAP_HEATMAP | LUXQMK_CAP_DIRECT_LIGHTING | LUXQMK_CAP_MULTI_GRADIENTS | LUXQMK_CAP_PERKEY_PROFILES);
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

            case USER_VAL_DIRECT_LIGHTING_ENABLE:
                if (*command_id == id_custom_get_value) {
                    data[3] = g_direct_lighting_enable ? 1 : 0;
                } else if (*command_id == id_custom_set_value) {
                    g_direct_lighting_enable = (data[3] != 0);
                    g_direct_lighting_timer  = timer_read32();
                    if (g_direct_lighting_enable) {
                        memset(g_direct_staging, 0, sizeof(g_direct_staging));
                        memset(g_direct_leds, 0, sizeof(g_direct_leds));
#ifdef RGB_MATRIX_ENABLE
                        rgb_matrix_set_color_all(0, 0, 0);
#endif
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

/**
 * Custom Dynamic Symmetric Defer Per-Key Debounce Engine (sym_defer_pk)
 * Completely eliminates switch contact chatter and double-clicks by requiring
 * stable contact state for g_debounce_time ms before actuating or releasing.
 */
static uint8_t s_debounce_counters[MATRIX_ROWS * MATRIX_COLS];
static bool    s_counters_need_update = false;
static bool    s_cooked_changed       = false;

void debounce_init(void) {
    memset(s_debounce_counters, 0, sizeof(s_debounce_counters));
}

static inline void luxqmk_update_debounce_counters(matrix_row_t raw[], matrix_row_t cooked[], uint8_t elapsed_time) {
    s_counters_need_update = false;

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        uint16_t row_offset = row * MATRIX_COLS;
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            uint16_t index = row_offset + col;
            if (s_debounce_counters[index] != 0) {
                if (s_debounce_counters[index] <= elapsed_time) {
                    s_debounce_counters[index] = 0;
                    matrix_row_t col_mask    = (MATRIX_ROW_SHIFTER << col);
                    matrix_row_t cooked_next = (cooked[row] & ~col_mask) | (raw[row] & col_mask);
                    s_cooked_changed |= cooked[row] ^ cooked_next;
                    cooked[row] = cooked_next;
                } else {
                    s_debounce_counters[index] -= elapsed_time;
                    s_counters_need_update = true;
                }
            }
        }
    }
}

static inline void luxqmk_start_debounce_counters(matrix_row_t raw[], matrix_row_t cooked[]) {
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        uint16_t     row_offset = row * MATRIX_COLS;
        matrix_row_t delta      = raw[row] ^ cooked[row];

        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            uint16_t     index    = row_offset + col;
            matrix_row_t col_mask = (MATRIX_ROW_SHIFTER << col);

            if (delta & col_mask) {
                s_debounce_counters[index] = g_debounce_time;
                s_counters_need_update     = true;
            } else {
                s_debounce_counters[index] = 0;
            }
        }
    }
}

bool debounce(matrix_row_t raw[], matrix_row_t cooked[], bool changed) {
    if (g_debounce_time == 0) {
        if (changed) {
            memcpy(cooked, raw, sizeof(matrix_row_t) * MATRIX_ROWS);
            return true;
        }
        return false;
    }

    static fast_timer_t last_time;
    bool                updated_last = false;
    s_cooked_changed                 = false;

    if (s_counters_need_update) {
        fast_timer_t now          = timer_read_fast();
        fast_timer_t elapsed_time = TIMER_DIFF_FAST(now, last_time);

        last_time    = now;
        updated_last = true;

        if (elapsed_time > 0) {
            luxqmk_update_debounce_counters(raw, cooked, (uint8_t)MIN(elapsed_time, UINT8_MAX));
        }
    }

    if (changed) {
        if (!updated_last) {
            last_time = timer_read_fast();
        }

        luxqmk_start_debounce_counters(raw, cooked);
    }

    return s_cooked_changed;
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
        case RGB_REV:
        case QK_USER_0:
            if (record->event.pressed) {
                g_custom_rgb_reverse = !g_custom_rgb_reverse;
                luxqmk_eeprom_save();
            }
            return false;

        case RGB_DEN_INC:
        case QK_USER_1:
            if (record->event.pressed) {
                if (g_effect_density <= 255 - 16) {
                    g_effect_density += 16;
                } else {
                    g_effect_density = 255;
                }
                luxqmk_eeprom_save();
            }
            return false;

        case RGB_DEN_DEC:
        case QK_USER_2:
            if (record->event.pressed) {
                if (g_effect_density >= 32 + 16) {
                    g_effect_density -= 16;
                } else {
                    g_effect_density = 32;
                }
                luxqmk_eeprom_save();
            }
            return false;

        case RGB_DEN_STEP:
        case QK_USER_3:
            if (record->event.pressed) {
                if (g_effect_density < 64) g_effect_density = 64;
                else if (g_effect_density < 96) g_effect_density = 96;
                else if (g_effect_density < 128) g_effect_density = 128;
                else if (g_effect_density < 160) g_effect_density = 160;
                else if (g_effect_density < 192) g_effect_density = 192;
                else if (g_effect_density < 224) g_effect_density = 224;
                else if (g_effect_density < 255) g_effect_density = 255;
                else g_effect_density = 64;
                luxqmk_eeprom_save();
            }
            return false;

        case RGB_DEN_RST:
        case QK_USER_4:
            if (record->event.pressed) {
                g_effect_density = 128;
                luxqmk_eeprom_save();
            }
            return false;

        case RGB_GRAD_STEP:
        case QK_USER_5:
            if (record->event.pressed) {
                g_active_gradient = (g_active_gradient + 1) % GRADIENT_PRESETS_TOTAL;
                luxqmk_eeprom_save();
            }
            return false;

        case RGB_REACT_STEP:
        case QK_USER_6:
            if (record->event.pressed) {
                if (!g_reactive_enable || g_reactive_mode == REACTIVE_MODE_OFF) {
                    g_reactive_mode = REACTIVE_MODE_FADE;
                    g_reactive_enable = true;
                } else if (g_reactive_mode >= REACTIVE_MODE_HEATMAP) {
                    g_reactive_mode = REACTIVE_MODE_OFF;
                    g_reactive_enable = false;
                } else {
                    g_reactive_mode++;
                    g_reactive_enable = true;
                }
                luxqmk_eeprom_save();
            }
            return false;

        case RGB_RSPD_INC:
        case QK_USER_7:
            if (record->event.pressed) {
                if (g_reactive_speed <= 127 - 16) {
                    g_reactive_speed += 16;
                } else {
                    g_reactive_speed = 127;
                }
                luxqmk_eeprom_save();
            }
            return false;

        case RGB_RSPD_DEC:
        case QK_USER_8:
            if (record->event.pressed) {
                if (g_reactive_speed >= 16 + 16) {
                    g_reactive_speed -= 16;
                } else {
                    g_reactive_speed = 16;
                }
                luxqmk_eeprom_save();
            }
            return false;

        case RGB_RSPD_STEP:
        case QK_USER_9:
            if (record->event.pressed) {
                if (g_reactive_speed < 32) g_reactive_speed = 32;
                else if (g_reactive_speed < 64) g_reactive_speed = 64;
                else if (g_reactive_speed < 96) g_reactive_speed = 96;
                else if (g_reactive_speed < 127) g_reactive_speed = 127;
                else g_reactive_speed = 32;
                luxqmk_eeprom_save();
            }
            return false;

        default:
            return process_record_user_custom(keycode, record);
    }
}

static inline bool luxqmk_is_rainbow_effect(uint8_t mode) {
    return (mode >= 12 && mode <= 22) || mode == 24 || (mode >= 28 && mode <= 30);
}

static inline uint8_t luxqmk_fast_rgb_to_hue(uint8_t r, uint8_t g, uint8_t b) {
    uint8_t max_v = r > g ? (r > b ? r : b) : (g > b ? g : b);
    uint8_t min_v = r < g ? (r < b ? r : b) : (g < b ? g : b);
    uint8_t delta = max_v - min_v;
    if (delta == 0) return 0;
    int16_t h;
    if (max_v == r) {
        h = (int16_t)(g - b) * 43 / delta;
    } else if (max_v == g) {
        h = 85 + (int16_t)(b - r) * 43 / delta;
    } else {
        h = 171 + (int16_t)(r - g) * 43 / delta;
    }
    if (h < 0) h += 256;
    return (uint8_t)(h & 0xFF);
}

/**
 * RGB Matrix indicator rendering pipeline (Dual-Layer Reactive -> Layer Lighting -> Board Hardware Modules)
 */
#ifdef RGB_MATRIX_ENABLE
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
        uint8_t half_count = (ug_count > 0) ? (ug_count / 2) : 1;

        for (uint8_t i = led_min; i < led_max; i++) {
            if (i == logo_idx) {
                continue;
            }
            if (i < DRIVER_LED_TOTAL && HAS_FLAGS(g_led_config.flags[i], LED_FLAG_UNDERGLOW)) {
                uint8_t y_scaled = 0;
                uint8_t dist_scaled = 0;
                uint8_t opt_step = 0;
                bool is_hidden = false;

                if (i < ug_first + half_count) {
                    // LEFT STRIP: Visible optical window is segments 1..8 (7 steps, center at 4.5)
                    uint8_t k_left = i - ug_first; // 0..9
                    if (k_left < 1 || k_left > 8) {
                        is_hidden = true;
                    }
                    int16_t norm_left = (int16_t)k_left - 1; // 0 at segment 1, 7 at segment 8
                    if (norm_left < 0) norm_left = 0;
                    if (norm_left > 7) norm_left = 7;
                    opt_step = (uint8_t)norm_left;
                    y_scaled = (uint8_t)(((uint32_t)norm_left * 255 * density) / (7 * 128));

                    uint8_t dist_sym = (uint8_t)abs((int16_t)(2 * k_left) - 9); // distance from center (4.5)
                    if (dist_sym > 7) dist_sym = 7;
                    dist_scaled = (uint8_t)(((uint32_t)dist_sym * 255 * density) / (7 * 128));
                } else {
                    // RIGHT STRIP: Visible optical window is segments 2..9 (7 steps, center at 5.5)
                    uint8_t k_right = (ug_first + ug_count - 1) - i; // 0..9
                    if (k_right < 2 || k_right > 9) {
                        is_hidden = true;
                    }
                    int16_t norm_right = (int16_t)k_right - 2; // 0 at segment 2, 7 at segment 9
                    if (norm_right < 0) norm_right = 0;
                    if (norm_right > 7) norm_right = 7;
                    opt_step = (uint8_t)norm_right;
                    y_scaled = (uint8_t)(((uint32_t)norm_right * 255 * density) / (7 * 128));

                    uint8_t dist_sym = (uint8_t)abs((int16_t)(2 * k_right) - 11); // distance from center (5.5)
                    if (dist_sym > 7) dist_sym = 7;
                    dist_scaled = (uint8_t)(((uint32_t)dist_sym * 255 * density) / (7 * 128));
                }

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

    // 3. Always ensure board-specific hardware indicators (Logo badge, Win Lock) are rendered
    board_indicators_render();

    return true;
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
                        if (g_layer_dim_level == 0) {
                            rgb_matrix_set_color(led, 0, 0, 0);
                        } else if (g_layer_dim_level < 255) {
                            uint8_t red = 0, green = 0, blue = 0;
#if defined(AW20216S_LED_COUNT)
                            aw20216s_get_color(led, &red, &green, &blue);
#endif
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

bool board_has_sidelights(void) {
#ifdef RGB_MATRIX_ENABLE
    for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {
        if (HAS_FLAGS(g_led_config.flags[i], LED_FLAG_UNDERGLOW)) {
            return true;
        }
    }
#endif
    return false;
}
#endif

