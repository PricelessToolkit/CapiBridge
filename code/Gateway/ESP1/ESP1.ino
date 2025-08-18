// For Older Board with RA-01H (SX1276)

#include <Arduino.h>
#include "config.h"
#include "radio.h"
#include <SPI.h>
#include <RadioLib.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <vector>


RADIO_CLASS radio = new Module(MODULE_ARGS);

volatile bool packetReceived = false;
void onReceive() { packetReceived = true; }

#define LED_PIN 2
#define BAUD 115200
#define RXPIN 18 // Connected to ESP2 "ESPNOW RECEIVER"
#define TXPIN 19 // Connected to ESP2 "ESPNOW RECEIVER"

// ===== MQTT / Topics =====
#define MQTT_RETAIN true
#define BINARY_SENSOR_TOPIC "homeassistant/binary_sensor/"
#define SENSOR_TOPIC "homeassistant/sensor/"

// Global LWT for availability of the bridge (and used by entities)
#define CAPIBRIDGE_LWT_TOPIC "capibridge/availability"
#define CAPIBRIDGE_RSSI_TOPIC "homeassistant/sensor/CapiBridge/rssi"
#define CAPIBRIDGE_COMMAND_TOPIC "homeassistant/sensor/CapiBridge/command" // kept as-is

// Availability payloads
#define AVAIL_ON  "online"
#define AVAIL_OFF "offline"

String mqttMessage = "";
String mqttMessagexor = "";
bool newCommandReceived = false;

unsigned long LedStartTime = 0;
unsigned long diagTimer = 60000;
unsigned long lastDiagTimer = 0;

// Track discovery we’ve already published: key = "<id>|<suffix>"
std::vector<String> publishedDiscovery;

WiFiClient espClient;
PubSubClient client(espClient);

// ---------- Helpers to avoid discovery spam ----------
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

// ---------- WiFi ----------
void setup_wifi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

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
    Serial.println("\nWiFi connect timeout; continuing, will keep retrying via loop().");
  }
}

// ---------- MQTT ----------
void callback(char* incomingTopic, byte* payload, unsigned int length) {
  String topicStr = String(incomingTopic);
  mqttMessage = "";

  for (unsigned int i = 0; i < length; i++) {
    mqttMessage += (char)payload[i];
  }

  if (topicStr == CAPIBRIDGE_COMMAND_TOPIC && mqttMessage.length() > 0) {
    newCommandReceived = true;
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection... ");

    client.setBufferSize(2048); // headroom for discovery payloads

    String client_id = "CapiBridge2-" + String((uint32_t)ESP.getEfuseMac(), HEX);

    // Global LWT so entities can use same availability_topic
    bool ok = client.connect(
      client_id.c_str(),
      MQTT_USERNAME, MQTT_PASSWORD,
      CAPIBRIDGE_LWT_TOPIC, 0, true, AVAIL_OFF
    );

    if (ok) {
      Serial.println("MQTT connected");
      client.setCallback(callback);
      client.subscribe(CAPIBRIDGE_COMMAND_TOPIC);

      // Bring CapiBridge availability online
      client.publish(CAPIBRIDGE_LWT_TOPIC, AVAIL_ON, true);
    } else {
      Serial.print("MQTT failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 1 second");
      delay(1000);
    }
  }
}



/*

// ---------- XOR cipher (optional) ----------
String xorCipher(const String& input) {
  const byte key[] = encryption_key;
  const int keyLength = encryption_key_length;
  String output;
  output.reserve(input.length());
  for (int i = 0; i < input.length(); i++) {
    byte keyByte = key[i % keyLength];
    output += char(input[i] ^ keyByte);
  }
  return output;
}

*/


// ------------------- XOR + BASE 64 ------------------//

// config.h has:
// #define encryption_key_length 4
// #define encryption_key { 0x4B, 0xA3, 0x3F, 0x9C }

String xorCipher(String in) {

  // 1) Use an ARRAY, not a pointer, for the macro initializer
  static const uint8_t key[] = encryption_key;
  const int K = encryption_key_length;

  auto isB64 = [](const String& s)->bool{
    if (s.length() % 4) return false;
    for (size_t i=0;i<s.length();++i){
      char c=s[i];
      if (!((c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='+'||c=='/'||c=='=')) return false;
    }
    return true;
  };

  auto b64enc = [](const uint8_t* b, size_t n)->String{
    static const char T[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    String o; o.reserve(((n+2)/3)*4);
    for(size_t i=0;i<n;i+=3){
      uint32_t v = ((uint32_t)b[i] << 16)
                 | ((i+1<n ? (uint32_t)b[i+1] : 0) << 8)
                 |  (i+2<n ? (uint32_t)b[i+2] : 0);
      o += T[(v>>18)&63]; o += T[(v>>12)&63];
      o += (i+1<n) ? T[(v>>6)&63] : '=';
      o += (i+2<n) ? T[v&63]      : '=';
    }
    return o;
  };

  auto b64val = [](char c)->int{
    if(c>='A'&&c<='Z') return c-'A';
    if(c>='a'&&c<='z') return 26+c-'a';
    if(c>='0'&&c<='9') return 52+c-'0';
    if(c=='+') return 62;
    if(c=='/') return 63;
    return -1;
  };

  auto b64dec = [&](const String& s)->String{
    String o; o.reserve((s.length()/4)*3);
    for(size_t i=0;i<s.length(); i+=4){
      int a=b64val(s[i]), b=b64val(s[i+1]);
      int c = (s[i+2]=='=') ? -1 : b64val(s[i+2]);
      int d = (s[i+3]=='=') ? -1 : b64val(s[i+3]);

      // 2) Upcast before shifting (AVR int is 16-bit)
      uint32_t v = ((uint32_t)(a & 63) << 18)
                 | ((uint32_t)(b & 63) << 12)
                 | ((uint32_t)((c < 0 ? 0 : (c & 63))) << 6)
                 |  (uint32_t)((d < 0 ? 0 : (d & 63)));

      o += (char)((v >> 16) & 0xFF);
      if (c >= 0) o += (char)((v >> 8) & 0xFF);
      if (d >= 0) o += (char)(v & 0xFF);
    }
    return o;
  };

  if (isB64(in)) {                 // decrypt: Base64 -> XOR -> JSON
    String bytes = b64dec(in);
    String out; out.reserve(bytes.length());
    for (size_t i=0;i<bytes.length();++i)
      out += (char)(((uint8_t)bytes[i]) ^ key[i % K]);
    return out;
  } else {                         // encrypt: JSON -> XOR -> Base64
    String x; x.reserve(in.length());
    for (size_t i=0;i<in.length();++i)
      x += (char)(((uint8_t)in[i]) ^ key[i % K]);
    return b64enc((const uint8_t*)x.c_str(), x.length());
  }
}



//---------------------- XOR END ----------------------//



// ---------- Discovery builders ----------
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
  #if !DISCOVERY_EVERY_PACKET
    if (discoveryPublishedFor(nodeId, suffix)) return;
  #endif


  StaticJsonDocument<768> cfg;
  cfg["name"] = friendlyName;
  if (icon.length()) cfg["icon"] = icon;
  if (device_class.length()) cfg["device_class"] = device_class;
  if (unit.length()) cfg["unit_of_measurement"] = unit;

  // mark as diagnostic if requested
  if (isDiagnostic) {
    cfg["entity_category"] = "diagnostic";
  }

  // State + availability
  String baseStateTopic = String(isBinarySensor ? BINARY_SENSOR_TOPIC : SENSOR_TOPIC) + nodeId + suffix;
  cfg["state_topic"] = baseStateTopic;
  cfg["unique_id"] = nodeId + suffix;
  cfg["availability_topic"] = CAPIBRIDGE_LWT_TOPIC;

  // Binary payload mapping (we publish "on"/"off")
  if (isBinarySensor) {
    cfg["payload_on"]  = "on";
    cfg["payload_off"] = "off";
  }

  // state_class where useful
  if (hasStateClassMeasurement) {
    cfg["state_class"] = "measurement";
  }

  // Proper HA device object
  JsonObject dev = cfg.createNestedObject("device");
  dev["identifiers"].add(nodeId);
  dev["name"] = nodeId;
  dev["model"] = nodeId;                   // or a real model label
  dev["manufacturer"] = "PricelessToolkit";

  String payload;
  serializeJson(cfg, payload);

  String configTopic = String(isBinarySensor ? BINARY_SENSOR_TOPIC : SENSOR_TOPIC) + nodeId + suffix + "/config";
  client.publish(configTopic.c_str(), payload.c_str(), MQTT_RETAIN);

  #if !DISCOVERY_EVERY_PACKET
  markDiscoveryPublished(nodeId, suffix);
  #endif
}

// ---------- Publishing states (with light formatting) ----------
static String fmtFloat(double v, uint8_t digits) {
  char buf[32];
  dtostrf(v, 0, digits, buf);
  return String(buf);
}

// Publish a single key if present.
// topicSuffix must match what discovery used.
void publishKeyIfPresent(const JsonDocument& doc, const char* key, const String& nodeId, const String& topicSuffix, bool isBinary, bool convertForHA = false) {
  if (!doc.containsKey(key)) return;

  String stateTopic = String(isBinary ? BINARY_SENSOR_TOPIC : SENSOR_TOPIC) + nodeId + topicSuffix;

  if (isBinary) {
    // Normalize to "on"/"off"
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
      if (convertForHA) v = v / 1000.0;       // mA -> A for "pw"
      String val = fmtFloat(v, convertForHA ? 3 : 2);
      client.publish(stateTopic.c_str(), val.c_str(), MQTT_RETAIN);
    } else {
      String val = doc[key].as<String>();
      client.publish(stateTopic.c_str(), val.c_str(), MQTT_RETAIN);
    }
  }
}

// ---------- Map JSON -> entities (discovery + state) ----------
void ensureDiscoveryAndPublish(const JsonDocument& doc) {
  // Only proceed if GATEWAY_KEY matches
  if (!doc.containsKey("k") || doc["k"].as<String>() != GATEWAY_KEY) {
    Serial.println("Network Key Not Found or Invalid in JSON");
    return;
  }
  if (!doc.containsKey("id")) {
    Serial.println("Missing 'id' in JSON");
    return;
  }

  String id = doc["id"].as<String>();

  // ---- Discovery (only once per id+key) ----
  // Numeric sensors
  if (doc.containsKey("r"))   publishSensorDiscoveryOnce(id, "/rssi",     "RSSI",          "mdi:signal",         "signal_strength", "dBm", false, true, true);
  if (doc.containsKey("b"))   publishSensorDiscoveryOnce(id, "/batt",     "Battery",       "mdi:battery",        "battery",         "%",   false, true, true);
  if (doc.containsKey("v"))   publishSensorDiscoveryOnce(id, "/volt",     "Volt",          "mdi:flash-triangle", "voltage",         "V",   false, true);
  if (doc.containsKey("pw"))  publishSensorDiscoveryOnce(id, "/current",  "Current",       "mdi:current-dc",     "current",         "A",   false, true); // mA -> A
  if (doc.containsKey("l"))   publishSensorDiscoveryOnce(id, "/lx",       "Lux",           "mdi:brightness-1",   "illuminance",     "lx",  false, true);
  if (doc.containsKey("w"))   publishSensorDiscoveryOnce(id, "/weight",   "Weight",        "mdi:weight",         "weight",          "g",   false, true);
  if (doc.containsKey("t"))   publishSensorDiscoveryOnce(id, "/tmp",      "Temperature",   "mdi:thermometer",    "temperature",     "°C",  false, true);
  if (doc.containsKey("t2"))  publishSensorDiscoveryOnce(id, "/tmp2",     "Temperature 2", "mdi:thermometer",    "temperature",     "°C",  false, true);
  if (doc.containsKey("hu"))  publishSensorDiscoveryOnce(id, "/humidity", "Humidity",      "mdi:water-percent",  "humidity",        "%",   false, true);
  if (doc.containsKey("mo"))  publishSensorDiscoveryOnce(id, "/moisture", "Moisture",      "mdi:water-percent",  "moisture",        "%",   false, true);
  if (doc.containsKey("atm")) publishSensorDiscoveryOnce(id, "/pressure", "Pressure",      "mdi:gauge",          "pressure",        "kPa", false, true);
  if (doc.containsKey("cd"))  publishSensorDiscoveryOnce(id, "/co2",      "Carbon Dioxide","mdi:molecule-co2",   "carbon_dioxide",  "ppm", false, true);

  // Text sensors
  if (doc.containsKey("rw"))  publishSensorDiscoveryOnce(id, "/row",      "Text",          "mdi:text",           "",                "",    false, false);
  if (doc.containsKey("s"))   publishSensorDiscoveryOnce(id, "/state",    "State",         "mdi:list-status",    "",                "",    false, false);

  // Binary sensors (on/off)
  if (doc.containsKey("bt"))  publishSensorDiscoveryOnce(id, "/button",   "Button",        "mdi:button-pointer", "",                "",    true,  false);
  if (doc.containsKey("m"))   publishSensorDiscoveryOnce(id, "/motion",   "Motion",        "mdi:motion",         "motion",          "",    true,  false);
  if (doc.containsKey("dr"))  publishSensorDiscoveryOnce(id, "/door",     "Door",          "mdi:door",           "door",            "",    true,  false);
  if (doc.containsKey("wd"))  publishSensorDiscoveryOnce(id, "/window",   "Window",        "mdi:window-closed",  "window",          "",    true,  false);
  if (doc.containsKey("vb"))  publishSensorDiscoveryOnce(id, "/vibration","Vibration",     "mdi:vibrate",        "vibration",       "",    true,  false);

  // ---- State publish ----
  publishKeyIfPresent(doc, "r",   id, "/rssi",     false);
  publishKeyIfPresent(doc, "b",   id, "/batt",     false);
  publishKeyIfPresent(doc, "v",   id, "/volt",     false);
  publishKeyIfPresent(doc, "pw",  id, "/current",  false, true); // mA -> A
  publishKeyIfPresent(doc, "l",   id, "/lx",       false);
  publishKeyIfPresent(doc, "w",   id, "/weight",   false);
  publishKeyIfPresent(doc, "t",   id, "/tmp",      false);
  publishKeyIfPresent(doc, "t2",  id, "/tmp2",     false);
  publishKeyIfPresent(doc, "hu",  id, "/humidity", false);
  publishKeyIfPresent(doc, "mo",  id, "/moisture", false);
  publishKeyIfPresent(doc, "rw",  id, "/row",      false);
  publishKeyIfPresent(doc, "s",   id, "/state",    false);
  publishKeyIfPresent(doc, "atm", id, "/pressure", false);
  publishKeyIfPresent(doc, "cd",  id, "/co2",      false);
  publishKeyIfPresent(doc, "bt",  id, "/button",   true);
  publishKeyIfPresent(doc, "m",   id, "/motion",   true);
  publishKeyIfPresent(doc, "dr",  id, "/door",     true);
  publishKeyIfPresent(doc, "wd",  id, "/window",   true);
  publishKeyIfPresent(doc, "vb",  id, "/vibration",true);
}

// ---------- JSON parsing ----------
void parseIncomingPacket(const String& serialrow) {
  StaticJsonDocument<2048> doc; // more headroom
  DeserializationError error = deserializeJson(doc, serialrow);
  if (error) {
    Serial.print("deserializeJson() returned ");
    Serial.println(error.f_str());
    return;
  }
  ensureDiscoveryAndPublish(doc);
}

// ---------- Diagnostics ----------
void diag() {
  if (millis() - lastDiagTimer >= diagTimer) {
    long rssi = WiFi.RSSI();

    // Auto-discovery for CapiBridge diagnostic RSSI (only once)
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
      dev["identifiers"].add("capibridge");
      dev["name"] = "CapiBridge";
      dev["model"] = "CapiBridge";
      dev["manufacturer"] = "PricelessToolkit";
      String payload;
      serializeJson(cfg, payload);
      client.publish((String(CAPIBRIDGE_RSSI_TOPIC) + "/config").c_str(), payload.c_str(), MQTT_RETAIN);
      diagDiscoverySent = true;
    }

    // Sends RSSI value
    client.publish(CAPIBRIDGE_RSSI_TOPIC, String(rssi).c_str(), MQTT_RETAIN);
    lastDiagTimer = millis();
  }
}


// ---------- Command paths ----------
void SendLoRaCommands() {
  if (newCommandReceived && mqttMessage.indexOf("\"rm\":\"lora\"") != -1) {
    // Temporarily disable the DIxx interrupt so the TX-done pulse isn't mistaken for RX-done.
    DETACH_IRQ();

    // Indicate “sending” with the LED
    digitalWrite(LED_PIN, LOW);
    LedStartTime = millis();

    // --- strip "rm" from payload (keep everything else) ---
    String cleaned = mqttMessage;
    {
      StaticJsonDocument<2048> doc;
      DeserializationError e = deserializeJson(doc, mqttMessage);
      if (!e) {
        doc.remove("rm");           // remove the routing hint
        cleaned = "";
        serializeJson(doc, cleaned);
      } // if parse fails, fall back to original message (no strip)
    }

    #if Encryption
      mqttMessagexor = xorCipher(cleaned);
    #else
      mqttMessagexor = cleaned;
    #endif

    int state = radio.transmit(mqttMessagexor);
    if (state == RADIOLIB_ERR_NONE) {
      Serial.print("LoRa Command sent: ");
      Serial.println(cleaned);
    } else {
      Serial.print("Failed to send Command, code ");
      Serial.println(state);
    }

    // Reset the MQTT command topic so it won't re-trigger on reconnect
    client.publish(CAPIBRIDGE_COMMAND_TOPIC, "", true);
    newCommandReceived = false;

    SET_DIO_ACTION(radio, onReceive);

    // Clear any RX flag we may have picked up spuriously
    packetReceived = false;

    // Go back into non-blocking receive
    if (radio.startReceive(0) != RADIOLIB_ERR_NONE) {
      Serial.println("Failed to restart RX");
    }
  }
}

void SendESPNOWCommands() {
  if (newCommandReceived && mqttMessage.indexOf("\"rm\":\"espnow\"") != -1) {
    if (Serial1.available() == 0) {

      // --- strip "rm" from payload (keep everything else) ---
      String cleaned = mqttMessage;
      {
        StaticJsonDocument<2048> doc;
        DeserializationError e = deserializeJson(doc, mqttMessage);
        if (!e) {
          doc.remove("rm");         // remove the routing hint
          cleaned = "";
          serializeJson(doc, cleaned);
        } // if parse fails, fall back to original message (no strip)
      }
/*
      String payload;
      #if Encryption
        payload = xorCipher(cleaned);
      #else
        payload = cleaned;
      #endif

      Serial1.print(payload);
*/
      Serial.print("ESP-NOW Command sent: ");
      Serial.println(cleaned);

      client.publish(CAPIBRIDGE_COMMAND_TOPIC, "", true);
      newCommandReceived = false;
    }
  }
}

void printLoRaConfig() {
  Serial.print(F("Frequency Band: "));
  Serial.print(BAND, 1);
  Serial.println(F(" MHz"));

  Serial.print(F("TX Power: "));
  Serial.print(LORA_TX_POWER);
  Serial.println(F(" dBm"));

  Serial.print(F("Signal Bandwidth: "));
  Serial.print(LORA_SIGNAL_BANDWIDTH, 1);
  Serial.println(F(" kHz"));

  Serial.print(F("Spreading Factor: SF"));
  Serial.println(LORA_SPREADING_FACTOR);

  Serial.print(F("Coding Rate: 4/"));
  Serial.println(LORA_CODING_RATE);

  Serial.print(F("Sync Word: 0x"));
  Serial.println(LORA_SYNC_WORD, HEX);

  Serial.print(F("Preamble Length: "));
  Serial.println(LORA_PREAMBLE_LENGTH);

  Serial.println(F("=============================="));
}

// ---------- Setup / Loop ----------
void setup() {
  Serial1.begin(BAUD, SERIAL_8N1, RXPIN, TXPIN); // Serial between ESP1 and ESP2
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);   // active-low: HIGH = OFF

  setup_wifi();

  client.setBufferSize(2048);
  client.setServer(MQTT_SERVER, MQTT_PORT);
  reconnect();

  // SPI + Radio
  SPI.begin(CONFIG_CLK, CONFIG_MISO, CONFIG_MOSI, CONFIG_NSS);

  #if (LORA_MODULE == LORA_MODULE_SX1276)
    Serial.println("LoRa Ra-01H SX1276 initialization...");
    printLoRaConfig();

  #else
    Serial.println("LoRa Ra-01SH SX1262 initialization...");
    printLoRaConfig();
  #endif

  int state = RADIO_BEGIN(radio);

  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("----- All set, waiting for incoming JSON payloads -----"));
  } else {
    Serial.print(F("LoRa init failed, code "));
    Serial.println(state);
    while (true) { delay(10); }
  }

  SET_DIO_ACTION(radio, onReceive);
  if (radio.startReceive(0) != RADIOLIB_ERR_NONE) {
    Serial.println(F("Failed to start non-blocking RX"));
  }

}

void loop() {
  // 1) Handle any ESP-NOW packets coming in over Serial1
if (Serial1.available() > 0) {
  String serialrow = Serial1.readStringUntil('\n');  // we know sender prints \r\n

  Serial.print("ESP-NOW ROW Message Received: ");
  Serial.println(serialrow);
/*
  #if Encryption
    serialrow = xorCipher(serialrow);
  #endif
*/
  parseIncomingPacket(serialrow);
  Serial.print("ESP-NOW Message Received: ");
  Serial.println(serialrow);
}

  // 2) Check for a completed LoRa packet
  if (packetReceived) {
    packetReceived = false;

    String recv;
    int16_t state = radio.readData(recv); // Non-blocking read

    if (state == RADIOLIB_ERR_NONE) {
      // Indicate reception
      digitalWrite(LED_PIN, LOW);
      LedStartTime = millis();


      //Serial.print("LoRa ROW Message: ");
      //Serial.println(recv);
      //Serial.println(" ");
      #if Encryption
        recv = xorCipher(recv);
      #endif

      // Safer RSSI injection: parse -> set -> publish
      StaticJsonDocument<2048> rx;
      DeserializationError e = deserializeJson(rx, recv);
      if (!e) {
        rx["r"] = radio.getRSSI();
        ensureDiscoveryAndPublish(rx);
        Serial.print("LoRa Message Received: ");
        serializeJson(rx, Serial);
        Serial.println();
      } else {
        Serial.print("RX JSON parse error: ");
        Serial.println(e.f_str());
      }
    } else {
      Serial.print("readData() error ");
      Serial.println(state);
    }

    // Restart listening for the next packet immediately
    radio.startReceive(0);
  }

  // 3) Turn off LED after 100 ms
  if (millis() - LedStartTime >= 100) {
    digitalWrite(LED_PIN, HIGH);
  }

  // 4) Connectivity/MQTT
  if (WiFi.status() != WL_CONNECTED) {
    static unsigned long lastWifiTry = 0;
    if (millis() - lastWifiTry > 5000) {
      WiFi.reconnect();
      lastWifiTry = millis();
    }
  }

  if (!client.connected()) {
    reconnect();
  }

  client.loop();
  diag();
  SendESPNOWCommands();
  SendLoRaCommands();
}
