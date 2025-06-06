/////////////////////////// LoRa/ESPNow Gateway Key ///////////////////////////

#define GATEWAY_KEY "xy"                           // Separation Key "Keep it small" Must match exactly sensors key
#define Encryption true                            // Global Payload Encryption, true or false
#define encryption_key_length 4                    // must match number of bytes in the XOR key array
#define encryption_key { 0x4B, 0xA3, 0x3F, 0x9C }  // Multi-byte XOR key (between 2–16 values).
                                                   // Use random-looking HEX values (from 0x00 to 0xFF).
                                                   // Must match exactly on both sender and receiver.
                                                   // Example: { 0x1F, 0x7E, 0xC2, 0x5A }  ➜ 4-byte key.

///////////////////////////////////////////////////////////////////////////////

////////////////////////////// WIFI / MQTT ////////////////////////////////////

#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_passwd"
#define MQTT_USERNAME "your_mqtt_user"
#define MQTT_PASSWORD "your_mqtt_passwd"
#define MQTT_SERVER "your_mqtt_broker_address"
#define MQTT_PORT 1883

///////////////////////////////////////////////////////////////////////////////

////////////////////////////// LORA CONFIG ////////////////////////////////////

// For sending 6 bytes "6 characters" of data using different Spreading Factors (SF), the estimated time on air (ToA)
// for each SF is as follows:

// SF7: Approximately 0.027 seconds (27.494 milliseconds)
// SF8: Approximately 0.052 seconds (52.224 milliseconds)
// SF9: Approximately 0.100 seconds (100.147 milliseconds)
// SF10: Approximately 0.193 seconds (193.413 milliseconds)
// SF11: Approximately 0.385 seconds (385.297 milliseconds)
// SF12: Approximately 0.746 seconds (746.127 milliseconds)
// These calculations demonstrate how the time on air increases with higher Spreading Factors
// due to the decreased data rate, which is a trade-off for increased communication range and signal robustness.

#define SIGNAL_BANDWITH 125E3  // signal bandwidth in Hz, defaults to 125E3
#define SPREADING_FACTOR 8    // ranges from 6-12,default 7 see API docs
#define CODING_RATE 5          // Supported values are between 5 and 8, these correspond to coding rates of 4/5 and 4/8. The coding rate numerator is fixed at 4.
#define SYNC_WORD 0x12         // byte value to use as the sync word, defaults to 0x12 // New modules support only 0x12
#define PREAMBLE_LENGTH 6      // Supported values are between 6 and 65535.
#define TX_POWER 20            // TX power in dB, defaults to 17, Supported values are 2 to 20
#define BAND 433E6             // 433E6 / 868E6 / 915E6

///////////////////////////////////////////////////////////////////////////////
