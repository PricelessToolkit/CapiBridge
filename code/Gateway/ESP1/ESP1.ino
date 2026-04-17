#include <Arduino.h>
#include "settings.h"
#include "radio.h"
#include "web_ui.h"
#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <vector>
#include <math.h>
#include <stdarg.h>

// Mirror the hardware serial output into an in-memory log so the web UI can show a live Raw View terminal.
class MirroredSerial : public Print {
 public:
  explicit MirroredSerial(HardwareSerial& serial) : serial_(serial) {}

  void begin(unsigned long baud) { serial_.begin(baud); }

  size_t write(uint8_t ch) override {
    serial_.write(ch);
    mirrorChar(static_cast<char>(ch));
    return 1;
  }

  size_t write(const uint8_t* buffer, size_t size) override {
    serial_.write(buffer, size);
    for (size_t i = 0; i < size; ++i) {
      mirrorChar(static_cast<char>(buffer[i]));
    }
    return size;
  }

  size_t printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    va_list copy;
    va_copy(copy, args);
    int needed = vsnprintf(nullptr, 0, format, copy);
    va_end(copy);
    if (needed <= 0) {
      va_end(args);
      return 0;
    }
    String buffer;
    buffer.reserve(static_cast<size_t>(needed) + 1);
    char temp[needed + 1];
    vsnprintf(temp, sizeof(temp), format, args);
    va_end(args);
    write(reinterpret_cast<const uint8_t*>(temp), static_cast<size_t>(needed));
    return static_cast<size_t>(needed);
  }

  using Print::write;

 private:
  HardwareSerial& serial_;

  void mirrorChar(char ch);
};

// LoRa RX interrupts only flip this flag; the main loop does the heavier packet handling work.
volatile bool packetReceived = false;
void onReceive() { packetReceived = true; }

#define LED_PIN 2
#define BAUD 115200
#define RXPIN 18
#define TXPIN 19

#define MQTT_RETAIN true
#define BINARY_SENSOR_TOPIC "homeassistant/binary_sensor/"
#define SENSOR_TOPIC "homeassistant/sensor/"
#define CAPIBRIDGE_LWT_TOPIC "capibridge/availability"
#define CAPIBRIDGE_RSSI_TOPIC "homeassistant/sensor/CapiBridge/rssi"
#define CAPIBRIDGE_COMMAND_TOPIC "homeassistant/sensor/CapiBridge/command"
#define AVAIL_ON  "online"
#define AVAIL_OFF "offline"

const char* FIRMWARE_VERSION = "V1.3";

// Global gateway state shared across setup, loop, radio handling, MQTT, and the web API.
RuntimeRadio radio;
GatewaySettings settings;
SettingsStore settingsStore;
WebServer server(80);
WiFiClient espClient;
PubSubClient client(espClient);
std::vector<String> publishedDiscovery;

static constexpr size_t MAX_SERIAL_LOG = 120;
std::vector<String> serialLog;
String serialLineBuffer;

// Keep only the newest serial lines so the browser-side Raw View stays bounded in RAM.
void pushSerialLogLine(const String& line) {
  if (serialLog.size() >= MAX_SERIAL_LOG) {
    serialLog.erase(serialLog.begin());
  }
  String clipped = line.length() <= 480 ? line : line.substring(0, 480) + "\n... truncated ...";
  serialLog.push_back(clipped);
}

// Build log lines one character at a time and flush them whenever Serial prints a newline.
void MirroredSerial::mirrorChar(char ch) {
  if (ch == '\r') {
    return;
  }
  if (ch == '\n') {
    pushSerialLogLine(serialLineBuffer);
    serialLineBuffer = "";
    return;
  }
  serialLineBuffer += ch;
  if (serialLineBuffer.length() > 480) {
    serialLineBuffer.remove(0, serialLineBuffer.length() - 480);
  }
}

static MirroredSerial SerialMirror(::Serial);
#define Serial SerialMirror

String mqttMessage = "";
bool newCommandReceived = false;
bool accessPointActive = false;
String accessPointSsid = "";
unsigned long LedStartTime = 0;
unsigned long diagTimer = 60000;
unsigned long lastDiagTimer = 0;
unsigned long lastWifiTry = 0;
unsigned long lastMqttAttempt = 0;
unsigned long rebootAt = 0;

static constexpr size_t MAX_TRAFFIC_LOG = 30;
static constexpr size_t MAX_WORKING_NODES = 48;
static constexpr size_t MAX_TRAFFIC_TEXT = 480;

struct TrafficEntry {
  unsigned long atMs;
  String source;
  String status;
  String nodeId;
  int rssi;
  String raw;
  String pretty;
};

struct WorkingNodeEntry {
  unsigned long atMs;
  String source;
  String status;
  String nodeId;
  int rssi;
  bool hasBattery;
  String battery;
};

std::vector<TrafficEntry> trafficLog;
std::vector<WorkingNodeEntry> workingNodes;

// Discovery tracking avoids re-sending the same Home Assistant config unless the user asked for it.
static bool discoveryPublishedFor(const String &id, const String &suffix) {
  String key = id + "|" + suffix;
  for (const auto &v : publishedDiscovery) {
    if (v == key) return true;
  }
  return false;
}

static void markDiscoveryPublished(const String &id, const String &suffix) {
  publishedDiscovery.push_back(id + "|" + suffix);
}

// Small formatting helpers used by the status API and boot diagnostics.
String ipToString(const IPAddress& ip) {
  return ip.toString();
}

String formatUptime(unsigned long ms) {
  unsigned long totalSeconds = ms / 1000;
  unsigned long days = totalSeconds / 86400;
  totalSeconds %= 86400;
  unsigned long hours = totalSeconds / 3600;
  totalSeconds %= 3600;
  unsigned long minutes = totalSeconds / 60;
  unsigned long seconds = totalSeconds % 60;

  char buffer[40];
  snprintf(buffer, sizeof(buffer), "%lud %02lu:%02lu:%02lu", days, hours, minutes, seconds);
  return String(buffer);
}

bool hasConfiguredWifi() {
  return strlen(settings.wifiSsid) > 0 && strcmp(settings.wifiSsid, "CHANGE_ME") != 0;
}

bool hasConfiguredMqtt() {
  return strlen(settings.mqttServer) > 0 && strcmp(settings.mqttServer, "CHANGE_ME") != 0;
}

bool webUiPasswordEnabled() {
  return strlen(WEBUI_PASSWORD) > 0;
}

bool ensureWebUiAuth() {
  if (!webUiPasswordEnabled()) {
    return true;
  }

  if (server.authenticate(WEBUI_USERNAME, WEBUI_PASSWORD)) {
    return true;
  }

  server.requestAuthentication(BASIC_AUTH, "CapiBridge", "Enter Web UI password");
  return false;
}

String settingsSourceLabel() {
  return settingsStore.wasLoadedFromFlash() ? "Saved settings" : "Upload defaults";
}

String formatTrafficTime(unsigned long atMs) {
  unsigned long totalSeconds = atMs / 1000;
  unsigned long hours = (totalSeconds / 3600) % 24;
  unsigned long minutes = (totalSeconds / 60) % 60;
  unsigned long seconds = totalSeconds % 60;
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%02lu:%02lu:%02lu", hours, minutes, seconds);
  return String(buffer);
}

String clipTrafficText(const String& text) {
  if (text.length() <= MAX_TRAFFIC_TEXT) {
    return text;
  }
  return text.substring(0, MAX_TRAFFIC_TEXT) + "\n... truncated ...";
}

String formatJsonPretty(const String& raw) {
  StaticJsonDocument<2048> doc;
  if (deserializeJson(doc, raw)) {
    return raw;
  }

  String pretty;
  serializeJsonPretty(doc, pretty);
  return pretty;
}

void updateWorkingNode(const String& source, const String& raw, const String& status, int rssi, const String& nodeId) {
  if (nodeId.isEmpty()) {
    return;
  }

  for (auto it = workingNodes.begin(); it != workingNodes.end(); ++it) {
    if (it->nodeId == nodeId) {
      workingNodes.erase(it);
      break;
    }
  }

  WorkingNodeEntry entry;
  entry.atMs = millis();
  entry.source = source;
  entry.status = status;
  entry.nodeId = nodeId;
  entry.rssi = rssi;
  entry.hasBattery = false;
  entry.battery = "";

  StaticJsonDocument<512> preview;
  if (!deserializeJson(preview, raw) && preview["b"].is<JsonVariantConst>()) {
    entry.hasBattery = true;
    entry.battery = preview["b"].as<String>();
  }

  workingNodes.push_back(entry);

  if (workingNodes.size() > MAX_WORKING_NODES) {
    workingNodes.erase(workingNodes.begin());
  }
}

// Store the most recent parsed payloads for the Monitoring page.
void pushTrafficEntry(const String& source, const String& raw, const String& status, int rssi, const String& nodeId = "") {
  TrafficEntry entry;
  entry.atMs = millis();
  entry.source = source;
  entry.status = status;
  entry.nodeId = nodeId;
  entry.rssi = rssi;
  entry.raw = clipTrafficText(raw);
  entry.pretty = clipTrafficText(formatJsonPretty(raw));

  if (trafficLog.size() >= MAX_TRAFFIC_LOG) {
    trafficLog.erase(trafficLog.begin());
  }
  trafficLog.push_back(entry);
  updateWorkingNode(source, entry.raw, status, rssi, nodeId);
}

// When normal WiFi is missing or broken, expose a local setup AP so the web UI stays reachable.
void startConfigPortal() {
  if (accessPointActive) {
    return;
  }

  WiFi.mode(WIFI_AP_STA);
  char suffix[7];
  snprintf(suffix, sizeof(suffix), "%06X", static_cast<uint32_t>(ESP.getEfuseMac() & 0xFFFFFF));
  accessPointSsid = String("CapiBridge-Setup-") + suffix;
  WiFi.softAP(accessPointSsid.c_str());
  accessPointActive = true;

  Serial.println();
  Serial.println("Setup AP enabled");
  Serial.print("AP SSID: ");
  Serial.println(accessPointSsid);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
}

void stopConfigPortal() {
  if (!accessPointActive) {
    return;
  }

  WiFi.softAPdisconnect(true);
  accessPointActive = false;
  accessPointSsid = "";
  WiFi.mode(WIFI_STA);
}

// Try to join the configured WiFi network first, then fall back to AP mode for setup.
void setup_wifi() {
  Serial.println();
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);

  if (!hasConfiguredWifi()) {
    Serial.println("WiFi not configured yet. Starting setup AP.");
    startConfigPortal();
    return;
  }

  Serial.print("Connecting to ");
  Serial.println(settings.wifiSsid);
  WiFi.begin(settings.wifiSsid, settings.wifiPassword);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi connect timeout; setup AP enabled for configuration.");
    startConfigPortal();
  }
}

// MQTT commands are buffered here and handled later from loop() so callback work stays minimal.
void callback(char* incomingTopic, byte* payload, unsigned int length) {
  String topicStr = String(incomingTopic);
  mqttMessage = "";

  for (unsigned int i = 0; i < length; i++) {
    mqttMessage += static_cast<char>(payload[i]);
  }

  if (topicStr == CAPIBRIDGE_COMMAND_TOPIC && mqttMessage.length() > 0) {
    newCommandReceived = true;
  }
}

// Reconnect to the broker with a retry delay, and re-subscribe to the command topic on success.
void reconnectMqtt() {
  if (client.connected() || WiFi.status() != WL_CONNECTED || !hasConfiguredMqtt()) {
    return;
  }

  if (millis() - lastMqttAttempt < 5000) {
    return;
  }

  lastMqttAttempt = millis();
  client.setBufferSize(2048);
  client.setServer(settings.mqttServer, settings.mqttPort);
  client.setCallback(callback);

  String clientId = "CapiBridge-" + String(static_cast<uint32_t>(ESP.getEfuseMac()), HEX);

  bool ok = client.connect(
    clientId.c_str(),
    settings.mqttUsername,
    settings.mqttPassword,
    CAPIBRIDGE_LWT_TOPIC, 0, true, AVAIL_OFF
  );

  if (ok) {
    Serial.println("MQTT connected");
    client.subscribe(CAPIBRIDGE_COMMAND_TOPIC);
    client.publish(CAPIBRIDGE_LWT_TOPIC, AVAIL_ON, true);
  } else {
    Serial.print("MQTT failed, rc=");
    Serial.println(client.state());
  }
}

// LoRa and ESP-NOW share the same XOR obfuscation helper.
void xorBuffer(uint8_t* buf, size_t len) {
  const int keyLen = settings.encryptionKeyLength;
  if (keyLen <= 0) {
    return;
  }

  for (size_t i = 0; i < len; ++i) {
    buf[i] ^= settings.encryptionKey[i % keyLen];
  }
}

// Publish Home Assistant discovery topics only once per node/entity unless forced by settings.
void publishSensorDiscoveryOnce(
  const String& nodeId,
  const String& suffix,
  const String& friendlyName,
  const String& icon,
  const String& device_class,
  const String& unit,
  bool isBinarySensor,
  bool hasStateClassMeasurement,
  bool isDiagnostic = false
) {
  if (!client.connected()) {
    return;
  }

  if (!settings.discoveryEveryPacket && discoveryPublishedFor(nodeId, suffix)) {
    return;
  }

  StaticJsonDocument<768> cfg;
  cfg["name"] = friendlyName;
  if (icon.length()) cfg["icon"] = icon;
  if (device_class.length()) cfg["device_class"] = device_class;
  if (unit.length()) cfg["unit_of_measurement"] = unit;
  if (isDiagnostic) cfg["entity_category"] = "diagnostic";

  String baseStateTopic = String(isBinarySensor ? BINARY_SENSOR_TOPIC : SENSOR_TOPIC) + nodeId + suffix;
  cfg["state_topic"] = baseStateTopic;
  cfg["unique_id"] = nodeId + suffix;
  cfg["availability_topic"] = CAPIBRIDGE_LWT_TOPIC;

  if (isBinarySensor) {
    cfg["payload_on"] = "on";
    cfg["payload_off"] = "off";
  }

  if (hasStateClassMeasurement) {
    cfg["state_class"] = "measurement";
  }

  JsonObject dev = cfg.createNestedObject("device");
  JsonArray identifiers = dev.createNestedArray("identifiers");
  identifiers.add(nodeId);
  dev["name"] = nodeId;
  dev["model"] = nodeId;
  dev["manufacturer"] = "PricelessToolkit";

  String payload;
  serializeJson(cfg, payload);

  String configTopic = String(isBinarySensor ? BINARY_SENSOR_TOPIC : SENSOR_TOPIC) + nodeId + suffix + "/config";
  client.publish(configTopic.c_str(), payload.c_str(), MQTT_RETAIN);

  if (!settings.discoveryEveryPacket) {
    markDiscoveryPublished(nodeId, suffix);
  }
}

static String fmtFloat(double v, uint8_t digits) {
  char buf[32];
  dtostrf(v, 0, digits, buf);
  return String(buf);
}

// Publish a single sensor field to MQTT if that key exists in the incoming payload.
void publishKeyIfPresent(const JsonDocument& doc, const char* key, const String& nodeId, const String& topicSuffix, bool isBinary, bool convertForHA = false) {
  if (!client.connected() || !doc.containsKey(key)) return;

  String stateTopic = String(isBinary ? BINARY_SENSOR_TOPIC : SENSOR_TOPIC) + nodeId + topicSuffix;

  if (isBinary) {
    String val = doc[key].as<String>();
    val.toLowerCase();
    if (val == "1" || val == "true") val = "on";
    if (val == "0" || val == "false") val = "off";
    if (val != "on" && val != "off") {
      val = (doc[key].as<String>().length() > 0) ? "on" : "off";
    }
    client.publish(stateTopic.c_str(), val.c_str(), MQTT_RETAIN);
  } else {
    if (doc[key].is<float>() || doc[key].is<double>() || doc[key].is<long>() || doc[key].is<int>()) {
      double v = doc[key].as<double>();
      if (convertForHA) v = v / 1000.0;
      String val = fmtFloat(v, convertForHA ? 3 : 2);
      client.publish(stateTopic.c_str(), val.c_str(), MQTT_RETAIN);
    } else {
      String val = doc[key].as<String>();
      client.publish(stateTopic.c_str(), val.c_str(), MQTT_RETAIN);
    }
  }
}

// Validate gateway key/id, announce entities, and publish the current payload values to MQTT.
bool ensureDiscoveryAndPublish(const JsonDocument& doc, String& status, String& nodeId) {
  if (!client.connected()) {
    status = "mqtt_offline";
    return false;
  }

  if (!doc.containsKey("k") || doc["k"].as<String>() != settings.gatewayKey) {
    Serial.println("Network Key Not Found or Invalid in JSON");
    status = "wrong_key";
    return false;
  }
  if (!doc.containsKey("id")) {
    Serial.println("Missing 'id' in JSON");
    status = "missing_id";
    return false;
  }

  String id = doc["id"].as<String>();
  nodeId = id;

  if (doc.containsKey("r"))   publishSensorDiscoveryOnce(id, "/rssi",     "RSSI",           "mdi:signal",         "signal_strength", "dBm", false, true, true);
  if (doc.containsKey("b"))   publishSensorDiscoveryOnce(id, "/batt",     "Battery",        "mdi:battery",        "battery",         "%",   false, true, true);
  if (doc.containsKey("v"))   publishSensorDiscoveryOnce(id, "/volt",     "Volt",           "mdi:flash-triangle", "voltage",         "V",   false, true);
  if (doc.containsKey("pw"))  publishSensorDiscoveryOnce(id, "/current",  "Current",        "mdi:current-dc",     "current",         "A",   false, true);
  if (doc.containsKey("l"))   publishSensorDiscoveryOnce(id, "/lx",       "Lux",            "mdi:brightness-1",   "illuminance",     "lx",  false, true);
  if (doc.containsKey("w"))   publishSensorDiscoveryOnce(id, "/weight",   "Weight",         "mdi:weight",         "weight",          "g",   false, true);
  if (doc.containsKey("t"))   publishSensorDiscoveryOnce(id, "/tmp",      "Temperature",    "mdi:thermometer",    "temperature",     "°C",  false, true);
  if (doc.containsKey("t2"))  publishSensorDiscoveryOnce(id, "/tmp2",     "Temperature 2",  "mdi:thermometer",    "temperature",     "°C",  false, true);
  if (doc.containsKey("hu"))  publishSensorDiscoveryOnce(id, "/humidity", "Humidity",       "mdi:water-percent",  "humidity",        "%",   false, true);
  if (doc.containsKey("mo"))  publishSensorDiscoveryOnce(id, "/moisture", "Moisture",       "mdi:water-percent",  "moisture",        "%",   false, true);
  if (doc.containsKey("atm")) publishSensorDiscoveryOnce(id, "/pressure", "Pressure",       "mdi:gauge",          "pressure",        "kPa", false, true);
  if (doc.containsKey("cd"))  publishSensorDiscoveryOnce(id, "/co2",      "Carbon Dioxide", "mdi:molecule-co2",   "carbon_dioxide",  "ppm", false, true);

  if (doc.containsKey("rw"))  publishSensorDiscoveryOnce(id, "/row",      "Text",           "mdi:text",           "",                "",    false, false);
  if (doc.containsKey("s"))   publishSensorDiscoveryOnce(id, "/state",    "State",          "mdi:list-status",    "",                "",    false, false);

  if (doc.containsKey("bt"))  publishSensorDiscoveryOnce(id, "/button",   "Button",         "mdi:button-pointer", "",                "",    true,  false);
  if (doc.containsKey("m"))   publishSensorDiscoveryOnce(id, "/motion",   "Motion",         "mdi:motion",         "motion",          "",    true,  false);
  if (doc.containsKey("dr"))  publishSensorDiscoveryOnce(id, "/door",     "Door",           "mdi:door",           "door",            "",    true,  false);
  if (doc.containsKey("wd"))  publishSensorDiscoveryOnce(id, "/window",   "Window",         "mdi:window-closed",  "window",          "",    true,  false);
  if (doc.containsKey("vb"))  publishSensorDiscoveryOnce(id, "/vibration", "Vibration",     "mdi:vibrate",        "vibration",       "",    true,  false);

  publishKeyIfPresent(doc, "r",   id, "/rssi",      false);
  publishKeyIfPresent(doc, "b",   id, "/batt",      false);
  publishKeyIfPresent(doc, "v",   id, "/volt",      false);
  publishKeyIfPresent(doc, "pw",  id, "/current",   false, true);
  publishKeyIfPresent(doc, "l",   id, "/lx",        false);
  publishKeyIfPresent(doc, "w",   id, "/weight",    false);
  publishKeyIfPresent(doc, "t",   id, "/tmp",       false);
  publishKeyIfPresent(doc, "t2",  id, "/tmp2",      false);
  publishKeyIfPresent(doc, "hu",  id, "/humidity",  false);
  publishKeyIfPresent(doc, "mo",  id, "/moisture",  false);
  publishKeyIfPresent(doc, "rw",  id, "/row",       false);
  publishKeyIfPresent(doc, "s",   id, "/state",     false);
  publishKeyIfPresent(doc, "atm", id, "/pressure",  false);
  publishKeyIfPresent(doc, "cd",  id, "/co2",       false);
  publishKeyIfPresent(doc, "bt",  id, "/button",    true);
  publishKeyIfPresent(doc, "m",   id, "/motion",    true);
  publishKeyIfPresent(doc, "dr",  id, "/door",      true);
  publishKeyIfPresent(doc, "wd",  id, "/window",    true);
  publishKeyIfPresent(doc, "vb",  id, "/vibration", true);
  status = "valid";
  return true;
}

// Shared JSON parsing path used by both ESP-NOW and LoRa receive handlers.
void parseIncomingPacket(const String& source, const String& serialrow, int rssi = 0) {
  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, serialrow);
  if (error) {
    Serial.print("RX JSON parse error: ");
    Serial.println(error.f_str());
    String logged = String("RX JSON parse error: ") + error.f_str();
    pushTrafficEntry(source, logged, "invalid_json", rssi);
    return;
  }

  String status;
  String nodeId;
  ensureDiscoveryAndPublish(doc, status, nodeId);
  pushTrafficEntry(source, serialrow, status, rssi, nodeId);
}

// Periodically publish the gateway WiFi RSSI as a diagnostic entity for Home Assistant.
void diag() {
  if (!client.connected()) {
    return;
  }

  if (millis() - lastDiagTimer >= diagTimer) {
    long rssi = WiFi.isConnected() ? WiFi.RSSI() : 0;

    static bool diagDiscoverySent = false;
    if (!diagDiscoverySent) {
      StaticJsonDocument<384> cfg;
      cfg["name"] = "RSSI";
      cfg["unit_of_measurement"] = "dBm";
      cfg["device_class"] = "signal_strength";
      cfg["icon"] = "mdi:signal";
      cfg["entity_category"] = "diagnostic";
      cfg["state_topic"] = CAPIBRIDGE_RSSI_TOPIC;
      cfg["unique_id"] = "capibridge_rssi";
      JsonObject dev = cfg.createNestedObject("device");
      JsonArray identifiers = dev.createNestedArray("identifiers");
      identifiers.add("capibridge");
      dev["name"] = "CapiBridge";
      dev["model"] = "CapiBridge";
      dev["manufacturer"] = "PricelessToolkit";
      String payload;
      serializeJson(cfg, payload);
      client.publish((String(CAPIBRIDGE_RSSI_TOPIC) + "/config").c_str(), payload.c_str(), MQTT_RETAIN);
      diagDiscoverySent = true;
    }

    client.publish(CAPIBRIDGE_RSSI_TOPIC, String(rssi).c_str(), MQTT_RETAIN);
    lastDiagTimer = millis();
  }
}

// Forward MQTT command payloads out over LoRa after removing the transport selector field.
void SendLoRaCommands() {
  if (newCommandReceived && mqttMessage.indexOf("\"rm\":\"lora\"") != -1) {
    radio.detachInterrupt();
    digitalWrite(LED_PIN, LOW);
    LedStartTime = millis();

    String cleaned = mqttMessage;
    StaticJsonDocument<2048> doc;
    DeserializationError e = deserializeJson(doc, mqttMessage);
    if (!e) {
      doc.remove("rm");
      cleaned = "";
      serializeJson(doc, cleaned);
    }

    int state = RADIOLIB_ERR_UNKNOWN;
    if (settings.loraEncryption) {
      size_t len = cleaned.length();
      uint8_t buf[len];
      memcpy(buf, cleaned.c_str(), len);
      xorBuffer(buf, len);
      state = radio.transmit(buf, len);
    } else {
      state = radio.transmit(cleaned);
    }

    if (state == RADIOLIB_ERR_NONE) {
      Serial.print("LoRa Command sent: ");
      Serial.println(cleaned);
    } else {
      Serial.print("Failed to send Command, code ");
      Serial.println(state);
    }

    client.publish(CAPIBRIDGE_COMMAND_TOPIC, "", true);
    newCommandReceived = false;
    packetReceived = false;

    if (!radio.armReceive(onReceive)) {
      Serial.println("Failed to re-arm LoRa receive mode");
    }
  }
}

// Forward MQTT command payloads to the ESP-NOW co-processor over UART.
void SendESPNOWCommands() {
  if (newCommandReceived && mqttMessage.indexOf("\"rm\":\"espnow\"") != -1) {
    if (Serial1.available() == 0) {
      String cleaned = mqttMessage;
      StaticJsonDocument<2048> doc;
      DeserializationError e = deserializeJson(doc, mqttMessage);
      if (!e) {
        doc.remove("rm");
        cleaned = "";
        serializeJson(doc, cleaned);
      }

      if (settings.espnowEncryption) {
        size_t len = cleaned.length();
        uint8_t buf[len];
        memcpy(buf, cleaned.c_str(), len);
        xorBuffer(buf, len);
        Serial1.write(buf, len);
        Serial1.write('\n');
      } else {
        Serial1.print(cleaned);
        Serial1.print('\n');
      }

      Serial.print("ESP-NOW Command sent: ");
      Serial.println(cleaned);

      client.publish(CAPIBRIDGE_COMMAND_TOPIC, "", true);
      newCommandReceived = false;
    }
  }
}

// Boot-time console output helps confirm the active radio settings after loading defaults or flash values.
void printLoRaConfig() {
  Serial.print(F("LoRa module: "));
  Serial.println(radio.moduleLabel());

  Serial.print(F("Frequency Band: "));
  Serial.print(settings.band, 1);
  Serial.println(F(" MHz"));

  Serial.print(F("TX Power: "));
  Serial.print(settings.loraTxPower);
  Serial.println(F(" dBm"));

  Serial.print(F("Signal Bandwidth: "));
  Serial.print(settings.loraSignalBandwidth, 1);
  Serial.println(F(" kHz"));

  Serial.print(F("Spreading Factor: SF"));
  Serial.println(settings.loraSpreadingFactor);

  Serial.print(F("Coding Rate: 4/"));
  Serial.println(settings.loraCodingRate);

  Serial.print(F("Sync Word: 0x"));
  Serial.println(settings.loraSyncWord, HEX);

  Serial.print(F("Preamble Length: "));
  Serial.println(settings.loraPreambleLength);
}

void printActiveConfig() {
  Serial.println();
  Serial.println(F("========== Active Config =========="));
  Serial.print(F("Config Source: "));
  Serial.println(settingsSourceLabel());

  Serial.println(F("[WiFi]"));
  Serial.print(F("SSID: "));
  Serial.println(settings.wifiSsid);
  Serial.print(F("Password: "));
  Serial.println(settings.wifiPassword);
  Serial.print(F("Connected: "));
  Serial.println(WiFi.status() == WL_CONNECTED ? F("yes") : F("no"));
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(F("IP Address: "));
    Serial.println(WiFi.localIP());
    Serial.print(F("RSSI: "));
    Serial.print(WiFi.RSSI());
    Serial.println(F(" dBm"));
  }
  if (accessPointActive) {
    Serial.print(F("Setup AP SSID: "));
    Serial.println(accessPointSsid);
    Serial.print(F("Setup AP IP: "));
    Serial.println(WiFi.softAPIP());
  }

  Serial.println(F("[MQTT]"));
  Serial.print(F("Server: "));
  Serial.println(settings.mqttServer);
  Serial.print(F("Port: "));
  Serial.println(settings.mqttPort);
  Serial.print(F("Username: "));
  Serial.println(settings.mqttUsername);
  Serial.print(F("Password: "));
  Serial.println(settings.mqttPassword);
  Serial.print(F("Connected: "));
  Serial.println(client.connected() ? F("yes") : F("no"));

  Serial.println(F("[Gateway]"));
  Serial.print(F("Gateway KEY: "));
  Serial.println(settings.gatewayKey);
  Serial.print(F("LoRa Encryption: "));
  Serial.println(settings.loraEncryption ? F("enabled") : F("disabled"));
  Serial.print(F("ESP-NOW Encryption: "));
  Serial.println(settings.espnowEncryption ? F("enabled") : F("disabled"));
  Serial.print(F("Encryption KEY: "));
  for (uint8_t i = 0; i < settings.encryptionKeyLength; ++i) {
    Serial.printf("0x%02X", settings.encryptionKey[i]);
    if (i + 1 < settings.encryptionKeyLength) Serial.print(", ");
  }
  Serial.println();
  Serial.print(F("Discovery Every Packet: "));
  Serial.println(settings.discoveryEveryPacket ? F("true") : F("false"));
  Serial.print(F("ROW Debug: "));
  Serial.println(settings.rowDebug ? F("true") : F("false"));

  Serial.println(F("[LoRa]"));
  printLoRaConfig();
  Serial.println(F("==================================="));
}

// Web API helpers serialize JSON responses for the embedded UI.
void sendJsonResponse(int statusCode, const JsonDocument& doc) {
  String payload;
  serializeJson(doc, payload);
  server.send(statusCode, "application/json", payload);
}

// Serve the single-page web UI shell.
void handleRoot() {
  if (!ensureWebUiAuth()) {
    return;
  }
  server.send_P(200, "text/html", WEB_UI_HTML);
}

// Return the current runtime settings so the Settings page can populate its form fields.
void handleGetSettings() {
  if (!ensureWebUiAuth()) {
    return;
  }
  StaticJsonDocument<2048> doc;
  settingsToJson(settings, doc, true);
  sendJsonResponse(200, doc);
}

// Return live gateway health information for the Status page.
void handleGetStatus() {
  if (!ensureWebUiAuth()) {
    return;
  }
  StaticJsonDocument<512> doc;
  doc["wifiConnected"] = WiFi.status() == WL_CONNECTED;
  doc["mqttConnected"] = client.connected();
  doc["ipAddress"] = WiFi.status() == WL_CONNECTED ? ipToString(WiFi.localIP()) : String("");
  doc["apEnabled"] = accessPointActive;
  doc["apSsid"] = accessPointSsid;
  doc["wifiRssi"] = WiFi.status() == WL_CONNECTED ? String(WiFi.RSSI()) + " dBm" : "Unavailable";
  doc["freeHeap"] = ESP.getFreeHeap();
  doc["uptime"] = formatUptime(millis());
  doc["firmwareVersion"] = FIRMWARE_VERSION;
  doc["settingsSource"] = settingsSourceLabel();
  sendJsonResponse(200, doc);
}

// Return the packet history and mirrored serial log for the Monitoring page.
void handleGetTraffic() {
  if (!ensureWebUiAuth()) {
    return;
  }
  StaticJsonDocument<24576> doc;
  JsonArray items = doc.createNestedArray("items");
  for (const auto& entry : trafficLog) {
    JsonObject item = items.createNestedObject();
    item["time"] = formatTrafficTime(entry.atMs);
    item["source"] = entry.source;
    item["status"] = entry.status;
    item["nodeId"] = entry.nodeId;
    if (entry.rssi != 0) {
      item["rssi"] = entry.rssi;
    }
    item["raw"] = entry.raw;
    item["pretty"] = entry.pretty;
  }
  JsonArray serialLines = doc.createNestedArray("serialLines");
  for (const auto& line : serialLog) {
    serialLines.add(line);
  }
  JsonArray nodes = doc.createNestedArray("nodes");
  for (const auto& entry : workingNodes) {
    JsonObject node = nodes.createNestedObject();
    node["time"] = formatTrafficTime(entry.atMs);
    node["ageSeconds"] = (millis() - entry.atMs) / 1000;
    node["source"] = entry.source;
    node["status"] = entry.status;
    node["nodeId"] = entry.nodeId;
    if (entry.rssi != 0) {
      node["rssi"] = entry.rssi;
    }
    if (entry.hasBattery) {
      node["battery"] = entry.battery;
    }
  }
  sendJsonResponse(200, doc);
}

// Delay the reboot slightly so HTTP responses finish before ESP.restart() is called.
void scheduleReboot() {
  rebootAt = millis() + 1500;
}

// Validate and save settings posted from the web UI, then reboot to apply them cleanly.
void handlePostSettings() {
  if (!ensureWebUiAuth()) {
    return;
  }
  if (!server.hasArg("plain")) {
    StaticJsonDocument<192> doc;
    doc["ok"] = false;
    doc["error"] = "Missing JSON body.";
    sendJsonResponse(400, doc);
    return;
  }

  StaticJsonDocument<2048> request;
  DeserializationError error = deserializeJson(request, server.arg("plain"));
  if (error) {
    StaticJsonDocument<192> doc;
    doc["ok"] = false;
    doc["error"] = "Invalid JSON body.";
    sendJsonResponse(400, doc);
    return;
  }

  GatewaySettings updated = settings;
  String validationError;
  if (!settingsFromJson(request, updated, validationError)) {
    StaticJsonDocument<256> doc;
    doc["ok"] = false;
    doc["error"] = validationError;
    sendJsonResponse(400, doc);
    return;
  }

  if (!settingsStore.save(updated)) {
    StaticJsonDocument<256> doc;
    doc["ok"] = false;
    doc["error"] = "Failed to save settings to flash.";
    sendJsonResponse(500, doc);
    return;
  }

  settings = updated;
  StaticJsonDocument<192> doc;
  doc["ok"] = true;
  doc["rebootScheduled"] = true;
  sendJsonResponse(200, doc);
  scheduleReboot();
}

// Manual reboot action from the Status page.
void handleReboot() {
  if (!ensureWebUiAuth()) {
    return;
  }
  StaticJsonDocument<128> doc;
  doc["ok"] = true;
  sendJsonResponse(200, doc);
  scheduleReboot();
}

// Restore runtime settings from internal upload defaults and clear the saved flash copy.
void handleReset() {
  if (!ensureWebUiAuth()) {
    return;
  }
  setDefaultSettings(settings);
  settingsStore.save(settings);

  StaticJsonDocument<128> doc;
  doc["ok"] = true;
  sendJsonResponse(200, doc);
  scheduleReboot();
}

void handleNotFound() {
  StaticJsonDocument<128> doc;
  doc["ok"] = false;
  doc["error"] = "Not found";
  sendJsonResponse(404, doc);
}

// Register every API route used by the embedded single-page web interface.
void setupWebServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/settings", HTTP_GET, handleGetSettings);
  server.on("/api/settings", HTTP_POST, handlePostSettings);
  server.on("/api/status", HTTP_GET, handleGetStatus);
  server.on("/api/traffic", HTTP_GET, handleGetTraffic);
  server.on("/api/reboot", HTTP_POST, handleReboot);
  server.on("/api/reset", HTTP_POST, handleReset);
  server.onNotFound(handleNotFound);
  server.begin();
}

// Run deferred reboot requests from the main loop.
void maybeReboot() {
  if (rebootAt != 0 && millis() >= rebootAt) {
    Serial.println("Rebooting to apply settings...");
    delay(150);
    ESP.restart();
  }
}

// Keep station mode connected when possible and keep the setup AP available as a fallback.
void maintainWifi() {
  if (WiFi.status() == WL_CONNECTED) {
    if (accessPointActive) {
      stopConfigPortal();
    }
    return;
  }

  if (!accessPointActive) {
    startConfigPortal();
  }

  if (hasConfiguredWifi() && millis() - lastWifiTry > 5000) {
    WiFi.reconnect();
    lastWifiTry = millis();
  }
}

// Hardware, settings, WiFi, MQTT, radio, and web UI all come online here at boot.
void setup() {
  Serial1.begin(BAUD, SERIAL_8N1, RXPIN, TXPIN);
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  setDefaultSettings(settings);
  settingsStore.load(settings);

  setup_wifi();
  setupWebServer();

  client.setBufferSize(2048);
  client.setCallback(callback);
  reconnectMqtt();

  SPI.begin(CONFIG_CLK, CONFIG_MISO, CONFIG_MOSI, CONFIG_NSS);

  String radioError;
  if (!radio.begin(settings, onReceive, radioError)) {
    Serial.println(radioError);
    while (true) { delay(10); }
  }

  printActiveConfig();

  if (radio.startReceive() != RADIOLIB_ERR_NONE) {
    Serial.println(F("Failed to start non-blocking RX"));
  } else {
    Serial.println(F("----- All set, waiting for incoming JSON payloads -----"));
  }

  Serial.println();
  Serial.println("Web UI ready:");
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("STA: http://");
    Serial.println(WiFi.localIP());
  }
  if (accessPointActive) {
    Serial.print("AP:  http://");
    Serial.println(WiFi.softAPIP());
  }
}

// Main loop: serve the web UI, process incoming radio/UART data, maintain connectivity, and send commands.
void loop() {
  server.handleClient();
  maybeReboot();

  // ESP-NOW packets arrive over the secondary UART with a 2-byte length prefix from ESP2.
  if (Serial1.available() >= 2) {
    uint16_t L = 0;
    if (Serial1.readBytes(reinterpret_cast<char*>(&L), 2) == 2 && L > 0 && L <= 512) {
      static uint8_t buf[513];
      size_t got = 0;
      unsigned long start = millis();

      while (got < L && (millis() - start) < 100) {
        got += Serial1.readBytes(reinterpret_cast<char*>(buf) + got, L - got);
      }

      if (got == L) {
        if (settings.rowDebug) {
          Serial.print("ESP-NOW Encrypted bytes: ");
          for (size_t i = 0; i < L; i++) {
            Serial.printf("%02X ", buf[i]);
          }
          Serial.println();
        }

        if (settings.espnowEncryption) {
          xorBuffer(buf, L);
        }

        buf[L] = 0;
        String serialrow = String(reinterpret_cast<char*>(buf));
        parseIncomingPacket("ESP-NOW", serialrow);
        StaticJsonDocument<64> preview;
        if (!deserializeJson(preview, serialrow)) {
          Serial.print("ESP-NOW Message Received: ");
          Serial.println(serialrow);
        } else {
          Serial.println("ESP-NOW Message Received: [invalid json payload omitted]");
        }
      } else {
        while (Serial1.available()) Serial1.read();
      }
    } else {
      while (Serial1.available()) Serial1.read();
    }
  }

  // LoRa packets are handled after the interrupt flag is raised, then the radio is re-armed for RX.
  if (packetReceived) {
    packetReceived = false;

    uint8_t buf[256];
    size_t len = radio.getPacketLength(true);
    if (len == 0 || len > sizeof(buf)) len = sizeof(buf);

    int16_t state = radio.readData(buf, len);
    radio.startReceive();

    if (state == RADIOLIB_ERR_NONE) {
      digitalWrite(LED_PIN, LOW);
      LedStartTime = millis();

      if (settings.rowDebug) {
        Serial.print("LoRa Encrypted bytes: ");
        for (size_t i = 0; i < len; i++) {
          Serial.printf("%02X ", buf[i]);
        }
        Serial.println();
      }

      if (settings.loraEncryption) {
        xorBuffer(buf, len);
      }

      String recv(reinterpret_cast<char*>(buf), len);
      StaticJsonDocument<2048> rx;
      DeserializationError e = deserializeJson(rx, recv);
      if (!e) {
        float packetRssi = radio.getRSSI();
        rx["r"] = packetRssi;
        String status;
        String nodeId;
        ensureDiscoveryAndPublish(rx, status, nodeId);
        String logged;
        serializeJson(rx, logged);
        pushTrafficEntry("LoRa", logged, status, static_cast<int>(lround(packetRssi)), nodeId);
        Serial.print("LoRa Message Received: ");
        serializeJson(rx, Serial);
        Serial.println();
      } else {
        Serial.print("RX JSON parse error: ");
        Serial.println(e.f_str());
        String logged = String("RX JSON parse error: ") + e.f_str();
        pushTrafficEntry("LoRa", logged, "invalid_json", static_cast<int>(lround(radio.getRSSI())));
      }
    } else {
      Serial.print("readData() error ");
      Serial.println(state);
    }
  }

  if (millis() - LedStartTime >= 100) {
    digitalWrite(LED_PIN, HIGH);
  }

  maintainWifi();
  reconnectMqtt();
  client.loop();
  diag();
  SendESPNOWCommands();
  SendLoRaCommands();
}
