#include "yin.h"
#include <stdio.h>

int main(void) {
    float buffer[256] = {0};

    int freq = (int)yin_getPitch(buffer, 256, 10000, 0.35f);
    printf("Estimated Frequency: %d Hz\n", freq);
    return 0;
}
