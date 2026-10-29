#include "firmware_update.h"
#include "web_auth.h"
#include "settings.h"
#include <Update.h>
#include <Preferences.h>
#include <esp_ota_ops.h>
#include <cassert>
#include <iostream>

std::vector<uint8_t> image(bool identity = true) {
  esp_image_header_t header{};
  header.magic = ESP_IMAGE_HEADER_MAGIC;
  header.chip_id = ESP_CHIP_ID_ESP32C3;
  std::vector<uint8_t> bytes(128, 0);
  memcpy(bytes.data(), &header, sizeof(header));
  if (identity) {
    const char* marker = "CapiBridge:ESP1:settings-v1";
    memcpy(bytes.data() + 37, marker, strlen(marker));
  }
  return bytes;
}
int main() {
  FirmwareUpload upload;
  auto bytes = image();
  assert(upload.begin(bytes.size()));
  // Header and identity span many chunks, including single-byte writes.
  for (auto byte : bytes) assert(upload.write(&byte, 1));
  assert(upload.finish() && Update.activated && Update.bytes == bytes);
  assert(upload.begin(bytes.size()));
  assert(upload.write(bytes.data(), bytes.size() - 1));
  assert(!upload.finish() && !Update.activated);
  assert(upload.begin(bytes.size()));
  assert(!upload.write(bytes.data(), bytes.size() + 1));
  auto wrong = bytes; wrong[0] = 0;
  assert(upload.begin(wrong.size()));
  assert(!upload.write(wrong.data(), wrong.size()));
  wrong = bytes; wrong[12] = 0; // ESP32 instead of C3.
  assert(upload.begin(wrong.size()));
  assert(!upload.write(wrong.data(), wrong.size()));
  wrong = image(false); // ESP2 or an unrelated sketch.
  assert(upload.begin(wrong.size()));
  assert(upload.write(wrong.data(), wrong.size()));
  assert(!upload.finish() && !Update.activated);
  slotAvailable = false;
  assert(!upload.begin(bytes.size()));
  slotAvailable = true;
  nextPartition.address = runningPartition.address;
  assert(!upload.begin(bytes.size()));
  nextPartition.address = 0x150000;
  assert(!upload.begin(nextPartition.size + 1));
  assert(!upload.begin(0));
  Update.failBegin = true;
  assert(upload.begin(bytes.size()));
  assert(!upload.write(bytes.data(), bytes.size()));
  Update.failBegin = false;
  Update.failWrite = true;
  assert(upload.begin(bytes.size()));
  assert(!upload.write(bytes.data(), bytes.size()));
  Update.failWrite = false;
  assert(upload.begin(bytes.size()));
  assert(upload.write(bytes.data(), bytes.size()));
  Update.failEnd = true;
  assert(!upload.finish() && !Update.activated);
  Update.failEnd = false;
  assert(upload.begin(bytes.size()));
  assert(upload.write(bytes.data(), 40));
  upload.cancel("disconnected");
  assert(!Update.isRunning() && !upload.finish());
  assert(upload.begin(bytes.size()));
  assert(upload.write(bytes.data(), bytes.size()));
  assert(upload.finish());

  // Actual version-1 settings survive a new runtime's compiled defaults.
  SettingsStore store;
  GatewaySettings original, restored;
  setDefaultSettings(original);
  strcpy(original.wifiSsid, "client-network");
  strcpy(original.wifiPassword, "client-wifi-password");
  strcpy(original.mqttServer, "client-broker");
  strcpy(original.mqttPassword, "client-mqtt-password");
  original.loraModule = LORA_MODULE_SX1276;
  assert(store.save(original));
  setDefaultSettings(restored);
  assert(store.load(restored));
  assert(memcmp(&original, &restored, sizeof(original)) == 0);
  assert(store.wasLoadedFromFlash());
  auto savedSettings = preferences["settings"];
  preferences["settings"][4] = 99; // Unknown schema must not overwrite saved data.
  assert(!store.load(restored));
  assert(preferences["settings"][4] == 99);
  assert(store.hasRejectedStoredSettings());
  preferences["settings"] = savedSettings;
  assert(store.load(restored));
  assert(!store.hasRejectedStoredSettings());
  memset(original.wifiSsid, 'x', sizeof(original.wifiSsid));
  String validationError;
  assert(!validateSettings(original, validationError));
  storageFails = true;
  assert(!store.save(restored));
  storageFails = false;
  assert(preferences["settings"] == savedSettings);

  // Legacy settings are left untouched, and a client's login beats new defaults.
  preferences["settings"] = {1, 2, 3, 4};
  WebUiCredentials credentials{};
  assert(loadOrCreateWebUiCredentials(credentials));
  strcpy(credentials.username, "client");
  strcpy(credentials.password, "client-password");
  Preferences prefs;
  prefs.putBytes("webAuth", &credentials, sizeof(credentials));
  WebUiCredentials loaded{};
  assert(loadOrCreateWebUiCredentials(loaded));
  assert(strcmp(loaded.password, "client-password") == 0);
  assert(strcmp(loaded.username, "client") == 0);
  assert((preferences["settings"] == std::vector<uint8_t>{1, 2, 3, 4}));
  preferences["webAuth"].resize(1);
  assert(!loadOrCreateWebUiCredentials(loaded));
  storageFails = true;
  assert(!loadOrCreateWebUiCredentials(loaded));
  std::cout << "OTA streaming, error handling, and login persistence checks passed\n";
}
