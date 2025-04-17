#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <dk_buttons_and_leds.h>

LOG_MODULE_REGISTER(app_main);

int main(void) {
    LOG_INF("Starting main loop...");

    int i = 0;
    while (1) {
        LOG_INF("Still alive %d", i);
        ++i;

        k_sleep(K_SECONDS(1));
    }

    return 0;
}
