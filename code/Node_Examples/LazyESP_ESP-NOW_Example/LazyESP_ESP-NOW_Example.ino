// Example code for LazyESP board

#include <esp_now.h>
#include <WiFi.h>
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "Wire.h"
#include "I2C_eeprom.h"
#include <ArduinoJson.h>

// Global Variables
StaticJsonDocument<256> doc;
uint8_t receiverAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // For broadcasting
String node_name = "LazyESP"; // Your node name
String gateway_key = "ab";     // Your gateway key

// EEPROM object with I2C address for 24LC01
I2C_eeprom ee(0x50, I2C_DEVICESIZE_24LC01);

// Structure to send data
typedef struct struct_message {
    char json[256]; // Adjusted size to accommodate larger JSON
} struct_message;

// Create an instance of struct_message to hold your data
struct_message myData;

void setup() {
    Serial.begin(115200);
    Wire.begin(6, 5); // Initialize I2C with custom SDA and SCL pins
    pinMode(4, OUTPUT);
    digitalWrite(4, LOW);

    WiFi.mode(WIFI_STA); // Set device as a Wi-Fi Station

    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, receiverAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }

    esp_now_register_send_cb(OnDataSent);

    // ADC initialization
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    digitalWrite(4, HIGH); // Power off after sending
}


void loop() {
    int raw = adc1_get_raw(ADC1_CHANNEL_0);
    float Vout = (raw * 3.3) / 4095.0;
    float Vbattery = Vout * 10.19; // Adjust for voltage divider

    // Clear previous data
    doc.clear();


    /*

    k   - Gateway Key
    id  - Node Name
    r   - RSSI
    b   - Battery Voltage
    v   - Voltage
    a   - Amps
    l   - Lux
    m   - Motion (on | off)
    w   - Weight
    s   - State (on | off)
    e   - Encoder
    t   - Temperature
    t2  - Second Temperature
    ah  - Air Humidity
    sm  - Soil Moisture
    rw  - Row Data
    p1  - Push Button State (on | off)
    p2  - Push Button State (on | off)
    p3  - Push Button State (on | off)
    p4  - Push Button State (on | off)

    */

    // Populate JSON document
    doc["k"] = gateway_key;
    doc["id"] = node_name;
    doc["b"] = String(Vbattery, 2);
    doc["rw"] = "test";

    // Serialize JSON document into the struct
    size_t jsonSize = serializeJson(doc, myData.json, sizeof(myData.json));

    // Send data via ESP-NOW
    esp_now_send(receiverAddress, (uint8_t *) &myData, jsonSize + 1);
}
