#include "luxqmk.h"
#include <lib/lib8tion/lib8tion.h>
#include <string.h>

/**
 * Global configuration variables for lighting, layer colors, and logo mode
 */
bool g_custom_rgb_reverse       = false;
uint8_t g_layer_lighting_enable = 0x0B; // Bitmask for active keys color highlight (Bit 0: Master, Bit 1: L1, Bit 2: L2, Bit 3: L3)
uint8_t g_layer_dim_enable      = 0x0B; // Bitmask for background dimming (Bit 0: Master, Bit 1: L1, Bit 2: L2, Bit 3: L3)
uint8_t g_layer_dim_level       = 128;  // 0..255 (128 = 50% background brightness)
uint8_t g_layer_dim_levels[4]   = { 128, 128, 255, 128 }; // Index 1: Layer 1, Index 2: Layer 2 (default 255 no dimming), Index 3: Layer 3
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
layer_color_t g_win_lock_color = { 0, 0 };         // Default white (#ffffff)

uint8_t g_caps_lock_mode = LUXQMK_DEFAULT_CAPS_LOCK_MODE;
layer_color_t g_caps_lock_color = { 0, 0 }; // Default white (#ffffff)

uint8_t g_num_lock_mode = LUXQMK_DEFAULT_NUM_LOCK_MODE;
layer_color_t g_num_lock_color = { 0, 0 }; // Default white (#ffffff)

uint8_t g_scroll_lock_mode = LUXQMK_DEFAULT_SCROLL_LOCK_MODE;
layer_color_t g_scroll_lock_color = { 0, 0 }; // Default white (#ffffff)

// Dual-Layer Reactive Lighting configuration
bool g_reactive_enable         = false;
uint8_t g_reactive_mode        = REACTIVE_MODE_OFF;
layer_color_t g_reactive_color = { 0, 0 };         // Default white
uint8_t g_reactive_speed       = 128;
uint8_t g_reactive_blend       = REACTIVE_BLEND_ADDITIVE; // 0 = Additive Glow, 1 = Override

// Performance & Switch Debounce configuration (ms)
uint8_t g_debounce_time        = 5; // Default 5ms

#if defined(RGB_MATRIX_ENABLE)
// Direct Software Live Lighting Streaming (LuxQMK Studio Audio Visualizer / PC FX)
bool g_direct_lighting_enable    = false;
uint32_t g_direct_lighting_timer = 0;
RGB g_direct_staging[144]        = {{0, 0, 0}};
RGB g_direct_leds[144]           = {{0, 0, 0}};
#endif

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

#if defined(RGB_MATRIX_ENABLE)
// Per-Key Custom RGB Lighting Profiles State
uint8_t g_active_perkey_profile = 0;
RGB g_per_key_profiles[LUXQMK_PERKEY_PROFILES_COUNT][LUXQMK_PERKEY_MAX_LEDS] = {{{0, 0, 0}}};
RGB g_eeprom_per_key_profiles[LUXQMK_PERKEY_PROFILES_COUNT][LUXQMK_PERKEY_MAX_LEDS] = {{{0, 0, 0}}};
#endif

// Sidelight / Underglow Custom Separate Effect State
bool g_sidelight_custom_enable  = false;
uint8_t g_sidelight_mode        = SIDELIGHT_MODE_FOLLOW_MAIN;
layer_color_t g_sidelight_color = { 0, 255 }; // Default red/custom
uint8_t g_sidelight_speed       = 128;
uint8_t g_sidelight_gradient    = GRADIENT_PRESET_RAINBOW;
bool g_sidelight_reverse        = false;
uint8_t g_sidelight_density     = 128;

// Hardware DIP / Physical Slider Switches Configuration State
dip_switch_config_t g_dip_switch_configs[LUXQMK_MAX_DIP_SWITCHES] = {
    {
        // Switch 0: Left OS Switch (2 positions)
        .pos = {
            { .target_layer = 2,    .swap_gui_alt = 1,    .perkey_profile = 0xFF, .win_lock_state = 0xFF }, // Pos 0: Mac layout (Pos 1)
            { .target_layer = 0,    .swap_gui_alt = 0,    .perkey_profile = 0xFF, .win_lock_state = 0xFF }  // Pos 1: Win layout (Pos 2)
        }
    }
};

/**
 * QMK keyboard post-initialization hook
 */
void keyboard_post_init_user(void) {
    board_init();
    luxqmk_eeprom_load();
    luxqmk_dip_switch_init();
}

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
