#ifndef BLUETOOTH_H
#define BLUETOOTH_H

/**
 * @brief Initialize the button GPIOs and related interrupts.
 *
 * @retval 0 If successful.
 * @retval <0 Error code if initialization failed.
 */
int buttons_init();

#endif  // BLUETOOTH_H
