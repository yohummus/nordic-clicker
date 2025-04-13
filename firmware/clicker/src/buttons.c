#include "buttons.h"

#include <hal/nrf_gpio.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app_buttons);

// Thread configuration
#define THREAD_STACK_SIZE 1024
#define THREAD_PRIORITY   6

// Long press timing; the _ON_WAKEUP threshold is lower to account for the boot time of the device
#define LONG_PRESS_THRESHOLD           K_MSEC(2000)
#define LONG_PRESS_THRESHOLD_ON_WAKEUP K_MSEC(1500)

// GPIO devices
#define BUTTON_DEF(node_label)                                                \
    (struct button_t) {                                                       \
        .spec = GPIO_DT_SPEC_GET(DT_NODELABEL(node_label), gpios),            \
        .port = DT_PROP(DT_GPIO_CTLR(DT_NODELABEL(node_label), gpios), port), \
    }

struct button_t {
    const struct gpio_dt_spec spec;
    const int port;
    struct gpio_callback cb_data;
};

static struct button_t buttons[] = {
    BUTTON_DEF(button_1),  // BUTTONS_BTN_1
    BUTTON_DEF(button_2),  // BUTTONS_BTN_2
    BUTTON_DEF(button_3),  // BUTTONS_BTN_3
    BUTTON_DEF(button_4),  // BUTTONS_BTN_4
    BUTTON_DEF(button_5),  // BUTTONS_BTN_5
    BUTTON_DEF(button_6),  // BUTTONS_BTN_SHIFT
};

// FIFO for button events (for communication with user code)
struct buttons_fifo_item_t {
    void *fifo_reserved;
    struct buttons_event_t event;
};

K_FIFO_DEFINE(buttons_fifo);

// Sempahore for the ISR to signal the thread to check for button presses/releases
K_SEM_DEFINE(buttons_sem, 0, 1);

// Status of the LATCH registers at startup to check which button triggered exit from System OFF
static uint32_t gpio_latch_at_startup[2] = {0};

/*********************************************************************************************************************
 * STARTUP HOOKS
 *********************************************************************************************************************/
static int detect_wakup_latch() {
    nrf_gpio_latches_read(0, ARRAY_SIZE(gpio_latch_at_startup), gpio_latch_at_startup);
    return 0;
}

SYS_INIT(detect_wakup_latch, PRE_KERNEL_1, 0);

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
        struct button_t *btn = &buttons[i];

        // Check that the GPIO is ready
        if (!gpio_is_ready_dt(&btn->spec)) {
            LOG_ERR("GPIO for button %d is not ready", i + 1);
            return false;
        }

        // Configure GPIO as input
        int res = gpio_pin_configure_dt(&btn->spec, GPIO_INPUT);
        if (res != 0) {
            LOG_ERR("Failed to configure GPIO for button %d as input: %d:", i + 1, res);
            return false;
        }

        // Setup the interrupt handler
        res = gpio_pin_interrupt_configure_dt(&btn->spec, GPIO_INT_EDGE_BOTH);
        if (res != 0) {
            LOG_ERR("Failed to configure interrupt for button %d: %d", i + 1, res);
            return false;
        }

        gpio_init_callback(&btn->cb_data, button_pressed, BIT(btn->spec.pin));
        res = gpio_add_callback_dt(&btn->spec, &btn->cb_data);
        if (res != 0) {
            LOG_ERR("Failed to add callback for button %d: %d", i + 1, res);
            return false;
        }
    }

    LOG_INF("Buttons module initialized OK; waiting for button presses");

    return true;
}

static bool is_button_pressed(const struct button_t *btn) {
    int res = gpio_pin_get_dt(&btn->spec);
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

    const struct button_t *pressed_button = NULL;
    k_timepoint_t long_press_exp_time     = sys_timepoint_calc(K_FOREVER);
    int preceding_short_shift_presses     = 0;

    LOG_INF("Buttons module initialized OK; waiting for button presses");

    // Check if any button was pressed at startup
    for (int i = 0; i < ARRAY_SIZE(buttons); i++) {
        const struct button_t *btn = &buttons[i];
        if (gpio_latch_at_startup[btn->port] & BIT(btn->spec.pin)) {
            long_press_exp_time = sys_timepoint_calc(LONG_PRESS_THRESHOLD_ON_WAKEUP);
            pressed_button      = btn;
            LOG_INF("Button %d was pressed at startup", i + 1);
            break;
        }
    }

    if (pressed_button == NULL) {
        LOG_INF("No button was pressed at startup");
    }

    // Main loop, waiting for button presses/releases
    while (true) {
        // If, until now, no button has been pressed, we check if any button is pressed now
        if (pressed_button == NULL) {
            for (int i = 0; i < ARRAY_SIZE(buttons); i++) {
                const struct button_t *btn = &buttons[i];

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

        // Wait for the signal from the ISR
        k_sem_take(&buttons_sem, sys_timepoint_timeout(long_press_exp_time));
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