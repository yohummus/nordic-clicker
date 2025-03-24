#ifndef LEDS_H
#define LEDS_H

#include <stdbool.h>
#include <stdint.h>

#define LEDS_RGB(r, g, b) ((struct leds_color_t){r, g, b})

enum leds_led_t {
    LEDS_D1,
    LEDS_D2,
};

enum leds_pattern_t {
    LEDS_SOLID,
    LEDS_FLASH,
    LEDS_BREATHE,
};

struct leds_color_t {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

typedef void (*leds_finished_cb_t)(bool aborted);

/**
 * @brief Play a pattern on an LED.
 *
 * If a pattern is currently still playing on any LED, it will be aborted and the pattern finished callback will be
 * called before playing the new pattern. Note that if the LEDS_SOLID pattern is selected or reps is -1, the callback
 * will only be called once a new pattern is played. You can only play a pattern on a single LED at a time.
 *
 * @param led The LED to play the pattern on.
 * @param pattern The pattern to play.
 * @param color The color of the pattern.
 * @param reps The number of times to repeat the pattern. Set to -1 to repeat indefinitely and 0 to switch LEDs off.
 * @param cb The callback to call when the pattern (with all its repetitions) has finished playing. Can be set to NULL;
 *
 * @retval 0 If successful.
 * @retval <0 Error code if playing the pattern failed.
 */
int leds_play(enum leds_led_t led, enum leds_pattern_t pattern, struct leds_color_t color, int reps,
              leds_finished_cb_t cb);

/**
 * @brief Stops the pattern currently playing, if there is any.
 *
 * This is a convenience function that calls leds_play() with reps set to 0.
 *
 * @retval 0 If successful.
 * @retval <0 Error code if stopping the pattern failed.
 */
int leds_off();

#endif  // LEDS_H
