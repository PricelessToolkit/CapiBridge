#include "firmware_update.h"

#include <Update.h>
#include <esp_ota_ops.h>

namespace {
const char kFirmwareIdentity[] = "CapiBridge:ESP1:settings-v1";
}

size_t FirmwareUpload::capacity() {
  const esp_partition_t* next = esp_ota_get_next_update_partition(nullptr);
  const esp_partition_t* running = esp_ota_get_running_partition();
  return next && running && next->address != running->address ? next->size : 0;
}

void FirmwareUpload::cancel(const String& reason) {
  if (Update.isRunning()) Update.abort();
  active_ = false;
  error_ = reason;
}

bool FirmwareUpload::begin(size_t size) {
  if (Update.isRunning()) Update.abort();
  expected_ = size;
  received_ = headerSize_ = markerMatched_ = 0;
  markerFound_ = false;
  active_ = false;
  error_ = "";
  if (capacity() == 0) {
    cancel("No OTA firmware slot. Install an OTA partition scheme over USB first.");
    return false;
  }
  if (size < sizeof(header_) || size > capacity()) {
    cancel("Firmware is empty, incomplete, or too large for the OTA slot.");
    return false;
  }
  active_ = true;
  return true;
}

bool FirmwareUpload::write(const uint8_t* data, size_t size) {
  if (!active_) return false;
  if (size > expected_ - received_) {
    cancel("Firmware exceeds the declared file size.");
    return false;
  }
  received_ += size;
  // Scan across chunk boundaries without retaining the firmware in RAM.
  for (size_t i = 0; i < size && !markerFound_; ++i) {
    if (data[i] == static_cast<uint8_t>(kFirmwareIdentity[markerMatched_])) {
      markerFound_ = ++markerMatched_ == sizeof(kFirmwareIdentity) - 1;
    } else {
      markerMatched_ = data[i] == kFirmwareIdentity[0] ? 1 : 0;
    }
  }
  if (headerSize_ < sizeof(header_)) {
    size_t take = min(size, sizeof(header_) - headerSize_);
    memcpy(header_ + headerSize_, data, take);
    headerSize_ += take;
    data += take;
    size -= take;
    if (headerSize_ < sizeof(header_)) return true;

    esp_image_header_t image;
    memcpy(&image, header_, sizeof(image));
    if (image.magic != ESP_IMAGE_HEADER_MAGIC || image.chip_id != ESP_CHIP_ID_ESP32C3) {
      cancel("Choose an ESP32-C3 CapiBridge ESP1 application .bin file.");
      return false;
    }
    if (!Update.begin(expected_, U_FLASH)) {
      cancel(String("Cannot start update: ") + Update.errorString());
      return false;
    }
    if (Update.write(header_, sizeof(header_)) != sizeof(header_)) {
      cancel(String("Firmware write failed: ") + Update.errorString());
      return false;
    }
  }
  if (size && Update.write(const_cast<uint8_t*>(data), size) != size) {
    cancel(String("Firmware write failed: ") + Update.errorString());
    return false;
  }
  return true;
}

bool FirmwareUpload::finish() {
  if (!active_) return false;
  if (received_ != expected_ || !markerFound_) {
    cancel(received_ != expected_ ? "Incomplete firmware upload." : "This file is not OTA-capable CapiBridge ESP1 firmware.");
    return false;
  }
  // end() verifies the complete image before selecting it for the next boot.
  if (!Update.end()) {
    cancel(String("Firmware verification failed: ") + Update.errorString());
    return false;
  }
  active_ = false;
  return true;
}
