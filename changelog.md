# Updates, Bugfixes, and Breaking Changes

- 29.10.2026 — **V1.4.0**
  - Added ESP1 browser OTA in Settings → Firmware ([#31](https://github.com/PricelessToolkit/CapiBridge/issues/31)).
  - Preserve configuration and Web UI login across updates.
  - Keep recovery Web UI available if LoRa initialization fails.
  - Fixed Monitoring page crash with ArduinoJson 6 ([#30](https://github.com/PricelessToolkit/CapiBridge/issues/30)).
  - **Upgrade:** First OTA installation and switching to Minimal SPIFFS require USB. Keep Erase All Flash disabled.
- 28.10.2026
  - Fixed LoRa reception stopping on SX1262/SX1268 ([#29](https://github.com/PricelessToolkit/CapiBridge/issues/29)).
  - Fixed encrypted ESP-NOW commands.
  - Fixed MQTT commands with spaces in JSON (LoRa and ESP-NOW).
  - **Breaking Change:** Update both ESP1 and ESP2 firmware together.
- 17.04.2026
  - WebUI for configuration and monitoring.
- 28.03.2026
  - Added support for "Ra-01S" 433Mhz Module.
  - `define LORA_MODULE LORA_MODULE_SX1268    // Ra-01S 433Mhz Module "433Mhz Version" Module CapiBridge v2`
