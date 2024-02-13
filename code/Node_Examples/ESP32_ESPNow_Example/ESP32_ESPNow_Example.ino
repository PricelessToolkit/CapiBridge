#include <esp_now.h>
#include <WiFi.h>

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
    char json[250]; // Ensure this is large enough to hold your JSON string
} struct_message;

// Create a struct_message called myData
struct_message myData;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("\r\nLast Packet Send Status:\t");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
    // Initialize Serial Monitor
    Serial.begin(115200);
    
    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    
    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(OnDataSent);
    
    // Replace with the receiver's MAC address
    uint8_t broadcastAddress[] = {0x58, 0xCF, 0x79, 0xDB, 0x09, 0x60};
    
    // Add peer        
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    
    // Add peer    
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add peer");
        return;
    }
}

void loop() {
    // Fill the myData struct with the JSON string
    strcpy(myData.json, "{\"k\":\"ab\",\"id\":\"espn\",\"b\":\"3.2\",\"rw\":\"test\"}"); // Example JSON, change as needed

    // Send the data
    esp_now_send(0, (uint8_t *) &myData, sizeof(myData));
    
    // Wait 2 seconds between sends
    delay(2000);
}
