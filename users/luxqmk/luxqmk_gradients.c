#include "luxqmk.h"
#include <lib/lib8tion/lib8tion.h>

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

bool luxqmk_is_rainbow_effect(uint8_t mode) {
    return (mode >= 12 && mode <= 22) || mode == 24 || (mode >= 28 && mode <= 30);
}

uint8_t luxqmk_fast_rgb_to_hue(uint8_t r, uint8_t g, uint8_t b) {
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
