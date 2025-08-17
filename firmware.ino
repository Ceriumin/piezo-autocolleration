#define SAMPLE_RATE 10000
#define BUFFER_SIZE 256 // Must be power of 2
#define ADC_PIN 34

float audioBuffer[BUFFER_SIZE];
int bufferIndex = 0;
unsigned long lastSampleMicros = 0;

float yinBuffer[BUFFER_SIZE / 2];
const float threshold = 0.35f; // tightened from 0.25f

// Simple 1st-order HPF to reduce tap/bang low-frequency thumps
const float HPF_CUTOFF = 40.0f; // Hz (tune 40–120)
float hpAlpha = 0.0f, hpYPrev = 0.0f, hpXPrev = 0.0f;

// Transient gating
float prevRms = 0.0f;
const float crestThreshold = 12.0f;     // peak/RMS
const float rmsJumpRatio = 3.0f;        // sudden energy jump factor
const unsigned long refractoryMs = 150; // mute window after a bang
unsigned long transientRefractoryUntil = 0;

void setup() {
  Serial.begin(115200);
  delay(2000);
  pinMode(ADC_PIN, INPUT);

  // Init HPF coefficient
  float dt = 1.0f / SAMPLE_RATE;
  float rc = 1.0f / (2.0f * 3.14159265f * HPF_CUTOFF);
  hpAlpha = rc / (rc + dt);
}

void loop() {
  if (micros() - lastSampleMicros >= 1000000UL / SAMPLE_RATE) {
    lastSampleMicros = micros();

    int adcValue = analogRead(ADC_PIN);
    // Apply HPF directly on raw ADC (no separate DC offset tracking)
    float x = (float)adcValue;
    float y = hpAlpha * (hpYPrev + x - hpXPrev);
    hpYPrev = y;
    hpXPrev = x;

    audioBuffer[bufferIndex++] = y;

    if (bufferIndex >= BUFFER_SIZE) {
      float sumSq = 0.0f;
      float maxAbs = 0.0f;
      for (int i = 0; i < BUFFER_SIZE; i++) {
        float v = audioBuffer[i];
        sumSq += v * v;
        float av = fabsf(v);
        if (av > maxAbs) maxAbs = av;
      }
      float rms = sqrtf(sumSq / BUFFER_SIZE);
      float crest = (rms > 1e-6f) ? (maxAbs / (rms + 1e-6f)) : 0.0f;

      // Transient detection and refractory gating
      bool transient = (crest > crestThreshold) || (prevRms > 0.0f && (rms / (prevRms + 1e-6f) > rmsJumpRatio));
      if (transient) {
        transientRefractoryUntil = millis() + refractoryMs;
      }

      const float playThreshold = 10.0f;
      const float minFreq = 70.0f, maxFreq = 1200.0f; // plausible guitar range

      float freq = 0.0f;
      bool allow = millis() >= transientRefractoryUntil;

      if (allow && rms > playThreshold) {
        freq = yin_getPitch(audioBuffer, BUFFER_SIZE, SAMPLE_RATE, threshold);
      }

      if (allow && rms > playThreshold && freq >= minFreq && freq <= maxFreq) {
        Serial.print("Estimated Frequency: ");
        Serial.print(freq, 2);
        Serial.println(" Hz");
      }

      prevRms = rms;
      bufferIndex = 0;
    }
  }
}

float yin_getPitch(float* buffer, int bufferSize, int sampleRate, float threshold) {
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
      return sampleRate / betterTau;
    }
  }
  return 0.0f;
}
