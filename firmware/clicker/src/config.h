#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

struct config_t {
    uint8_t gazell_secret_key[16];
    uint8_t gazell_pairing_addr[5];
    uint8_t gazell_packet_valid_id[3];
    uint8_t gazell_system_addr[5];
    uint8_t gazell_host_id[5];
};

/**
 * @brief Load the configuration from persistent storage.
 *
 * @param config Pointer to the configuration structure to fill.
 *
 * @retval 0 If successful.
 * @retval <0 Error code if loading the configuration failed.
 */
int config_load(struct config_t* config);

/**
 * @brief Save the configuration to persistent storage.
 *
 * @param config Pointer to the configuration structure to save.
 *
 * @retval 0 If successful.
 * @retval <0 Error code if saving the configuration failed.
 */
int config_save(const struct config_t* config);

#endif  // CONFIG_H
