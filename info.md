## Supported nodes

Sensors that work out of the box with CapiBridge:
- [PirBOX-LITE](https://github.com/PricelessToolkit/PirBOX-LITE) - LoRa long-range motion sensor for mailbox / garage
- [PirBOX-MAX](https://github.com/PricelessToolkit/PirBOX-MAX) - LoRa long-range 2-way motion sensor with reed switch inputs and relays
- [SOILSENS-V5W](https://github.com/PricelessToolkit/SOILSENS-V5W) - Soil moisture sensor

## How to configure

1. Configure `ESP1` gateway settings.
- Set `GATEWAY_KEY`.
- Set `LoRa_Encryption` and `ESPNOW_Encryption` if needed.
- Set the XOR `encryption_key` and `encryption_key_length` so they match your nodes.

2. Configure WiFi.
- Set `WIFI_SSID`.
- Set `WIFI_PASSWORD`.

3. Configure MQTT.
- Set `MQTT_SERVER`.
- Set `MQTT_PORT`.
- Set `MQTT_USERNAME` and `MQTT_PASSWORD` if your broker requires login.
- Choose `DISCOVERY_EVERY_PACKET` depending on how often you want Home Assistant discovery to be republished.

4. Configure the LoRa radio.
- Select the correct LoRa module used by your CapiBridge:
  - `LORA_MODULE_SX1276`
  - `LORA_MODULE_SX1262`
  - `LORA_MODULE_SX1268`
- Match `BAND`, `LORA_SIGNAL_BANDWIDTH`, `LORA_SPREADING_FACTOR`, `LORA_CODING_RATE`, `LORA_SYNC_WORD`, and `LORA_PREAMBLE_LENGTH` with your nodes.

5. Upload and test.
- Upload `ESP1.ino` to `ESP1`.
- Upload `ESP2.ino` to `ESP2`.
- Open the web UI to review status, settings, prettified payloads, and raw view.

## Notes

- LoRa configuration must match the configuration used in the nodes / sensors.
- `ESP2` for ESP-NOW prints its MAC address in the serial monitor after upload.
- If packets are not received, first check gateway key, encryption settings, and LoRa radio parameters.
