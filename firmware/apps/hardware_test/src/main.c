#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

static const struct gpio_dt_spec lp5813_en = GPIO_DT_SPEC_GET(DT_NODELABEL(lp5813_en), gpios);
static const struct gpio_dt_spec button_1 = GPIO_DT_SPEC_GET(DT_NODELABEL(button_1), gpios);
static const struct gpio_dt_spec button_2 = GPIO_DT_SPEC_GET(DT_NODELABEL(button_2), gpios);
static const struct gpio_dt_spec button_3 = GPIO_DT_SPEC_GET(DT_NODELABEL(button_3), gpios);
static const struct gpio_dt_spec button_4 = GPIO_DT_SPEC_GET(DT_NODELABEL(button_4), gpios);
static const struct gpio_dt_spec button_5 = GPIO_DT_SPEC_GET(DT_NODELABEL(button_5), gpios);
static const struct gpio_dt_spec button_6 = GPIO_DT_SPEC_GET(DT_NODELABEL(button_6), gpios);

int main(void) {
    if (!gpio_is_ready_dt(&lp5813_en) || !gpio_is_ready_dt(&button_1) || !gpio_is_ready_dt(&button_2) ||
        !gpio_is_ready_dt(&button_3) || !gpio_is_ready_dt(&button_4) || !gpio_is_ready_dt(&button_5) ||
        !gpio_is_ready_dt(&button_6) || gpio_pin_configure_dt(&lp5813_en, GPIO_OUTPUT_INACTIVE) < 0 ||
        gpio_pin_configure_dt(&button_1, GPIO_INPUT) < 0 || gpio_pin_configure_dt(&button_2, GPIO_INPUT) < 0 ||
        gpio_pin_configure_dt(&button_3, GPIO_INPUT) < 0 || gpio_pin_configure_dt(&button_4, GPIO_INPUT) < 0 ||
        gpio_pin_configure_dt(&button_5, GPIO_INPUT) < 0 || gpio_pin_configure_dt(&button_6, GPIO_INPUT) < 0) {
        return 0;
    }

    LOG_INF("Starting main loop...");

    while (1) {
        int ret = gpio_pin_toggle_dt(&lp5813_en);
        if (ret < 0) {
            return 0;
        }

        k_msleep(2000);
    }

    return 0;
}
