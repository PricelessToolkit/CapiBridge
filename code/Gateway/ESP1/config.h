/////////////////////////// LoRa/ESPNow Gateway Key ///////////////////////////

#define GATEWAY_KEY "xy"                           // Separation Key "Keep it small" Must match exactly sensors key
#define Encryption true                            // Global Payload Encryption, true or false
#define encryption_key_length 4                    // must match number of bytes in the XOR key array
#define encryption_key { 0x4B, 0xA3, 0x3F, 0x9C }  // Multi-byte XOR key (2–16 bytes)

/////////////////////////////////// WIFI //////////////////////////////////////

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

/////////////////////////////////// MQTT //////////////////////////////////////

#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""
#define MQTT_SERVER ""
#define MQTT_PORT 1883
#define DISCOVERY_EVERY_PACKET true      // true  = publish discovery every time data is received from that sensor
                                         // false = publish discovery once per CapiBridge boot (remembers if that sensor is already published)
                                         // Using false reduces MQTT traffic, but if you delete a sensor in Home Assistant,
                                         // you must reboot CapiBridge to clear the cache so it can rediscover that sensor
                                     
////////////////////////////// LORA CONFIG ////////////////////////////////////

// Select which LoRa module this CapiBridge is uses:

//#define LORA_MODULE LORA_MODULE_SX1276  // SX1276 Module (orders shipped before Aug 2025)
#define LORA_MODULE LORA_MODULE_SX1262  // SX1262 Module CapiBridge v2 (orders shipped after Aug 2025)

#define BAND 868.0                       // 433.0 / 868.0 / 915.0
#define LORA_TX_POWER 20                 // dBm; check regulatory limits in your region (SX1262 supports up to 22) (SX1276 supports up to 20)

/// Bandwidth vs SF support (SX126x):
// 125 kHz: SF5–SF9
// 250 kHz: SF5–SF10
// 500 kHz: SF5–SF11

#define LORA_SIGNAL_BANDWIDTH 250.0     // kHz: 125.0, 250.0, 500.0
#define LORA_SPREADING_FACTOR 10        // "5–11 on SX1262" "6-12 on SX1276"
#define LORA_CODING_RATE 5              // 5–8 → coding rate 4/5 .. 4/8
#define LORA_SYNC_WORD 0x12             // Default 0x12 Dont Change
#define LORA_PREAMBLE_LENGTH 12         // 6..65535

///////////////////////////////////////////////////////////////////////////////
