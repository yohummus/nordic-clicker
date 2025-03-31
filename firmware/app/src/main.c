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

static void on_leds_finished(bool aborted) {
    LOG_INF("on_leds_finished(): %s", aborted ? "aborted" : "finished");
}

static void on_speaker_finished(bool aborted) {
    LOG_INF("on_speaker_finished(): %s", aborted ? "aborted" : "finished");
}

int main(void) {
    bool ok = true;
    ok &= bluetooth_init() == 0;

    if (!ok) {
        LOG_ERR("Initialization failed.");
        return 0;
    }

    LOG_INF("Starting main loop...");

    int i = 0;
    while (1) {
        // k_msleep(1000);
        LOG_INF("Still alive %d", i);
        ++i;

        // leds_play(LEDS_D2, LEDS_BREATHE, LEDS_RGB(100, 100, 0), 2, on_leds_finished);
        // speaker_play(SPEAKER_MELODY_ERROR, on_speaker_finished);
        // k_msleep(10000);

        // k_msleep(3000);
        struct buttons_event_t event;
        if (buttons_get_event(&event, K_SECONDS(1)) == 0) {
            LOG_INF("Button: %d, long: %d, shift: %d", event.button + 1, event.is_long_press,
                    event.preceding_short_shift_presses);
        }
    }

    return 0;
}
