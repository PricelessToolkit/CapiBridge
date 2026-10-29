#pragma once
#include <cstdint>
constexpr uint8_t ESP_IMAGE_HEADER_MAGIC = 0xE9;
constexpr uint16_t ESP_CHIP_ID_ESP32C3 = 5;
struct esp_image_header_t {
  uint8_t magic;
  uint8_t padding[11];
  uint16_t chip_id;
  uint8_t rest[10];
};
static_assert(sizeof(esp_image_header_t) == 24);
