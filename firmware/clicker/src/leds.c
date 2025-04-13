#include "leds.h"

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app_leds);

// Thread configuration
#define THREAD_STACK_SIZE 1024
#define THREAD_PRIORITY   5

// Maximum LED current as a fraction (0 .. 255) of the GLOBAL_MAX_CURRENT_*
#define MAX_LED_CURRENT_FRACTION 0x2F

// I2C address of the LP5813 chip (depends on the chip variant)
#define I2C_ADDR 0x16

// Average intensity of the LEDs (from the datasheet)
#define LED_AVG_INTENSITY_R 1000
#define LED_AVG_INTENSITY_G 2400
#define LED_AVG_INTENSITY_B 530

// Aliases for registers for the LEDs on our board
#define REG_AUTO_DC_D1_R REG_AUTO_DC_A0
#define REG_AUTO_DC_D1_G REG_AUTO_DC_A1
#define REG_AUTO_DC_D1_B REG_AUTO_DC_A2
#define REG_AUTO_DC_D2_R REG_AUTO_DC_B0
#define REG_AUTO_DC_D2_G REG_AUTO_DC_B1
#define REG_AUTO_DC_D2_B REG_AUTO_DC_B2

#define REG_BASE_ANIM_D1_R REG_BASE_ANIM_LED_A0
#define REG_BASE_ANIM_D1_G REG_BASE_ANIM_LED_A1
#define REG_BASE_ANIM_D1_B REG_BASE_ANIM_LED_A2
#define REG_BASE_ANIM_D2_R REG_BASE_ANIM_LED_B0
#define REG_BASE_ANIM_D2_G REG_BASE_ANIM_LED_B1
#define REG_BASE_ANIM_D2_B REG_BASE_ANIM_LED_B2

// LP5813 registers
#define REG_CHIP_EN 0x000

#define REG_DEV_CONFIG_0      0x001
#define REG_DEV_CONFIG_1      0x002
#define REG_DEV_CONFIG_2      0x003
#define REG_DEV_CONFIG_3      0x004
#define REG_DEV_CONFIG_4      0x005
#define REG_DEV_CONFIG_5      0x006
#define REG_DEV_CONFIG_6      0x007
#define REG_DEV_CONFIG_7      0x008
#define REG_DEV_CONFIG_8      0x009
#define REG_DEV_CONFIG_9      0x00A
#define REG_DEV_CONFIG_10     0x00B
#define REG_DEV_CONFIG_11     0x00C
#define REG_DEV_CONFIG_12     0x00D
#define REG_CMD_UPDATE        0x010
#define REG_CMD_START         0x011
#define REG_CMD_STOP          0x012
#define REG_CMD_PAUSE         0x013
#define REG_CMD_CONTINUE      0x014
#define REG_LED_EN_1          0x020
#define REG_LED_EN_2          0x021
#define REG_FAULT_CLEAR       0x022
#define REG_SW_RESET          0x023
#define REG_MANUAL_DC_0       0x030
#define REG_MANUAL_DC_1       0x031
#define REG_MANUAL_DC_2       0x032
#define REG_MANUAL_DC_3       0x033
#define REG_MANUAL_DC_A0      0x034
#define REG_MANUAL_DC_A1      0x035
#define REG_MANUAL_DC_A2      0x036
#define REG_MANUAL_DC_B0      0x037
#define REG_MANUAL_DC_B1      0x038
#define REG_MANUAL_DC_B2      0x039
#define REG_MANUAL_DC_C0      0x03A
#define REG_MANUAL_DC_C1      0x03B
#define REG_MANUAL_DC_C2      0x03C
#define REG_MANUAL_DC_D0      0x03D
#define REG_MANUAL_DC_D1      0x03E
#define REG_MANUAL_DC_D2      0x03F
#define REG_MANUAL_PWM_0      0x040
#define REG_MANUAL_PWM_1      0x041
#define REG_MANUAL_PWM_2      0x042
#define REG_MANUAL_PWM_3      0x043
#define REG_MANUAL_PWM_A0     0x044
#define REG_MANUAL_PWM_A1     0x045
#define REG_MANUAL_PWM_A2     0x046
#define REG_MANUAL_PWM_B0     0x047
#define REG_MANUAL_PWM_B1     0x048
#define REG_MANUAL_PWM_B2     0x049
#define REG_MANUAL_PWM_C0     0x04A
#define REG_MANUAL_PWM_C1     0x04B
#define REG_MANUAL_PWM_C2     0x04C
#define REG_MANUAL_PWM_D0     0x04D
#define REG_MANUAL_PWM_D1     0x04E
#define REG_MANUAL_PWM_D2     0x04F
#define REG_AUTO_DC_0         0x050
#define REG_AUTO_DC_1         0x051
#define REG_AUTO_DC_2         0x052
#define REG_AUTO_DC_3         0x053
#define REG_AUTO_DC_A0        0x054
#define REG_AUTO_DC_A1        0x055
#define REG_AUTO_DC_A2        0x056
#define REG_AUTO_DC_B0        0x057
#define REG_AUTO_DC_B1        0x058
#define REG_AUTO_DC_B2        0x059
#define REG_AUTO_DC_C0        0x05A
#define REG_AUTO_DC_C1        0x05B
#define REG_AUTO_DC_C2        0x05C
#define REG_AUTO_DC_D0        0x05D
#define REG_AUTO_DC_D1        0x05E
#define REG_AUTO_DC_D2        0x05F
#define REG_BASE_ANIM_LED_0   0x080  // Base address; add REG_OFF_ANIM_* offsets to get the correct register
#define REG_BASE_ANIM_LED_1   0x09A  // Base address; add REG_OFF_ANIM_* offsets to get the correct register
#define REG_BASE_ANIM_LED_2   0x0B4  // Base address; add REG_OFF_ANIM_* offsets to get the correct register
#define REG_BASE_ANIM_LED_3   0x0CE  // Base address; add REG_OFF_ANIM_* offsets to get the correct register
#define REG_BASE_ANIM_LED_A0  0x0E8  // Base address; add REG_OFF_ANIM_* offsets to get the correct register
#define REG_BASE_ANIM_LED_A1  0x102  // Base address; add REG_OFF_ANIM_* offsets to get the correct register
#define REG_BASE_ANIM_LED_A2  0x11C  // Base address; add REG_OFF_ANIM_* offsets to get the correct register
#define REG_BASE_ANIM_LED_B0  0x136  // Base address; add REG_OFF_ANIM_* offsets to get the correct register
#define REG_BASE_ANIM_LED_B1  0x150  // Base address; add REG_OFF_ANIM_* offsets to get the correct register
#define REG_BASE_ANIM_LED_B2  0x16A  // Base address; add REG_OFF_ANIM_* offsets to get the correct register
#define REG_BASE_ANIM_LED_C0  0x184  // Base address; add REG_OFF_ANIM_* offsets to get the correct register
#define REG_BASE_ANIM_LED_C1  0x19E  // Base address; add REG_OFF_ANIM_* offsets to get the correct register
#define REG_BASE_ANIM_LED_C2  0x1B8  // Base address; add REG_OFF_ANIM_* offsets to get the correct register
#define REG_BASE_ANIM_LED_D0  0x1D2  // Base address; add REG_OFF_ANIM_* offsets to get the correct register
#define REG_BASE_ANIM_LED_D1  0x1EC  // Base address; add REG_OFF_ANIM_* offsets to get the correct register
#define REG_BASE_ANIM_LED_D2  0x206  // Base address; add REG_OFF_ANIM_* offsets to get the correct register
#define REG_TSD_CONFIG_STATUS 0x300
#define REG_LOD_STATUS_0      0x301
#define REG_LOD_STATUS_1      0x302
#define REG_LSD_STATUS_0      0x303
#define REG_LSD_STATUS_1      0x304
#define REG_AUTO_PWM_0        0x305
#define REG_AUTO_PWM_1        0x306
#define REG_AUTO_PWM_2        0x307
#define REG_AUTO_PWM_3        0x308
#define REG_AUTO_PWM_4        0x309
#define REG_AUTO_PWM_5        0x30A
#define REG_AUTO_PWM_6        0x30B
#define REG_AUTO_PWM_7        0x30C
#define REG_AUTO_PWM_8        0x30D
#define REG_AUTO_PWM_9        0x30E
#define REG_AUTO_PWM_10       0x30F
#define REG_AUTO_PWM_11       0x310
#define REG_AUTO_PWM_12       0x311
#define REG_AUTO_PWM_13       0x312
#define REG_AUTO_PWM_14       0x313
#define REG_AUTO_PWM_15       0x314
#define REG_AEP_STATUS_0      0x315
#define REG_AEP_STATUS_1      0x316
#define REG_AEP_STATUS_2      0x317
#define REG_AEP_STATUS_3      0x318
#define REG_AEP_STATUS_4      0x319
#define REG_AEP_STATUS_5      0x31A
#define REG_AEP_STATUS_6      0x31B
#define REG_AEP_STATUS_7      0x31C

// For REG_DEV_CONFIG_0
#define BOOST_VOUT_3V0           (0 << 1)
#define BOOST_VOUT_3V1           (1 << 1)
#define BOOST_VOUT_3V2           (2 << 1)
#define BOOST_VOUT_3V3           (3 << 1)
#define BOOST_VOUT_3V4           (4 << 1)
#define BOOST_VOUT_3V5           (5 << 1)
#define BOOST_VOUT_3V6           (6 << 1)
#define BOOST_VOUT_3V7           (7 << 1)
#define BOOST_VOUT_3V8           (8 << 1)
#define BOOST_VOUT_3V9           (9 << 1)
#define BOOST_VOUT_4V0           (10 << 1)
#define BOOST_VOUT_4V1           (11 << 1)
#define BOOST_VOUT_4V2           (12 << 1)
#define BOOST_VOUT_4V3           (13 << 1)
#define BOOST_VOUT_4V4           (14 << 1)
#define BOOST_VOUT_4V5           (15 << 1)
#define BOOST_VOUT_4V6           (16 << 1)
#define BOOST_VOUT_4V7           (17 << 1)
#define BOOST_VOUT_4V8           (18 << 1)
#define BOOST_VOUT_4V9           (19 << 1)
#define BOOST_VOUT_5V0           (20 << 1)
#define BOOST_VOUT_5V1           (21 << 1)
#define BOOST_VOUT_5V2           (22 << 1)
#define BOOST_VOUT_5V3           (23 << 1)
#define BOOST_VOUT_5V4           (24 << 1)
#define BOOST_VOUT_5V5           (25 << 1)
#define GLOBAL_MAX_CURRENT_25MA5 0x00
#define GLOBAL_MAX_CURRENT_51MA  0x01

// For REG_DEV_CONFIG_1
#define PWM_FRE_24KHZ    0x00
#define PWM_FRE_12KHZ    0x80
#define LED_MODE_1_SCANS (0x1 << 4)
#define LED_MODE_2_SCANS (0x2 << 4)
#define LED_MODE_3_SCANS (0x3 << 4)
#define LED_MODE_4_SCANS (0x4 << 4)

// For REG_DEV_CONFIG_2
#define SCAN_ORDER_0_0H (0x0 << 0)
#define SCAN_ORDER_0_1H (0x1 << 0)
#define SCAN_ORDER_0_2H (0x2 << 0)
#define SCAN_ORDER_0_3H (0x3 << 0)
#define SCAN_ORDER_1_0H (0x0 << 2)
#define SCAN_ORDER_1_1H (0x1 << 2)
#define SCAN_ORDER_1_2H (0x2 << 2)
#define SCAN_ORDER_1_3H (0x3 << 2)
#define SCAN_ORDER_2_0H (0x0 << 4)
#define SCAN_ORDER_2_1H (0x1 << 4)
#define SCAN_ORDER_2_2H (0x2 << 4)
#define SCAN_ORDER_2_3H (0x3 << 4)
#define SCAN_ORDER_3_0H (0x0 << 6)
#define SCAN_ORDER_3_1H (0x1 << 6)
#define SCAN_ORDER_3_2H (0x2 << 6)
#define SCAN_ORDER_3_3H (0x3 << 6)

// For REG_TSD_CONFIG_STATUS
#define TSD_STATUS        0x02
#define CONFIG_ERR_STATUS 0x01

// For REG_DEV_CONFIG_3-6 and REG_LED_EN_1
#define LED_EN_0  0x01
#define LED_EN_1  0x02
#define LED_EN_2  0x04
#define LED_EN_3  0x08
#define LED_EN_A0 0x10
#define LED_EN_A1 0x20
#define LED_EN_A2 0x40
#define LED_EN_B0 0x80

// For REG_DEV_CONFIG_3-6 and REG_LED_EN_2
#define LED_EN_B1 0x01
#define LED_EN_B2 0x02
#define LED_EN_C0 0x04
#define LED_EN_C1 0x08
#define LED_EN_C2 0x10
#define LED_EN_D0 0x20
#define LED_EN_D1 0x40
#define LED_EN_D2 0x80

// For REG_LED_*_AUTO_PAUSE and REG_LED_*_AEU*_T*;
// may need to be shifted left by 4 bits to select the correct field
#define DUR_0_MS    0x0
#define DUR_90_MS   0x1
#define DUR_180_MS  0x2
#define DUR_360_MS  0x3
#define DUR_540_MS  0x4
#define DUR_800_MS  0x5
#define DUR_1070_MS 0x6
#define DUR_1520_MS 0x7
#define DUR_2060_MS 0x8
#define DUR_2500_MS 0x9
#define DUR_3040_MS 0xA
#define DUR_4020_MS 0xB
#define DUR_5010_MS 0xC
#define DUR_5990_MS 0xD
#define DUR_7060_MS 0xE
#define DUR_8050_MS 0xF

// Offsets for REG_BASE_ANIM_LED_* registers
#define REG_OFF_ANIM_AUTO_PAUSE    0
#define REG_OFF_ANIM_AUTO_PLAYBACK 1
#define REG_OFF_ANIM_AEU1_PWM_1    2
#define REG_OFF_ANIM_AEU1_PWM_2    3
#define REG_OFF_ANIM_AEU1_PWM_3    4
#define REG_OFF_ANIM_AEU1_PWM_4    5
#define REG_OFF_ANIM_AEU1_PWM_5    6
#define REG_OFF_ANIM_AEU1_T12      7
#define REG_OFF_ANIM_AEU1_T34      8
#define REG_OFF_ANIM_AEU1_PLAYBACK 9
#define REG_OFF_ANIM_AEU2_PWM_1    10
#define REG_OFF_ANIM_AEU2_PWM_2    11
#define REG_OFF_ANIM_AEU2_PWM_3    12
#define REG_OFF_ANIM_AEU2_PWM_4    13
#define REG_OFF_ANIM_AEU2_PWM_5    14
#define REG_OFF_ANIM_AEU2_T12      15
#define REG_OFF_ANIM_AEU2_T34      16
#define REG_OFF_ANIM_AEU2_PLAYBACK 17
#define REG_OFF_ANIM_AEU3_PWM_1    18
#define REG_OFF_ANIM_AEU3_PWM_2    19
#define REG_OFF_ANIM_AEU3_PWM_3    20
#define REG_OFF_ANIM_AEU3_PWM_4    21
#define REG_OFF_ANIM_AEU3_PWM_5    22
#define REG_OFF_ANIM_AEU3_T12      23
#define REG_OFF_ANIM_AEU3_T34      24
#define REG_OFF_ANIM_AEU3_PLAYBACK 25

// For REG_OFF_ANIM_AUTO_PLAYBACK
#define ACTIVE_AEU_1       0x00
#define ACTIVE_AEU_1_2     0x10
#define ACTIVE_AEU_1_2_3   0x20
#define PLAYBACK_TIMES_1   0x00
#define PLAYBACK_TIMES_2   0x01
#define PLAYBACK_TIMES_3   0x02
#define PLAYBACK_TIMES_4   0x03
#define PLAYBACK_TIMES_5   0x04
#define PLAYBACK_TIMES_6   0x05
#define PLAYBACK_TIMES_7   0x06
#define PLAYBACK_TIMES_8   0x07
#define PLAYBACK_TIMES_9   0x08
#define PLAYBACK_TIMES_10  0x09
#define PLAYBACK_TIMES_11  0x0A
#define PLAYBACK_TIMES_12  0x0B
#define PLAYBACK_TIMES_13  0x0C
#define PLAYBACK_TIMES_14  0x0D
#define PLAYBACK_TIMES_15  0x0E
#define PLAYBACK_TIMES_INF 0x0F

// For REG_OFF_ANIM_AEU*_PLAYBACK
#define AEU_PLAYBACK_TIMES_1   0x00
#define AEU_PLAYBACK_TIMES_2   0x01
#define AEU_PLAYBACK_TIMES_3   0x02
#define AEU_PLAYBACK_TIMES_INF 0x03

// I2C device and EN output
static const struct device *i2c_dev      = DEVICE_DT_GET(DT_NODELABEL(i2c0));
static const struct gpio_dt_spec en_gpio = GPIO_DT_SPEC_GET(DT_NODELABEL(lp5813_en), gpios);

// FIFO for communicating with the LEDs thread
struct play_cmd_t {
    void *fifo_reserved;
    enum leds_led_t led;
    enum leds_pattern_t pattern;
    struct leds_color_t color;
    int reps;  // -1 = infinite, 0 = stop current pattern and shut down LED driver
    leds_finished_cb_t cb;
};

K_FIFO_DEFINE(leds_play_cmd_fifo);

// Global state
static bool led_driver_enabled = false;

/*********************************************************************************************************************
 * PRIVATE FUNCTIONS
 *********************************************************************************************************************/
static bool init_gpio_and_i2c() {
    if (!device_is_ready(i2c_dev)) {
        LOG_ERR("I2C device is not ready");
        return false;
    }

    if (!gpio_is_ready_dt(&en_gpio)) {
        LOG_ERR("GPIO for EN signal is not ready");
        return false;
    }

    int res = gpio_pin_configure_dt(&en_gpio, GPIO_OUTPUT_INACTIVE);
    if (res != 0) {
        LOG_ERR("Failed to configure EN signal GPIO: %d", res);
        return false;
    }

    res = gpio_pin_set_dt(&en_gpio, 0);
    if (res != 0) {
        LOG_ERR("Failed to set EN signal to 0: %d", res);
        return false;
    }

    led_driver_enabled = false;

    return true;
}

static int lp5813_write_reg(uint16_t reg, uint8_t value) {
    if (reg >> 10) {
        LOG_ERR("Invalid LP5813 register address: %X", (int)reg);
        return -EINVAL;
    }

    int res = i2c_reg_write_byte(i2c_dev, (I2C_ADDR << 2) | (reg >> 8), reg & 0xFF, value);
    if (res < 0) {
        LOG_ERR("Failed to write to LP5813 register %X", (int)reg);
    }

    return res;
}

static int lp5813_write_multiple(uint16_t start_reg, const uint8_t *values, uint32_t num_values) {
    if (start_reg >> 10) {
        LOG_ERR("Invalid LP5813 register address: %X", (int)start_reg);
        return -EINVAL;
    }

    int res = i2c_burst_write(i2c_dev, (I2C_ADDR << 2) | (start_reg >> 8), start_reg & 0xFF, values, num_values);
    if (res < 0) {
        LOG_ERR("Failed to write to LP5813 registers starting from %X", (int)start_reg);
    }

    return res;
}

static int lp5813_read_reg(uint16_t reg, uint8_t *value) {
    if (reg >> 10) {
        LOG_ERR("Invalid LP5813 register address: %X", (int)reg);
        return -EINVAL;
    }

    int res = i2c_reg_read_byte(i2c_dev, (I2C_ADDR << 2) | (reg >> 8), reg & 0xFF, value);
    if (res < 0) {
        LOG_ERR("Failed to read from LP5813 register %X", (int)reg);
    }

    return res;
}

static int lp5813_send_cmd(uint16_t cmd_reg) {
    uint8_t value;
    switch (cmd_reg) {
        case REG_SW_RESET:
            value = 0x66;
            break;

        case REG_CMD_UPDATE:
            value = 0x55;
            break;

        case REG_CMD_START:
            value = 0xFF;
            break;

        case REG_CMD_STOP:
            value = 0xAA;
            break;

        case REG_CMD_PAUSE:
            value = 0x33;
            break;

        case REG_CMD_CONTINUE:
            value = 0xCC;
            break;

        default:
            LOG_ERR("Invalid LP5813 command register: %X", (int)cmd_reg);
            return -EINVAL;
    }

    return lp5813_write_reg(cmd_reg, value);
}

static bool enabled_led_driver() {
    // Set the EN pin to enable/disable the LED driver
    int res = gpio_pin_set_dt(&en_gpio, 1);
    if (res != 0) {
        LOG_ERR("Failed to set EN signal to 1: %d", res);
        return false;
    }

    // Wait around 1 ms for the boost converter to stabilize (datasheet says to wait around 1 ms)
    k_msleep(1);

    // Initialize the LED driver; RGB leds have a max fwd voltage of 3.6V @ 20mA
    res = lp5813_write_reg(REG_CHIP_EN, 0x01);
    if (res != 0) goto error;

    res = lp5813_write_reg(REG_DEV_CONFIG_0, BOOST_VOUT_3V6 | GLOBAL_MAX_CURRENT_25MA5);
    if (res != 0) goto error;

    res = lp5813_write_reg(REG_DEV_CONFIG_1, PWM_FRE_24KHZ | LED_MODE_2_SCANS);
    if (res != 0) goto error;

    res = lp5813_write_reg(REG_DEV_CONFIG_2, SCAN_ORDER_0_0H | SCAN_ORDER_1_1H);
    if (res != 0) goto error;

    res = lp5813_write_reg(REG_DEV_CONFIG_3, LED_EN_A0 | LED_EN_A1 | LED_EN_A2 | LED_EN_B0);
    if (res != 0) goto error;

    res = lp5813_write_reg(REG_DEV_CONFIG_4, LED_EN_B1 | LED_EN_B2);
    if (res != 0) goto error;

    res = lp5813_write_reg(REG_DEV_CONFIG_12, 0x0B);  // Avoid incorrect LSD detection
    if (res != 0) goto error;

    res = lp5813_send_cmd(REG_CMD_UPDATE);
    if (res != 0) goto error;

    // Check that the configuration is proper
    uint8_t value = 0xFF;

    res = lp5813_read_reg(REG_TSD_CONFIG_STATUS, &value);
    if (res != 0) goto error;

    if (value & CONFIG_ERR_STATUS) {
        LOG_ERR("LP5813 configuration is not proper (config_err_status in TSD_Config_Status register is 1)");
        goto error;
    }

    // Set the maximum LED currents
    res = lp5813_write_reg(REG_AUTO_DC_D1_R, MAX_LED_CURRENT_FRACTION);
    if (res < 0) goto error;

    res = lp5813_write_reg(REG_AUTO_DC_D1_G, MAX_LED_CURRENT_FRACTION);
    if (res < 0) goto error;

    res = lp5813_write_reg(REG_AUTO_DC_D1_B, MAX_LED_CURRENT_FRACTION);
    if (res < 0) goto error;

    res = lp5813_write_reg(REG_AUTO_DC_D2_R, MAX_LED_CURRENT_FRACTION);
    if (res < 0) goto error;

    res = lp5813_write_reg(REG_AUTO_DC_D2_G, MAX_LED_CURRENT_FRACTION);
    if (res < 0) goto error;

    res = lp5813_write_reg(REG_AUTO_DC_D2_B, MAX_LED_CURRENT_FRACTION);
    if (res < 0) goto error;

    led_driver_enabled = true;

    return true;

    // In case of an error, we pull the enable pin low to disable the LED driver
error:
    gpio_pin_set_dt(&en_gpio, 0);
    led_driver_enabled = false;
    LOG_ERR("Failed to enable the LED driver");
    return false;
}

static bool disable_led_driver() {
    int res = gpio_pin_set_dt(&en_gpio, 0);
    if (res != 0) {
        LOG_ERR("Failed to set EN signal to 0: %d", res);
        return false;
    }

    led_driver_enabled = false;

    return true;
}

static bool start_animation(enum leds_led_t led, enum leds_pattern_t pattern, struct leds_color_t color, int reps,
                            k_timepoint_t *finish_time) {
    // Stop the current animation if one is playing
    lp5813_send_cmd(REG_CMD_STOP);

    // If reps == 0, we do not start a new animation
    if (reps == 0) {
        *finish_time = sys_timepoint_calc(K_FOREVER);
        return true;
    }

    // Convert the number of repetitions to the appropriate value for the LP5813 register
    if (reps < -1 || reps > 15) {
        LOG_ERR("Invalid number of repetitions: %d", reps);
        return false;
    }

    uint8_t playback_times = reps == -1 ? PLAYBACK_TIMES_INF : (uint8_t)(reps - 1);

    // Make an indexable arrays for colors and base register addresses
    uint8_t rgb_brightnesses[] = {color.r, color.g, color.b};
    uint16_t reg_base_d1[]     = {REG_BASE_ANIM_D1_R, REG_BASE_ANIM_D1_G, REG_BASE_ANIM_D1_B};
    uint16_t reg_base_d2[]     = {REG_BASE_ANIM_D2_R, REG_BASE_ANIM_D2_G, REG_BASE_ANIM_D2_B};
    uint16_t *reg_base_sel     = led == LEDS_D1 ? reg_base_d1 : reg_base_d2;

    // Calculate timing information for the AEU registers and for the finish time
    uint8_t dur_fade_in  = DUR_0_MS;
    uint8_t dur_on       = DUR_0_MS;
    uint8_t dur_fade_out = DUR_0_MS;
    uint8_t dur_off      = DUR_0_MS;

    switch (pattern) {
        case LEDS_SOLID:
            *finish_time = sys_timepoint_calc(K_FOREVER);
            break;

        case LEDS_FLASH:
            dur_on       = DUR_540_MS;
            dur_off      = DUR_540_MS;
            *finish_time = sys_timepoint_calc(K_MSEC(reps * (540 + 540)));
            break;

        case LEDS_BREATHE:
            dur_fade_in  = DUR_540_MS;
            dur_fade_out = DUR_540_MS;
            *finish_time = sys_timepoint_calc(K_MSEC(reps * (540 + 540)));
            break;

        default:
            LOG_ERR("Invalid LED pattern: %d", pattern);
            return false;
    }

    // Assign the registers according to the given LED pattern
    for (int i = 0; i < 3; ++i) {
        uint8_t brightness     = rgb_brightnesses[i];
        uint8_t brightness_off = pattern == LEDS_SOLID ? brightness : 0;

        uint8_t values[] = {
            (DUR_0_MS << 4) | DUR_0_MS,     // Auto_Pause
            ACTIVE_AEU_1 | playback_times,  // Auto_Playback
            brightness_off,                 // AEU1_PWM_1
            brightness,                     // AEU1_PWM_2
            brightness,                     // AEU1_PWM_3
            brightness_off,                 // AEU1_PWM_4
            brightness_off,                 // AEU1_PWM_5
            (dur_on << 4) | dur_fade_in,    // AEU1_T12
            (dur_off << 4) | dur_fade_out,  // AEU1_T34
            AEU_PLAYBACK_TIMES_1,           // AEU1_Playback
        };

        lp5813_write_multiple(reg_base_sel[i], values, ARRAY_SIZE(values));
    }

    // Enable only the selected RGB LED (otherwise we get some pre-programmed animations - datasheet error?)
    uint8_t led_en_vals[2];
    if (led == LEDS_D1) {
        led_en_vals[0] = LED_EN_A0 | LED_EN_A1 | LED_EN_A2;
        led_en_vals[1] = 0;
    } else {
        led_en_vals[0] = LED_EN_B0;
        led_en_vals[1] = LED_EN_B1 | LED_EN_B2;
    }

    if (lp5813_write_multiple(REG_LED_EN_1, led_en_vals, ARRAY_SIZE(led_en_vals)) != 0) goto error;

    // Start the animation
    if (lp5813_send_cmd(REG_CMD_UPDATE) != 0) goto error;
    if (lp5813_send_cmd(REG_CMD_START) != 0) goto error;
    return true;

error:
    LOG_ERR("Failed to start the animation");
    return false;
}

/*********************************************************************************************************************
 * THREADS
 *********************************************************************************************************************/
static void leds_thread_fn() {
    if (!init_gpio_and_i2c()) {
        return;
    }

    LOG_INF("LEDs module initialized OK; waiting for commands");

    k_timepoint_t finish_time      = sys_timepoint_calc(K_FOREVER);
    leds_finished_cb_t finished_cb = NULL;

    // Main loop, executing commands and updating the LED driver
    while (true) {
        // Wait for a new play command or until the finish time has been reached
        k_timeout_t timeout    = sys_timepoint_timeout(finish_time);
        struct play_cmd_t *cmd = k_fifo_get(&leds_play_cmd_fifo, timeout);

        // Disable the LED driver (LED driver will go into low-power mode)
        if (led_driver_enabled) {
            disable_led_driver();
        }

        // Call the finished callback with the aborted flag set appropriately
        if (finished_cb) {
            bool aborted = !sys_timepoint_expired(finish_time);
            finished_cb(aborted);
            finished_cb = NULL;
        }

        // If we didn't receive a command, wait for the next one
        if (!cmd) {
            finish_time = sys_timepoint_calc(K_FOREVER);
            continue;
        }

        // If the command is not a stop-command, enabled the LED driver and start the animation
        bool ok = true;
        if (cmd->reps != 0) {
            ok = enabled_led_driver() && start_animation(cmd->led, cmd->pattern, cmd->color, cmd->reps, &finish_time);
        }

        // If we successfully started the animation, we save the finished callback, otherwise
        // we call the callback with aborted=true (more information will be in the logs)
        if (ok) {
            finished_cb = cmd->cb;
        } else if (cmd->cb) {
            finish_time = sys_timepoint_calc(K_FOREVER);
            cmd->cb(true);
        }

        // Free the command (it was allocated leds_play())
        k_free(cmd);
    }
}

K_THREAD_DEFINE(leds_thread_id, THREAD_STACK_SIZE, leds_thread_fn, NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);

/*********************************************************************************************************************
 * PUBLIC FUNCTIONS
 *********************************************************************************************************************/
int leds_play(enum leds_led_t led, enum leds_pattern_t pattern, struct leds_color_t color, int reps,
              leds_finished_cb_t cb) {
    struct play_cmd_t *cmd = k_malloc(sizeof(struct play_cmd_t));
    if (!cmd) {
        LOG_ERR("Unable to allocate FIFO data item from heap");
        return -ENOMEM;
    }

    cmd->fifo_reserved = NULL;
    cmd->led           = led;
    cmd->pattern       = pattern;
    cmd->color         = color;
    cmd->reps          = reps;
    cmd->cb            = cb;

    k_fifo_put(&leds_play_cmd_fifo, cmd);

    return 0;
}

int leds_off() {
    return leds_play(LEDS_D1, LEDS_SOLID, LEDS_RGB(0, 0, 0), 0, NULL);
}
