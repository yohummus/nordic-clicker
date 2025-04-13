#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdbool.h>
#include <zephyr/kernel.h>

enum buttons_button_t {
    BUTTONS_BTN_1,
    BUTTONS_BTN_2,
    BUTTONS_BTN_3,
    BUTTONS_BTN_4,
    BUTTONS_BTN_5,
    BUTTONS_BTN_SHIFT,
};

struct buttons_event_t {
    enum buttons_button_t button;
    bool is_long_press;
    int preceding_short_shift_presses;
};

/**
 * @brief Get/wait for the next button event.
 *
 * @param event Pointer to the event structure to fill.
 * @param timeout Waiting period to obtain the next event, or one of the special
 *                values K_NO_WAIT and K_FOREVER.
 *
 * @retval 0 If successful.
 * @retval -ETIMEDOUT If the timeout expired before an event was received.
 */
int buttons_get_event(struct buttons_event_t *event, k_timeout_t timeout);

#endif  // BUTTONS_H
