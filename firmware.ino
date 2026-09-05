#define SAMPLE_RATE 10000
#define BUFFER_SIZE 256 // Must be power of 2
#define ADC_PIN 34

float audioBuffer[BUFFER_SIZE];
int bufferIndex = 0;
unsigned long lastSampleMicros = 0;

float yinBuffer[BUFFER_SIZE / 2];
const float threshold = 0.35f;

// Only report pitch when the signal is loud enough
const float rmsThreshold = 10.0f;
const float minFreq = 70.0f, maxFreq = 1200.0f; // plausible guitar range

void setup() {
  Serial.begin(115200);
  delay(2000);
  pinMode(ADC_PIN, INPUT);
}

void loop() {
  // Sample the piezo at a fixed rate
  if (micros() - lastSampleMicros < 1000000UL / SAMPLE_RATE) return;
  lastSampleMicros = micros();

  audioBuffer[bufferIndex++] = (float)analogRead(ADC_PIN);
  if (bufferIndex < BUFFER_SIZE) return;
  bufferIndex = 0;

  // Compute signal level (remove DC offset so silence reads as ~0)
  float mean = 0.0f;
  for (int i = 0; i < BUFFER_SIZE; i++) mean += audioBuffer[i];
  mean /= BUFFER_SIZE;

  float sumSq = 0.0f;
  for (int i = 0; i < BUFFER_SIZE; i++) {
    audioBuffer[i] -= mean;
    sumSq += audioBuffer[i] * audioBuffer[i];
  }
  float rms = sqrtf(sumSq / BUFFER_SIZE);

  if (rms < rmsThreshold) return;

  float freq = yin_getPitch(audioBuffer, BUFFER_SIZE, SAMPLE_RATE, threshold);
  if (freq >= minFreq && freq <= maxFreq) {
    Serial.print("Estimated Frequency: ");
    Serial.print(freq, 2);
    Serial.println(" Hz");
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
