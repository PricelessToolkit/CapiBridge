/////////////////////////// LoRa/ESPNow Network Key ///////////////////////////

#define NETWORK_KEY "abc" // Keep it small

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

#define SIGNAL_BANDWITH 125E3  // signal bandwidth in Hz, defaults to 125E3
#define SPREADING_FACTOR 12    // ranges from 6-12,default 7 see API docs
#define CODING_RATE 5          // Supported values are between 5 and 8, these correspond to coding rates of 4/5 and 4/8. The coding rate numerator is fixed at 4.
#define SYNC_WORD 0xF3         // byte value to use as the sync word, defaults to 0x12
#define PREAMBLE_LENGTH 8      // Supported values are between 6 and 65535.
#define TX_POWER 20            // TX power in dB, defaults to 17, Supported values are 2 to 20
#define BAND 433E6             // 433E6 / 868E6 / 915E6

///////////////////////////////////////////////////////////////////////////////