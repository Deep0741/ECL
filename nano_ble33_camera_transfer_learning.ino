#include <edge-impulse-sdk/classifier/ei_run_classifier.h>
#include <ei_camera.h>

ei_device_snapshot_resolutions_t resolution = EI_CAMERA_OUTPUT_RGB888;
uint8_t *image_data;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (ei_camera_init() != 0) {
    ei_printf("Failed to initialize camera\n");
    return;
  }

  if (ei_camera_select_res(resolution) != 0) {
    ei_printf("Failed to select camera resolution\n");
    return;
  }

  ei_printf("Nano BLE + Camera - Transfer Learning (Image Classification)\n");
}

void loop() {
  if (ei_camera_capture(&image_data, resolution, false) != 0) {
    ei_printf("Failed to capture image\n");
    return;
  }

  signal_t signal;
  int err = numpy::signal_from_buffer(image_data, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
  if (err != 0) {
    ei_printf("Failed to create signal from image buffer (%d)\n", err);
    return;
  }

  ei_impulse_result_t result = { 0 };
  EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);
  if (res != EI_IMPULSE_OK) {
    ei_printf("Classifier failed (%d)\n", res);
    return;
  }

  ei_printf("Classification results:\n");
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    ei_printf("%s: %.2f\n", result.classification[ix].label, result.classification[ix].value);
  }

  delay(2000); // Wait before next frame
}
