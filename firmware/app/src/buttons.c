#include "buttons.h"

#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app_buttons);

// GPIO devices
static const struct gpio_dt_spec button_1   = GPIO_DT_SPEC_GET(DT_NODELABEL(button_1), gpios);
static const struct gpio_dt_spec button_2   = GPIO_DT_SPEC_GET(DT_NODELABEL(button_2), gpios);
static const struct gpio_dt_spec button_3   = GPIO_DT_SPEC_GET(DT_NODELABEL(button_3), gpios);
static const struct gpio_dt_spec button_4   = GPIO_DT_SPEC_GET(DT_NODELABEL(button_4), gpios);
static const struct gpio_dt_spec button_5   = GPIO_DT_SPEC_GET(DT_NODELABEL(button_5), gpios);
static const struct gpio_dt_spec button_6   = GPIO_DT_SPEC_GET(DT_NODELABEL(button_6), gpios);
static const struct gpio_dt_spec *buttons[] = {&button_1, &button_2, &button_3, &button_4, &button_5, &button_6};

// Interrupt callback data
static struct gpio_callback buttons_cb_data[ARRAY_SIZE(buttons)];

/*********************************************************************************************************************
 * INTERRUPT HANDLERS
 *********************************************************************************************************************/

static void button_pressed(const struct device *port, struct gpio_callback *cb, gpio_port_pins_t pins) {
    for (int i = 0; i < ARRAY_SIZE(buttons); i++) {
        const struct gpio_dt_spec *btn = buttons[i];
        if (pins & BIT(btn->pin)) {
            LOG_INF("Button %d pressed", i + 1);
        }
    }
}

/*********************************************************************************************************************
 * PUBLIC FUNCTIONS
 *********************************************************************************************************************/

int buttons_init() {
    gpio_port_pins_t pin_mask = 0;
    for (int i = 0; i < ARRAY_SIZE(buttons); i++) {
        // Check that the GPIO is ready
        const struct gpio_dt_spec *btn = buttons[i];
        if (!gpio_is_ready_dt(btn)) {
            LOG_ERR("GPIO for button %d is not ready", i + 1);
            return -EIO;
        }

        // Configure GPIO as input
        int res = gpio_pin_configure_dt(btn, GPIO_INPUT);
        if (res != 0) {
            LOG_ERR("Failed to configure GPIO for button %d as input: %d:", i + 1, res);
            return res;
        }

        // Setup the interrupt handler
        res = gpio_pin_interrupt_configure_dt(btn, GPIO_INT_EDGE_TO_ACTIVE);
        if (res != 0) {
            LOG_ERR("Failed to configure interrupt for button %d: %d", i + 1, res);
            return res;
        }

        gpio_init_callback(&buttons_cb_data[i], button_pressed, BIT(btn->pin));
        res = gpio_add_callback(btn->port, &buttons_cb_data[i]);
        if (res != 0) {
            LOG_ERR("Failed to add callback for button %d: %d", i + 1, res);
            return res;
        }

        pin_mask |= BIT(btn->pin);
    }

    return 0;
}
