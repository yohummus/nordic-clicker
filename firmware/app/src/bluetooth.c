#include "bluetooth.h"

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app_bluetooth);

// UUIDs
#define BT_UUID_REMOTE_SERV_VAL BT_UUID_128_ENCODE(0xe9ea0001, 0xe19b, 0x482d, 0x9293, 0xc7907585fc48)

// Advertising data
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, (sizeof(CONFIG_BT_DEVICE_NAME) - 1))};

static const struct bt_data sd[] = {
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_REMOTE_SERV_VAL),
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

    int err = bt_enable(on_bluetooth_ready);
    if (err) {
        LOG_ERR("bt_enable() returned %d", err);
        return err;
    }

    k_sem_take(&bluetooth_ready, K_FOREVER);

    err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (err) {
        LOG_ERR("bt_le_adv_start() returned %d", err);
        return err;
    }

    LOG_INF("Bluetooth initialized OK");

    return 0;
}
