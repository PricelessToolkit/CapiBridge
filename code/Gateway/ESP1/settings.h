#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include "config.h"

// Persistent settings are stored as one validated blob in ESP32 Preferences.
static constexpr uint32_t SETTINGS_MAGIC = 0x43424944;  // CBID
static constexpr uint16_t SETTINGS_VERSION = 1;
static constexpr uint8_t SETTINGS_MAX_KEY_LENGTH = 16;

// This struct is the runtime source of truth for every editable gateway setting.
struct GatewaySettings {
  uint32_t magic;
  uint16_t version;
  char gatewayKey[17];
  bool loraEncryption;
  bool espnowEncryption;
  uint8_t encryptionKeyLength;
  uint8_t encryptionKey[SETTINGS_MAX_KEY_LENGTH];
  char wifiSsid[33];
  char wifiPassword[65];
  char mqttUsername[65];
  char mqttPassword[65];
  char mqttServer[65];
  uint16_t mqttPort;
  bool discoveryEveryPacket;
  uint8_t loraModule;
  float band;
  int8_t loraTxPower;
  float loraSignalBandwidth;
  uint8_t loraSpreadingFactor;
  uint8_t loraCodingRate;
  uint8_t loraSyncWord;
  uint16_t loraPreambleLength;
  bool rowDebug;
};

// SettingsStore loads, saves, and resets the settings blob in non-volatile flash.
class SettingsStore {
public:
  bool load(GatewaySettings& settings);
  bool save(const GatewaySettings& settings);
  bool reset();
  bool wasLoadedFromFlash() const;

private:
  bool loadedFromFlash_ = false;
};

// Helper functions convert between upload defaults, runtime validation, and the web API JSON format.
void setDefaultSettings(GatewaySettings& settings);
bool validateSettings(const GatewaySettings& settings, String& error);
void settingsToJson(const GatewaySettings& settings, JsonDocument& doc, bool includeSecrets);
bool settingsFromJson(const JsonDocument& doc, GatewaySettings& settings, String& error);
const char* getLoRaModuleLabel(uint8_t module);
