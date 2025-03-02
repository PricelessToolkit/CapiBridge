#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h> // only for esp_wifi_set_channel()

#define LED_PIN 2
#define BAUD 115200
#define RXPIN 18
#define TXPIN 19

unsigned long LedStartTime = 0;

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
    char json[250]; // Ensure this matches the sender's structure size
} struct_message;

// Create a struct_message to hold incoming data
struct_message incomingData;

// Callback function that will be executed when data is received
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
    //Serial.print("Bytes received: ");
    //Serial.println(len);
    const struct_message* msg = reinterpret_cast<const struct_message*>(incomingData);

    Serial1.println(msg->json);
    digitalWrite(LED_PIN, LOW);   // HIGH = OFF
    LedStartTime = millis(); 
}



void setup() {
  Serial1.begin(BAUD, SERIAL_8N1, RXPIN, TXPIN); // Communication Between ESPs
  Serial.begin(115200); // Communication Between ESP2 and PC
  Serial.println();
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  Serial.print("ESPNOW Gateway MAC Address:  ");
  Serial.println(WiFi.macAddress());

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
      Serial.println("Error initializing ESP-NOW");
      return;
    }

  // Once ESPNow is successfully initialized, register to receive incoming data
  esp_now_register_recv_cb(OnDataRecv);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);   // HIGH = OFF
}



// the loop function runs over and over again forever
void loop() {
  
 
  if (millis() -  LedStartTime >= 100) {       // Turning OFF LED_PIN after 100ms
    digitalWrite(LED_PIN, HIGH);             // HIGH = LED is OFF    
  }
  
}
