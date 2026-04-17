#include "settings.h"
#include "settings_defaults.h"

namespace {
// Preferences keys and low-level helpers stay private to this translation unit.
const char* kPrefsNamespace = "capibridge";
const char* kPrefsBlobKey = "settings";
const uint8_t kDefaultEncryptionKey[] = encryption_key;

// Copy a C string into a fixed-size field while always keeping it null-terminated.
void copyStringField(char* dest, size_t destSize, const char* value) {
  if (destSize == 0) {
    return;
  }

  if (value == nullptr) {
    dest[0] = '\0';
    return;
  }

  strlcpy(dest, value, destSize);
}

// Parse values like 0x12 or 18 from JSON into a single encryption-key byte.
bool parseHexByte(const char* text, uint8_t& value) {
  if (text == nullptr || *text == '\0') {
    return false;
  }

  char* end = nullptr;
  unsigned long parsed = strtoul(text, &end, 0);
  if (end == text || *end != '\0' || parsed > 0xFFUL) {
    return false;
  }

  value = static_cast<uint8_t>(parsed);
  return true;
}

// Clamp the encryption key to the supported size range and clear unused bytes.
void sanitizeEncryptionKey(GatewaySettings& settings) {
  if (settings.encryptionKeyLength < 2) {
    settings.encryptionKeyLength = 2;
  }
  if (settings.encryptionKeyLength > SETTINGS_MAX_KEY_LENGTH) {
    settings.encryptionKeyLength = SETTINGS_MAX_KEY_LENGTH;
  }

  for (uint8_t i = settings.encryptionKeyLength; i < SETTINGS_MAX_KEY_LENGTH; ++i) {
    settings.encryptionKey[i] = 0;
  }
}
}

// Load settings from flash only if the stored blob matches our version and passes validation.
bool SettingsStore::load(GatewaySettings& settings) {
  Preferences prefs;
  if (!prefs.begin(kPrefsNamespace, true)) {
    loadedFromFlash_ = false;
    return false;
  }

  GatewaySettings stored;
  size_t read = prefs.getBytes(kPrefsBlobKey, &stored, sizeof(stored));
  prefs.end();

  if (read != sizeof(stored) ||
      stored.magic != SETTINGS_MAGIC ||
      stored.version != SETTINGS_VERSION) {
    loadedFromFlash_ = false;
    return false;
  }

  String error;
  sanitizeEncryptionKey(stored);
  if (!validateSettings(stored, error)) {
    loadedFromFlash_ = false;
    return false;
  }

  settings = stored;
  loadedFromFlash_ = true;
  return true;
}

// Save the full settings struct as one blob so boot-time loading stays simple and fast.
bool SettingsStore::save(const GatewaySettings& settings) {
  Preferences prefs;
  if (!prefs.begin(kPrefsNamespace, false)) {
    return false;
  }

  size_t written = prefs.putBytes(kPrefsBlobKey, &settings, sizeof(settings));
  prefs.end();
  loadedFromFlash_ = written == sizeof(settings);
  return loadedFromFlash_;
}

// Remove the stored blob so the next boot falls back to internal upload defaults.
bool SettingsStore::reset() {
  Preferences prefs;
  if (!prefs.begin(kPrefsNamespace, false)) {
    return false;
  }

  bool ok = prefs.remove(kPrefsBlobKey);
  prefs.end();
  loadedFromFlash_ = false;
  return ok;
}

bool SettingsStore::wasLoadedFromFlash() const {
  return loadedFromFlash_;
}

// Build the initial runtime settings object from internal upload defaults.
void setDefaultSettings(GatewaySettings& settings) {
  memset(&settings, 0, sizeof(settings));
  settings.magic = SETTINGS_MAGIC;
  settings.version = SETTINGS_VERSION;
  copyStringField(settings.gatewayKey, sizeof(settings.gatewayKey), GATEWAY_KEY);
  settings.loraEncryption = LoRa_Encryption;
  settings.espnowEncryption = ESPNOW_Encryption;
  settings.encryptionKeyLength = encryption_key_length;
  memset(settings.encryptionKey, 0, sizeof(settings.encryptionKey));
  memcpy(settings.encryptionKey, kDefaultEncryptionKey,
         min(static_cast<size_t>(settings.encryptionKeyLength), sizeof(kDefaultEncryptionKey)));
  copyStringField(settings.wifiSsid, sizeof(settings.wifiSsid), WIFI_SSID);
  copyStringField(settings.wifiPassword, sizeof(settings.wifiPassword), WIFI_PASSWORD);
  copyStringField(settings.mqttUsername, sizeof(settings.mqttUsername), MQTT_USERNAME);
  copyStringField(settings.mqttPassword, sizeof(settings.mqttPassword), MQTT_PASSWORD);
  copyStringField(settings.mqttServer, sizeof(settings.mqttServer), MQTT_SERVER);
  settings.mqttPort = MQTT_PORT;
  settings.discoveryEveryPacket = DISCOVERY_EVERY_PACKET;
  settings.loraModule = LORA_MODULE;
  settings.band = BAND;
  settings.loraTxPower = LORA_TX_POWER;
  settings.loraSignalBandwidth = LORA_SIGNAL_BANDWIDTH;
  settings.loraSpreadingFactor = LORA_SPREADING_FACTOR;
  settings.loraCodingRate = LORA_CODING_RATE;
  settings.loraSyncWord = LORA_SYNC_WORD;
  settings.loraPreambleLength = LORA_PREAMBLE_LENGTH;
  settings.rowDebug = ROW_Debug;
  sanitizeEncryptionKey(settings);
}

// Reject impossible or unsupported combinations before they reach the radio or web UI.
bool validateSettings(const GatewaySettings& settings, String& error) {
  if (strlen(settings.gatewayKey) == 0) {
    error = "Gateway key cannot be empty.";
    return false;
  }

  if (settings.encryptionKeyLength < 2 || settings.encryptionKeyLength > SETTINGS_MAX_KEY_LENGTH) {
    error = "Encryption key length must be between 2 and 16 bytes.";
    return false;
  }

  if (settings.mqttPort == 0) {
    error = "MQTT port must be greater than 0.";
    return false;
  }

  if (settings.loraModule != LORA_MODULE_SX1276 &&
      settings.loraModule != LORA_MODULE_SX1262 &&
      settings.loraModule != LORA_MODULE_SX1268) {
    error = "Unsupported LoRa module selected.";
    return false;
  }

  if (settings.band != 433.0f && settings.band != 868.0f && settings.band != 915.0f) {
    error = "Band must be 433.0, 868.0 or 915.0 MHz.";
    return false;
  }

  if (settings.loraSignalBandwidth != 125.0f &&
      settings.loraSignalBandwidth != 250.0f &&
      settings.loraSignalBandwidth != 500.0f) {
    error = "Bandwidth must be 125.0, 250.0 or 500.0 kHz.";
    return false;
  }

  if (settings.loraCodingRate < 5 || settings.loraCodingRate > 8) {
    error = "Coding rate must be between 5 and 8.";
    return false;
  }

  if (settings.loraPreambleLength < 6) {
    error = "Preamble length must be at least 6.";
    return false;
  }

  if (settings.loraModule == LORA_MODULE_SX1276) {
    if (settings.loraSpreadingFactor < 6 || settings.loraSpreadingFactor > 12) {
      error = "SX1276 supports spreading factor 6 to 12.";
      return false;
    }
    if (settings.loraTxPower < 2 || settings.loraTxPower > 20) {
      error = "SX1276 TX power must be between 2 and 20 dBm.";
      return false;
    }
  } else {
    if (settings.loraSpreadingFactor < 5 || settings.loraSpreadingFactor > 11) {
      error = "SX1262/SX1268 support spreading factor 5 to 11.";
      return false;
    }
    if (settings.loraTxPower < -9 || settings.loraTxPower > 22) {
      error = "SX1262/SX1268 TX power must be between -9 and 22 dBm.";
      return false;
    }
    if (settings.loraSignalBandwidth == 125.0f && settings.loraSpreadingFactor > 9) {
      error = "At 125 kHz, SX126x supports spreading factor up to 9.";
      return false;
    }
    if (settings.loraSignalBandwidth == 250.0f && settings.loraSpreadingFactor > 10) {
      error = "At 250 kHz, SX126x supports spreading factor up to 10.";
      return false;
    }
  }

  return true;
}

// Convert the runtime struct into JSON for the embedded web UI.
void settingsToJson(const GatewaySettings& settings, JsonDocument& doc, bool includeSecrets) {
  doc["gatewayKey"] = settings.gatewayKey;
  doc["loraEncryption"] = settings.loraEncryption;
  doc["espnowEncryption"] = settings.espnowEncryption;
  doc["encryptionKeyLength"] = settings.encryptionKeyLength;

  JsonArray encryptionArray = doc.createNestedArray("encryptionKey");
  for (uint8_t i = 0; i < settings.encryptionKeyLength; ++i) {
    char hexValue[5];
    snprintf(hexValue, sizeof(hexValue), "0x%02X", settings.encryptionKey[i]);
    encryptionArray.add(hexValue);
  }

  doc["wifiSsid"] = settings.wifiSsid;
  doc["wifiPassword"] = includeSecrets ? settings.wifiPassword : "********";
  doc["mqttUsername"] = settings.mqttUsername;
  doc["mqttPassword"] = includeSecrets ? settings.mqttPassword : "********";
  doc["mqttServer"] = settings.mqttServer;
  doc["mqttPort"] = settings.mqttPort;
  doc["discoveryEveryPacket"] = settings.discoveryEveryPacket;
  doc["loraModule"] = settings.loraModule;
  doc["loraModuleLabel"] = getLoRaModuleLabel(settings.loraModule);
  doc["band"] = settings.band;
  doc["loraTxPower"] = settings.loraTxPower;
  doc["loraSignalBandwidth"] = settings.loraSignalBandwidth;
  doc["loraSpreadingFactor"] = settings.loraSpreadingFactor;
  doc["loraCodingRate"] = settings.loraCodingRate;
  doc["loraSyncWord"] = settings.loraSyncWord;
  doc["loraPreambleLength"] = settings.loraPreambleLength;
  doc["rowDebug"] = settings.rowDebug;
}

// Merge JSON from the web UI into a copy of the current settings and validate the result.
bool settingsFromJson(const JsonDocument& doc, GatewaySettings& settings, String& error) {
  GatewaySettings updated = settings;

  if (doc["gatewayKey"].is<const char*>()) {
    copyStringField(updated.gatewayKey, sizeof(updated.gatewayKey), doc["gatewayKey"]);
  }
  if (doc["loraEncryption"].is<bool>()) {
    updated.loraEncryption = doc["loraEncryption"].as<bool>();
  }
  if (doc["espnowEncryption"].is<bool>()) {
    updated.espnowEncryption = doc["espnowEncryption"].as<bool>();
  }
  if (doc["wifiSsid"].is<const char*>()) {
    copyStringField(updated.wifiSsid, sizeof(updated.wifiSsid), doc["wifiSsid"]);
  }
  if (doc["wifiPassword"].is<const char*>()) {
    copyStringField(updated.wifiPassword, sizeof(updated.wifiPassword), doc["wifiPassword"]);
  }
  if (doc["mqttUsername"].is<const char*>()) {
    copyStringField(updated.mqttUsername, sizeof(updated.mqttUsername), doc["mqttUsername"]);
  }
  if (doc["mqttPassword"].is<const char*>()) {
    copyStringField(updated.mqttPassword, sizeof(updated.mqttPassword), doc["mqttPassword"]);
  }
  if (doc["mqttServer"].is<const char*>()) {
    copyStringField(updated.mqttServer, sizeof(updated.mqttServer), doc["mqttServer"]);
  }
  if (doc["mqttPort"].is<int>()) {
    updated.mqttPort = doc["mqttPort"].as<uint16_t>();
  }
  if (doc["discoveryEveryPacket"].is<bool>()) {
    updated.discoveryEveryPacket = doc["discoveryEveryPacket"].as<bool>();
  }
  if (doc["loraModule"].is<int>()) {
    updated.loraModule = doc["loraModule"].as<uint8_t>();
  }
  if (doc["band"].is<int>() || doc["band"].is<float>() || doc["band"].is<double>()) {
    updated.band = doc["band"].as<float>();
  }
  if (doc["loraTxPower"].is<int>()) {
    updated.loraTxPower = doc["loraTxPower"].as<int8_t>();
  }
  if (doc["loraSignalBandwidth"].is<int>() || doc["loraSignalBandwidth"].is<float>() || doc["loraSignalBandwidth"].is<double>()) {
    updated.loraSignalBandwidth = doc["loraSignalBandwidth"].as<float>();
  }
  if (doc["loraSpreadingFactor"].is<int>()) {
    updated.loraSpreadingFactor = doc["loraSpreadingFactor"].as<uint8_t>();
  }
  if (doc["loraCodingRate"].is<int>()) {
    updated.loraCodingRate = doc["loraCodingRate"].as<uint8_t>();
  }
  if (doc["loraSyncWord"].is<int>()) {
    updated.loraSyncWord = doc["loraSyncWord"].as<uint8_t>();
  }
  if (doc["loraPreambleLength"].is<int>()) {
    updated.loraPreambleLength = doc["loraPreambleLength"].as<uint16_t>();
  }
  if (doc["rowDebug"].is<bool>()) {
    updated.rowDebug = doc["rowDebug"].as<bool>();
  }

  if (doc["encryptionKeyLength"].is<int>()) {
    updated.encryptionKeyLength = doc["encryptionKeyLength"].as<uint8_t>();
  }

  if (doc["encryptionKey"].is<JsonArrayConst>()) {
    JsonArrayConst keys = doc["encryptionKey"].as<JsonArrayConst>();
    if (keys.size() < 2 || keys.size() > SETTINGS_MAX_KEY_LENGTH) {
      error = "Encryption key must contain between 2 and 16 bytes.";
      return false;
    }

    updated.encryptionKeyLength = keys.size();
    memset(updated.encryptionKey, 0, sizeof(updated.encryptionKey));
    uint8_t index = 0;
    for (JsonVariantConst item : keys) {
      uint8_t value = 0;
      if (item.is<const char*>()) {
        if (!parseHexByte(item.as<const char*>(), value)) {
          error = "Encryption key bytes must be valid hex or decimal values.";
          return false;
        }
      } else if (item.is<int>()) {
        int raw = item.as<int>();
        if (raw < 0 || raw > 255) {
          error = "Encryption key bytes must be between 0 and 255.";
          return false;
        }
        value = static_cast<uint8_t>(raw);
      } else {
        error = "Encryption key bytes must be strings or integers.";
        return false;
      }
      updated.encryptionKey[index++] = value;
    }
  }

  sanitizeEncryptionKey(updated);

  if (!validateSettings(updated, error)) {
    return false;
  }

  settings = updated;
  return true;
}

// Human-readable labels are used by the UI and boot diagnostics.
const char* getLoRaModuleLabel(uint8_t module) {
  switch (module) {
    case LORA_MODULE_SX1276:
      return "SX1276 / RA-01H";
    case LORA_MODULE_SX1262:
      return "SX1262 / RA-01SH";
    case LORA_MODULE_SX1268:
      return "SX1268 / RA-01S";
    default:
      return "Unknown";
  }
}
