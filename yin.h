#ifndef YIN_H
#define YIN_H

#include <stddef.h>
#include <stdint.h>

/*
 * Default configuration values. Users can override via YinConfig.
 */
#define YIN_DEFAULT_SAMPLE_RATE 44100
#define YIN_DEFAULT_THRESHOLD   0.35f
#define YIN_DEFAULT_BUFFER_SIZE 1024

/*
 * Configuration structure for the YIN algorithm.
 */
typedef struct {
    int gpioPin;            /* GPIO pin number for piezo input (if used)
                               * Not required for the core algorithm.
                               */
    int sampleRate;         /* Sampling rate in Hz. Default: YIN_DEFAULT_SAMPLE_RATE */
    float threshold;        /* Absolute threshold for the algorithm. Default: YIN_DEFAULT_THRESHOLD */
    size_t bufferSize;      /* Number of samples per analysis frame. Default: YIN_DEFAULT_BUFFER_SIZE */
} YinConfig;

/*
 * Initialise the library with a configuration. The function stores the
 * provided settings internally and applies them to subsequent calls.
 */
int yin_init(const YinConfig *config);

/*
 * Compute pitch using the YIN algorithm. This is a low‑level function that
 * expects callers to supply raw samples, buffer size, sample rate and
 * threshold. It is kept for backward compatibility.
 */
float yin_getPitch(const float *buffer, int bufferSize, int sampleRate, float threshold);

/*
 * Convenience wrapper that uses the configuration supplied via yin_init.
 * If yin_init has not been called, default values are used.
 */
float yin_compute_pitch(const float *buffer, size_t bufferSize);

#endif // YIN_H
