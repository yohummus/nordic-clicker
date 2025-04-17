#ifndef BATTERY_SVC_H
#define BATTERY_SVC_H

#include <zephyr/bluetooth/services/bas.h>
#include <zephyr/bluetooth/uuid.h>

// UUIDs
#define BT_UUID_BATTERY_SVC_VAL BT_UUID_16_ENCODE(BT_UUID_BAS_VAL)

/**
 * @brief Initializes the battery service.
 *
 * @retval 0 If successful.
 * @retval <0 Error code if initialization failed.
 */
void battery_svc_init();

#endif  // BATTERY_SVC_H
