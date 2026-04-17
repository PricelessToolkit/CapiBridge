#pragma once

#define LORA_MODULE_SX1262 1
#define LORA_MODULE_SX1276 2
#define LORA_MODULE_SX1268 3

// Internal fallback defaults for settings stored in flash.
// Users can change these from the Web UI after first boot, so they stay out of
// the smaller user-facing config.h file.

//////////////////////////////// SECURITY /////////////////////////////////////

#define GATEWAY_KEY "xy"                           // Must match the sensor key.
#define LoRa_Encryption true                       // Global LoRa encryption toggle.
#define ESPNOW_Encryption false                    // Global ESP-NOW encryption toggle.
#define encryption_key_length 4                    // Must match the XOR key array length.
#define encryption_key { 0x4B, 0xA3, 0x3F, 0x9C }  // Multi-byte XOR key (2-16 bytes).

/////////////////////////////////// MQTT //////////////////////////////////////

#define MQTT_USERNAME "test"
#define MQTT_PASSWORD "test123"
#define MQTT_SERVER "192.168.99.2"
#define MQTT_PORT 1883
#define DISCOVERY_EVERY_PACKET true  // False sends discovery once per boot.

////////////////////////////// LORA DEFAULTS //////////////////////////////////

// Bandwidth vs SF support (SX126x):
// 125 kHz: SF5-SF9
// 250 kHz: SF5-SF10
// 500 kHz: SF5-SF11

#define LORA_SIGNAL_BANDWIDTH 250.0  // kHz: 125.0, 250.0, 500.0
#define LORA_SPREADING_FACTOR 10     // SX126x: 5-11, SX1276: 6-12
#define LORA_CODING_RATE 5           // 5-8 => coding rate 4/5 .. 4/8
#define LORA_SYNC_WORD 0x12          // Default 0x12, normally unchanged.
#define LORA_PREAMBLE_LENGTH 12      // 6..65535
#define LORA_TX_POWER 20             // Check regulatory limits for your region.

//////////////////////////////// DIAGNOSTICS //////////////////////////////////

#define ROW_Debug false  // Print RAW hex instead of JSON.
