#ifndef YIN_H
#define YIN_H

#include <stddef.h>

/**
 * Compute pitch using the YIN algorithm.
 * @param buffer      Pointer to input samples (floating point).
 * @param bufferSize  Number of samples in the buffer.
 * @param sampleRate  Sampling rate in Hz.
 * @param threshold   Absolute threshold for the YIN algorithm.
 * @return Estimated frequency in Hz, or 0.0 if no pitch found.
 */
float yin_getPitch(const float *buffer, int bufferSize, int sampleRate, float threshold);

#endif // YIN_H
