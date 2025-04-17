#include "bluetooth.h"
#include "battery.h"
#include "services/battery_svc.h"
#include "services/config_svc.h"

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app_bluetooth);

// Advertising data
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),  // General flags
    BT_DATA_BYTES(BT_DATA_UUID16_ALL,                                      // 16-bit UUIDs
                  BT_UUID_BATTERY_SVC_VAL),                                // Battery Service
    BT_DATA_BYTES(BT_DATA_UUID128_ALL,                                     // 128-bit UUIDs
                  BT_UUID_CONFIG_SVC_VAL),                                 // Clicker Service
};

static const struct bt_data sd[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, (sizeof(CONFIG_BT_DEVICE_NAME) - 1)),
};

// Semaphores
K_SEM_DEFINE(bluetooth_ready, 1, 1);

/*********************************************************************************************************************
 * PRIVATE FUNCTIONS
 *********************************************************************************************************************/
static void on_bluetooth_ready(int err) {
    if (err) {
        LOG_ERR("on_bluetooth_ready() called with error %d", err);
    }

    k_sem_give(&bluetooth_ready);
}

/*********************************************************************************************************************
 * PUBLIC FUNCTIONS
 *********************************************************************************************************************/
int bluetooth_init() {
    LOG_INF("Initializing Bluetooth...");

    // Enable Bluetooth
    int err = bt_enable(on_bluetooth_ready);
    if (err) {
        LOG_ERR("bt_enable() returned %d", err);
        return err;
    }

    k_sem_take(&bluetooth_ready, K_FOREVER);

    // Initialize the services
    config_svc_init();
    battery_svc_init();

    // Start advertising
    err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (err) {
        LOG_ERR("bt_le_adv_start() returned %d", err);
        return err;
    }

    LOG_INF("Bluetooth initialized OK");

    return 0;
}
