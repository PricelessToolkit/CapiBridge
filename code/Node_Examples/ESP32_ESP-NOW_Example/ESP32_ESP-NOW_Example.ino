// Example code for LazyESP board

#include <esp_now.h>
#include <WiFi.h>
#include <ArduinoJson.h>

// Global Variables
StaticJsonDocument<256> doc;
uint8_t receiverAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // For broadcasting
String node_name = "ESP32"; // Your node name
String gateway_key = "xy";     // Your gateway key

String Vbattery = "3.3";
String amp = "0.55";

// Structure to send data
typedef struct struct_message {
    char json[256]; // Adjusted size to accommodate larger JSON
} struct_message;

// Create an instance of struct_message to hold your data
struct_message myData;

void setup() {
    Serial.begin(115200);

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

}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Do something when sending data is complete
    delay(2000);
}


void loop() {

    // Clear previous data
    doc.clear();


    /*

| Key   | Description               | Unit of Measurement | Required |
|-------|---------------------------|---------------------|----------|
| `k`   | Private Gateway key       | -                   | Yes      |
| `id`  | Node Name                 | -                   | Yes      |
| `r`   | RSSI                      | dBm                 | No       |
| `b`   | Battery Voltage           | Volts               | No       |
| `v`   | Volts                     | Volts               | No       |
| `pw`  | Current                   | mAh                 | No       |
| `l`   | Luminance                 | lux                 | No       |
| `m`   | Motion                    | Binary on/off       | No       |
| `w`   | Weight                    | grams               | No       |
| `s`   | State                     | Anything            | No       |
| `t`   | Temperature               | °C                  | No       |
| `t2`  | Temperature 2             | °C                  | No       |
| `hu`  | Humidity                  | %                   | No       |
| `mo`  | Moisture                  | %                   | No       |
| `rw`  | ROW                       | Anything            | No       |
| `bt`  | Button                    | Binary on/off       | No       |
| `atm` | Pressure                  | kph                 | No       |
| `cd`  | Dioxyde de carbone        | ppm                 | No       |
| `dr`  | Door                      | Binary on/off       | No       |
| `wd`  | Window                    | Binary on/off       | No       |
| `vb`  | Vibration                 | Binary on/off       | No       |

    */

    // Populate JSON document
    doc["k"] = gateway_key;
    doc["id"] = node_name;
    doc["b"] = Vbattery;
    doc["rw"] = "Test";
    doc["v"] = "12";
    doc["pw"] = "0.35";
    doc["hu"] = "16";
    doc["mo"] = "83";
    doc["w"] = "79";
    doc["dr"] = "on";
    doc["wd"] = "off";
    doc["s"] = "xxx";
    doc["l"] = "125";
    doc["t2"] = "26";
    doc["t"] = "9.9";
    doc["vb"] = "on";

    // Serialize JSON document into the struct
    size_t jsonSize = serializeJson(doc, myData.json, sizeof(myData.json));

    // Send data via ESP-NOW
    esp_now_send(receiverAddress, (uint8_t *) &myData, jsonSize + 1);
}
