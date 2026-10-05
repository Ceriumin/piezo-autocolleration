#include "yin.h"
#include <stdio.h>

int main(void) {
    float buffer[256] = {0};

    /* Initialize library with default configuration */
    YinConfig cfg = { .gpioPin = -1,
                      .sampleRate = YIN_DEFAULT_SAMPLE_RATE,
                      .threshold = YIN_DEFAULT_THRESHOLD,
                      .bufferSize = 256 };
    yin_init(&cfg);

    int freq = (int)yin_compute_pitch(buffer, 256);
    printf("Estimated Frequency: %d Hz\n", freq);
    return 0;
}
