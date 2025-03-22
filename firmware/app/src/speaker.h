#ifndef SPEAKER_H
#define SPEAKER_H

#include <stdint.h>

// Resonant frequency of the speaker in Hz (from the datasheet)
#define SPEAKER_RESONANT_FREQUENCY 4100

/**
 * @brief Initialize this module.
 *
 * @retval 0 If successful.
 * @retval <0 Error code if initialization failed.
 */
int speaker_init();

/**
 * @brief Set the frequency of the speaker.
 *
 * Setting the frequency to 0 disables the speaker.
 *
 * @param frequency The frequency in Hz.
 *
 * @retval 0 If successful.
 * @retval <0 Error code if setting the frequency failed.
 */
int speaker_set_frequency(uint32_t frequency);

#endif  // SPEAKER_H