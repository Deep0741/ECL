#include <edge-impulse-sdk/classifier/ei_run_classifier.h>
#include <PDM.h>

#define AUDIO_BUFFER_SIZE 1024
static int16_t audioBuffer[AUDIO_BUFFER_SIZE];
volatile bool audioReady = false;

signal_t signal;
EI_IMPULSE_ERROR res;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME) {
    Serial.println("RAW mode not supported.");
    return;
  }

  Serial.println("Edge Impulse NN Classifier - Audio Classification");

  // Start microphone (PDM)
  if (!PDM.begin(1, EI_CLASSIFIER_FREQUENCY)) {
    Serial.println("PDM mic setup failed");
    while (1);
  }

  PDM.setBufferSize(AUDIO_BUFFER_SIZE);
  PDM.onReceive(onPDMdata);
}

void loop() {
  if (!audioReady) return;
  audioReady = false;

  signal.total_length = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
  signal.get_data = &get_signal_data;

  ei_impulse_result_t result;
  res = run_classifier(&signal, &result, false);

  if (res != EI_IMPULSE_OK) {
    Serial.print("run_classifier failed: ");
    Serial.println(res);
    return;
  }

  Serial.println("Predictions:");
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    Serial.print(result.classification[ix].label);
    Serial.print(": ");
    Serial.println(result.classification[ix].value, 3);
  }

  // Optional trigger
  if (result.classification[0].value > 0.8) {
    Serial.println("Action triggered: class 0 detected");
  }
}

void onPDMdata() {
  int bytesAvailable = PDM.available();
  PDM.read((char *)audioBuffer, bytesAvailable);
  audioReady = true;
}

int get_signal_data(size_t offset, size_t length, float *out_ptr) {
  for (size_t i = 0; i < length; i++) {
    out_ptr[i] = (float)audioBuffer[offset + i] / 32768.0f;
  }
  return 0;
}
