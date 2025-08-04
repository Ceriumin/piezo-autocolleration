#define SAMPLE_RATE 10000 // 10 kHz sample rate
#define BUFFER_SIZE 512  // Must be power of 2

float audioBuffer[BUFFER_SIZE];
int bufferIndex = 0;
unsigned long lastSampleMicros = 0;

// ADC and DC offset correction parameters
#define ADC_PIN 34
float dcOffset = 0.0;
const float alpha = 0.01; // Smoothing factor for DC offset

// YIN parameters
float yinBuffer[BUFFER_SIZE / 2];
float threshold = 0.15; // Adjust as needed

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("ADC Read Example with DC Offset Correction and YIN Pitch Detection");
  pinMode(34, INPUT);
}

void loop() {
  // Sample at fixed rate
  if (micros() - lastSampleMicros >= 1000000UL / SAMPLE_RATE) {
    lastSampleMicros = micros();

    int adcValue = analogRead(ADC_PIN);
    dcOffset = (1 - alpha) * dcOffset + alpha * adcValue;
    float signal = adcValue - dcOffset;

    audioBuffer[bufferIndex++] = signal;

    if (bufferIndex >= BUFFER_SIZE) {
      float freq = yin_getPitch(audioBuffer, BUFFER_SIZE, SAMPLE_RATE, threshold);
      Serial.print("Estimated Frequency: ");
      Serial.print(freq, 2);
      Serial.println(" Hz");
      bufferIndex = 0;
    }
  }
}

// YIN algorithm implementation
float yin_getPitch(float* buffer, int bufferSize, int sampleRate, float threshold) {
  int tau, i;
  float minVal = 1e10;
  int minTau = -1;

  // Step 1: Difference function
  for (tau = 0; tau < bufferSize / 2; tau++) {
    float sum = 0;
    for (i = 0; i < bufferSize / 2; i++) {
      float delta = buffer[i] - buffer[i + tau];
      sum += delta * delta;
    }
    yinBuffer[tau] = sum;
  }

  // Step 2: Cumulative mean normalized difference
  yinBuffer[0] = 1;
  float runningSum = 0;
  for (tau = 1; tau < bufferSize / 2; tau++) {
    runningSum += yinBuffer[tau];
    yinBuffer[tau] *= tau / runningSum;
  }

  // Step 3: Absolute threshold
  for (tau = 2; tau < bufferSize / 2; tau++) {
    if (yinBuffer[tau] < threshold) {
      // Step 4: Parabolic interpolation for better accuracy
      while (tau + 1 < bufferSize / 2 && yinBuffer[tau + 1] < yinBuffer[tau]) tau++;
      float betterTau = tau;
      if (tau > 1 && tau < bufferSize / 2 - 1) {
        float s0 = yinBuffer[tau - 1];
        float s1 = yinBuffer[tau];
        float s2 = yinBuffer[tau + 1];
        betterTau = tau + (s2 - s0) / (2 * (2 * s1 - s2 - s0));
      }
      return sampleRate / betterTau;
    }
  }
  // No pitch found
  return 0.0;
}