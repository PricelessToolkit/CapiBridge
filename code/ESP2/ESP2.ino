#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

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
  Serial.print("ESPNOW Gateway MAC Address:  ");
  Serial.println(WiFi.macAddress());
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

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
  
  // Test JSON strings imitating received ESPNOW data
  /*
  Serial1.println("{\"k\":\"abc\",\"id\":\"node1\",\"r\":\"85\",\"b\":\"4.15\",\"v\":\"12\",\"a\":\"0.5\",\"l\":\"224\",\"m\":\"on\",\"w\":\"1.45\",\"s\":\"on\",\"e\":\"1024\",\"t\":\"31\",\"t2\":\"25\",\"ah\":\"56\",\"sh\":\"19\",\"rw\":\"Test Test\",\"p1\":\"on\",\"p2\":\"off\",\"p3\":\"on\",\"p4\":\"off\"}");
  delay(2000);
  Serial1.println("{\"k\":\"abc\",\"id\":\"node2\",\"r\":\"115\",\"b\":\"3.2\",\"rw\":\"smile\"}");
  delay(2000);
  */


  if (millis() -  LedStartTime >= 100) {       // Turning OFF LED_PIN after 100ms
    digitalWrite(LED_PIN, HIGH);             // HIGH = LED is OFF    
  }
  
}
