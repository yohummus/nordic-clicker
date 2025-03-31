#include "buttons.h"

#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app_buttons);

// Thread configuration
#define THREAD_STACK_SIZE 1024
#define THREAD_PRIORITY   6

// Timing configuration
#define LONG_PRESS_THRESHOLD K_MSEC(2000)

// GPIO devices
static const struct gpio_dt_spec buttons[] = {
    GPIO_DT_SPEC_GET(DT_NODELABEL(button_1), gpios),  // BUTTONS_BTN_1
    GPIO_DT_SPEC_GET(DT_NODELABEL(button_2), gpios),  // BUTTONS_BTN_2
    GPIO_DT_SPEC_GET(DT_NODELABEL(button_3), gpios),  // BUTTONS_BTN_3
    GPIO_DT_SPEC_GET(DT_NODELABEL(button_4), gpios),  // BUTTONS_BTN_4
    GPIO_DT_SPEC_GET(DT_NODELABEL(button_5), gpios),  // BUTTONS_BTN_5
    GPIO_DT_SPEC_GET(DT_NODELABEL(button_6), gpios),  // BUTTONS_BTN_SHIFT
};

// Interrupt callback data
static struct gpio_callback buttons_cb_data[ARRAY_SIZE(buttons)];

// FIFO for button events (for communication with user code)
struct buttons_fifo_item_t {
    void *fifo_reserved;
    struct buttons_event_t event;
};

K_FIFO_DEFINE(buttons_fifo);

// Sempahore for the ISR to signal the thread to check for button presses/releases;
// by setting the initial value to 1, we ensure that the thread checks the buttons on startup
K_SEM_DEFINE(buttons_sem, 1, 1);

/*********************************************************************************************************************
 * INTERRUPT HANDLERS
 *********************************************************************************************************************/

static void button_pressed(const struct device *port, struct gpio_callback *cb, gpio_port_pins_t pins) {
    // Signal the thread to check for button presses/releases
    k_sem_give(&buttons_sem);
}

/*********************************************************************************************************************
 * PRIVATE FUNCTIONS
 *********************************************************************************************************************/

static bool setup_gpios_and_interrupts() {
    for (int i = 0; i < ARRAY_SIZE(buttons); i++) {
        // Check that the GPIO is ready
        const struct gpio_dt_spec *btn = &buttons[i];
        if (!gpio_is_ready_dt(btn)) {
            LOG_ERR("GPIO for button %d is not ready", i + 1);
            return false;
        }

        // Configure GPIO as input
        int res = gpio_pin_configure_dt(btn, GPIO_INPUT);
        if (res != 0) {
            LOG_ERR("Failed to configure GPIO for button %d as input: %d:", i + 1, res);
            return false;
        }

        // Setup the interrupt handler
        res = gpio_pin_interrupt_configure_dt(btn, GPIO_INT_EDGE_BOTH);
        if (res != 0) {
            LOG_ERR("Failed to configure interrupt for button %d: %d", i + 1, res);
            return false;
        }

        gpio_init_callback(&buttons_cb_data[i], button_pressed, BIT(btn->pin));
        res = gpio_add_callback_dt(btn, &buttons_cb_data[i]);
        if (res != 0) {
            LOG_ERR("Failed to add callback for button %d: %d", i + 1, res);
            return false;
        }
    }

    LOG_INF("Buttons module initialized OK; waiting for button presses");

    return true;
}

static bool is_button_pressed(const struct gpio_dt_spec *btn) {
    int res = gpio_pin_get_dt(btn);
    if (res < 0) {
        LOG_ERR("Failed to get GPIO state for button: %d", res);
        return false;
    }

    return (res == 1);
}

/*********************************************************************************************************************
 * THREADS
 *********************************************************************************************************************/

static void buttons_thread_fn() {
    if (!setup_gpios_and_interrupts()) {
        return;
    }

    LOG_INF("Buttons module initialized OK; waiting for button presses");

    const struct gpio_dt_spec *pressed_button = NULL;
    k_timepoint_t long_press_exp_time         = sys_timepoint_calc(K_FOREVER);
    int preceding_short_shift_presses         = 0;

    // Main loop, waiting for button presses/releases
    while (true) {
        // Wait for the signal from the ISR
        k_sem_take(&buttons_sem, sys_timepoint_timeout(long_press_exp_time));

        // If, until now, no button has been pressed, we check if any button is pressed now
        if (pressed_button == NULL) {
            for (int i = 0; i < ARRAY_SIZE(buttons); i++) {
                const struct gpio_dt_spec *btn = &buttons[i];

                if (is_button_pressed(btn)) {
                    long_press_exp_time = sys_timepoint_calc(LONG_PRESS_THRESHOLD);
                    pressed_button      = btn;
                    break;
                }
            }
        }

        // Otherwise, if, until now, a button has been pressed, we check if that button has now been released,
        // and we send the corresponding event
        else {
            bool is_pressed    = is_button_pressed(pressed_button);
            bool is_long_press = sys_timepoint_expired(long_press_exp_time);

            if (!is_pressed || is_long_press) {
                // Generate the event
                struct buttons_fifo_item_t *item = k_malloc(sizeof(struct buttons_fifo_item_t));
                item->fifo_reserved              = NULL;
                item->event.button               = (enum buttons_button_t)(ARRAY_INDEX(buttons, pressed_button));
                item->event.is_long_press        = is_long_press;
                item->event.preceding_short_shift_presses = preceding_short_shift_presses;
                LOG_INF("New button event: button=%d, long=%d, pssp=%d", item->event.button + 1,
                        item->event.is_long_press, item->event.preceding_short_shift_presses);
                k_fifo_put(&buttons_fifo, item);

                // Update the state
                if (pressed_button == &buttons[BUTTONS_BTN_SHIFT] && !is_long_press) {
                    preceding_short_shift_presses++;
                } else {
                    preceding_short_shift_presses = 0;
                }

                pressed_button      = NULL;
                long_press_exp_time = sys_timepoint_calc(K_FOREVER);
            }
        }
    }
}

K_THREAD_DEFINE(buttons_thread_id, THREAD_STACK_SIZE, buttons_thread_fn, NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);

/*********************************************************************************************************************
 * PUBLIC FUNCTIONS
 *********************************************************************************************************************/

int buttons_get_event(struct buttons_event_t *event, k_timeout_t timeout) {
    struct buttons_fifo_item_t *item = k_fifo_get(&buttons_fifo, timeout);
    if (item == NULL) {
        return -ETIMEDOUT;
    }

    *event = item->event;
    k_free(item);
    return 0;
}