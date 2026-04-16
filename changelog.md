## 📣 Updates, Bugfixes, and Breaking Changes
- 28.03.2026
  - Added support for "Ra-01S" 433Mhz Module.
  - `#define LORA_MODULE LORA_MODULE_SX1268    // Ra-01S 433Mhz Module "433Mhz Version" Module CapiBridge v2`
- 25.08.2025 - Breaking Change (XOR obfuscation "Encryption" for ESP-NOW).
  - Possibility to Enable/Disable Encryption separately for LoRa and ESP-NOW
  - [2-way communication,](https://github.com/PricelessToolkit/CapiBridge/tree/main?tab=readme-ov-file#-2-way-communication--sending-commands) For LoRa and ESP-NOW
- 18.08.2025 - Hardware modification, The new LoRa module RA-01SH "SX1262"
  - Added option to select LoRA module type via config.
```c
     //#define LORA_MODULE LORA_MODULE_SX1276  // SX1276 Module (orders shipped before Aug 2025)
   #define LORA_MODULE LORA_MODULE_SX1262  // SX1262 Module CapiBridge v2 (orders shipped after Aug 2025)
```
- 22.05.2025 - Breaking Change (XOR obfuscation "Encryption" for LoRa).
- - All LoRa sensors' firmware needs to be updated.
- 14.05.2025 - [2-way communication,](https://github.com/PricelessToolkit/CapiBridge/tree/main?tab=readme-ov-file#-2-way-communication--sending-commands) for now only "LoRa".
- 02.03.2025 - ESP-NOW "ESP2" Serial outputs incorrect MAC address "00:00:00..."
- 25.11.2024 - Button autodiscovery topic.
- 28.07.2024 - Binary sensors topics and Motion sensor autodiscovery.
- 15.06.2024 - Publishing battery in percent.
