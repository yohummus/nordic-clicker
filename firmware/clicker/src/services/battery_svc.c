#include "battery_svc.h"
#include "../battery.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app_battery_svc);

// Thread configuration
#define THREAD_STACK_SIZE 1024
#define THREAD_PRIORITY   10
#define UPDATE_INTERVAL   K_SECONDS(30)

// Semaphores
K_SEM_DEFINE(battery_svc_thread_enable, 1, 1);

/*********************************************************************************************************************
 * THREADS
 *********************************************************************************************************************/
static void bas_thread_fn() {
    // Wait for the signal to start the thread
    k_sem_take(&battery_svc_thread_enable, K_FOREVER);
    LOG_INF("Battery service thread initialized OK");

    // Main loop, measuring the state of charge and updating the battery service
    while (true) {
        int mv  = battery_get_voltage_mv();
        int soc = battery_get_soc_percent(mv);

        int res = bt_bas_set_battery_level((uint8_t)soc);
        if (res) {
            LOG_ERR("bt_bas_set_battery_level() returned %d", res);
        }

        LOG_INF("Battery voltage: %d mV, SOC: %d%%", mv, soc);

        k_sleep(UPDATE_INTERVAL);
    }
}

K_THREAD_DEFINE(battery_svc_thread_id, THREAD_STACK_SIZE, bas_thread_fn, NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);

/*********************************************************************************************************************
 * PUBLIC FUNCTIONS
 *********************************************************************************************************************/

void battery_svc_init() {
    // Start the battery service thread
    k_sem_give(&battery_svc_thread_enable);
}
