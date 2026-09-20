#pragma once

#include QMK_KEYBOARD_H

#ifndef RGB_MATRIX_LED_COUNT
#    if defined(DRIVER_LED_TOTAL)
#        define RGB_MATRIX_LED_COUNT DRIVER_LED_TOTAL
#    else
#        define RGB_MATRIX_LED_COUNT 0
#    endif
#endif

#ifndef DRIVER_LED_TOTAL
#    define DRIVER_LED_TOTAL RGB_MATRIX_LED_COUNT
#endif

/**
 * Custom layer definitions (shared across keyboards)
 */
enum custom_layers {
    _BL = 0,    // Base Layer (Default keymap layout)
    _FL,        // Function Layer (Media & function keys)
    _CL         // Custom Layer (User defined layout)
};

/**
 * Custom keycode definitions
 */
enum custom_keycodes {
    RGB_REV = QK_KB_0, // 0x7E00 -> CUSTOM(0) in VIA / LuxQMK Studio
    NEW_SAFE_RANGE = QK_KB_1
};

typedef struct {
    uint8_t h;
    uint8_t s;
} layer_color_t;

/**
 * Logo LED indicator modes (for keyboards with dedicated logo badge, e.g. GMMK 3)
 */
enum logo_led_mode {
    LOGO_MODE_RGB = 0,             // Follow active RGB animation
    LOGO_MODE_INDICATOR_RGB_IDLE,  // Lock status indicator (RGB animation when no locks active)
    LOGO_MODE_INDICATOR_OFF_IDLE   // Lock status indicator (LED turned off when no locks active)
};

/**
 * Windows Key lock (Win Lock) indicator modes
 */
enum win_lock_led_mode {
    WIN_LOCK_MODE_ANIMATION = 0,   // Default: continue standard RGB animation
    WIN_LOCK_MODE_OFF,             // Turn off Win key LED when locked
    WIN_LOCK_MODE_COLOR            // Set solid custom color when locked
};

/**
 * Dual-Layer reactive lighting overlay modes
 */
enum reactive_overlay_mode {
    REACTIVE_MODE_OFF = 0,
    REACTIVE_MODE_FADE,           // 1: Key-press fade trail (Reactive Simple)
    REACTIVE_MODE_SPLASH,         // 2: Expanding single-color ripple
    REACTIVE_MODE_SPLASH_RAINBOW, // 3: Expanding rainbow ripple
    REACTIVE_MODE_CROSS,          // 4: Expanding cross (+)
    REACTIVE_MODE_NEXUS,          // 5: Expanding nexus star (X)
    REACTIVE_MODE_WIDE,           // 6: Wide expanding wave
    REACTIVE_MODE_HEATMAP         // 7: Typing intensity heatmap
};

enum reactive_blend_mode {
    REACTIVE_BLEND_ADDITIVE = 0,  // Additive glow blend
    REACTIVE_BLEND_OVERRIDE       // Solid color overlay blend
};

/**
 * VIA Custom Channel & Value IDs for LuxQMK WebHID Protocol
 */
#define USER_CUSTOM_CHANNEL              1

#define USER_VAL_RGB_REVERSE             1
#define USER_VAL_LAYER_LIGHTING_ENABLE   2
#define USER_VAL_LAYER_DIM_LEVEL         3
#define USER_VAL_LAYER_1_COLOR           4
#define USER_VAL_LAYER_2_COLOR           5
#define USER_VAL_LAYER_3_COLOR           6

#define USER_VAL_LOGO_MODE               7
#define USER_VAL_LOGO_COLOR_CAPS         8
#define USER_VAL_LOGO_COLOR_NUM          9
#define USER_VAL_LOGO_COLOR_SCROLL       10
#define USER_VAL_LOGO_COLOR_CAPS_NUM     11
#define USER_VAL_LOGO_COLOR_CAPS_SCROLL  12
#define USER_VAL_LOGO_COLOR_NUM_SCROLL   13
#define USER_VAL_LOGO_COLOR_ALL          14
#define USER_VAL_ACTIVE_LAYER            15
#define USER_VAL_HOST_LEDS               16
#define USER_VAL_WIN_LOCK_MODE           17
#define USER_VAL_WIN_LOCK_COLOR          18
#define USER_VAL_WIN_LOCK_STATE          19
#define USER_VAL_REACTIVE_ENABLE         20
#define USER_VAL_REACTIVE_MODE           21
#define USER_VAL_REACTIVE_COLOR          22
#define USER_VAL_REACTIVE_SPEED          23
#define USER_VAL_REACTIVE_BLEND          24

/**
 * LuxQMK Semantic Versioning & Capabilities
 */
#define LUXQMK_VERSION_MAJOR             0
#define LUXQMK_VERSION_MINOR             1
#define LUXQMK_VERSION_PATCH             4
#define LUXQMK_VERSION_STRING            "0.1.4"

#define LUXQMK_CAP_REACTIVE_OVERLAY      (1 << 0)
#define LUXQMK_CAP_DIRECTION_REVERSE     (1 << 1)
#define LUXQMK_CAP_LOGO_LED              (1 << 2)
#define LUXQMK_CAP_WIN_LOCK              (1 << 3)
#define LUXQMK_CAP_LAYER_LIGHTING        (1 << 4)
#define LUXQMK_CAP_HEATMAP               (1 << 5)
#define LUXQMK_CAP_DIRECT_LIGHTING       (1 << 6)

#define USER_VAL_LUXQMK_VERSION          25
#define USER_VAL_QMK_VERSION             26
#define USER_VAL_DEBOUNCE_TIME           27
#define USER_VAL_DIRECT_LIGHTING_ENABLE  28
#define USER_VAL_DIRECT_LIGHTING_BLOCK   29
#define USER_VAL_BOOTLOADER_JUMP         0xFE

/**
 * Global configuration variables
 */
extern uint8_t g_debounce_time;
extern bool g_custom_rgb_reverse;
extern bool g_layer_lighting_enable;
extern uint8_t g_layer_dim_level;
extern layer_color_t g_layer_colors[4];

extern uint8_t g_logo_mode;
extern layer_color_t g_logo_lock_colors[8];

extern uint8_t g_win_lock_mode;
extern layer_color_t g_win_lock_color;

extern bool g_reactive_enable;
extern uint8_t g_reactive_mode;
extern layer_color_t g_reactive_color;
extern uint8_t g_reactive_speed;
extern uint8_t g_reactive_blend;

extern bool g_direct_lighting_enable;
extern uint32_t g_direct_lighting_timer;
extern RGB g_direct_staging[144];
extern RGB g_direct_leds[144];

/**
 * LuxQMK core function declarations
 */
void luxqmk_eeprom_save(void);
void luxqmk_eeprom_load(void);
bool process_record_user_custom(uint16_t keycode, keyrecord_t *record);

/**
 * Board-specific hardware module interface
 */
void board_init(void);
void board_indicators_render(void);
uint8_t board_get_logo_led_index(void);
uint8_t board_get_win_led_index(void);
