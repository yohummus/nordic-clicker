#include "speaker.h"

#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app_speaker);

// Thread configuration
#define THREAD_STACK_SIZE 1024
#define THREAD_PRIORITY   4

// Macros for readability
#define MELODY_END {.note = 0, .length = 0}

// Notes in Hz
enum note_t {
    REST = 0,
    C4   = 262,
    Db4  = 277,
    D4   = 294,
    Eb4  = 311,
    E4   = 330,
    F4   = 349,
    Gb4  = 370,
    G4   = 392,
    Ab4  = 415,
    A4   = 440,
    Bb4  = 466,
    B4   = 494,
    C5   = 523,
    Db5  = 554,
    D5   = 587,
    Eb5  = 622,
    E5   = 659,
    F5   = 698,
    Gb5  = 740,
    G5   = 784,
    Ab5  = 831,
    A5   = 880,
    Bb5  = 932,
    B5   = 988,
    C6   = 1046,
    Db6  = 1109,
    D6   = 1175,
    Eb6  = 1245,
    E6   = 1319,
    F6   = 1397,
    Gb6  = 1480,
    G6   = 1568,
    Ab6  = 1661,
    A6   = 1760,
    Bb6  = 1865,
    B6   = 1976,
    C7   = 2093,
    Db7  = 2217,
    D7   = 2349,
    Eb7  = 2489,
    E7   = 2637,
    F7   = 2794,
    Gb7  = 2960,
    G7   = 3136,
    Ab7  = 3322,
    A7   = 3520,
    Bb7  = 3729,
    B7   = 3951,
    C8   = 4186,
    Db8  = 4435,
    D8   = 4699,
    Eb8  = 4978,
    E8   = 5274,
    F8   = 5588,
    Gb8  = 5920,
    G8   = 6272,
    Ab8  = 6645,
    A8   = 7040,
    Bb8  = 7459,
    B8   = 7902,
};

// Note lengths in ms
enum note_length_t {
    SIXTEENTH = 38,
    EIGTH     = 75,
    QUARTER   = 150,
    HALF      = 300,
    WHOLE     = 600,
};

// A note and its lengths within a melody
struct melody_note_t {
    uint16_t note;
    uint16_t length;
};

// PWM device connected to the speaker
static const struct device *pwm = DEVICE_DT_GET(DT_NODELABEL(pwm0));

// FIFO for communicating with the speaker thread
struct play_cmd_t {
    void *fifo_reserved;
    const struct melody_note_t *melody;
    speaker_finished_cb_t cb;
};

K_FIFO_DEFINE(speaker_play_cmd_fifo);

/*********************************************************************************************************************
 * PRIVATE FUNCTIONS
 *********************************************************************************************************************/
static const struct melody_note_t *get_melody(enum speaker_melody_t melody) {
    switch (melody) {
        case SPEAKER_MELODY_SUCCESS: {
            static struct melody_note_t mel[] = {
                {.note = C5,   .length = QUARTER}, //
                {.note = REST, .length = EIGTH  }, //
                {.note = C5,   .length = QUARTER}, //
                {.note = Bb4,  .length = QUARTER}, //
                {.note = C5,   .length = QUARTER}, //
                {.note = REST, .length = QUARTER}, //
                {.note = G4,   .length = QUARTER}, //
                {.note = REST, .length = QUARTER}, //
                {.note = G4,   .length = QUARTER}, //
                {.note = C5,   .length = QUARTER}, //
                {.note = F5,   .length = QUARTER}, //
                {.note = E5,   .length = QUARTER}, //
                {.note = C5,   .length = QUARTER}, //
                MELODY_END,
            };
            return mel;
        }

        case SPEAKER_MELODY_ERROR: {
            static struct melody_note_t mel[] = {
                {.note = E6,   .length = QUARTER}, //
                {.note = REST, .length = EIGTH  }, //
                {.note = E6,   .length = QUARTER}, //
                {.note = REST, .length = QUARTER}, //
                {.note = E6,   .length = QUARTER}, //
                {.note = REST, .length = QUARTER}, //
                {.note = C6,   .length = QUARTER}, //
                {.note = E6,   .length = HALF   }, //
                {.note = G6,   .length = HALF   }, //
                {.note = REST, .length = QUARTER}, //
                {.note = G4,   .length = WHOLE  }, //
                {.note = REST, .length = WHOLE  }, //
                {.note = C6,   .length = HALF   }, //
                {.note = REST, .length = QUARTER}, //
                {.note = G5,   .length = HALF   }, //
                {.note = REST, .length = QUARTER}, //
                {.note = E5,   .length = HALF   }, //
                {.note = REST, .length = QUARTER}, //
                {.note = A5,   .length = QUARTER}, //
                {.note = REST, .length = QUARTER}, //
                {.note = B5,   .length = QUARTER}, //
                {.note = REST, .length = QUARTER}, //
                {.note = Bb5,  .length = QUARTER}, //
                {.note = A5,   .length = HALF   }, //
                {.note = G5,   .length = QUARTER}, //
                {.note = E6,   .length = QUARTER}, //
                {.note = G6,   .length = QUARTER}, //
                {.note = A6,   .length = HALF   }, //
                {.note = F6,   .length = QUARTER}, //
                {.note = G6,   .length = QUARTER}, //
                {.note = REST, .length = QUARTER}, //
                {.note = E6,   .length = QUARTER}, //
                {.note = REST, .length = QUARTER}, //
                {.note = C6,   .length = QUARTER}, //
                {.note = D6,   .length = QUARTER}, //
                {.note = B5,   .length = QUARTER}, //
                MELODY_END,
            };
            return mel;
        }

        case SPEAKER_MELODY_LOW_BATTERY: {
            static struct melody_note_t mel[] = {
                {.note = C6,   .length = QUARTER}, //
                {.note = REST, .length = 100    }, //
                {.note = G5,   .length = 100    }, //
                {.note = A5,   .length = 100    }, //
                {.note = Bb5,  .length = 100    }, //
                {.note = REST, .length = 100    }, //
                {.note = Bb5,  .length = 100    }, //
                {.note = REST, .length = QUARTER}, //
                {.note = C5,   .length = HALF   }, //
                {.note = REST, .length = HALF   }, //
                {.note = REST, .length = QUARTER}, //
                {.note = C6,   .length = QUARTER}, //
                MELODY_END,
            };
            return mel;
        }

        default:
            LOG_ERR("Invalid melody: %d", melody);
            return NULL;
    }
}

static bool set_speaker_frequency(uint32_t frequency) {
    // Calculate the period and pulse length from the given frequency
    uint32_t period;
    uint32_t pulse;

    if (frequency == 0) {
        period = PWM_HZ(1);
        pulse  = 0;
    } else {
        period = PWM_HZ(frequency);
        pulse  = period / 2;
    }

    // First, stop the PWM channels (otherwise we'll get an error when changing the period)
    int res;
    res = pwm_set(pwm, 0, 1, 0, PWM_POLARITY_NORMAL);
    if (res < 0) goto error;

    res = pwm_set(pwm, 1, 1, 0, PWM_POLARITY_NORMAL);
    if (res < 0) goto error;

    // Now set the period and pulse length according to the requested frequency
    // (we run the two channels out-of-phase to increase the volume)
    res = pwm_set(pwm, 0, period, pulse, PWM_POLARITY_NORMAL);
    if (res < 0) goto error;

    res = pwm_set(pwm, 1, period, pulse, PWM_POLARITY_INVERTED);
    if (res < 0) goto error;

    return true;

error:
    LOG_ERR("Failed to set PWM parameters: %d", res);
    return false;
}

static int put_play_cmd(const struct melody_note_t *melody, speaker_finished_cb_t cb) {
    struct play_cmd_t *cmd = k_malloc(sizeof(struct play_cmd_t));
    if (!cmd) {
        LOG_ERR("Unable to allocate FIFO data item from heap");
        return -ENOMEM;
    }

    cmd->fifo_reserved = NULL;
    cmd->melody        = melody;
    cmd->cb            = cb;

    k_fifo_put(&speaker_play_cmd_fifo, cmd);

    return 0;
}

/*********************************************************************************************************************
 * THREADS
 *********************************************************************************************************************/

static void speaker_thread_fn() {
    if (!device_is_ready(pwm)) {
        LOG_ERR("PWM device is not ready");
        return;
    }

    LOG_INF("Speaker module initialized OK; waiting for commands");

    const struct melody_note_t *next_note = NULL;
    k_timepoint_t next_note_time          = sys_timepoint_calc(K_FOREVER);
    speaker_finished_cb_t finished_cb     = NULL;

    // Main loop, executing commands and playing melodies
    while (true) {
        // Wait for a new play command or until the next note time has been reached
        k_timeout_t timeout    = sys_timepoint_timeout(next_note_time);
        struct play_cmd_t *cmd = k_fifo_get(&speaker_play_cmd_fifo, timeout);

        // Play the next note in the melody if it's time or stop the melody if it's finished
        if (sys_timepoint_expired(next_note_time)) {
            // Play the next note
            if (next_note && (next_note->note != 0 || next_note->length != 0)) {
                set_speaker_frequency(next_note->note);
                next_note_time = sys_timepoint_calc(K_MSEC(next_note->length));
                next_note++;
            }
            // Melody is finished
            else {
                set_speaker_frequency(0);
                next_note      = NULL;
                next_note_time = sys_timepoint_calc(K_FOREVER);

                if (finished_cb) {
                    finished_cb(false);
                    finished_cb = NULL;
                }
            }
        }

        // If we didn't receive a command, wait for the next one
        if (!cmd) {
            continue;
        }

        // If a melody is currently playing, call the finished callback
        if (next_note) {
            next_note      = NULL;
            next_note_time = sys_timepoint_calc(K_NO_WAIT);

            if (finished_cb) {
                finished_cb(true);
                finished_cb = NULL;
            }
        }

        // If the command is not a stop-command, start playing the new melody
        if (cmd->melody) {
            next_note      = cmd->melody;
            next_note_time = sys_timepoint_calc(K_NO_WAIT);
            finished_cb    = cmd->cb;
        }

        // Free the command (it was allocated in speaker_play())
        k_free(cmd);
    }
}

K_THREAD_DEFINE(speaker_thread_id, THREAD_STACK_SIZE, speaker_thread_fn, NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);

/*********************************************************************************************************************
 * PUBLIC FUNCTIONS
 *********************************************************************************************************************/

int speaker_play(enum speaker_melody_t melody, speaker_finished_cb_t cb) {
    return put_play_cmd(get_melody(melody), cb);
}

int speaker_off() {
    return put_play_cmd(NULL, NULL);
}
