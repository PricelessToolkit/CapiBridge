/*
  ESP-NOW Sensor (Broadcast) + XOR/Base64 encryption
  - Sends JSON with all supported keys your gateway understands.
  - Encrypts using xorCipher (same as gateway), then broadcasts to FF:FF:FF:FF:FF:FF.
  - Keep encrypted string under ~249 chars (ESP-NOW practical payload limit).

  Keys (all included here with example values):
    k   (string, required)   = private gateway key (must match gateway)
    id  (string, required)   = node name / device id
    r   (number, optional)   = RSSI in dBm
    b   (number, optional)   = battery percent (%)
    v   (number, optional)   = voltage (V)
    pw  (number, optional)   = current (mA)   [gateway converts to A for HA]
    l   (number, optional)   = illuminance (lx)
    m   (string/bool)        = motion ("on"/"off")
    w   (number)             = weight (g)
    s   (string)             = state (text)
    t   (number)             = temperature (°C)
    t2  (number)             = temperature 2 (°C)
    hu  (number)             = humidity (%)
    mo  (number)             = moisture (%)
    rw  (string)             = row / text
    bt  (string/bool)        = button ("on"/"off")
    atm (number)             = pressure (kPa)
    cd  (number)             = CO2 (ppm)
    dr  (string/bool)        = door ("on"/"off")
    wd  (string/bool)        = window ("on"/"off")
    vb  (string/bool)        = vibration ("on"/"off")
*/

/*
  ESP-NOW Sensor (Broadcast) + raw XOR
  - Builds JSON the gateway understands.
  - XORs bytes in-place (no Base64).
  - Sends exact-length payload via esp_now_send.

  Match the XOR key and Wi-Fi channel with your gateway/receiver.
*/

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_err.h>
#include <ArduinoJson.h>

// ---------- USER SETTINGS ----------
#define DEVICE_ID            "TestESP"
#define GATEWAY_KEY          "xy"
#define ESPNOW_WIFI_CHANNEL  1
#define SEND_PERIOD_MS       5000

// XOR key (must match gateway)
#define ENCRYPTION 0   // set 0 = off, 1 = on
#define encryption_key_length 4
#define encryption_key { 0x4B, 0xA3, 0x3F, 0x9C }
// -----------------------------------

// Broadcast MAC
static const uint8_t BROADCAST_MAC[6] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF };

unsigned long lastSend = 0;

// ---- helper: XOR in place (manual loop) ----
static inline void xorInPlace(uint8_t* data, size_t len) {
  static const uint8_t key[] = encryption_key;
  const size_t K = encryption_key_length;
  for (size_t i = 0; i < len; ++i) {
    data[i] ^= key[i % K];
  }
}

// Build JSON with all fields (replace sample values with real sensor data)
static size_t buildSensorJson(char* out, size_t outCap) {
  // v7 style. For v6, replace with DynamicJsonDocument doc(512);
  JsonDocument doc;

  doc["k"]  = GATEWAY_KEY;     // REQUIRED
  doc["id"] = DEVICE_ID;       // REQUIRED

  // Optional sample values — adjust to your sensors
  doc["b"]   = 99;             // Battery %
  doc["v"]   = 3.97;           // Voltage V
  doc["pw"]  = 350;            // Current mA (gateway can convert to A)
  doc["l"]   = 125;            // Lux
  doc["m"]   = "off";          // Motion
  doc["w"]   = 79;             // Weight g
  doc["s"]   = "ok";           // State text
  doc["t"]   = 21.5;           // Temp °C
  doc["t2"]  = 22.0;           // Temp2 °C
  doc["hu"]  = 46;             // Humidity %
  doc["mo"]  = 83;             // Moisture %
  doc["rw"]  = "hello";        // Row/Text
  doc["bt"]  = "off";          // Button
  doc["atm"] = 101.3;          // Pressure kPa
  doc["cd"]  = 560;            // CO2 ppm
  doc["dr"]  = "off";          // Door
  doc["wd"]  = "off";          // Window
  doc["vb"]  = "off";          // Vibration

  // Serialize directly into your fixed buffer
  // returns number of bytes written (no trailing NUL required by esp_now_send)
  return serializeJson(doc, out, outCap);
}

// Send encrypted JSON via ESP-NOW broadcast (raw XOR bytes)
static bool sendEncryptedBroadcast() {
  // Small, fixed buffer to avoid heap churn
  // Keep under ~249B for ESP-NOW practical payload limits
  char json[256];
  size_t plainLen = buildSensorJson(json, sizeof(json));
  if (plainLen == 0 || plainLen >= sizeof(json)) {
    Serial.println("JSON build too large or error.");
    return false;
  }

  // (Optional) debug print plaintext JSON
  Serial.print("JSON: ");
  Serial.write((const uint8_t*)json, plainLen);
  Serial.println();

  // Encrypt in-place (manual XOR loop)
  #if ENCRYPTION
  xorInPlace(reinterpret_cast<uint8_t*>(json), plainLen);
  #endif

  // Broadcast exact-length encrypted bytes
  esp_err_t err = esp_now_send(
    BROADCAST_MAC,
    reinterpret_cast<const uint8_t*>(json),
    plainLen
  );

  if (err != ESP_OK) {
    Serial.print("esp_now_send error: ");
    Serial.println(esp_err_to_name(err));
    return false;
  }
  return true;
}

// ---- Send callback (new + old cores) ----
#if defined(ESP_IDF_VERSION_MAJOR) && (ESP_IDF_VERSION_MAJOR >= 5)
// ESP32 core 3.x / IDF v5
void onDataSent(const wifi_tx_info_t* info, esp_now_send_status_t status) {
  Serial.print("TX -> ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "SUCCESS" : "FAIL");
}
#else
// ESP32 core 2.x / IDF v4
void onDataSent(const uint8_t* mac, esp_now_send_status_t status) {
  Serial.print("TX -> ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "SUCCESS" : "FAIL");
}
#endif

void setup() {
  Serial.begin(115200);
  delay(100);

  // 1) Wi-Fi STA on fixed channel
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  esp_wifi_set_ps(WIFI_PS_NONE);
  esp_wifi_start();
  delay(50);

  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(ESPNOW_WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  Serial.print("STA MAC: "); Serial.println(WiFi.macAddress());
  Serial.print("Channel: "); Serial.println(ESPNOW_WIFI_CHANNEL);

  // 2) Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    while (true) delay(1000);
  }
  esp_now_register_send_cb(onDataSent);

  // (Optional) Add broadcast as peer (helps on some builds)
  esp_now_peer_info_t bcast{};
  memcpy(bcast.peer_addr, BROADCAST_MAC, 6);
  bcast.ifidx   = WIFI_IF_STA;
  bcast.channel = ESPNOW_WIFI_CHANNEL;
  bcast.encrypt = false;
  esp_err_t e = esp_now_add_peer(&bcast);
  if (e != ESP_OK && e != ESP_ERR_ESPNOW_EXIST) {
    Serial.print("add_peer(broadcast) warning: ");
    Serial.println(esp_err_to_name(e));
  }
}

void loop() {
  if (millis() - lastSend >= SEND_PERIOD_MS) {
    lastSend = millis();

    bool ok = sendEncryptedBroadcast();
    Serial.println(ok ? "Sent (XOR raw)" : "Send failed");
  }
}
