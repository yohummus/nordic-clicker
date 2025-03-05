#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main);

static const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c0));
static const struct gpio_dt_spec en_gpio = GPIO_DT_SPEC_GET(DT_NODELABEL(lp5813_en), gpios);
static const struct device *pwm = DEVICE_DT_GET(DT_NODELABEL(pwm0));
static const struct gpio_dt_spec button_1 = GPIO_DT_SPEC_GET(DT_NODELABEL(button_1), gpios);
static const struct gpio_dt_spec button_2 = GPIO_DT_SPEC_GET(DT_NODELABEL(button_2), gpios);
static const struct gpio_dt_spec button_3 = GPIO_DT_SPEC_GET(DT_NODELABEL(button_3), gpios);
static const struct gpio_dt_spec button_4 = GPIO_DT_SPEC_GET(DT_NODELABEL(button_4), gpios);
static const struct gpio_dt_spec button_5 = GPIO_DT_SPEC_GET(DT_NODELABEL(button_5), gpios);
static const struct gpio_dt_spec button_6 = GPIO_DT_SPEC_GET(DT_NODELABEL(button_6), gpios);
static const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart0));

#define LP5813_ADDR 0x16
#define LP5813_REG_CHIP_EN 0x000
#define LP5813_REG_DEV_CONFIG_0 0x001
#define LP5813_REG_DEV_CONFIG_1 0x002
#define LP5813_REG_DEV_CONFIG_2 0x003
#define LP5813_REG_DEV_CONFIG_3 0x004
#define LP5813_REG_DEV_CONFIG_4 0x005
#define LP5813_REG_DEV_CONFIG_5 0x006
#define LP5813_REG_DEV_CONFIG_6 0x007
#define LP5813_REG_DEV_CONFIG_7 0x008
#define LP5813_REG_DEV_CONFIG_8 0x009
#define LP5813_REG_DEV_CONFIG_9 0x00A
#define LP5813_REG_DEV_CONFIG_10 0x00B
#define LP5813_REG_DEV_CONFIG_11 0x00C
#define LP5813_REG_DEV_CONFIG_12 0x00D
#define LP5813_REG_CMD_UPDATE 0x010
#define LP5813_REG_CMD_START 0x011
#define LP5813_REG_CMD_STOP 0x012
#define LP5813_REG_CMD_PAUSE 0x013
#define LP5813_REG_CMD_CONTINUE 0x014
#define LP5813_REG_LED_EN_1 0x020
#define LP5813_REG_LED_EN_2 0x021
#define LP5813_REG_MANUAL_DC_0 0x030
#define LP5813_REG_MANUAL_DC_1 0x031
#define LP5813_REG_MANUAL_DC_2 0x032
#define LP5813_REG_MANUAL_DC_3 0x033
#define LP5813_REG_MANUAL_DC_A0 0x034
#define LP5813_REG_MANUAL_DC_A1 0x035
#define LP5813_REG_MANUAL_DC_A2 0x036
#define LP5813_REG_MANUAL_DC_B0 0x037
#define LP5813_REG_MANUAL_DC_B1 0x038
#define LP5813_REG_MANUAL_DC_B2 0x039
#define LP5813_REG_MANUAL_DC_C0 0x03A
#define LP5813_REG_MANUAL_DC_C1 0x03B
#define LP5813_REG_MANUAL_DC_C2 0x03C
#define LP5813_REG_MANUAL_DC_D0 0x03D
#define LP5813_REG_MANUAL_DC_D1 0x03E
#define LP5813_REG_MANUAL_DC_D2 0x03F
#define LP5813_REG_MANUAL_PWM_0 0x040
#define LP5813_REG_MANUAL_PWM_1 0x041
#define LP5813_REG_MANUAL_PWM_2 0x042
#define LP5813_REG_MANUAL_PWM_3 0x043
#define LP5813_REG_MANUAL_PWM_A0 0x044
#define LP5813_REG_MANUAL_PWM_A1 0x045
#define LP5813_REG_MANUAL_PWM_A2 0x046
#define LP5813_REG_MANUAL_PWM_B0 0x047
#define LP5813_REG_MANUAL_PWM_B1 0x048
#define LP5813_REG_MANUAL_PWM_B2 0x049
#define LP5813_REG_MANUAL_PWM_C0 0x04A
#define LP5813_REG_MANUAL_PWM_C1 0x04B
#define LP5813_REG_MANUAL_PWM_C2 0x04C
#define LP5813_REG_MANUAL_PWM_D0 0x04D
#define LP5813_REG_MANUAL_PWM_D1 0x04E
#define LP5813_REG_MANUAL_PWM_D2 0x04F
#define LP5813_REG_TSD_CONFIG_STATUS 0x300

// For LP5813_REG_DEV_CONFIG_0
#define LP5813_BOOST_VOUT_3V0 (0 << 1)
#define LP5813_BOOST_VOUT_3V1 (1 << 1)
#define LP5813_BOOST_VOUT_3V2 (2 << 1)
#define LP5813_BOOST_VOUT_3V3 (3 << 1)
#define LP5813_BOOST_VOUT_3V4 (4 << 1)
#define LP5813_BOOST_VOUT_3V5 (5 << 1)
#define LP5813_BOOST_VOUT_3V6 (6 << 1)
#define LP5813_BOOST_VOUT_3V7 (7 << 1)
#define LP5813_BOOST_VOUT_3V8 (8 << 1)
#define LP5813_BOOST_VOUT_3V9 (9 << 1)
#define LP5813_BOOST_VOUT_4V0 (10 << 1)
#define LP5813_BOOST_VOUT_4V1 (11 << 1)
#define LP5813_BOOST_VOUT_4V2 (12 << 1)
#define LP5813_BOOST_VOUT_4V3 (13 << 1)
#define LP5813_BOOST_VOUT_4V4 (14 << 1)
#define LP5813_BOOST_VOUT_4V5 (15 << 1)
#define LP5813_BOOST_VOUT_4V6 (16 << 1)
#define LP5813_BOOST_VOUT_4V7 (17 << 1)
#define LP5813_BOOST_VOUT_4V8 (18 << 1)
#define LP5813_BOOST_VOUT_4V9 (19 << 1)
#define LP5813_BOOST_VOUT_5V0 (20 << 1)
#define LP5813_BOOST_VOUT_5V1 (21 << 1)
#define LP5813_BOOST_VOUT_5V2 (22 << 1)
#define LP5813_BOOST_VOUT_5V3 (23 << 1)
#define LP5813_BOOST_VOUT_5V4 (24 << 1)
#define LP5813_BOOST_VOUT_5V5 (25 << 1)
#define LP5813_GLOBAL_MAX_CURRENT_25MA5 0x00
#define LP5813_GLOBAL_MAX_CURRENT_51MA 0x01

// For LP5813_REG_DEV_CONFIG_1
#define LP5813_PWM_FRE_24KHZ 0x00
#define LP5813_PWM_FRE_12KHZ 0x80
#define LP5813_LED_MODE_1_SCANS (0x1 << 4)
#define LP5813_LED_MODE_2_SCANS (0x2 << 4)
#define LP5813_LED_MODE_3_SCANS (0x3 << 4)
#define LP5813_LED_MODE_4_SCANS (0x4 << 4)

// For LP5813_REG_DEV_CONFIG_2
#define LP5813_SCAN_ORDER_0_0H (0x0 << 0)
#define LP5813_SCAN_ORDER_0_1H (0x1 << 0)
#define LP5813_SCAN_ORDER_0_2H (0x2 << 0)
#define LP5813_SCAN_ORDER_0_3H (0x3 << 0)
#define LP5813_SCAN_ORDER_1_0H (0x0 << 2)
#define LP5813_SCAN_ORDER_1_1H (0x1 << 2)
#define LP5813_SCAN_ORDER_1_2H (0x2 << 2)
#define LP5813_SCAN_ORDER_1_3H (0x3 << 2)
#define LP5813_SCAN_ORDER_2_0H (0x0 << 4)
#define LP5813_SCAN_ORDER_2_1H (0x1 << 4)
#define LP5813_SCAN_ORDER_2_2H (0x2 << 4)
#define LP5813_SCAN_ORDER_2_3H (0x3 << 4)
#define LP5813_SCAN_ORDER_3_0H (0x0 << 6)
#define LP5813_SCAN_ORDER_3_1H (0x1 << 6)
#define LP5813_SCAN_ORDER_3_2H (0x2 << 6)
#define LP5813_SCAN_ORDER_3_3H (0x3 << 6)

// For LP5813_REG_TSD_CONFIG_STATUS
#define LP5813_TSD_STATUS 0x02
#define LP5813_CONFIG_ERR_STATUS 0x01

// For LP5813_REG_LED_EN_1
#define LP5813_LED_EN_0 0x01
#define LP5813_LED_EN_1 0x02
#define LP5813_LED_EN_2 0x04
#define LP5813_LED_EN_3 0x08
#define LP5813_LED_EN_A0 0x10
#define LP5813_LED_EN_A1 0x20
#define LP5813_LED_EN_A2 0x40
#define LP5813_LED_EN_B0 0x80

// For LP5813_REG_LED_EN_2
#define LP5813_LED_EN_B1 0x01
#define LP5813_LED_EN_B2 0x02
#define LP5813_LED_EN_C0 0x04
#define LP5813_LED_EN_C1 0x08
#define LP5813_LED_EN_C2 0x10
#define LP5813_LED_EN_D0 0x20
#define LP5813_LED_EN_D1 0x40
#define LP5813_LED_EN_D2 0x80

#define AUDIO_CHANNEL 0
#define BOOST_CHANNEL 1

#define SPEAKER_FREQUENCY_HZ 4100
#define SPEAKER_PERIOD PWM_HZ(SPEAKER_FREQUENCY_HZ)
#define SPEAKER_PULSE (SPEAKER_PERIOD / 2)

/**
 * @brief Initializes this module
 *
 * @retval 0 If successful.
 * @retval <0 Error code if initialization failed.
 */
int leds_init() {
    if (!device_is_ready(i2c_dev)) {
        LOG_ERR("ERROR: I2C device is not ready");
        return -EIO;
    }

    if (!gpio_is_ready_dt(&en_gpio)) {
        LOG_ERR("ERROR: GPIO for EN signal is not ready");
        return -EIO;
    }

    int res = gpio_pin_configure_dt(&en_gpio, GPIO_OUTPUT_INACTIVE);
    if (res != 0) {
        LOG_ERR("ERROR: Failed to configure EN signal GPIO");
        return res;
    }

    return 0;
}

int leds_set_lp5813_en(int en) {
    int res = gpio_pin_set_dt(&en_gpio, en);
    if (res < 0) {
        LOG_ERR("Failed to set EN signal to %d", en);
    }

    return res;
}

int leds_write_lp5813_reg(uint16_t reg_addr, uint8_t value) {
    if (reg_addr >> 10) {
        LOG_ERR("Invalid LP5813 register address: %X", reg_addr);
        return -EINVAL;
    }

    int res = i2c_reg_write_byte(i2c_dev, (LP5813_ADDR << 2) | (reg_addr >> 8), reg_addr & 0xFF, value);
    if (res < 0) {
        LOG_ERR("Failed to write to LP5813 register %X", reg_addr);
    }

    return res;
}

int leds_read_lp5813_reg(uint16_t reg_addr, uint8_t *value) {
    if (reg_addr >> 10) {
        LOG_ERR("Invalid LP5813 register address: %X", reg_addr);
        return -EINVAL;
    }

    int res = i2c_reg_read_byte(i2c_dev, (LP5813_ADDR << 2) | (reg_addr >> 8), reg_addr & 0xFF, value);
    if (res < 0) {
        LOG_ERR("Failed to read from LP5813 register %X", reg_addr);
    }

    return res;
}

int leds_send_cmd(uint16_t cmd_reg_addr) { return leds_write_lp5813_reg(cmd_reg_addr, 0x55); }

int main(void) {
    bool ok = true;
    ok &= device_is_ready(pwm);
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
    ok &= leds_init() == 0;

    if (!ok) {
        LOG_ERR("SHIT: Something is not ready.");
        return 0;
    }

    LOG_INF("Starting main loop...");

    while (1) {
        int res = uart_tx(uart, "Hello\n\r", 7, 0);
        if (res != 0) {
            LOG_ERR("Failed to send data via UART: %d", res);
        }

        printk("Hey dude!\n\r");

        leds_set_lp5813_en(true);

        k_msleep(10);

        leds_write_lp5813_reg(LP5813_REG_CHIP_EN, 0x01);
        leds_write_lp5813_reg(LP5813_REG_DEV_CONFIG_0, LP5813_BOOST_VOUT_4V5 | LP5813_GLOBAL_MAX_CURRENT_51MA);
        leds_write_lp5813_reg(LP5813_REG_DEV_CONFIG_1, LP5813_PWM_FRE_24KHZ | LP5813_LED_MODE_2_SCANS);
        leds_write_lp5813_reg(LP5813_REG_DEV_CONFIG_2, LP5813_SCAN_ORDER_0_0H | LP5813_SCAN_ORDER_1_1H);
        leds_write_lp5813_reg(LP5813_REG_DEV_CONFIG_12, 0x0B);  // Avoid incorrect LSD detection
        leds_send_cmd(LP5813_REG_CMD_UPDATE);

        uint8_t value = 0xFF;
        leds_read_lp5813_reg(LP5813_REG_TSD_CONFIG_STATUS, &value);
        LOG_INF("STATUS: %X", (int)value);

        if ((value & LP5813_CONFIG_ERR_STATUS) == 0) {
            leds_write_lp5813_reg(LP5813_REG_LED_EN_1,
                                  LP5813_LED_EN_A0 | LP5813_LED_EN_A1 | LP5813_LED_EN_A2 | LP5813_LED_EN_B0);
            leds_write_lp5813_reg(LP5813_REG_LED_EN_2, LP5813_LED_EN_B1 | LP5813_LED_EN_B2);
            leds_write_lp5813_reg(LP5813_REG_MANUAL_DC_A0, 0xFF);
            leds_write_lp5813_reg(LP5813_REG_MANUAL_DC_A1, 0xFF);
            leds_write_lp5813_reg(LP5813_REG_MANUAL_DC_A2, 0xFF);
            leds_write_lp5813_reg(LP5813_REG_MANUAL_DC_B0, 0xFF);
            leds_write_lp5813_reg(LP5813_REG_MANUAL_DC_B1, 0xFF);
            leds_write_lp5813_reg(LP5813_REG_MANUAL_DC_B2, 0xFF);
            LOG_INF("LEDs enabled");

            leds_write_lp5813_reg(LP5813_REG_MANUAL_PWM_A0, 0x0F);
            k_msleep(500);
            leds_write_lp5813_reg(LP5813_REG_MANUAL_PWM_A0, 0x00);
            k_msleep(500);

            leds_write_lp5813_reg(LP5813_REG_MANUAL_PWM_A1, 0x0F);
            k_msleep(500);
            leds_write_lp5813_reg(LP5813_REG_MANUAL_PWM_A1, 0x00);
            k_msleep(500);

            leds_write_lp5813_reg(LP5813_REG_MANUAL_PWM_A2, 0x0F);
            k_msleep(500);
            leds_write_lp5813_reg(LP5813_REG_MANUAL_PWM_A2, 0x00);
            k_msleep(500);

            leds_write_lp5813_reg(LP5813_REG_MANUAL_PWM_B0, 0x0F);
            k_msleep(500);
            leds_write_lp5813_reg(LP5813_REG_MANUAL_PWM_B0, 0x00);
            k_msleep(500);

            leds_write_lp5813_reg(LP5813_REG_MANUAL_PWM_B1, 0x0F);
            k_msleep(500);
            leds_write_lp5813_reg(LP5813_REG_MANUAL_PWM_B1, 0x00);
            k_msleep(500);

            leds_write_lp5813_reg(LP5813_REG_MANUAL_PWM_B2, 0x0F);
            k_msleep(500);
            leds_write_lp5813_reg(LP5813_REG_MANUAL_PWM_B2, 0x00);
            k_msleep(500);
        }

        LOG_INF("Next tick");
        k_msleep(100);

        // // Test buttons
        // LOG_INF("Toggling");
        // char a = gpio_pin_get_dt(&button_1) ? 'X' : '_';
        // char b = gpio_pin_get_dt(&button_2) ? 'X' : '_';
        // char c = gpio_pin_get_dt(&button_3) ? 'X' : '_';
        // char d = gpio_pin_get_dt(&button_4) ? 'X' : '_';
        // char e = gpio_pin_get_dt(&button_5) ? 'X' : '_';
        // char f = gpio_pin_get_dt(&button_6) ? 'X' : '_';
        // LOG_INF("Buttons: %c%c%c%c%c%c", a, b, c, d, e, f);

        // // Test PWM
        // if (pwm_set(pwm, AUDIO_CHANNEL, SPEAKER_PERIOD, SPEAKER_PULSE, PWM_POLARITY_NORMAL) != 0 ||
        //     pwm_set(pwm, BOOST_CHANNEL, SPEAKER_PERIOD, SPEAKER_PULSE, PWM_POLARITY_INVERTED) != 0) {
        //     LOG_ERR("Failed to set PWM parameters");
        //     continue;
        // }

        // k_msleep(500);

        // if (pwm_set(pwm, AUDIO_CHANNEL, SPEAKER_PERIOD, SPEAKER_PULSE, PWM_POLARITY_NORMAL) != 0 ||
        //     pwm_set(pwm, BOOST_CHANNEL, SPEAKER_PERIOD, 0, PWM_POLARITY_NORMAL) != 0) {
        //     LOG_ERR("Failed to set PWM parameters");
        //     continue;
        // }

        // k_msleep(500);

        // if (pwm_set(pwm, AUDIO_CHANNEL, SPEAKER_PERIOD, 0, PWM_POLARITY_NORMAL) != 0 ||
        //     pwm_set(pwm, BOOST_CHANNEL, SPEAKER_PERIOD, 0, PWM_POLARITY_NORMAL) != 0) {
        //     LOG_ERR("Failed to switch PWM off");
        //     continue;
        // }

        // k_msleep(500);

        // 011000001
        // ist:  001011001 (in hex: 0x169)
        // soll: 10110000
    }

    return 0;
}
