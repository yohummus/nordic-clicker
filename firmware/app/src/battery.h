#ifndef BATTERY_H
#define BATTERY_H

/**
 * @brief Gets the latest measurement of the battery voltage.
 *
 * @retval >0 Battery voltage in mV.
 * @retval -1 In case of an error.
 */
int battery_get_voltage_mv();

/**
 * @brief Estimates the remaining capacity of the CR2032 battery as a percentage.
 *
 * This is a (very) rough estimate based on the voltage reading.
 *
 * @param bat_voltage_mv The battery voltage in mV.
 * @retval 0..100 Remaining capacity in percentage.
 */
int battery_get_soc_percent(int bat_voltage_mv);

#endif  // BATTERY_H
