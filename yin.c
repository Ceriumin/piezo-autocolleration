#include "yin.h"
#include <math.h>

static float yinBufferInternal[BUFFER_SIZE / 2]; // BUFFER_SIZE defined in firmware. But not accessible here.


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
