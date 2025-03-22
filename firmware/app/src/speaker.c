#include "speaker.h"

#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app_speaker);

#define AUDIO_CHANNEL 0
#define BOOST_CHANNEL 1

static const struct device *pwm = DEVICE_DT_GET(DT_NODELABEL(pwm0));

// Public functions
int speaker_init() {
    if (!device_is_ready(pwm)) {
        LOG_ERR("PWM device is not ready");
        return -EIO;
    }

    return 0;
}

int speaker_set_frequency(uint32_t frequency) {
    // Calculate the period and pulse length from the given frequency
    uint32_t period;
    uint32_t pulse;

    if (frequency <= 0) {
        period = PWM_HZ(1);
        pulse  = 0;
    } else {
        period = PWM_HZ(frequency);
        pulse  = period / 2;
    }

    // Set the two PWM channels out-of phase to increase the volume
    int res;
    res = pwm_set(pwm, 0, period, pulse, PWM_POLARITY_NORMAL);
    if (res < 0) goto error;

    res = pwm_set(pwm, 1, period, pulse, PWM_POLARITY_INVERTED);
    if (res < 0) goto error;

    return 0;

error:
    LOG_ERR("Failed to set PWM parameters: %d", res);
    return res;
}
