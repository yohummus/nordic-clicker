#ifndef BLUETOOTH_H
#define BLUETOOTH_H

/**
 * @brief Initialize Bluetooth and start advertising.
 *
 * @retval 0 If successful.
 * @retval <0 Error code if initialization failed.
 */
int bluetooth_init();

#endif  // BLUETOOTH_H
