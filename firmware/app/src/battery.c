#include <zephyr/drivers/adc.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app_battery);

// Thread configuration
#define THREAD_STACK_SIZE 1024
#define THREAD_PRIORITY   10

// ADC device
static const struct adc_dt_spec adc = ADC_DT_SPEC_GET(DT_PATH(zephyr_user));

/*********************************************************************************************************************
 * PRIVATE FUNCTIONS
 *********************************************************************************************************************/

bool init_adc() {
    if (!adc_is_ready_dt(&adc)) {
        LOG_ERR("ADC device is not ready");
        return false;
    }

    int err = adc_channel_setup_dt(&adc);
    if (err) {
        LOG_ERR("Failed to setup ADC channel: %d", err);
        return false;
    }

    return true;
}

int read_adc() {
    return 0;
}

/*********************************************************************************************************************
 * THREADS
 *********************************************************************************************************************/
static void battery_thread_fn() {
    if (!init_adc()) {
        return;
    }
}

K_THREAD_DEFINE(battery_thread_id, THREAD_STACK_SIZE, battery_thread_fn, NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);

/*********************************************************************************************************************
 * PUBLIC FUNCTIONS
 *********************************************************************************************************************/
int battery_get_voltage() {
    int16_t buf;
    struct adc_sequence seq = {0};

    int res = adc_sequence_init_dt(&adc, &seq);
    if (res) {
        LOG_ERR("Failed to initialize ADC sequence: %d", res);
        return -1;
    }

    seq.buffer      = &buf;
    seq.buffer_size = sizeof(buf);

    res = adc_read_dt(&adc, &seq);
    if (res) {
        LOG_ERR("Failed to read ADC: %d", res);
        return -1;
    }

    int32_t mv = (int32_t)buf;
    res        = adc_raw_to_millivolts_dt(&adc, &mv);
    if (res) {
        LOG_ERR("Failed to convert ADC raw value to millivolts: %d", res);
        return -1;
    }

    return (int)mv;
}

// ADC_REF_INTERNAL = 0.6V
// 0.6V / (2 ^ 12) = 0.000146484375V resolution
// At VDD=3.000V, we measure 3440
// 3440 * 0.000146484375V * 6 = 3.000V
// 3 / 5 = 0.6V