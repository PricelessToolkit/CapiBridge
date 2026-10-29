#pragma once

#include <Arduino.h>
#include <esp_app_format.h>

// This marker is deliberately part of every OTA-capable ESP1 image. It prevents
// accidentally installing ESP2, merged flash images, or unrelated ESP32 sketches.
class FirmwareUpload {
 public:
  static size_t capacity();
  bool begin(size_t size);
  bool write(const uint8_t* data, size_t size);
  bool finish();
  void cancel(const String& reason);
  const String& error() const { return error_; }

 private:
  size_t expected_ = 0;
  size_t received_ = 0;
  uint8_t header_[sizeof(esp_image_header_t)]{};
  size_t headerSize_ = 0;
  size_t markerMatched_ = 0;
  bool markerFound_ = false;
  bool active_ = false;
  String error_;
};
