#include "config_svc.h"
#include "../config.h"

#include <zephyr/bluetooth/gatt.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app_config_svc);

// UUIDs
#define BT_UUID_CONFIG_SVC_GAZELL_SECRET_KEY_VAL \
    BT_UUID_128_ENCODE(0x456bdbd8, 0x0ad5, 0x401a, 0x898f, 0xd0505330d97a)  // Secret Key for Gazell (16 bytes)

#define BT_UUID_CONFIG_SVC_GAZELL_PAIRING_ADDR_VAL \
    BT_UUID_128_ENCODE(0x456bdbd9, 0x0ad5, 0x401a, 0x898f, 0xd0505330d97a)  // Pairing address for Gazell (5 bytes)

#define BT_UUID_CONFIG_SVC_GAZELL_PACKET_VALID_ID_VAL \
    BT_UUID_128_ENCODE(0x456bdbda, 0x0ad5, 0x401a, 0x898f, 0xd0505330d97a)  // Packet validation ID for Gazell (3 bytes)

#define BT_UUID_CONFIG_SVC_GAZELL_SYSTEM_ADDR_VAL \
    BT_UUID_128_ENCODE(0x456bdbdb, 0x0ad5, 0x401a, 0x898f, 0xd0505330d97a)  // System address for Gazell (5 bytes)

#define BT_UUID_CONFIG_SVC_GAZELL_HOST_ID_VAL \
    BT_UUID_128_ENCODE(0x456bdbdc, 0x0ad5, 0x401a, 0x898f, 0xd0505330d97a)  // Host ID for Gazell (5 bytes)

#define BT_UUID_CONFIG_SVC                        BT_UUID_DECLARE_128(BT_UUID_CONFIG_SVC_VAL)
#define BT_UUID_CONFIG_SVC_GAZELL_SECRET_KEY      BT_UUID_DECLARE_128(BT_UUID_CONFIG_SVC_GAZELL_SECRET_KEY_VAL)
#define BT_UUID_CONFIG_SVC_GAZELL_PAIRING_ADDR    BT_UUID_DECLARE_128(BT_UUID_CONFIG_SVC_GAZELL_PAIRING_ADDR_VAL)
#define BT_UUID_CONFIG_SVC_GAZELL_PACKET_VALID_ID BT_UUID_DECLARE_128(BT_UUID_CONFIG_SVC_GAZELL_PACKET_VALID_ID_VAL)
#define BT_UUID_CONFIG_SVC_GAZELL_SYSTEM_ADDR     BT_UUID_DECLARE_128(BT_UUID_CONFIG_SVC_GAZELL_SYSTEM_ADDR_VAL)
#define BT_UUID_CONFIG_SVC_GAZELL_HOST_ID         BT_UUID_DECLARE_128(BT_UUID_CONFIG_SVC_GAZELL_HOST_ID_VAL)

// Global state
static struct config_t config;

/*********************************************************************************************************************
 * SERVICE CALLBACKS
 *********************************************************************************************************************/
static ssize_t read_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len,
                       uint16_t offset) {
    const uint8_t *data = attr->user_data;
    uint16_t data_len   = 0;

    if (data == config.gazell_secret_key)
        data_len = sizeof(config.gazell_secret_key);
    else if (data == config.gazell_pairing_addr)
        data_len = sizeof(config.gazell_pairing_addr);
    else if (data == config.gazell_packet_valid_id)
        data_len = sizeof(config.gazell_packet_valid_id);
    else if (data == config.gazell_system_addr)
        data_len = sizeof(config.gazell_system_addr);
    else if (data == config.gazell_host_id)
        data_len = sizeof(config.gazell_host_id);

    return bt_gatt_attr_read(conn, attr, buf, len, offset, data, data_len);
}

static ssize_t write_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, uint16_t len,
                        uint16_t offset, uint8_t flags) {
    uint8_t *data     = attr->user_data;
    uint16_t data_len = 0;

    if (data == config.gazell_secret_key)
        data_len = sizeof(config.gazell_secret_key);
    else if (data == config.gazell_pairing_addr)
        data_len = sizeof(config.gazell_pairing_addr);
    else if (data == config.gazell_packet_valid_id)
        data_len = sizeof(config.gazell_packet_valid_id);
    else if (data == config.gazell_system_addr)
        data_len = sizeof(config.gazell_system_addr);
    else if (data == config.gazell_host_id)
        data_len = sizeof(config.gazell_host_id);

    if (len != data_len) {
        LOG_ERR("Invalid length for write: %d != %d", len, data_len);
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    }

    if (offset != 0) {
        LOG_ERR("Invalid offset for write: %d != 0", offset);
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    memcpy(data, buf, len);
    config_save(&config);

    return len;
}

/*********************************************************************************************************************
 * SERVICE DECLARATION
 *********************************************************************************************************************/
BT_GATT_SERVICE_DEFINE(                           // Service declaration
    config_svc,                                   // Service name
    BT_GATT_PRIMARY_SERVICE(BT_UUID_CONFIG_SVC),  // Service UUID

    // Secret key characteristic
    BT_GATT_CHARACTERISTIC(BT_UUID_CONFIG_SVC_GAZELL_SECRET_KEY,    // UUID
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,  // Attribute properties
                           BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,  // Attribute access permissions
                           read_cb,                                 // Attribute read callback
                           write_cb,                                // Attribute write callback
                           config.gazell_secret_key),               // Attribute user data

    // Pairing address characteristic
    BT_GATT_CHARACTERISTIC(BT_UUID_CONFIG_SVC_GAZELL_PAIRING_ADDR,  // UUID
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,  // Attribute properties
                           BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,  // Attribute access permissions
                           read_cb,                                 // Attribute read callback
                           write_cb,                                // Attribute write callback
                           config.gazell_pairing_addr),             // Attribute user data

    // Packet validation ID characteristic
    BT_GATT_CHARACTERISTIC(BT_UUID_CONFIG_SVC_GAZELL_PACKET_VALID_ID,  // UUID
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,     // Attribute properties
                           BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,     // Attribute access permissions
                           read_cb,                                    // Attribute read callback
                           write_cb,                                   // Attribute write callback
                           config.gazell_packet_valid_id),             // Attribute user data

    // System address characteristic
    BT_GATT_CHARACTERISTIC(BT_UUID_CONFIG_SVC_GAZELL_SYSTEM_ADDR,   // UUID
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,  // Attribute properties
                           BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,  // Attribute access permissions
                           read_cb,                                 // Attribute read callback
                           write_cb,                                // Attribute write callback
                           config.gazell_system_addr),              // Attribute user data

    // Host ID characteristic
    BT_GATT_CHARACTERISTIC(BT_UUID_CONFIG_SVC_GAZELL_HOST_ID,       // UUID
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,  // Attribute properties
                           BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,  // Attribute access permissions
                           read_cb,                                 // Attribute read callback
                           write_cb,                                // Attribute write callback
                           config.gazell_host_id),                  // Attribute user data
);

/*********************************************************************************************************************
 * PUBLIC FUNCTIONS
 *********************************************************************************************************************/

void config_svc_init() {
    config_load(&config);
}
