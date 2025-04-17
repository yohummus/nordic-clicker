#ifndef CONFIG_SVC_H
#define CONFIG_SVC_H

#include <zephyr/bluetooth/uuid.h>

// UUIDs
#define BT_UUID_CONFIG_SVC_VAL BT_UUID_128_ENCODE(0x456bdbd7, 0x0ad5, 0x401a, 0x898f, 0xd0505330d97a)

/**
 * @brief Initializes the config service.
 *
 * @retval 0 If successful.
 * @retval <0 Error code if initialization failed.
 */
void config_svc_init();

#endif  // CONFIG_SVC_H
