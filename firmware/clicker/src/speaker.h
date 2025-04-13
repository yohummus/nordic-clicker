#ifndef SPEAKER_H
#define SPEAKER_H

#include <stdbool.h>
#include <stdint.h>

// Resonant frequency of the speaker in Hz (from the datasheet)
#define SPEAKER_RESONANT_FREQUENCY 4100

enum speaker_melody_t {
    SPEAKER_MELODY_SUCCESS,
    SPEAKER_MELODY_ERROR,
    SPEAKER_MELODY_LOW_BATTERY,
};

typedef void (*speaker_finished_cb_t)(bool aborted);

/**
 * @brief Play a melody on the speaker.
 *
 * If a melody is currently still playing, it will be aborted and the melody finished callback will be called before
 * playing the new melody.
 *
 * @param melody The melody to play.
 * @param cb The callback to call when the melody has finished playing. Can be set to NULL.
 *
 * @retval 0 If successful.
 * @retval <0 Error code if playing the melody failed.
 */
int speaker_play(enum speaker_melody_t melody, speaker_finished_cb_t cb);

/**
 * @brief Stops the melody currently playing, if there is any.
 *
 * @retval 0 If successful.
 * @retval <0 Error code if stopping the melody failed.
 */
int speaker_off();

#endif  // SPEAKER_H