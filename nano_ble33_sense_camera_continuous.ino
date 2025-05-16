#include <edge-impulse-sdk/classifier/ei_run_classifier.h>
#include <ei_camera.h>

ei_device_snapshot_resolutions_t resolution = EI_CAMERA_OUTPUT_BAYER;
uint8_t *image_data;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (ei_camera_init() != 0) {
    ei_printf("Failed to initialize camera\n");
    return;
  }

  if (ei_camera_select_res(resolution) != 0) {
    ei_printf("Failed to set camera resolution\n");
    return;
  }

  if (ei_camera_set_crop(0, 0) != 0) {
    ei_printf("Failed to crop camera image\n");
    return;
  }

  ei_printf("Edge Impulse object detection example started\n");
}

void loop() {
  // Capture a frame
  if (ei_camera_capture(&image_data, resolution, false) != 0) {
    ei_printf("Failed to capture image\n");
    return;
  }

  ei_impulse_result_t result = { 0 };
  signal_t signal;
  int err = numpy::signal_from_buffer(image_data, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
  if (err != 0) {
    ei_printf("Failed to create signal from buffer (%d)\n", err);
    return;
  }

  EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);
  ei_printf("run_classifier returned: %d\n", res);

  if (res != EI_IMPULSE_OK) return;

  // Print results
  ei_printf("Object detection results:\n");
  for (uint32_t i = 0; i < result.bounding_boxes_count; i++) {
    auto bb = result.bounding_boxes[i];
    if (bb.value == 0) continue;

    ei_printf("Found '%s' (%.2f%%) at [%lu, %lu, %lu, %lu]\n",
              bb.label, bb.value * 100,
              bb.x, bb.y, bb.width, bb.height);
  }

  delay(2000); // wait before next capture
}
