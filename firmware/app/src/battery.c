#include <zephyr/drivers/adc.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app_battery);

// Thread configuration
#define THREAD_STACK_SIZE 1024
#define THREAD_PRIORITY   10

// Timing configuration
#define SAMPLE_INTERVAL       K_SECONDS(1)
#define SAMPLE_AVERAGE_COUNT  4
#define OVERSAMPLING_EXPONENT 3  // Each sample is averaged from 2^OVERSAMPLING_EXPONENT conversion results

// ADC device
static const struct adc_dt_spec adc = ADC_DT_SPEC_GET(DT_PATH(zephyr_user));

// Semaphores and global state
static atomic_t avg_adc_reading;  // 0 initially until first read, -1 in case of an error

/*********************************************************************************************************************
 * PRIVATE FUNCTIONS
 *********************************************************************************************************************/

static bool init_adc() {
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

static bool read_adc(int16_t *val) {
    struct adc_sequence seq = {0};

    int res = adc_sequence_init_dt(&adc, &seq);
    if (res) {
        LOG_ERR("Failed to initialize ADC sequence: %d", res);
        return false;
    }

    seq.buffer       = val;
    seq.buffer_size  = sizeof(*val);
    seq.oversampling = OVERSAMPLING_EXPONENT;

    res = adc_read_dt(&adc, &seq);
    if (res) {
        LOG_ERR("Failed to read ADC: %d", res);
        return false;
    }

    return true;
}

/*********************************************************************************************************************
 * THREADS
 *********************************************************************************************************************/
static void battery_thread_fn() {
    if (!init_adc()) {
        return;
    }

    // Create a buffer for the samples
    int16_t samples[SAMPLE_AVERAGE_COUNT];
    int sample_idx = 0;

    // First reading; set all samples to the same first reading
    int16_t sample;
    if (!read_adc(&sample)) goto error;
    atomic_set(&avg_adc_reading, (int32_t)sample);

    for (int i = 0; i < SAMPLE_AVERAGE_COUNT; ++i) {
        samples[i] = sample;
    }

    LOG_INF("Battery module initialized OK; waiting for commands");

    // Main loop, periodically reading the battery voltage
    while (true) {
        k_sleep(SAMPLE_INTERVAL);

        // Read the ADC into the next sample slot
        if (!read_adc(&samples[sample_idx])) goto error;
        sample_idx = (sample_idx + 1) % SAMPLE_AVERAGE_COUNT;

        // Update the average ADC reading
        int32_t sum = 0;
        for (int i = 0; i < SAMPLE_AVERAGE_COUNT; ++i) {
            sum += samples[i];
        }

        atomic_set(&avg_adc_reading, sum / SAMPLE_AVERAGE_COUNT);
    }

error:
    atomic_set(&avg_adc_reading, -1);
    LOG_ERR("Failed to read ADC; battery module disabled");
}

K_THREAD_DEFINE(battery_thread_id, THREAD_STACK_SIZE, battery_thread_fn, NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);

/*********************************************************************************************************************
 * PUBLIC FUNCTIONS
 *********************************************************************************************************************/
int battery_get_voltage_mv() {
    // Get the reading from the battery thread, if it is 0, we are still initializing
    int32_t raw = atomic_get(&avg_adc_reading);
    while (raw == 0) {
        k_sleep(K_MSEC(1));
        raw = atomic_get(&avg_adc_reading);
    }

    // Convert the ADC reading to millivolts
    int32_t mv = raw;
    int res    = adc_raw_to_millivolts_dt(&adc, &mv);
    if (res) {
        LOG_ERR("Failed to convert ADC raw value to millivolts: %d", res);
        return false;
    }

    return (int)mv;
}

int battery_get_soc_percent(int bat_voltage_mv) {
    if (bat_voltage_mv > 3000) {
        return 100;
    } else if (bat_voltage_mv > 2900) {
        return 50;
    } else if (bat_voltage_mv > 2800) {
        return 5;
    } else if (bat_voltage_mv < 2500) {
        return 0;
    }
}