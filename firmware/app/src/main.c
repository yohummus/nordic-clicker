#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "battery.h"
#include "bluetooth.h"
#include "buttons.h"
#include "leds.h"
#include "speaker.h"

LOG_MODULE_REGISTER(app_main);

static const struct gpio_dt_spec button_1 = GPIO_DT_SPEC_GET(DT_NODELABEL(button_1), gpios);
static const struct gpio_dt_spec button_2 = GPIO_DT_SPEC_GET(DT_NODELABEL(button_2), gpios);
static const struct gpio_dt_spec button_3 = GPIO_DT_SPEC_GET(DT_NODELABEL(button_3), gpios);
static const struct gpio_dt_spec button_4 = GPIO_DT_SPEC_GET(DT_NODELABEL(button_4), gpios);
static const struct gpio_dt_spec button_5 = GPIO_DT_SPEC_GET(DT_NODELABEL(button_5), gpios);
static const struct gpio_dt_spec button_6 = GPIO_DT_SPEC_GET(DT_NODELABEL(button_6), gpios);
static const struct device *uart          = DEVICE_DT_GET(DT_NODELABEL(uart0));

static void on_leds_finished(bool aborted) {
    LOG_INF("on_leds_finished(): %s", aborted ? "aborted" : "finished");
}

int main(void) {
    bool ok = true;
    ok &= device_is_ready(uart);
    ok &= gpio_is_ready_dt(&button_1);
    ok &= gpio_is_ready_dt(&button_2);
    ok &= gpio_is_ready_dt(&button_3);
    ok &= gpio_is_ready_dt(&button_4);
    ok &= gpio_is_ready_dt(&button_5);
    ok &= gpio_is_ready_dt(&button_6);
    ok &= gpio_pin_configure_dt(&button_1, GPIO_INPUT) == 0;
    ok &= gpio_pin_configure_dt(&button_2, GPIO_INPUT) == 0;
    ok &= gpio_pin_configure_dt(&button_3, GPIO_INPUT) == 0;
    ok &= gpio_pin_configure_dt(&button_4, GPIO_INPUT) == 0;
    ok &= gpio_pin_configure_dt(&button_5, GPIO_INPUT) == 0;
    ok &= gpio_pin_configure_dt(&button_6, GPIO_INPUT) == 0;
    ok &= bluetooth_init() == 0;
    ok &= speaker_init() == 0;

    if (!ok) {
        LOG_ERR("SHIT: Something is not ready.");
        return 0;
    }

    LOG_INF("Starting main loop...");

    int i = 0;
    while (1) {
        k_msleep(1000);
        LOG_INF("Still alive %d", i);
        ++i;

        // leds_set_enabled(true);
        // leds_set_led(LEDS_D1, LEDS_RGB(255, 0, 0), LEDS_FLASH);
        // k_msleep(5000);
        // leds_set_enabled(false);

        leds_play(LEDS_D1, LEDS_FLASH, LEDS_RGB(255, 255, 255), 3, on_leds_finished);
        k_msleep(2000);
        leds_off();

        // speaker_set_frequency(SPEAKER_RESONANT_FREQUENCY);
        // k_msleep(500);
        // speaker_set_frequency(0);
        // k_msleep(2000);

        // int res = uart_tx(uart, "Hello\n\r", 7, 0);
        // if (res != 0) {
        //     LOG_ERR("Failed to send data via UART: %d", res);
        // }

        // printk("Hey dude!\n\r");

        // if ((value & LP5813_CONFIG_ERR_STATUS) == 0) {
        //     leds_write_lp5813_reg(LP5813_REG_LED_EN_1,
        //                           LP5813_LED_EN_A0 | LP5813_LED_EN_A1 | LP5813_LED_EN_A2 | LP5813_LED_EN_B0);
        //     leds_write_lp5813_reg(LP5813_REG_LED_EN_2, LP5813_LED_EN_B1 | LP5813_LED_EN_B2);
        //     leds_write_lp5813_reg(LP5813_REG_MANUAL_DC_A0, 0xFF);
        //     leds_write_lp5813_reg(LP5813_REG_MANUAL_DC_A1, 0xFF);
        //     leds_write_lp5813_reg(LP5813_REG_MANUAL_DC_A2, 0xFF);
        //     leds_write_lp5813_reg(LP5813_REG_MANUAL_DC_B0, 0xFF);
        //     leds_write_lp5813_reg(LP5813_REG_MANUAL_DC_B1, 0xFF);
        //     leds_write_lp5813_reg(LP5813_REG_MANUAL_DC_B2, 0xFF);
        //     LOG_INF("LEDs enabled");

        //     leds_write_lp5813_reg(LP5813_REG_MANUAL_PWM_A0, 0x0F);
        //     k_msleep(500);
        //     leds_write_lp5813_reg(LP5813_REG_MANUAL_PWM_A0, 0x00);
        //     k_msleep(500);

        //     leds_write_lp5813_reg(LP5813_REG_MANUAL_PWM_A1, 0x0F);
        //     k_msleep(500);
        //     leds_write_lp5813_reg(LP5813_REG_MANUAL_PWM_A1, 0x00);
        //     k_msleep(500);

        //     leds_write_lp5813_reg(LP5813_REG_MANUAL_PWM_A2, 0x0F);
        //     k_msleep(500);
        //     leds_write_lp5813_reg(LP5813_REG_MANUAL_PWM_A2, 0x00);
        //     k_msleep(500);

        //     leds_write_lp5813_reg(LP5813_REG_MANUAL_PWM_B0, 0x0F);
        //     k_msleep(500);
        //     leds_write_lp5813_reg(LP5813_REG_MANUAL_PWM_B0, 0x00);
        //     k_msleep(500);

        //     leds_write_lp5813_reg(LP5813_REG_MANUAL_PWM_B1, 0x0F);
        //     k_msleep(500);
        //     leds_write_lp5813_reg(LP5813_REG_MANUAL_PWM_B1, 0x00);
        //     k_msleep(500);

        //     leds_write_lp5813_reg(LP5813_REG_MANUAL_PWM_B2, 0x0F);
        //     k_msleep(500);
        //     leds_write_lp5813_reg(LP5813_REG_MANUAL_PWM_B2, 0x00);
        //     k_msleep(500);
        // }

        // LOG_INF("Next tick");
        // k_msleep(100);

        // // Test buttons
        // LOG_INF("Toggling");
        // char a = gpio_pin_get_dt(&button_1) ? 'X' : '_';
        // char b = gpio_pin_get_dt(&button_2) ? 'X' : '_';
        // char c = gpio_pin_get_dt(&button_3) ? 'X' : '_';
        // char d = gpio_pin_get_dt(&button_4) ? 'X' : '_';
        // char e = gpio_pin_get_dt(&button_5) ? 'X' : '_';
        // char f = gpio_pin_get_dt(&button_6) ? 'X' : '_';
        // LOG_INF("Buttons: %c%c%c%c%c%c", a, b, c, d, e, f);
    }

    return 0;
}
