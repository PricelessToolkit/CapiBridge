#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>   // esp_wifi_start(), esp_wifi_set_channel(), esp_wifi_get_mac()
#include <esp_err.h>    // esp_err_to_name()

// ---------- Pins / Serial ----------
#define LED_PIN 2
#define BAUD   115200
#define RXPIN  18        // UART from ESP1 (gateway)
#define TXPIN  19

// ---------- ESPNOW settings ----------
#define ESPNOW_WIFI_CHANNEL 1   // MUST match your receivers (1..13)

// Broadcast address (send to ALL devices on same channel)
static const uint8_t BROADCAST_MAC[6] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF };

unsigned long LedStartTime = 0;

// Payload struct (keep in sync with your nodes)
typedef struct {
  char json[250];   // text payload (max 249 chars + null)
} struct_message;

// -------------------- Helpers --------------------

// Print STA MAC; return true if it’s valid (not all zeros)
static bool printStaMac() {
  uint8_t mac[6] = {0};
  if (esp_wifi_get_mac(WIFI_IF_STA, mac) != ESP_OK) {
    Serial.println("esp_wifi_get_mac failed");
    return false;
  }
  bool allZero = true;
  for (int i = 0; i < 6; i++) if (mac[i] != 0x00) { allZero = false; break; }

  Serial.print("My MAC: ");
  if (allZero) {
    Serial.println("00:00:00:00:00:00");
  } else {
    for (int i = 0; i < 6; i++) { if (i) Serial.print(':'); Serial.printf("%02X", mac[i]); }
    Serial.println();
  }
  return !allZero;
}

// Send a text line via ESPNOW broadcast
static bool sendEspNowBroadcast(const String& line) {
  struct_message out{};
  size_t n = min(line.length(), sizeof(out.json) - 1);
  memcpy(out.json, line.c_str(), n);
  out.json[n] = '\0';

  esp_err_t err = esp_now_send(BROADCAST_MAC, reinterpret_cast<const uint8_t*>(&out), sizeof(out));
  if (err != ESP_OK) {
    Serial.print("esp_now_send() error: ");
    Serial.println(esp_err_to_name(err));
    return false;
  }
  return true;
}

// -------------------- Callbacks --------------------

// Receive callback (IDF v5 style)
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  uint16_t L = (uint16_t)len;
  Serial1.write((uint8_t*)&L, 2);   // length prefix (little-endian)
  Serial1.write(data, len);         // raw encrypted bytes
  digitalWrite(LED_PIN, LOW);
  LedStartTime = millis();
}



// Send callback
#if defined(ESP_IDF_VERSION_MAJOR) && (ESP_IDF_VERSION_MAJOR >= 5)
// IDF v5 (Arduino core 3.x): wifi_tx_info_t*, field name is des_addr
void OnDataSent(const wifi_tx_info_t* info, esp_now_send_status_t status) {
  Serial.print("ESP-NOW send -> ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "SUCCESS" : "FAIL");
}
#else
// IDF v4 (Arduino core 2.x): legacy signature
void OnDataSent(const uint8_t* mac, esp_now_send_status_t status) {
  Serial.print("ESP-NOW send -> ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "SUCCESS" : "FAIL");
}
#endif

// -------------------- Setup / Loop --------------------

void setup() {
  Serial1.begin(BAUD, SERIAL_8N1, RXPIN, TXPIN); // ESP2 <-> ESP1 link
  Serial.begin(115200);
  delay(100);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // LED OFF

  // Bring up Wi-Fi STA cleanly
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  esp_wifi_set_ps(WIFI_PS_NONE); // better ESPNOW reliability
  esp_wifi_start();              // ensure driver started
  delay(50);

  // Verify MAC not zero; retry once if needed
  if (!printStaMac()) {
    Serial.println("STA MAC is zero, restarting WiFi driver...");
    esp_wifi_stop();
    delay(50);
    esp_wifi_start();
    delay(100);
    printStaMac();
  }

  // Lock ESPNOW channel (must match your receivers)
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(ESPNOW_WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  Serial.print("ESPNOW Channel: ");
  Serial.println(ESPNOW_WIFI_CHANNEL);

  // Init ESPNOW
  esp_err_t e = esp_now_init();
  if (e != ESP_OK) {
    Serial.print("Error initializing ESP-NOW: ");
    Serial.println(esp_err_to_name(e));
    return;
  }

  // (Optional) Some builds are happier if we also add broadcast as a peer.
  // It's not strictly required for broadcast, but harmless:
  esp_now_peer_info_t bcast{};
  memcpy(bcast.peer_addr, BROADCAST_MAC, 6);
  bcast.ifidx   = WIFI_IF_STA;
  bcast.channel = ESPNOW_WIFI_CHANNEL;
  bcast.encrypt = false;
  e = esp_now_add_peer(&bcast);
  if (e != ESP_OK && e != ESP_ERR_ESPNOW_EXIST) {
    Serial.print("add_peer(broadcast) warning: ");
    Serial.println(esp_err_to_name(e));
  }

  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);
}

void loop() {
  // Read a line from ESP1 and broadcast it over ESPNOW
  if (Serial1.available() > 0) {
    String line = Serial1.readStringUntil('\n'); // ESP1 sends newline-terminated
    line.trim();
    if (line.length()) {
      if (sendEspNowBroadcast(line)) {
        Serial.print("Broadcast: ");
        Serial.println(line);
        digitalWrite(LED_PIN, LOW);
        LedStartTime = millis();
      } else {
        Serial.println("ESP-NOW send failed");
      }
    }
  }

  // Turn LED off after 100 ms
  if (millis() - LedStartTime >= 100) {
    digitalWrite(LED_PIN, HIGH);
  }
}
