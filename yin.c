#include "yin.h"
#include <math.h>

static YinConfig g_config = {
    .gpioPin = -1,
    .sampleRate = YIN_DEFAULT_SAMPLE_RATE,
    .threshold = YIN_DEFAULT_THRESHOLD,
    .bufferSize = YIN_DEFAULT_BUFFER_SIZE
};

int yin_init(const YinConfig *config) {
    if (!config) return -1;
    g_config = *config;
    return 0;
}

float yin_getPitch(const float *buffer, int bufferSize, int sampleRate, float threshold) {
    float yinBuffer[bufferSize / 2];
    int tau;
    // Step 1: Difference function
    for (tau = 0; tau < bufferSize / 2; tau++) {
        float sum = 0.0f;
        for (int i = 0; i < bufferSize / 2; i++) {
            float delta = buffer[i] - buffer[i + tau];
            sum += delta * delta;
        }
        yinBuffer[tau] = sum;
    }

    // Step 2: Cumulative mean normalized difference
    yinBuffer[0] = 1.0f;
    float runningSum = 0.0f;
    for (tau = 1; tau < bufferSize / 2; tau++) {
        runningSum += yinBuffer[tau];
        yinBuffer[tau] *= tau / runningSum;
    }

    // Step 3: Absolute threshold
    for (tau = 2; tau < bufferSize / 2; tau++) {
        if (yinBuffer[tau] < threshold) {
            // Step 4: Parabolic interpolation
            while (tau + 1 < bufferSize / 2 && yinBuffer[tau + 1] < yinBuffer[tau]) tau++;
            float betterTau = tau;
            if (tau > 1 && tau < bufferSize / 2 - 1) {
                float s0 = yinBuffer[tau - 1];
                float s1 = yinBuffer[tau];
                float s2 = yinBuffer[tau + 1];
                betterTau = tau + (s2 - s0) / (2.0f * (2.0f * s1 - s2 - s0));
            }
            return (float)sampleRate / betterTau;
        }
    }
    return 0.0f;
}

/* Convenience wrapper that uses the global configuration set by yin_init */
float yin_compute_pitch(const float *buffer, size_t bufferSize) {
    return yin_getPitch(buffer, (int)bufferSize, g_config.sampleRate, g_config.threshold);
}
